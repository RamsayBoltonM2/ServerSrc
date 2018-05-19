
#ifndef __INC_METIN_II_GAME_CONFIG_H__
#define __INC_METIN_II_GAME_CONFIG_H__

enum
{
	ADDRESS_MAX_LEN = 15
};

void config_init(const std::string& st_localeServiceName); // default "" is CONFIG

extern char sql_addr[256];

extern WORD mother_port;
extern WORD p2p_port;

extern char db_addr[ADDRESS_MAX_LEN + 1];
extern WORD db_port;

extern int passes_per_sec;
extern int save_event_second_cycle;
extern int ping_event_second_cycle;
extern int test_server;
extern bool	guild_mark_server;
extern BYTE guild_mark_min_level;
extern bool	distribution_test_server;
extern bool	china_event_server;

extern bool	g_bNoMoreClient;
extern bool	g_bNoRegen;

extern int otomatiktemizlik;
extern int g_iShoutLimitLevel;
extern bool	g_bTrafficProfileOn;		///< true 이면 TrafficProfiler 를 켠다.

extern BYTE	g_bChannel;

extern bool	map_allow_find(int index);
extern void	map_allow_copy(long * pl, int size);
#ifdef __MOUNT_SYSTEM__
extern void Mount_map_unallowed_add(int idx);
extern bool Mount_map_unallowed_find(int idx);
#endif
#ifdef __PET_SYSTEM_LIKE_RUBINUM__
extern void Pet_map_unallowed_add(int idx);
extern bool Pet_map_unallowed_find(int idx);
#endif
extern bool	no_wander;

extern int		g_iUserLimit;
extern time_t	g_global_time;

const char *	get_table_postfix();

extern std::string	g_stHostname;
extern std::string	g_stLocale;
extern std::string	g_stLocaleFilename;

extern char		g_szPublicIP[16];
extern char		g_szInternalIP[16];

extern int (*is_twobyte) (const char * str);
extern int (*check_name) (const char * str);

extern bool		g_bSkillDisable;

extern int		g_iFullUserCount;
extern int		g_iBusyUserCount;
extern void		LoadStateUserCount();

extern bool	g_bEmpireWhisper;

extern BYTE	g_bAuthServer;
extern BYTE	g_bBilling;

extern BYTE	PK_PROTECT_LEVEL;

extern void	LoadValidCRCList();
extern bool	IsValidProcessCRC(DWORD dwCRC);
extern bool	IsValidFileCRC(DWORD dwCRC);

extern std::string	g_stAuthMasterIP;
extern WORD		g_wAuthMasterPort;

extern std::string	g_stClientVersion;
extern bool		g_bCheckClientVersion;
extern void		CheckClientVersion();

extern std::string	g_stQuestDir;
//extern std::string	g_stQuestObjectDir;
extern std::set<std::string> g_setQuestObjectDir;


extern std::vector<std::string>	g_stAdminPageIP;
extern std::string	g_stAdminPagePassword;

extern int	SPEEDHACK_LIMIT_COUNT;
extern int 	SPEEDHACK_LIMIT_BONUS;

extern int g_iSyncHackLimitCount;

extern int g_server_id;
extern std::string g_strWebMallURL;

extern int VIEW_RANGE;
extern int VIEW_BONUS_RANGE;

extern bool g_bCheckMultiHack;
extern bool g_protectNormalPlayer;      // 범법자가 "평화모드" 인 일반유저를 공격하지 못함
extern bool g_noticeBattleZone;         // 중립지대에 입장하면 안내메세지를 알려줌

extern DWORD g_GoldDropTimeLimitValue;

extern int gPlayerMaxLevel;
extern int gSpecialShout;
extern int MasterGhostChat;
extern int MasterColorEmpire;
extern int MasterLevelChat;
extern int PlayerColorEmpire;
extern int PlayerLevelChat;

extern bool v_syslog_notice;
extern bool v_syschat_notice;
extern bool v_ban_waithack;

extern int g_iOfflineShopSaveTime;

extern bool g_BlockCharCreation;
#ifdef __ATTR_TRANSFER_SYSTEM__
extern int gAttrTransferLimit;
#endif

extern bool g_bEnableNewKingdomForWolfman;

#endif /* __INC_METIN_II_GAME_CONFIG_H__ */

