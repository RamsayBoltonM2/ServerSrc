#include "stdafx.h"
#include "constants.h"
#include "config.h"
#include "utils.h"
#include "input.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "char.h"
#include "char_manager.h"
#include "cmd.h"
#include "buffer_manager.h"
#include "protocol.h"
#include "pvp.h"
#include "start_position.h"
#include "messenger_manager.h"
#include "guild_manager.h"
#include "party.h"
#include "dungeon.h"
#include "war_map.h"
#include "questmanager.h"
#include "building.h"
#include "wedding.h"
#include "affect.h"
#include "arena.h"
#include "OXEvent.h"
#include "priv_manager.h"
#include "block_country.h"
#include "dev_log.h"
#include "log.h"
#ifdef __MAINTENANCE_SYSTEM__
	#include "maintenance.h"
#endif
#ifdef __PVP_TOURNAMENT_SYSTEM__
	#include "tournament.h"
#endif
#include "horsename_manager.h"
#ifdef __BIOLOG_SYSTEM__
	#include "biolog.h"
#endif
#include "MarkManager.h"
#ifdef __MELEY_LAIR_DUNGEON__
#include "MeleyLair.h"
#endif
#ifdef GROUP_MATCH
#include "GroupMatchManager.h"
#endif
#include "../../common/service.h"

#ifdef __YMIR_UPDATE_WOLFMAN__

// #define USE_LYCAN_CREATE_POSITION
#ifdef USE_LYCAN_CREATE_POSITION


DWORD g_lycan_create_position[4][2] =
{
	{		0,		0 },
	{ 768000+38300, 896000+35500 },
	{ 819200+38300, 896000+35500 },
	{ 870400+38300, 896000+35500 },
};

inline DWORD LYCAN_CREATE_START_X(BYTE e, BYTE job)
{
	if (1 <= e && e <= 3)
		return (job==JOB_WOLFMAN)?g_lycan_create_position[e][0]:g_create_position[e][0];
	return 0;
}

inline DWORD LYCAN_CREATE_START_Y(BYTE e, BYTE job)
{
	if (1 <= e && e <= 3)
		return (job==JOB_WOLFMAN)?g_lycan_create_position[e][1]:g_create_position[e][1];
	return 0;
}

#endif


#endif

static void _send_bonus_info(LPCHARACTER ch)
{
	int	item_drop_bonus = 0;
	int gold_drop_bonus = 0;
	int gold10_drop_bonus	= 0;
	int exp_bonus		= 0;

	item_drop_bonus		= CPrivManager::instance().GetPriv(ch, PRIV_ITEM_DROP);
	gold_drop_bonus		= CPrivManager::instance().GetPriv(ch, PRIV_GOLD_DROP);
	gold10_drop_bonus	= CPrivManager::instance().GetPriv(ch, PRIV_GOLD10_DROP);
	exp_bonus			= CPrivManager::instance().GetPriv(ch, PRIV_EXP_PCT);

	if (item_drop_bonus)
	{
		ch->ChatPacket(CHAT_TYPE_NOTICE,
				LC_TEXT("¾ÆÀÌÅÛ µå·Ó·ü  %d%% Ãß°¡ ÀÌº¥Æ® ÁßÀÔ´Ï´Ù."), item_drop_bonus);
	}
	if (gold_drop_bonus)
	{
		ch->ChatPacket(CHAT_TYPE_NOTICE,
				LC_TEXT("°ñµå µå·Ó·ü %d%% Ãß°¡ ÀÌº¥Æ® ÁßÀÔ´Ï´Ù."), gold_drop_bonus);
	}
	if (gold10_drop_bonus)
	{
		ch->ChatPacket(CHAT_TYPE_NOTICE,
				LC_TEXT("´ë¹Ú°ñµå µå·Ó·ü %d%% Ãß°¡ ÀÌº¥Æ® ÁßÀÔ´Ï´Ù."), gold10_drop_bonus);
	}
	if (exp_bonus)
	{
		ch->ChatPacket(CHAT_TYPE_NOTICE,
				LC_TEXT("°æÇèÄ¡ %d%% Ãß°¡ È¹µæ ÀÌº¥Æ® ÁßÀÔ´Ï´Ù."), exp_bonus);
	}
}

static bool FN_is_battle_zone(LPCHARACTER ch)
{
	switch (ch->GetMapIndex())
	{
		case 1:         // ½Å¼ö 1Â÷ ¸¶À»
		case 2:         // ½Å¼ö 2Â÷ ¸¶À»
		case 21:        // ÃµÁ¶ 1Â÷ ¸¶À»
		case 23:        // ÃµÁ¶ 2Â÷ ¸¶À»
		case 41:        // Áø³ë 1Â÷ ¸¶À»
		case 43:        // Áø³ë 2Â÷ ¸¶À»
		case 113:       // OX ¸Ê
			return false;
	}

	return true;
}

void CInputLogin::Login(LPDESC d, const char * data)
{
	TPacketCGLogin * pinfo = (TPacketCGLogin *) data;

	char login[LOGIN_MAX_LEN + 1];
	trim_and_lower(pinfo->login, login, sizeof(login));

	sys_log(0, "InputLogin::Login : %s", login);

	TPacketGCLoginFailure failurePacket;

	if (g_iUseLocale && !test_server)
	{
		failurePacket.header = HEADER_GC_LOGIN_FAILURE;
		strlcpy(failurePacket.szStatus, "VERSION", sizeof(failurePacket.szStatus));
		d->Packet(&failurePacket, sizeof(TPacketGCLoginFailure));
		return;
	}

	if (g_bNoMoreClient)
	{
		failurePacket.header = HEADER_GC_LOGIN_FAILURE;
		strlcpy(failurePacket.szStatus, "SHUTDOWN", sizeof(failurePacket.szStatus));
		d->Packet(&failurePacket, sizeof(TPacketGCLoginFailure));
		return;
	}

	if (g_iUserLimit > 0)
	{
		int iTotal;
		int * paiEmpireUserCount;
		int iLocal;

		DESC_MANAGER::instance().GetUserCount(iTotal, &paiEmpireUserCount, iLocal);

		if (g_iUserLimit <= iTotal)
		{
			failurePacket.header = HEADER_GC_LOGIN_FAILURE;
			strlcpy(failurePacket.szStatus, "FULL", sizeof(failurePacket.szStatus));
			d->Packet(&failurePacket, sizeof(TPacketGCLoginFailure));
			return;
		}
	}

	TLoginPacket login_packet;

	strlcpy(login_packet.login, login, sizeof(login_packet.login));
	strlcpy(login_packet.passwd, pinfo->passwd, sizeof(login_packet.passwd));

	db_clientdesc->DBPacket(HEADER_GD_LOGIN, d->GetHandle(), &login_packet, sizeof(TLoginPacket));
}

void CInputLogin::LoginByKey(LPDESC d, const char * data)
{
	TPacketCGLogin2 * pinfo = (TPacketCGLogin2 *) data;

	char login[LOGIN_MAX_LEN + 1];
	trim_and_lower(pinfo->login, login, sizeof(login));

	// is blocked ip?
	{
		dev_log(LOG_DEB0, "check_blocked_country_start");

		if (!is_block_exception(login) && is_blocked_country_ip(d->GetHostName()))
		{
			sys_log(0, "BLOCK_COUNTRY_IP (%s)", d->GetHostName());
			d->SetPhase(PHASE_CLOSE);
			return;
		}

		dev_log(LOG_DEB0, "check_blocked_country_end");
	}

	if (g_bNoMoreClient)
	{
		TPacketGCLoginFailure failurePacket;

		failurePacket.header = HEADER_GC_LOGIN_FAILURE;
		strlcpy(failurePacket.szStatus, "SHUTDOWN", sizeof(failurePacket.szStatus));
		d->Packet(&failurePacket, sizeof(TPacketGCLoginFailure));
		return;
	}

	if (g_iUserLimit > 0)
	{
		int iTotal;
		int * paiEmpireUserCount;
		int iLocal;

		DESC_MANAGER::instance().GetUserCount(iTotal, &paiEmpireUserCount, iLocal);

		if (g_iUserLimit <= iTotal)
		{
			TPacketGCLoginFailure failurePacket;

			failurePacket.header = HEADER_GC_LOGIN_FAILURE;
			strlcpy(failurePacket.szStatus, "FULL", sizeof(failurePacket.szStatus));

			d->Packet(&failurePacket, sizeof(TPacketGCLoginFailure));
			return;
		}
	}

	sys_log(0, "LOGIN_BY_KEY: %s key %u", login, pinfo->dwLoginKey);

	d->SetLoginKey(pinfo->dwLoginKey);
#ifndef _IMPROVED_PACKET_ENCRYPTION_
	d->SetSecurityKey(pinfo->adwClientKey);
#endif

	TPacketGDLoginByKey ptod;

	strlcpy(ptod.szLogin, login, sizeof(ptod.szLogin));
	ptod.dwLoginKey = pinfo->dwLoginKey;
	thecore_memcpy(ptod.adwClientKey, pinfo->adwClientKey, sizeof(DWORD) * 4);
	strlcpy(ptod.szIP, d->GetHostName(), sizeof(ptod.szIP));

	db_clientdesc->DBPacket(HEADER_GD_LOGIN_BY_KEY, d->GetHandle(), &ptod, sizeof(TPacketGDLoginByKey));
}

void CInputLogin::ChangeName(LPDESC d, const char * data)
{
	TPacketCGChangeName * p = (TPacketCGChangeName *) data;
	const TAccountTable & c_r = d->GetAccountTable();

	if (!c_r.id)
	{
		sys_err("no account table");
		return;
	}

	if (!c_r.players[p->index].bChangeName)
		return;

	if (!check_name(p->name))
	{
		TPacketGCCreateFailure pack;
		pack.header = HEADER_GC_CHARACTER_CREATE_FAILURE;
		pack.bType = 0;
		d->Packet(&pack, sizeof(pack));
		return;
	}

	TPacketGDChangeName pdb;

	pdb.pid = c_r.players[p->index].dwID;
	strlcpy(pdb.name, p->name, sizeof(pdb.name));
	db_clientdesc->DBPacket(HEADER_GD_CHANGE_NAME, d->GetHandle(), &pdb, sizeof(TPacketGDChangeName));
}

void CInputLogin::CharacterSelect(LPDESC d, const char * data)
{
	struct command_player_select * pinfo = (struct command_player_select *) data;
	const TAccountTable & c_r = d->GetAccountTable();

	sys_log(0, "player_select: login: %s index: %d", c_r.login, pinfo->index);

	if (!c_r.id)
	{
		sys_err("no account table");
		return;
	}

	if (pinfo->index >= PLAYER_PER_ACCOUNT)
	{
		sys_err("index overflow %d, login: %s", pinfo->index, c_r.login);
		return;
	}

	if (c_r.players[pinfo->index].bChangeName)
	{
		sys_err("name must be changed idx %d, login %s, name %s",
				pinfo->index, c_r.login, c_r.players[pinfo->index].szName);
		return;
	}

	TPlayerLoadPacket player_load_packet;

	player_load_packet.account_id	= c_r.id;
	player_load_packet.player_id	= c_r.players[pinfo->index].dwID;
	player_load_packet.account_index	= pinfo->index;

	db_clientdesc->DBPacket(HEADER_GD_PLAYER_LOAD, d->GetHandle(), &player_load_packet, sizeof(TPlayerLoadPacket));
}

bool NewPlayerTable(TPlayerTable * table,
		const char * name,
		BYTE job,
		BYTE shape,
		BYTE bEmpire,
		BYTE bCon,
		BYTE bInt,
		BYTE bStr,
		BYTE bDex)
{
	if (job >= JOB_MAX_NUM)
		return false;

	memset(table, 0, sizeof(TPlayerTable));

	strlcpy(table->name, name, sizeof(table->name));

	table->level = 1;
	table->job = job;
	table->voice = 0;
	table->part_base = shape;

	table->st = JobInitialPoints[job].st;
	table->dx = JobInitialPoints[job].dx;
	table->ht = JobInitialPoints[job].ht;
	table->iq = JobInitialPoints[job].iq;

	table->hp = JobInitialPoints[job].max_hp + table->ht * JobInitialPoints[job].hp_per_ht;
	table->sp = JobInitialPoints[job].max_sp + table->iq * JobInitialPoints[job].sp_per_iq;
	table->stamina = JobInitialPoints[job].max_stamina;

#if defined(__YMIR_UPDATE_WOLFMAN__) && defined(USE_LYCAN_CREATE_POSITION)
	table->x 	= LYCAN_CREATE_START_X(bEmpire, job) + number(-300, 300);
	table->y 	= LYCAN_CREATE_START_Y(bEmpire, job) + number(-300, 300);
#else
	table->x 	= CREATE_START_X(bEmpire) + number(-300, 300);
	table->y 	= CREATE_START_Y(bEmpire) + number(-300, 300);
#endif
	table->z	= 0;
	table->dir	= 0;
	table->playtime = 0;
	table->gold 	= 0;
#ifdef __CHEQUE_SYSTEM__
	table->cheque 	= 0;
#endif

	table->skill_group = 0;

	if (china_event_server)
	{
		table->level = 35;

		for (int i = 1; i < 35; ++i)
		{
			int iHP = number(JobInitialPoints[job].hp_per_lv_begin, JobInitialPoints[job].hp_per_lv_end);
			int iSP = number(JobInitialPoints[job].sp_per_lv_begin, JobInitialPoints[job].sp_per_lv_end);
			table->sRandomHP += iHP;
			table->sRandomSP += iSP;
			table->stat_point += 3;
		}

		table->hp += table->sRandomHP;
		table->sp += table->sRandomSP;

		table->gold = 1000000;
	}

	return true;
}

bool RaceToJob(unsigned race, unsigned* ret_job)
{
	*ret_job = 0;

	if (race >= MAIN_RACE_MAX_NUM)
		return false;

	switch (race)
	{
		case MAIN_RACE_WARRIOR_M:
			*ret_job = JOB_WARRIOR;
			break;

		case MAIN_RACE_WARRIOR_W:
			*ret_job = JOB_WARRIOR;
			break;

		case MAIN_RACE_ASSASSIN_M:
			*ret_job = JOB_ASSASSIN;
			break;

		case MAIN_RACE_ASSASSIN_W:
			*ret_job = JOB_ASSASSIN;
			break;

		case MAIN_RACE_SURA_M:
			*ret_job = JOB_SURA;
			break;

		case MAIN_RACE_SURA_W:
			*ret_job = JOB_SURA;
			break;

		case MAIN_RACE_SHAMAN_M:
			*ret_job = JOB_SHAMAN;
			break;

		case MAIN_RACE_SHAMAN_W:
			*ret_job = JOB_SHAMAN;
			break;
#ifdef __YMIR_UPDATE_WOLFMAN__
		case MAIN_RACE_WOLFMAN_M:
			*ret_job = JOB_WOLFMAN;
			break;
#endif
		default:
			return false;
			break;
	}
	return true;
}

// ½Å±Ô Ä³¸¯ÅÍ Áö¿ø
bool NewPlayerTable2(TPlayerTable * table, const char * name, BYTE race, BYTE shape, BYTE bEmpire)
{
	if (race >= MAIN_RACE_MAX_NUM)
	{
		sys_err("NewPlayerTable2.OUT_OF_RACE_RANGE(%d >= max(%d))\n", race, MAIN_RACE_MAX_NUM);
		return false;
	}

	unsigned job;

	if (!RaceToJob(race, &job))
	{
		sys_err("NewPlayerTable2.RACE_TO_JOB_ERROR(%d)\n", race);
		return false;
	}

	sys_log(0, "NewPlayerTable2(name=%s, race=%d, job=%d)", name, race, job);

	memset(table, 0, sizeof(TPlayerTable));

	strlcpy(table->name, name, sizeof(table->name));

	table->level		= 1;
	table->job			= race;	// Á÷¾÷´ë½Å Á¾Á·À» ³Ö´Â´Ù
	table->voice		= 0;
	table->part_base	= shape;

	table->st		= JobInitialPoints[job].st;
	table->dx		= JobInitialPoints[job].dx;
	table->ht		= JobInitialPoints[job].ht;
	table->iq		= JobInitialPoints[job].iq;

	table->hp		= JobInitialPoints[job].max_hp + table->ht * JobInitialPoints[job].hp_per_ht;
	table->sp		= JobInitialPoints[job].max_sp + table->iq * JobInitialPoints[job].sp_per_iq;
	table->stamina	= JobInitialPoints[job].max_stamina;

#if defined(__YMIR_UPDATE_WOLFMAN__) && defined(USE_LYCAN_CREATE_POSITION)
	table->x 		= LYCAN_CREATE_START_X(bEmpire, job) + number(-300, 300);
	table->y 		= LYCAN_CREATE_START_Y(bEmpire, job) + number(-300, 300);
#else
	table->x		= CREATE_START_X(bEmpire) + number(-300, 300);
	table->y		= CREATE_START_Y(bEmpire) + number(-300, 300);
#endif
	table->z		= 0;
	table->dir		= 0;
	table->playtime = 0;
	table->gold 	= 0;
#ifdef __CHEQUE_SYSTEM__
	table->cheque 	= 0;
#endif

	table->skill_group = 0;

	return true;
}

void CInputLogin::CharacterCreate(LPDESC d, const char * data)
{
	struct command_player_create * pinfo = (struct command_player_create *) data;
	TPlayerCreatePacket player_create_packet;

	sys_log(0, "PlayerCreate: name %s pos %d job %d shape %d",
			pinfo->name,
			pinfo->index,
			pinfo->job,
			pinfo->shape);

	TPacketGCLoginFailure packFailure;
	memset(&packFailure, 0, sizeof(packFailure));
	packFailure.header = HEADER_GC_CHARACTER_CREATE_FAILURE;

	if (true == g_BlockCharCreation)
	{
		d->Packet(&packFailure, sizeof(packFailure));
		return;
	}
#ifdef ENABLE_ANTI_LONG_NAME_GENERATOR_AFTER_CHARACTER_CREATION
	if (strlen(pinfo->name) > 12){
		d->Packet(&packFailure, sizeof(packFailure));
		return;
	}
#endif
	if (!check_name(pinfo->name) || pinfo->shape > 1)
	{
		d->Packet(&packFailure, sizeof(packFailure));
		return;
	}

	const TAccountTable & c_rAccountTable = d->GetAccountTable();

	if (0 == strcmp(c_rAccountTable.login, pinfo->name))
	{
		TPacketGCCreateFailure pack;
		pack.header = HEADER_GC_CHARACTER_CREATE_FAILURE;
		pack.bType = 1;

		d->Packet(&pack, sizeof(pack));
		return;
	}

	memset(&player_create_packet, 0, sizeof(TPlayerCreatePacket));

	if (!NewPlayerTable2(&player_create_packet.player_table, pinfo->name, pinfo->job, pinfo->shape, d->GetEmpire()))
	{
		sys_err("player_prototype error: job %d face %d ", pinfo->job);
		d->Packet(&packFailure, sizeof(packFailure));
		return;
	}

	trim_and_lower(c_rAccountTable.login, player_create_packet.login, sizeof(player_create_packet.login));
	strlcpy(player_create_packet.passwd, c_rAccountTable.passwd, sizeof(player_create_packet.passwd));

	player_create_packet.account_id	= c_rAccountTable.id;
	player_create_packet.account_index	= pinfo->index;

	sys_log(0, "PlayerCreate: name %s account_id %d, TPlayerCreatePacketSize(%d), Packet->Gold %d",
			pinfo->name,
			pinfo->index,
			sizeof(TPlayerCreatePacket),
			player_create_packet.player_table.gold);

	db_clientdesc->DBPacket(HEADER_GD_PLAYER_CREATE, d->GetHandle(), &player_create_packet, sizeof(TPlayerCreatePacket));
}

void CInputLogin::CharacterDelete(LPDESC d, const char * data)
{
	struct command_player_delete * pinfo = (struct command_player_delete *) data;
	const TAccountTable & c_rAccountTable = d->GetAccountTable();

	if (!c_rAccountTable.id)
	{
		sys_err("PlayerDelete: no login data");
		return;
	}

	sys_log(0, "PlayerDelete: login: %s index: %d, social_id %s", c_rAccountTable.login, pinfo->index, pinfo->private_code);

	if (pinfo->index >= PLAYER_PER_ACCOUNT)
	{
		sys_err("PlayerDelete: index overflow %d, login: %s", pinfo->index, c_rAccountTable.login);
		return;
	}

	if (!c_rAccountTable.players[pinfo->index].dwID)
	{
		sys_err("PlayerDelete: Wrong Social ID index %d, login: %s", pinfo->index, c_rAccountTable.login);
		d->Packet(encode_byte(HEADER_GC_CHARACTER_DELETE_WRONG_SOCIAL_ID), 1);
		return;
	}

	TPlayerDeletePacket	player_delete_packet;

	trim_and_lower(c_rAccountTable.login, player_delete_packet.login, sizeof(player_delete_packet.login));
	player_delete_packet.player_id	= c_rAccountTable.players[pinfo->index].dwID;
	player_delete_packet.account_index	= pinfo->index;
	strlcpy(player_delete_packet.private_code, pinfo->private_code, sizeof(player_delete_packet.private_code));

	db_clientdesc->DBPacket(HEADER_GD_PLAYER_DELETE, d->GetHandle(), &player_delete_packet, sizeof(TPlayerDeletePacket));
}

#pragma pack(1)
typedef struct SPacketGTLogin
{
	BYTE header;
	WORD empty;
	DWORD id;
} TPacketGTLogin;
#pragma pack()

#ifdef __MAP_LEVEL_LOGIN_BUGFIX__ //by vegas
void AnalyzeWarpIsDenied(LPCHARACTER ch)
{
	typedef std::map<DWORD,DWORD> TMapIndexes;
	TMapIndexes m_map;
	m_map[72] = 75;	//--** Grotto of Exile 1st Floor
	m_map[73] = 75; //--** Grotto of Exile 2nd Floor
//	m_map[216] = 75; //--** Devils Catacomb,
	m_map[301] = 90; //--** Cape Dragon Head
	m_map[218] = 90; //--** Dawn Mist Wood
	m_map[220] = 90; //--** Mount Thunder
	m_map[219] = 90; //--** Bay Black Sand
	itertype(m_map) key = m_map.find(ch->GetMapIndex());
	if (key != m_map.end() && ch->GetLevel() < key->second)
	{
		ch->ChatPacket(CHAT_TYPE_NOTICE, "Seviyeniz o haritada olmak icin çok dusuk. Otomatik olarak sehre yonlendirileceksiniz..");
		ch->ChatPacket(CHAT_TYPE_NOTICE, "%d seviyesine sahip oldugunuzda geri donun.", key->second);		
		ch->GoHome();
	}
}
#endif

void CInputLogin::Entergame(LPDESC d, const char * data)
{
	LPCHARACTER ch;

//#ifdef ENABLE_UPDATE_LASTPLAY_REAL_TIME
//
//  int pLevel = 5;
//
//  if (ch->GetLevel() > pLevel) 
//  {	
//    char pUpdateTime[1024];
//    snprintf(pUpdateTime, sizeof(pUpdateTime), "UPDATE player.player SET last_play = NOW() WHERE id = %u", ch->GetPlayerID());
//    std::unique_ptr<SQLMsg> sUpdate(DBManager::instance().DirectQuery(pUpdateTime));
//  }
//#endif

	//LPCHARACTER ch;

	if (!(ch = d->GetCharacter()))
	{
		d->SetPhase(PHASE_CLOSE);
		return;
	}
	PIXEL_POSITION pos = ch->GetXYZ();

	if (!SECTREE_MANAGER::instance().GetMovablePosition(ch->GetMapIndex(), pos.x, pos.y, pos))
	{
		PIXEL_POSITION pos2;
		SECTREE_MANAGER::instance().GetRecallPositionByEmpire(ch->GetMapIndex(), ch->GetEmpire(), pos2);

		sys_err("!GetMovablePosition (name %s %dx%d map %d changed to %dx%d)",
				ch->GetName(),
				pos.x, pos.y,
				ch->GetMapIndex(),
				pos2.x, pos2.y);
		pos = pos2;
	}

	CGuildManager::instance().LoginMember(ch);

	// Ä³¸¯ÅÍ¸¦ ¸Ê¿¡ Ãß°¡
	ch->Show(ch->GetMapIndex(), pos.x, pos.y, pos.z);

	SECTREE_MANAGER::instance().SendNPCPosition(ch);
	ch->ReviveInvisible(5);
#ifdef __MOUNT_SYSTEM__
	ch->CheckMount();
#endif
#ifdef GROUP_MATCH
	CGroupMatchManager::instance().AramayiDurdur(ch->GetPlayerID());
#endif

	d->SetPhase(PHASE_GAME);

	if(ch->GetItemAward_cmd())																		//°ÔÀÓÆäÀÌÁî µé¾î°¡¸é
		quest::CQuestManager::instance().ItemInformer(ch->GetPlayerID(),ch->GetItemAward_vnum());	//questmanager È£Ãâ

	sys_log(0, "ENTERGAME: %s %dx%dx%d %s map_index %d",
			ch->GetName(), ch->GetX(), ch->GetY(), ch->GetZ(), d->GetHostName(), ch->GetMapIndex());

	if (ch->GetHorseLevel() > 0)
	{
		ch->EnterHorse();
	}

	// ÇÃ·¹ÀÌ½Ã°£ ·¹ÄÚµù ½ÃÀÛ
	ch->ResetPlayTime();

	// ÀÚµ¿ ÀúÀå ÀÌº¥Æ® Ãß°¡
	ch->StartSaveEvent();
	ch->StartRecoveryEvent();
	ch->StartCheckSpeedHackEvent();

    ch->SetQuestFlag("item.last_time", get_global_time());

	CPVPManager::instance().Connect(ch);
	CPVPManager::instance().SendList(d);

	MessengerManager::instance().Login(ch->GetName());

	CPartyManager::instance().SetParty(ch);
	CGuildManager::instance().SendGuildWar(ch);

	building::CManager::instance().SendLandList(d, ch->GetMapIndex());

	marriage::CManager::instance().Login(ch);

	TPacketGCTime p;
	p.bHeader = HEADER_GC_TIME;
	p.time = get_global_time();
	d->Packet(&p, sizeof(p));

	TPacketGCChannel p2;
	p2.header = HEADER_GC_CHANNEL;
	p2.channel = g_bChannel;
	d->Packet(&p2, sizeof(p2));

	ch->SendGreetMessage();
#ifdef __MAP_LEVEL_LOGIN_BUGFIX__
	AnalyzeWarpIsDenied(ch);
#endif
#ifdef __ENABLE_PVP_ADVANCED__ // If something is wrong and server is crashed or stopping when you was in duel.
	int isDuel = ch->GetQuestFlag(CHECK_IS_FIGHT);
	if (isDuel)
		ch->SetQuestFlag(CHECK_IS_FIGHT, 0);	
#endif
#ifdef __BIOLOG_SYSTEM__	
	int pGetCompleted[] =
	{
		(ch->GetBiologState("biolog_lv92.completed")), (ch->GetBiologState("biolog_lv94.completed")), (ch->GetBiologState("biolog_lv92.used_window")), (ch->GetBiologState("biolog_lv94.used_window"))
	};
		
	if (pGetCompleted[1] != 1) {
		BiologManager::instance().SendUpdate_Binary(ch); }
	
	if (pGetCompleted[0] == 1 && pGetCompleted[2] < 1) {
		BiologManager::instance().SendWindow_SelectType(ch, 1); }
		
	if (pGetCompleted[1] == 1 && pGetCompleted[3] < 1) {
		BiologManager::instance().SendWindow_SelectType(ch, 2); }
#endif

#ifdef __MAINTENANCE_SYSTEM__
	MaintenanceManager::instance().Send_CheckTable(ch);
#endif

	_send_bonus_info(ch);

#ifdef __SHOPEX_SYSTEM__
	ch->RefreshDragonCoin();
#endif
	for (int i = 0; i <= PREMIUM_MAX_NUM; ++i)
	{
		int remain = ch->GetPremiumRemainSeconds(i);

		if (remain <= 0)
			continue;

		ch->AddAffect(AFFECT_PREMIUM_START + i, POINT_NONE, 0, 0, remain, 0, true);
		sys_log(0, "PREMIUM: %s type %d %dmin", ch->GetName(), i, remain);
	}

	if (LC_IsEurope())
	{
		if (g_bCheckClientVersion)
		{
			int version = atoi(g_stClientVersion.c_str());
			int date = atoi(d->GetClientVersion());

			sys_log(0, "VERSION CHECK %d %d %s %s", version, date, g_stClientVersion.c_str(), d->GetClientVersion());

			if (!d->GetClientVersion())
			{
				d->DelayedDisconnect(10);
			}
			else
			{
				//if (0 != g_stClientVersion.compare(d->GetClientVersion()))
				if (version > date)
				{
					ch->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("Å¬¶óÀÌ¾ðÆ® ¹öÀüÀÌ Æ²·Á ·Î±×¾Æ¿ô µË´Ï´Ù. Á¤»óÀûÀ¸·Î ÆÐÄ¡ ÈÄ Á¢¼ÓÇÏ¼¼¿ä."));
					d->DelayedDisconnect(10);
					LogManager::instance().HackLog("VERSION_CONFLICT", ch);

					sys_log(0, "VERSION : WRONG VERSION USER : account:%s name:%s hostName:%s server_version:%s client_version:%s",
							d->GetAccountTable().login,
							ch->GetName(),
							d->GetHostName(),
							g_stClientVersion.c_str(),
							d->GetClientVersion());
				}
			}
		}
		else
		{
			sys_log(0, "VERSION : NO CHECK");
		}
	}
	else
	{
		sys_log(0, "VERSION : NO LOGIN");
	}

	if (LC_IsEurope() == true)
	{
		if (ch->IsGM() == true)
			ch->ChatPacket(CHAT_TYPE_COMMAND, "ConsoleEnable");
	}

	if (ch->GetMapIndex() >= 10000)
	{
		if (CWarMapManager::instance().IsWarMap(ch->GetMapIndex()))
		{
			CWarMap * pMap = CWarMapManager::instance().Find(ch->GetMapIndex());
#ifdef __IMPROVED_GUILD_WAR__
			if (pMap)
			{
				BYTE bIdx;
				if (ch->GetGuild() && pMap->GetTeamIndex(ch->GetGuild()->GetID(), bIdx))
				{
					// sys_err("[TEST]Current: %d Max: %d", pMap->GetCurrentPlayer(bIdx), pMap->GetMaxPlayer(bIdx));

					if (pMap->GetCurrentPlayer(bIdx) + 1 > pMap->GetMaxPlayer(bIdx)) {
						ch->ChatPacket(CHAT_TYPE_BIG_NOTICE, "oyuncu limiti dolu!");
						ch->GoHome();
						return;
					}
				}
				else
				{
					if (!(IS_SET(pMap->GetWarFlags(bIdx), GWAR_FLAG_CAN_OBSERVER)))
					{
						ch->ChatPacket(CHAT_TYPE_BIG_NOTICE, "savas izleyiciye kapalý!");
						ch->GoHome();
						return;
					}
				}
			}
#endif
			ch->SetWarMap(pMap);
		}
		else if (marriage::WeddingManager::instance().IsWeddingMap(ch->GetMapIndex()))
			ch->SetWeddingMap(marriage::WeddingManager::instance().Find(ch->GetMapIndex()));
		#ifdef __MELEY_LAIR_DUNGEON__
		else if (MeleyLair::CMgr::instance().IsMeleyMap(ch->GetMapIndex()))
			MeleyLair::CMgr::instance().Leave(ch->GetGuild(), ch, true);
		#endif
		else {
			ch->SetDungeon(CDungeonManager::instance().FindByMapIndex(ch->GetMapIndex()));
		}
	}
	else if (CArenaManager::instance().IsArenaMap(ch->GetMapIndex()) == true)
	{
		int memberFlag = CArenaManager::instance().IsMember(ch->GetMapIndex(), ch->GetPlayerID());
		if (memberFlag == MEMBER_OBSERVER)
		{
			ch->SetObserverMode(true);
			ch->SetArenaObserverMode(true);
			if (CArenaManager::instance().RegisterObserverPtr(ch, ch->GetMapIndex(), ch->GetX()/100, ch->GetY()/100))
			{
				sys_log(0, "ARENA : Observer add failed");
			}

			if (ch->IsHorseRiding() == true)
			{
				ch->StopRiding();
				ch->HorseSummon(false);
			}
		}
		else if (memberFlag == MEMBER_DUELIST)
		{
			TPacketGCDuelStart duelStart;
			duelStart.header = HEADER_GC_DUEL_START;
			duelStart.wSize = sizeof(TPacketGCDuelStart);

			ch->GetDesc()->Packet(&duelStart, sizeof(TPacketGCDuelStart));

			if (ch->IsHorseRiding() == true)
			{
				ch->StopRiding();
				ch->HorseSummon(false);
			}

			LPPARTY pParty = ch->GetParty();
			if (pParty != NULL)
			{
				if (pParty->GetMemberCount() == 2)
				{
					CPartyManager::instance().DeleteParty(pParty);
				}
				else
				{
					pParty->Quit(ch->GetPlayerID());
				}
			}
		}
		else if (memberFlag == MEMBER_NO)
		{
			if (ch->GetGMLevel() == GM_PLAYER)
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
		}
		else
		{
			// wtf
		}
	}
	else if (ch->GetMapIndex() == 113)
	{
#ifdef __EXTENDED_OXEVENT_SYSTEM__
		COXEventManager::instance().InitializePacket(ch);
#endif
		if (COXEventManager::instance().Enter(ch) == false)
		{
			// ox ¸Ê ÁøÀÔ Çã°¡°¡ ³ªÁö ¾ÊÀ½. ÇÃ·¹ÀÌ¾î¸é ¸¶À»·Î º¸³»ÀÚ
			if (ch->GetGMLevel() == GM_PLAYER)
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
		}
	}
	else
	{
		if (CWarMapManager::instance().IsWarMap(ch->GetMapIndex()) ||
				marriage::WeddingManager::instance().IsWeddingMap(ch->GetMapIndex()))
		{
			if (!test_server)
				ch->WarpSet(EMPIRE_START_X(ch->GetEmpire()), EMPIRE_START_Y(ch->GetEmpire()));
		}
	}

	if (ch->GetHorseLevel() > 0)
	{
		DWORD pid = ch->GetPlayerID();

		if (pid != 0 && CHorseNameManager::instance().GetHorseName(pid) == NULL)
			db_clientdesc->DBPacket(HEADER_GD_REQ_HORSE_NAME, 0, &pid, sizeof(DWORD));

		ch->SetHorseLevel(ch->GetHorseLevel());
		ch->SkillLevelPacket();
	}

	// Áß¸³¸Ê¿¡ µé¾î°¬À»¶§ ¾È³»ÇÏ±â
	if (g_noticeBattleZone)
	{
		if (FN_is_battle_zone(ch))
		{
			ch->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("ÀÌ ¸Ê¿¡¼± °­Á¦ÀûÀÎ ´ëÀüÀÌ ÀÖÀ»¼ö µµ ÀÖ½À´Ï´Ù."));
			ch->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("ÀÌ Á¶Ç×¿¡ µ¿ÀÇÇÏÁö ¾ÊÀ»½Ã"));
			ch->ChatPacket(CHAT_TYPE_NOTICE, LC_TEXT("º»ÀÎÀÇ ÁÖ¼º ¹× ºÎ¼ºÀ¸·Î µ¹¾Æ°¡½Ã±â ¹Ù¶ø´Ï´Ù."));
		}
	}
#ifdef __PVP_TOURNAMENT_SYSTEM__
	CTournamentPvP::instance().OnLogin(ch);
#endif
}

void CInputLogin::Empire(LPDESC d, const char * c_pData)
{
	const TPacketCGEmpire* p = reinterpret_cast<const TPacketCGEmpire*>(c_pData);

	if (EMPIRE_MAX_NUM <= p->bEmpire)
	{
		d->SetPhase(PHASE_CLOSE);
		return;
	}

	const TAccountTable& r = d->GetAccountTable();

	if (r.bEmpire != 0)
	{
		for (int i = 0; i < PLAYER_PER_ACCOUNT; ++i)
		{
			if (0 != r.players[i].dwID)
			{
				sys_err("EmpireSelectFailed %d", r.players[i].dwID);
				return;
			}
		}
	}

	TEmpireSelectPacket pd;

	pd.dwAccountID = r.id;
	pd.bEmpire = p->bEmpire;

	db_clientdesc->DBPacket(HEADER_GD_EMPIRE_SELECT, d->GetHandle(), &pd, sizeof(pd));
}

int CInputLogin::GuildSymbolUpload(LPDESC d, const char* c_pData, size_t uiBytes)
{
	if (uiBytes < sizeof(TPacketCGGuildSymbolUpload))
		return -1;

	sys_log(0, "GuildSymbolUpload uiBytes %u", uiBytes);

	TPacketCGGuildSymbolUpload* p = (TPacketCGGuildSymbolUpload*) c_pData;

	if (uiBytes < p->size)
		return -1;

	int iSymbolSize = p->size - sizeof(TPacketCGGuildSymbolUpload);

	if (iSymbolSize <= 0 || iSymbolSize > 64 * 1024)
	{
		// 64k º¸´Ù Å« ±æµå ½Éº¼Àº ¿Ã¸±¼ö¾ø´Ù
		// Á¢¼ÓÀ» ²÷°í ¹«½Ã
		d->SetPhase(PHASE_CLOSE);
		return 0;
	}

	// ¶¥À» ¼ÒÀ¯ÇÏÁö ¾ÊÀº ±æµåÀÎ °æ¿ì.
	if (!test_server)
		if (!building::CManager::instance().FindLandByGuild(p->guild_id))
		{
			d->SetPhase(PHASE_CLOSE);
			return 0;
		}

	sys_log(0, "GuildSymbolUpload Do Upload %02X%02X%02X%02X %d", c_pData[7], c_pData[8], c_pData[9], c_pData[10], sizeof(*p));

	CGuildMarkManager::instance().UploadSymbol(p->guild_id, iSymbolSize, (const BYTE*)(c_pData + sizeof(*p)));
	CGuildMarkManager::instance().SaveSymbol(GUILD_SYMBOL_FILENAME);
	return iSymbolSize;
}

void CInputLogin::GuildSymbolCRC(LPDESC d, const char* c_pData)
{
	const TPacketCGSymbolCRC & CGPacket = *((TPacketCGSymbolCRC *) c_pData);

	sys_log(0, "GuildSymbolCRC %u %u %u", CGPacket.guild_id, CGPacket.crc, CGPacket.size);

	const CGuildMarkManager::TGuildSymbol * pkGS = CGuildMarkManager::instance().GetGuildSymbol(CGPacket.guild_id);

	if (!pkGS)
		return;

	sys_log(0, "  Server %u %u", pkGS->crc, pkGS->raw.size());

	if (pkGS->raw.size() != CGPacket.size || pkGS->crc != CGPacket.crc)
	{
		TPacketGCGuildSymbolData GCPacket;

		GCPacket.header = HEADER_GC_SYMBOL_DATA;
		GCPacket.size = sizeof(GCPacket) + pkGS->raw.size();
		GCPacket.guild_id = CGPacket.guild_id;

		d->BufferedPacket(&GCPacket, sizeof(GCPacket));
		d->Packet(&pkGS->raw[0], pkGS->raw.size());

		sys_log(0, "SendGuildSymbolHead %02X%02X%02X%02X Size %d",
				pkGS->raw[0], pkGS->raw[1], pkGS->raw[2], pkGS->raw[3], pkGS->raw.size());
	}
}

void CInputLogin::GuildMarkUpload(LPDESC d, const char* c_pData)
{
	TPacketCGMarkUpload * p = (TPacketCGMarkUpload *) c_pData;
	CGuildManager& rkGuildMgr = CGuildManager::instance();
	CGuild * pkGuild;

	if (!(pkGuild = rkGuildMgr.FindGuild(p->gid)))
	{
		sys_err("MARK_SERVER: GuildMarkUpload: no guild. gid %u", p->gid);
		return;
	}

	if (pkGuild->GetLevel() < guild_mark_min_level)
	{
		sys_log(0, "MARK_SERVER: GuildMarkUpload: level < %u (%u)", guild_mark_min_level, pkGuild->GetLevel());
		return;
	}

	CGuildMarkManager & rkMarkMgr = CGuildMarkManager::instance();

	sys_log(0, "MARK_SERVER: GuildMarkUpload: gid %u", p->gid);

	bool isEmpty = true;

	for (DWORD iPixel = 0; iPixel < SGuildMark::SIZE; ++iPixel)
		if (*((DWORD *) p->image + iPixel) != 0x00000000)
			isEmpty = false;

	if (isEmpty)
		rkMarkMgr.DeleteMark(p->gid);
	else
		rkMarkMgr.SaveMark(p->gid, p->image);
}

void CInputLogin::GuildMarkIDXList(LPDESC d, const char* c_pData)
{
	CGuildMarkManager & rkMarkMgr = CGuildMarkManager::instance();

	DWORD bufSize = sizeof(WORD) * 2 * rkMarkMgr.GetMarkCount();
	char * buf = NULL;

	if (bufSize > 0)
	{
		buf = (char *) malloc(bufSize);
		rkMarkMgr.CopyMarkIdx(buf);
	}

	TPacketGCMarkIDXList p;
	p.header = HEADER_GC_MARK_IDXLIST;
	p.bufSize = sizeof(p) + bufSize;
	p.count = rkMarkMgr.GetMarkCount();

	if (buf)
	{
		d->BufferedPacket(&p, sizeof(p));
		d->LargePacket(buf, bufSize);
		free(buf);
	}
	else
		d->Packet(&p, sizeof(p));

	sys_log(0, "MARK_SERVER: GuildMarkIDXList %d bytes sent.", p.bufSize);
}

void CInputLogin::GuildMarkCRCList(LPDESC d, const char* c_pData)
{
	TPacketCGMarkCRCList * pCG = (TPacketCGMarkCRCList *) c_pData;

	std::map<BYTE, const SGuildMarkBlock *> mapDiffBlocks;
	CGuildMarkManager::instance().GetDiffBlocks(pCG->imgIdx, pCG->crclist, mapDiffBlocks);

	DWORD blockCount = 0;
	TEMP_BUFFER buf(1024 * 1024); // 1M ¹öÆÛ

	for (itertype(mapDiffBlocks) it = mapDiffBlocks.begin(); it != mapDiffBlocks.end(); ++it)
	{
		BYTE posBlock = it->first;
		const SGuildMarkBlock & rkBlock = *it->second;

		buf.write(&posBlock, sizeof(BYTE));
		buf.write(&rkBlock.m_sizeCompBuf, sizeof(DWORD));
		buf.write(rkBlock.m_abCompBuf, rkBlock.m_sizeCompBuf);

		++blockCount;
	}

	TPacketGCMarkBlock pGC;

	pGC.header = HEADER_GC_MARK_BLOCK;
	pGC.imgIdx = pCG->imgIdx;
	pGC.bufSize = buf.size() + sizeof(TPacketGCMarkBlock);
	pGC.count = blockCount;

	sys_log(0, "MARK_SERVER: Sending blocks. (imgIdx %u diff %u size %u)", pCG->imgIdx, mapDiffBlocks.size(), pGC.bufSize);

	if (buf.size() > 0)
	{
		d->BufferedPacket(&pGC, sizeof(TPacketGCMarkBlock));
		d->LargePacket(buf.read_peek(), buf.size());
	}
	else
		d->Packet(&pGC, sizeof(TPacketGCMarkBlock));
}

int CInputLogin::Analyze(LPDESC d, BYTE bHeader, const char * c_pData)
{
	int iExtraLen = 0;

	switch (bHeader)
	{
		case HEADER_CG_PONG:
			Pong(d);
			break;

		case HEADER_CG_TIME_SYNC:
			Handshake(d, c_pData);
			break;

		case HEADER_CG_LOGIN:
			Login(d, c_pData);
			break;

		case HEADER_CG_LOGIN2:
			LoginByKey(d, c_pData);
			break;

		case HEADER_CG_CHARACTER_SELECT:
			CharacterSelect(d, c_pData);
			break;

		case HEADER_CG_CHARACTER_CREATE:
			CharacterCreate(d, c_pData);
			break;

		case HEADER_CG_CHARACTER_DELETE:
			CharacterDelete(d, c_pData);
			break;

		case HEADER_CG_ENTERGAME:
			Entergame(d, c_pData);
			break;

		case HEADER_CG_EMPIRE:
			Empire(d, c_pData);
			break;

		case HEADER_CG_MOVE:
			break;

			///////////////////////////////////////
			// Guild Mark
			/////////////////////////////////////
		case HEADER_CG_MARK_CRCLIST:
			GuildMarkCRCList(d, c_pData);
			break;

		case HEADER_CG_MARK_IDXLIST:
			GuildMarkIDXList(d, c_pData);
			break;

		case HEADER_CG_MARK_UPLOAD:
			GuildMarkUpload(d, c_pData);
			break;

			//////////////////////////////////////
			// Guild Symbol
			/////////////////////////////////////
		case HEADER_CG_GUILD_SYMBOL_UPLOAD:
			if ((iExtraLen = GuildSymbolUpload(d, c_pData, m_iBufferLeft)) < 0)
				return -1;
			break;

		case HEADER_CG_SYMBOL_CRC:
			GuildSymbolCRC(d, c_pData);
			break;
			/////////////////////////////////////

		case HEADER_CG_MARK_LOGIN:
			break;

		case HEADER_CG_HACK:
			break;

		case HEADER_CG_CHANGE_NAME:
			ChangeName(d, c_pData);
			break;

		case HEADER_CG_CLIENT_VERSION:
			Version(d->GetCharacter(), c_pData);
			break;

		case HEADER_CG_CLIENT_VERSION2:
			Version(d->GetCharacter(), c_pData);
			break;

		case HEADER_CG_PetSetName:
			break;

		default:
			//sys_err("login phase does not handle this packet! header %d", bHeader);
			//d->SetPhase(PHASE_CLOSE);
			return (0);
	}

	return (iExtraLen);
}

