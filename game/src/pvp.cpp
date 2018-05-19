#include "stdafx.h"
#include "constants.h"
#include "pvp.h"
#include "crc32.h"
#include "packet.h"
#include "desc.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "config.h"
#include "sectree_manager.h"
#include "buffer_manager.h"
#ifdef __PVP_TOURNAMENT_SYSTEM__
	#include "tournament.h"
#endif

#include "locale_service.h"

#ifdef __ENABLE_PVP_ADVANCED__
#ifdef __NEWPET_SYSTEM__
	#include "New_PetSystem.h"
#endif
#ifdef __SUPPORT_SYSTEM__
	#include "SupportSystem.h"
#endif
	#include "PetSystem.h"
	#include "affect.h"
	#include "party.h"
	#include "guild.h"
	#include "skill.h"
#endif

using namespace std;

#ifdef __ENABLE_PVP_ADVANCED__

EVENTINFO(TPVPDuelEventInfo)
{
    DynamicCharacterPtr ch;
    DynamicCharacterPtr victim;
    CPVP * pvp;
    BYTE state;
    
    TPVPDuelEventInfo() : ch(), victim(), state(0) {}
};

EVENTINFO(TPVPCheckDisconnect)
{
    DynamicCharacterPtr ch;
    DynamicCharacterPtr victim;

    TPVPCheckDisconnect() : ch(), victim() {}
};

EVENTFUNC(pvp_duel_counter)
{
    if (event == NULL)
        return 0;
    
    if (event->info == NULL)
        return 0;
    
    TPVPDuelEventInfo* info = dynamic_cast<TPVPDuelEventInfo*>(event->info);
    
    if (info == NULL)
    {
       // sys_err("ready_to_start_event> <Factor> Null pointer");
        return 0;
    }
    
    LPCHARACTER chA = info->ch;
    LPCHARACTER chB = info->victim;
    
    if (chA == NULL)
    {
     //   sys_err("Duel: Duel start event info is null.");
        return 0;
    }
    
    if (chB == NULL)
    {
      //  sys_err("Duel: Duel start event info is null.");
        return 0;
    }
    
    switch (info->state)
    {
		case 0:
		{
			chA->SpecificEffectPacket("D:/ymir work/ui/game/pvp_advanced/3.mse");
			chB->SpecificEffectPacket("D:/ymir work/ui/game/pvp_advanced/3.mse");
			
			info->state++;
			return PASSES_PER_SEC(1); break;
		}
		case 1:
		{
			chA->SpecificEffectPacket("D:/ymir work/ui/game/pvp_advanced/2.mse");
			chB->SpecificEffectPacket("D:/ymir work/ui/game/pvp_advanced/2.mse");
			info->state++;
			return PASSES_PER_SEC(1);
			break;
		}
		case 2:
		{
			if ((chA->GetDuel("BlockParty")) && (chB->GetDuel("BlockParty")))	
			{	
				LPPARTY chParty = chA->GetParty();
				LPPARTY victimParty = chB->GetParty();
				
				if (chA->GetParty())
					chParty->Quit(chA->GetPlayerID());
				
				if (chB->GetParty())
					victimParty->Quit(chB->GetPlayerID());
			}
			
			if ((chA->GetDuel("BlockPet")) && (chB->GetDuel("BlockPet")))
			{	
				CPetSystem* chPet = chA->GetPetSystem(); 
				CPetSystem* victimPet = chB->GetPetSystem();
				
				for (int petIndex = 34001; petIndex < 34150; petIndex++)
				{
					CPetActor* chPetActor = chPet->GetByVnum(petIndex);
					CPetActor* victimPetActor = victimPet->GetByVnum(petIndex);	
					
					if (chPetActor != NULL)
						chPet->Unsummon(petIndex); 	
					
					if (victimPetActor != NULL)
						victimPet->Unsummon(petIndex); 
#ifdef __NEWPET_SYSTEM__
					CNewPetSystem* chPet = chA->GetNewPetSystem(); 
					CNewPetSystem* victimNewPet = chB->GetNewPetSystem();
					
					CNewPetSystem* chNewPetActor = chPet->GetByVnum(petIndex);
					CNewPetSystem* victimNewPetActor = victimNewPet->GetByVnum(petIndex);	
					
					if (chNewPetActor != NULL)
						chPet->Unsummon(petIndex); 	
					
					if (victimNewPetActor != NULL)
						victimNewPet->Unsummon(petIndex); 
#endif
				}
			}

#ifdef __SUPPORT_SYSTEM__
			if ((chA->GetDuel("BlockSupport")) && (chB->GetDuel("BlockSupport")))
			{	
				CSupportSystem* chPet = chA->GetSupportSystem(); 
				CSupportSystem* victimPet = chB->GetSupportSystem();
				
				for (int supportIndex = 34065; supportIndex < 34065; supportIndex++)
				{
					CPetActor* chPetActor = chPet->GetByVnum(supportIndex);
					CPetActor* victimPetActor = victimPet->GetByVnum(supportIndex);	
					
					if (chPetActor != NULL)
						chSupport->Unsummon(supportIndex); 	
					
					if (victimPetActor != NULL)
						victimSupport->Unsummon(supportIndex); 
			}
#endif

			if ((chA->GetDuel("BlockPoly")) && (chB->GetDuel("BlockPoly")))
			{
				if (chA->IsPolymorphed())
					chA->SetPolymorph(0);
					chA->RemoveAffect(AFFECT_POLYMORPH);
				
				if (chB->IsPolymorphed())
					chB->SetPolymorph(0);
					chB->RemoveAffect(AFFECT_POLYMORPH);
			}	
			
			if ((chA->GetDuel("BlockRide")) && (chB->GetDuel("BlockRide")))
			{
				if (chA->FindAffect(AFFECT_MOUNT))
					chA->RemoveAffect(AFFECT_MOUNT);
					chA->RemoveAffect(AFFECT_MOUNT_BONUS);
					chA->MountVnum(0);
				
				if (chB->FindAffect(AFFECT_MOUNT))
					chB->RemoveAffect(AFFECT_MOUNT);
					chB->RemoveAffect(AFFECT_MOUNT_BONUS);
					chB->MountVnum(0);
				
				if (chA->IsHorseRiding())
					chA->StopRiding();
				
				if (chB->IsHorseRiding())
					chB->StopRiding();
				
				if (chA->GetHorse())
					chA->HorseSummon(false);
				
				if (chB->GetHorse())
					chB->HorseSummon(false);
			}				
			
			int m_nTableSkill[] = {94, 95, 96, 109, 110, 111, 175};	
			
			for (int j = 0; j < _countof(m_nTableSkill); j++)
			{
				if ((chA->GetDuel("BlockBuff")) && (chB->GetDuel("BlockBuff"))) {
					if (chA->GetJob() != JOB_SHAMAN || chB->GetJob() != JOB_SHAMAN) {
						chA->RemoveAffect(m_nTableSkill[j]);
						chB->RemoveAffect(m_nTableSkill[j]);
					}
				}
			}
			
			chA->SpecificEffectPacket("D:/ymir work/ui/game/pvp_advanced/1.mse");
			chB->SpecificEffectPacket("D:/ymir work/ui/game/pvp_advanced/1.mse");
			
			info->state++;
			return PASSES_PER_SEC(1);
			break; 
		}
		case 3:
		{   
			chA->SpecificEffectPacket("D:/ymir work/ui/game/pvp_advanced/go.mse");
			chB->SpecificEffectPacket("D:/ymir work/ui/game/pvp_advanced/go.mse");
			
			info->state++;
			return PASSES_PER_SEC(1);
			break;
		}
		case 4:
		{
			const char* szTableStaticPvP[] = {BLOCK_CHANGEITEM, BLOCK_BUFF, BLOCK_POTION, BLOCK_RIDE, BLOCK_PET, BLOCK_POLY, BLOCK_PARTY, BLOCK_EXCHANGE_, BET_WINNER, CHECK_IS_FIGHT};

			const char* chA_Name = chA->GetName();
			const char* chB_Name = chB->GetName();
			
			int chA_Level = chA->GetLevel();
			int chB_Level = chB->GetLevel();
			
			DWORD chA_Race = chA->GetRaceNum();
			DWORD chB_Race = chB->GetRaceNum();
			
			int chA_[] = {(chA->GetQuestFlag(szTableStaticPvP[0])), (chA->GetQuestFlag(szTableStaticPvP[1])), (chA->GetQuestFlag(szTableStaticPvP[2])), (chA->GetQuestFlag(szTableStaticPvP[3])), (chA->GetQuestFlag(szTableStaticPvP[4])), (chA->GetQuestFlag(szTableStaticPvP[5])), (chA->GetQuestFlag(szTableStaticPvP[6])), (chA->GetQuestFlag(szTableStaticPvP[7])), (chA->GetQuestFlag(szTableStaticPvP[8]))};
			int chB_[] = {(chB->GetQuestFlag(szTableStaticPvP[0])), (chB->GetQuestFlag(szTableStaticPvP[1])), (chB->GetQuestFlag(szTableStaticPvP[2])), (chB->GetQuestFlag(szTableStaticPvP[3])), (chB->GetQuestFlag(szTableStaticPvP[4])), (chB->GetQuestFlag(szTableStaticPvP[5])), (chB->GetQuestFlag(szTableStaticPvP[6])), (chB->GetQuestFlag(szTableStaticPvP[7])), (chB->GetQuestFlag(szTableStaticPvP[8]))};
			
			char chA_buf[CHAT_MAX_LEN + 1], chB_buf[CHAT_MAX_LEN + 1];
			
			snprintf(chA_buf, sizeof(chA_buf), "BINARY_Duel_LiveInterface %s %d %d %d %d %d %d %d %d %d %d %d", chB_Name, chB_Level, chB_Race, chA_[0], chA_[1], chA_[2], chA_[3], chA_[4], chA_[5], chA_[6], chA_[7], chA_[8]);
			snprintf(chB_buf, sizeof(chB_buf), "BINARY_Duel_LiveInterface %s %d %d %d %d %d %d %d %d %d %d %d", chA_Name, chA_Level, chA_Race, chB_[0], chB_[1], chB_[2], chB_[3], chB_[4], chB_[5], chB_[6], chB_[7], chB_[8]);
			
			chA->ChatPacket(CHAT_TYPE_COMMAND, chA_buf);
			chB->ChatPacket(CHAT_TYPE_COMMAND, chB_buf);
			
			chA->SetHP(chA->GetMaxHP());
			chB->SetHP(chB->GetMaxHP());
			
			info->pvp->Packet();
			return 0;
			break;
		}
		default:
		{
			return 0;
			break;
		}
    }
}
#endif

CPVP::CPVP(DWORD dwPID1, DWORD dwPID2)
{
	if (dwPID1 > dwPID2)
	{
		m_players[0].dwPID = dwPID1;
		m_players[1].dwPID = dwPID2;
		m_players[0].bAgree = true;
	}
	else
	{
		m_players[0].dwPID = dwPID2;
		m_players[1].dwPID = dwPID1;
		m_players[1].bAgree = true;
	}

	DWORD adwID[2];
	adwID[0] = m_players[0].dwPID;
	adwID[1] = m_players[1].dwPID;
	m_dwCRC = GetFastHash((const char *) &adwID, 8);
	m_bRevenge = false;

	SetLastFightTime();
}

CPVP::CPVP(CPVP & k)
{
	m_players[0] = k.m_players[0];
	m_players[1] = k.m_players[1];

	m_dwCRC = k.m_dwCRC;
	m_bRevenge = k.m_bRevenge;

	SetLastFightTime();
}

CPVP::~CPVP()
{
}

void CPVP::Packet(bool bDelete)
{
	if (!m_players[0].dwVID || !m_players[1].dwVID)
	{
		if (bDelete)
		//	sys_err("null vid when removing %u %u", m_players[0].dwVID, m_players[0].dwVID);

		return;
	}

	TPacketGCPVP pack;

	pack.bHeader = HEADER_GC_PVP;

	if (bDelete)
	{
		pack.bMode = PVP_MODE_NONE;
		pack.dwVIDSrc = m_players[0].dwVID;
		pack.dwVIDDst = m_players[1].dwVID;
	}
	else if (IsFight())
	{
		pack.bMode = PVP_MODE_FIGHT;
		pack.dwVIDSrc = m_players[0].dwVID;
		pack.dwVIDDst = m_players[1].dwVID;
	}
	else
	{
		pack.bMode = m_bRevenge ? PVP_MODE_REVENGE : PVP_MODE_AGREE;

		if (m_players[0].bAgree)
		{
			pack.dwVIDSrc = m_players[0].dwVID;
			pack.dwVIDDst = m_players[1].dwVID;
		}
		else
		{
			pack.dwVIDSrc = m_players[1].dwVID;
			pack.dwVIDDst = m_players[0].dwVID;
		}
	}

	const DESC_MANAGER::DESC_SET & c_rSet = DESC_MANAGER::instance().GetClientSet();
	DESC_MANAGER::DESC_SET::const_iterator it = c_rSet.begin();

	while (it != c_rSet.end())
	{
		LPDESC d = *it++;

		if (d->IsPhase(PHASE_GAME) || d->IsPhase(PHASE_DEAD))
			d->Packet(&pack, sizeof(pack));
	}
}

static LPEVENT m_pCheckDisconnect = NULL; 

EVENTFUNC(pvp_check_disconnect)
{
    if (event == NULL)
        return 0;
    
    if (event->info == NULL)
        return 0;
    
    TPVPCheckDisconnect* info = dynamic_cast<TPVPCheckDisconnect*>(event->info);
	
    if (info == NULL)
    {
        return 0;
    }
    
    LPCHARACTER chA = info->ch;
    LPCHARACTER chB = info->victim;
	
	const char* szTableStaticPvP[] = {BLOCK_CHANGEITEM, BLOCK_BUFF, BLOCK_POTION, BLOCK_RIDE, BLOCK_PET, BLOCK_POLY, BLOCK_PARTY, BLOCK_EXCHANGE_, BET_WINNER, CHECK_IS_FIGHT};
	
    if (chA == NULL)
    {
        int betMoney = chB->GetQuestFlag(szTableStaticPvP[8]);
        
        if (betMoney > 0)
		{
            chB->PointChange(POINT_GOLD, betMoney, true);
            chB->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("duel_victim_disconnect_receive_money"));
        }
        
        char buf[CHAT_MAX_LEN + 1];
        snprintf(buf, sizeof(buf), "BINARY_Duel_Delete");
        chB->ChatPacket(CHAT_TYPE_COMMAND, buf);	
        
        for (int i = 0; i < _countof(szTableStaticPvP); i++) {
            chB->SetQuestFlag(szTableStaticPvP[i], 0);	
        }
		
        chB->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("duel_victim_disconnect"));	
		//sys_err("pvp_check_disconnect: Victim chA was logout.");
        return 0;
    }	
    
    if (chB == NULL)
    {
        int betMoney = chA->GetQuestFlag(szTableStaticPvP[8]);
        
        if (betMoney > 0)
		{
            chA->PointChange(POINT_GOLD, betMoney, true);
            chA->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("duel_victim_disconnect_receive_money"));
        }
        
        char buf[CHAT_MAX_LEN + 1];
        snprintf(buf, sizeof(buf), "BINARY_Duel_Delete");
        chA->ChatPacket(CHAT_TYPE_COMMAND, buf);	
        
        for (int i = 0; i < _countof(szTableStaticPvP); i++) {
            chA->SetQuestFlag(szTableStaticPvP[i], 0);	
        }
		
        chA->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("duel_victim_disconnect"));	
     //   sys_err("pvp_check_disconnect: Victim chB was logout.");
        return 0;
    }
	
	return PASSES_PER_SEC(1);
}

bool CPVP::Agree(DWORD dwPID)
{
    m_players[m_players[0].dwPID != dwPID ? 1 : 0].bAgree = true; // dai compilare si ma duc sa mananc ceva

#ifdef __ENABLE_PVP_ADVANCED__
    if (IsFight())
    {
        if (m_pAdvancedDuelTimer != NULL)
		{
            event_cancel(&m_pAdvancedDuelTimer);
        }
		
        LPCHARACTER chA = CHARACTER_MANAGER::Instance().FindByPID(dwPID);
        LPCHARACTER chB = CHARACTER_MANAGER::Instance().FindByPID(m_players[m_players[0].dwPID != dwPID ? 0 : 1].dwPID);
		
		const char* szTableStaticPvP[] = {BLOCK_CHANGEITEM, BLOCK_BUFF, BLOCK_POTION, BLOCK_RIDE, BLOCK_PET, BLOCK_POLY, BLOCK_PARTY, BLOCK_EXCHANGE_, BET_WINNER, CHECK_IS_FIGHT};
		
        if (chA->GetQuestFlag(szTableStaticPvP[9]) != 1 && chB->GetQuestFlag(szTableStaticPvP[9]) != 1) {
            chA->SetDuel("IsFight", 1);
            chB->SetDuel("IsFight", 1);
        }
		
        {
            TPVPCheckDisconnect* info = AllocEventInfo<TPVPCheckDisconnect>();
            info->ch = chA;
            info->victim = chB;
            
			event_create(pvp_check_disconnect, info, 1);
        }

		{
			TPVPDuelEventInfo* info = AllocEventInfo<TPVPDuelEventInfo>();
			info->ch = chA;
			info->victim = chB;
			info->state = 0;
			info->pvp = this;
			m_pAdvancedDuelTimer = event_create(pvp_duel_counter, info, PASSES_PER_SEC(1));
		}
		
		return true;
    }
#else
    if (IsFight())
    {
        Packet();
        return true;
    }
#endif
    return false;
}

bool CPVP::IsFight()
{
	return (m_players[0].bAgree == m_players[1].bAgree) && m_players[0].bAgree;
}

void CPVP::Win(DWORD dwPID)
{
	int iSlot = m_players[0].dwPID != dwPID ? 1 : 0;

	m_bRevenge = true;

	m_players[iSlot].bAgree = true; // 자동으로 동의
	m_players[!iSlot].bCanRevenge = true;
	m_players[!iSlot].bAgree = false;

	Packet();
}

bool CPVP::CanRevenge(DWORD dwPID)
{
	return m_players[m_players[0].dwPID != dwPID ? 1 : 0].bCanRevenge;
}

void CPVP::SetVID(DWORD dwPID, DWORD dwVID)
{
	if (m_players[0].dwPID == dwPID)
		m_players[0].dwVID = dwVID;
	else
		m_players[1].dwVID = dwVID;
}

void CPVP::SetLastFightTime()
{
	m_dwLastFightTime = get_dword_time();
}

DWORD CPVP::GetLastFightTime()
{
	return m_dwLastFightTime;
}

CPVPManager::CPVPManager()
{
}

CPVPManager::~CPVPManager()
{
}

void CPVPManager::Insert(LPCHARACTER pkChr, LPCHARACTER pkVictim)
{
	if (pkChr->IsDead() || pkVictim->IsDead())
		return;

	CPVP kPVP(pkChr->GetPlayerID(), pkVictim->GetPlayerID());

	CPVP * pkPVP;

	if ((pkPVP = Find(kPVP.m_dwCRC)))
	{
		// 복수할 수 있으면 바로 싸움!
		if (pkPVP->Agree(pkChr->GetPlayerID()))
		{
			pkVictim->ChatPacket(CHAT_TYPE_INFO, "%s", pkChr->GetName());
			pkChr->ChatPacket(CHAT_TYPE_INFO, "%s", pkVictim->GetName());
		}
		return;
	}

	pkPVP = M2_NEW CPVP(kPVP);

	pkPVP->SetVID(pkChr->GetPlayerID(), pkChr->GetVID());
	pkPVP->SetVID(pkVictim->GetPlayerID(), pkVictim->GetVID());

	m_map_pkPVP.insert(map<DWORD, CPVP *>::value_type(pkPVP->m_dwCRC, pkPVP));

	m_map_pkPVPSetByID[pkChr->GetPlayerID()].insert(pkPVP);
	m_map_pkPVPSetByID[pkVictim->GetPlayerID()].insert(pkPVP);

	pkPVP->Packet();

	char msg[CHAT_MAX_LEN + 1];
	snprintf(msg, sizeof(msg), "%s", pkChr->GetName());

	pkVictim->ChatPacket(CHAT_TYPE_INFO, msg);
	pkChr->ChatPacket(CHAT_TYPE_INFO, "%s", pkVictim->GetName());

	// NOTIFY_PVP_MESSAGE
	LPDESC pkVictimDesc = pkVictim->GetDesc();
#ifdef __ENABLE_PVP_ADVANCED__
   if (pkVictimDesc)
    {
		const char* szTableStaticPvP[] = {BLOCK_CHANGEITEM, BLOCK_BUFF, BLOCK_POTION, BLOCK_RIDE, BLOCK_PET, BLOCK_POLY, BLOCK_PARTY, BLOCK_EXCHANGE_, BET_WINNER, CHECK_IS_FIGHT};

        int mTable[] = {(pkChr->GetQuestFlag(szTableStaticPvP[0])), (pkChr->GetQuestFlag(szTableStaticPvP[1])), (pkChr->GetQuestFlag(szTableStaticPvP[2])), (pkChr->GetQuestFlag(szTableStaticPvP[3])), (pkChr->GetQuestFlag(szTableStaticPvP[4])), (pkChr->GetQuestFlag(szTableStaticPvP[5])), (pkChr->GetQuestFlag(szTableStaticPvP[6])), (pkChr->GetQuestFlag(szTableStaticPvP[7])), (pkChr->GetQuestFlag(szTableStaticPvP[8]))};
        
        CGuild * g = pkChr->GetGuild();

        const char* m_Name = pkChr->GetName();
        const char* m_GuildName = "-";
		
        int m_Vid = pkChr->GetVID();	
        int m_Level = pkChr->GetLevel();
        int m_PlayTime = pkChr->GetRealPoint(POINT_PLAYTIME);
        int m_MaxHP = pkChr->GetMaxHP();
        int m_MaxSP = pkChr->GetMaxSP();
        int PVP_BLOCK_VIEW_EQUIPMENT = pkChr->GetQuestFlag(BLOCK_EQUIPMENT_);
		
        DWORD m_Race = pkChr->GetRaceNum();	
        
        if (g)
		{ 
            pkVictim->ChatPacket(CHAT_TYPE_COMMAND, "BINARY_Duel_Request %d %s %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d", m_Vid, m_Name, g->GetName(), m_Level, m_Race, m_PlayTime, m_MaxHP, m_MaxSP, mTable[0], mTable[1], mTable[2], mTable[3], mTable[4], mTable[5], mTable[6], mTable[7], mTable[8]);
            
            if (PVP_BLOCK_VIEW_EQUIPMENT < 1)
                pkChr->SendEquipment(pkVictim);	
        }
        else
            pkVictim->ChatPacket(CHAT_TYPE_COMMAND, "BINARY_Duel_Request %d %s %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d", m_Vid, m_Name, m_GuildName, m_Level, m_Race, m_PlayTime, m_MaxHP, m_MaxSP, mTable[0], mTable[1], mTable[2], mTable[3], mTable[4], mTable[5], mTable[6], mTable[7], mTable[8]);
            
            if (PVP_BLOCK_VIEW_EQUIPMENT < 1)
                pkChr->SendEquipment(pkVictim);
    }
#else
	if (pkVictimDesc)
	{
		TPacketGCWhisper pack;
		int len = MIN(CHAT_MAX_LEN, strlen(msg) + 1);
		pack.bHeader = HEADER_GC_WHISPER;
		pack.wSize = sizeof(TPacketGCWhisper) + len;
		pack.bType = WHISPER_TYPE_SYSTEM;
		strlcpy(pack.szNameFrom, pkChr->GetName(), sizeof(pack.szNameFrom));
		TEMP_BUFFER buf;
		buf.write(&pack, sizeof(TPacketGCWhisper));
		buf.write(msg, len);
		pkVictimDesc->Packet(buf.read_peek(), buf.size());
	}	
#endif
}

void CPVPManager::ConnectEx(LPCHARACTER pkChr, bool bDisconnect)
{
	CPVPSetMap::iterator it = m_map_pkPVPSetByID.find(pkChr->GetPlayerID());

	if (it == m_map_pkPVPSetByID.end())
		return;

	DWORD dwVID = bDisconnect ? 0 : pkChr->GetVID();

	TR1_NS::unordered_set<CPVP*>::iterator it2 = it->second.begin();

	while (it2 != it->second.end())
	{
		CPVP * pkPVP = *it2++;
		pkPVP->SetVID(pkChr->GetPlayerID(), dwVID);
	}
}

void CPVPManager::Connect(LPCHARACTER pkChr)
{
	ConnectEx(pkChr, false);
}

void CPVPManager::Disconnect(LPCHARACTER pkChr)
{
#ifdef __ENABLE_PVP_ADVANCED__
    CPVPSetMap::iterator it = m_map_pkPVPSetByID.find(pkChr->GetPlayerID());
    
    if (it == m_map_pkPVPSetByID.end())
        return;
    
    TR1_NS::unordered_set<CPVP*>::iterator it2 = it->second.begin();
    
    while (it2 != it->second.end()) {
        CPVP * pkPVP = *it2++;
        pkPVP->Packet(true);
        Delete(pkPVP);	
    }
#endif
}

void CPVPManager::GiveUp(LPCHARACTER pkChr, DWORD dwKillerPID) // This method is calling from no where yet.
{
	CPVPSetMap::iterator it = m_map_pkPVPSetByID.find(pkChr->GetPlayerID());

	if (it == m_map_pkPVPSetByID.end())
		return;

	sys_log(1, "PVPManager::Dead %d", pkChr->GetPlayerID());
	TR1_NS::unordered_set<CPVP*>::iterator it2 = it->second.begin();

	while (it2 != it->second.end())
	{
		CPVP * pkPVP = *it2++;

		DWORD dwCompanionPID;

		if (pkPVP->m_players[0].dwPID == pkChr->GetPlayerID())
			dwCompanionPID = pkPVP->m_players[1].dwPID;
		else
			dwCompanionPID = pkPVP->m_players[0].dwPID;

		if (dwCompanionPID != dwKillerPID)
			continue;

		pkPVP->SetVID(pkChr->GetPlayerID(), 0);

		m_map_pkPVPSetByID.erase(dwCompanionPID);

		it->second.erase(pkPVP);

		if (it->second.empty())
			m_map_pkPVPSetByID.erase(it);

		m_map_pkPVP.erase(pkPVP->m_dwCRC);

		pkPVP->Packet(true);
		M2_DELETE(pkPVP);
		break;
	}
}

#ifdef __ENABLE_PVP_ADVANCED__//COMPILE
void CPVPManager::Decline(LPCHARACTER pkChr, LPCHARACTER pkVictim)
{
    CPVPSetMap::iterator it = m_map_pkPVPSetByID.find(pkChr->GetPlayerID());
    
    if (it == m_map_pkPVPSetByID.end())
        return;
    
    bool found = false;
    
    TR1_NS::unordered_set<CPVP*>::iterator it2 = it->second.begin();
    
    while (it2 != it->second.end()) {
        CPVP * pkPVP = *it2++;
        DWORD dwCompanionPID;
        
        if (pkPVP->m_players[0].dwPID == pkChr->GetPlayerID())
            dwCompanionPID = pkPVP->m_players[1].dwPID;
        else
            dwCompanionPID = pkPVP->m_players[0].dwPID;
        
        if (dwCompanionPID == pkVictim->GetPlayerID()) {
            if (pkPVP->IsFight()) {
                pkChr->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("duel_already_exist"));
                return;
            }
            
            pkPVP->Packet(true);
            Delete(pkPVP);
            pkPVP->SetLastFightTime();
            found = true;

			const char* szTableStaticPvP[] = {BLOCK_CHANGEITEM, BLOCK_BUFF, BLOCK_POTION, BLOCK_RIDE, BLOCK_PET, BLOCK_POLY, BLOCK_PARTY, BLOCK_EXCHANGE_, BET_WINNER, CHECK_IS_FIGHT};
            
            for (int i = 0; i < _countof(szTableStaticPvP); i++) {
                char pkChr_Buf[CHAT_MAX_LEN + 1], pkVictim_Buf[CHAT_MAX_LEN + 1];
                
                snprintf(pkChr_Buf, sizeof(pkChr_Buf), "BINARY_Duel_Delete");
                snprintf(pkVictim_Buf, sizeof(pkVictim_Buf), "BINARY_Duel_Delete");
                
                pkChr->ChatPacket(CHAT_TYPE_COMMAND, pkChr_Buf);	
                pkChr->SetQuestFlag(szTableStaticPvP[i], 0);
                
                pkVictim->ChatPacket(CHAT_TYPE_COMMAND, pkVictim_Buf);	
                pkVictim->SetQuestFlag(szTableStaticPvP[i], 0);	
            }
            pkVictim->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("duel_rejected"), pkChr->GetName());
        }
    }
}
#endif

bool CPVPManager::Dead(LPCHARACTER pkChr, DWORD dwKillerPID)
{
	CPVPSetMap::iterator it = m_map_pkPVPSetByID.find(pkChr->GetPlayerID());

	if (it == m_map_pkPVPSetByID.end())
		return false;

	bool found = false;

	sys_log(1, "PVPManager::Dead %d", pkChr->GetPlayerID());
	TR1_NS::unordered_set<CPVP*>::iterator it2 = it->second.begin();

	while (it2 != it->second.end())
	{
		CPVP * pkPVP = *it2++;

		DWORD dwCompanionPID;

		if (pkPVP->m_players[0].dwPID == pkChr->GetPlayerID())
			dwCompanionPID = pkPVP->m_players[1].dwPID;
		else
			dwCompanionPID = pkPVP->m_players[0].dwPID;

		if (dwCompanionPID == dwKillerPID)
		{
			if (pkPVP->IsFight())
			{
				pkPVP->SetLastFightTime();
#ifdef __ENABLE_PVP_ADVANCED__
				pkPVP->Packet(true);
				Delete(pkPVP);
#else
				pkPVP->Win(dwKillerPID);
#endif
				found = true;
				break;
			}
			else if (get_dword_time() - pkPVP->GetLastFightTime() <= 15000)
			{
				found = true;
				break;
			}
		}
	}

	return found;
}

bool CPVPManager::CanAttack(LPCHARACTER pkChr, LPCHARACTER pkVictim)
{
	switch (pkVictim->GetCharType())
	{
		case CHAR_TYPE_NPC:
		case CHAR_TYPE_WARP:
		case CHAR_TYPE_GOTO:
			return false;
	}

	if (pkChr == pkVictim)  // 내가 날 칠라고 하네 -_-
		return false;

	if (pkVictim->IsNPC() && pkChr->IsNPC() && !pkChr->IsGuardNPC())
		return false;

	if( true == pkChr->IsHorseRiding() )
	{
		if( pkChr->GetHorseLevel() > 0 && 1 == pkChr->GetHorseGrade() )
			return false;
	}
	else
	{
		switch( pkChr->GetMountVnum() )
		{
			case 0:
			case 20030:
			case 20110:
			case 20111:
			case 20112:
			case 20113:
			case 20114:
			case 20115:
			case 20116:
			case 20117:
			case 20118:
				//신규 탈것 고급
			case 20205:
			case 20206:
			case 20207:
			case 20208:
			case 20209:
			case 20210:
			case 20211:
			case 20212:
			case 20119:		// 라마단 흑마
			case 20219:		// 라마단 흑마 클론 (할로윈용)
			case 20220:		// 크리스마스 탈것
			case 20221:		// 전갑 백웅
			case 20222:		// 전갑 팬더
			case 20120:
			case 20121:
			case 20122:
			case 20123:
			case 20124:
			case 20125:
			case 20214:		// 난폭한 전갑순순록	
			case 20215:		// 용맹한 전갑순순록	
			case 20217:		// 난폭한 전갑암순록	
			case 20218:		// 용맹한 전갑암순록
			case 20224:		// 난폭한 전갑석룡자
			case 20225:		// 용맹한 전갑석룡자
			case 20226:		//	유니콘
			case 20227:
			case 20229:
			case 20230:
			case 20231:
			case 20232:
			case 20233:
			case 20234:
			case 20235:
			case 20236:
			case 20237:
			case 20238:
			case 20239:
			case 20240:
			case 20241:
			case 20242:
			case 20243:
			case 20244:
			case 20245:
			case 20246:
			case 20247:
			case 20248:
			case 20249:
			case 20250:
				break;

			default:
				return false;
		}
	}

	if (pkVictim->IsNPC() || pkChr->IsNPC())
	{
		return true;
	}

	if (pkVictim->IsObserverMode() || pkChr->IsObserverMode())
		return false;

	{
		BYTE bMapEmpire = SECTREE_MANAGER::instance().GetEmpireFromMapIndex(pkChr->GetMapIndex());

		if ( ((pkChr->GetPKMode() == PK_MODE_PROTECT) && (pkChr->GetEmpire() == bMapEmpire)) ||
				((pkVictim->GetPKMode() == PK_MODE_PROTECT) && (pkVictim->GetEmpire() == bMapEmpire)) )
		{
			return false;
		}
	}

#ifdef __PVP_TOURNAMENT_SYSTEM__
	if (pkChr->GetEmpire() != pkVictim->GetEmpire() && pkChr->GetMapIndex() != TOURNAMENT_MAP_INDEX)
#else
	if (pkChr->GetEmpire() != pkVictim->GetEmpire())
#endif
	{
		// @warme005
		{
			if ( pkChr->GetPKMode() == PK_MODE_PROTECT || pkVictim->GetPKMode() == PK_MODE_PROTECT )
			{
				return false;
			}
		}

		return true;
	}

	bool beKillerMode = false;

	if (pkVictim->GetParty() && pkVictim->GetParty() == pkChr->GetParty())
	{
		return false;
		// Cannot attack same party on any pvp model
	}
	else
	{
		if (pkVictim->IsKillerMode())
		{
			return true;
		}

		if (pkChr->GetAlignment() < 0 && pkVictim->GetAlignment() >= 0)
		{
		    if (g_protectNormalPlayer)
		    {
			// 범법자는 평화모드인 착한사람을 공격할 수 없다.
			if (PK_MODE_PEACE == pkVictim->GetPKMode())
			    return false;
		    }
		}


		switch (pkChr->GetPKMode())
		{
			case PK_MODE_PEACE:
			case PK_MODE_REVENGE:
				// Cannot attack same guild
				if (pkVictim->GetGuild() && pkVictim->GetGuild() == pkChr->GetGuild())
					break;

				if (pkChr->GetPKMode() == PK_MODE_REVENGE)
				{
					if (pkChr->GetAlignment() < 0 && pkVictim->GetAlignment() >= 0)
					{
						pkChr->SetKillerMode(true);
						return true;
					}
					else if (pkChr->GetAlignment() >= 0 && pkVictim->GetAlignment() < 0)
						return true;
				}
				break;

			case PK_MODE_GUILD:
				// Same implementation from PK_MODE_FREE except for attacking same guild
				if (!pkChr->GetGuild() || (pkVictim->GetGuild() != pkChr->GetGuild()))
				{
					if (pkVictim->GetAlignment() >= 0)
						pkChr->SetKillerMode(true);
					else if (pkChr->GetAlignment() < 0 && pkVictim->GetAlignment() < 0)
						pkChr->SetKillerMode(true);

					return true;
				}
				break;

			case PK_MODE_FREE:
				if (pkVictim->GetAlignment() >= 0)
					pkChr->SetKillerMode(true);
				else if (pkChr->GetAlignment() < 0 && pkVictim->GetAlignment() < 0)
					pkChr->SetKillerMode(true);
				return true;
				break;
		}
	}

#ifdef __PVP_TOURNAMENT_SYSTEM__
	if (pkChr->GetMapIndex() == TOURNAMENT_MAP_INDEX && pkVictim->GetMapIndex() == TOURNAMENT_MAP_INDEX)
	{
		if (pkChr->GetPKMode() == pkVictim->GetPKMode())
			return false;
		
		if (pkChr->GetPKMode() != pkVictim->GetPKMode())
			return true;
	}
#endif

	CPVP kPVP(pkChr->GetPlayerID(), pkVictim->GetPlayerID());
	CPVP * pkPVP = Find(kPVP.m_dwCRC);

	if (!pkPVP || !pkPVP->IsFight())
	{
		if (beKillerMode)
			pkChr->SetKillerMode(true);

		return (beKillerMode);
	}

	pkPVP->SetLastFightTime();
	return true;
}

CPVP * CPVPManager::Find(DWORD dwCRC)
{
	map<DWORD, CPVP *>::iterator it = m_map_pkPVP.find(dwCRC);

	if (it == m_map_pkPVP.end())
		return NULL;

	return it->second;
}

void CPVPManager::Delete(CPVP * pkPVP)
{
	map<DWORD, CPVP *>::iterator it = m_map_pkPVP.find(pkPVP->m_dwCRC);

	if (it == m_map_pkPVP.end())
		return;

	m_map_pkPVP.erase(it);
	m_map_pkPVPSetByID[pkPVP->m_players[0].dwPID].erase(pkPVP);
	m_map_pkPVPSetByID[pkPVP->m_players[1].dwPID].erase(pkPVP);

	M2_DELETE(pkPVP);
}

void CPVPManager::SendList(LPDESC d)
{
	map<DWORD, CPVP *>::iterator it = m_map_pkPVP.begin();

	DWORD dwVID = d->GetCharacter()->GetVID();

	TPacketGCPVP pack;

	pack.bHeader = HEADER_GC_PVP;

	while (it != m_map_pkPVP.end())
	{
		CPVP * pkPVP = (it++)->second;

		if (!pkPVP->m_players[0].dwVID || !pkPVP->m_players[1].dwVID)
			continue;

		// VID가 둘다 있을 경우에만 보낸다.
		if (pkPVP->IsFight())
		{
			pack.bMode = PVP_MODE_FIGHT;
			pack.dwVIDSrc = pkPVP->m_players[0].dwVID;
			pack.dwVIDDst = pkPVP->m_players[1].dwVID;
		}
		else
		{
			pack.bMode = pkPVP->m_bRevenge ? PVP_MODE_REVENGE : PVP_MODE_AGREE;

			if (pkPVP->m_players[0].bAgree)
			{
				pack.dwVIDSrc = pkPVP->m_players[0].dwVID;
				pack.dwVIDDst = pkPVP->m_players[1].dwVID;
			}
			else
			{
				pack.dwVIDSrc = pkPVP->m_players[1].dwVID;
				pack.dwVIDDst = pkPVP->m_players[0].dwVID;
			}
		}

		d->Packet(&pack, sizeof(pack));
		sys_log(1, "PVPManager::SendList %d %d", pack.dwVIDSrc, pack.dwVIDDst);

		if (pkPVP->m_players[0].dwVID == dwVID)
		{
			LPCHARACTER ch = CHARACTER_MANAGER::instance().Find(pkPVP->m_players[1].dwVID);
			if (ch && ch->GetDesc())
			{
				LPDESC d = ch->GetDesc();
				d->Packet(&pack, sizeof(pack));
			}
		}
		else if (pkPVP->m_players[1].dwVID == dwVID)
		{
			LPCHARACTER ch = CHARACTER_MANAGER::instance().Find(pkPVP->m_players[0].dwVID);
			if (ch && ch->GetDesc())
			{
				LPDESC d = ch->GetDesc();
				d->Packet(&pack, sizeof(pack));
			}
		}
	}
}

void CPVPManager::Process()
{
	map<DWORD, CPVP *>::iterator it = m_map_pkPVP.begin();

	while (it != m_map_pkPVP.end())
	{
		CPVP * pvp = (it++)->second;

		if (get_dword_time() - pvp->GetLastFightTime() > 600000) // 10분 이상 싸움이 없었으면
		{
			pvp->Packet(true);
			Delete(pvp);
		}
	}
}

