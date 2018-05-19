#ifndef __INC_METIN_II_LENGTH_H__
#define __INC_METIN_II_LENGTH_H__

#include "service.h"

#define WORD_MAX 0xffff
enum EMisc
{
#ifdef __OFFLINESHOP_SYSTEM__
	OFFLINE_SHOP_HOST_ITEM_MAX_NUM 	= 45,
	OFFLINE_SHOP_MAX_COUNT			= 100000,	// Channel
	OFFLINE_SHOP_SAVE_TIME			= 5,	// Minitue
	OFFLINE_SHOP_PRICE_GOLD_BAR_1	= 80003,
	OFFLINE_SHOP_PRICE_GOLD_BAR_2	= 80004,
	OFFLINE_SHOP_PRICE_GOLD_BAR_3	= 80005,
	OFFLINE_SHOP_PRICE_SOUL_STONE	= 50513,
	OFFLINE_SHOP_PRICE_DRAGON_SCALE	= 71123,
	OFFLINE_SHOP_PRICE_DRAGON_CLAW	= 71129,
	OFFLINE_SHOP_DEFAULT_TIME		= 43200*2,
	OFFLINE_SHOP_STONE_TIME			= 300,
	OFFLINE_SHOP_BOSS_TIME			= 900,
#endif
	
	MAX_HOST_LENGTH			= 15,
	AUTOBACKUP_SAVE_TIME	= 21600,   //*  auto yedek alýr//
	AUTOBACKUPDEL_SAVE_TIME	= 900,  //*  auto alýnan yedekleri siler 48 saatte bir// 3600= 1saat 3600*48= 172800 saniye
	AUTOLOGCLEAR_SAVE_TIME	= 10800,   //*  auto log siler //
	IP_ADDRESS_LENGTH		= 15,
	LOGIN_MAX_LEN			= 30,
	PASSWD_MAX_LEN			= 16,
#ifdef ENABLE_PLAYER_PER_ACCOUNT5
	PLAYER_PER_ACCOUNT		= 5,
#else
	PLAYER_PER_ACCOUNT		= 4,
#endif
	ACCOUNT_STATUS_MAX_LEN	= 8,
	CHARACTER_NAME_MAX_LEN	= 24,
	SHOP_SIGN_MAX_LEN		= 32,
	INVENTORY_MAX_NUM		= 180,
	ABILITY_MAX_NUM			= 50,
	EMPIRE_MAX_NUM			= 4,
	BANWORD_MAX_LEN			= 24,
	SMS_MAX_LEN				= 80,
	MOBILE_MAX_LEN			= 32,
	SOCIAL_ID_MAX_LEN		= 18,

	GUILD_NAME_MAX_LEN		= 12,

	SHOP_HOST_ITEM_MAX_NUM	= 100,	/* È£½ºÆ®ÀÇ ÃÖ´ë ¾ÆÀÌÅÛ °³¼ö */
	SHOP_GUEST_ITEM_MAX_NUM = 18,	/* °Ô½ºÆ®ÀÇ ÃÖ´ë ¾ÆÀÌÅÛ °³¼ö */

	SHOP_PRICELIST_MAX_NUM	= 100,	///< °³ÀÎ»óÁ¡ °¡°ÝÁ¤º¸ ¸®½ºÆ®¿¡¼­ À¯ÁöÇÒ °¡°ÝÁ¤º¸ÀÇ ÃÖ´ë °¹¼ö

#ifdef __INGAME_SUPPORT_TICKET_SYSTEM__
	CHAT_MAX_LEN			= 700,
#else
	CHAT_MAX_LEN			= 512,
#endif

	QUICKSLOT_MAX_NUM		= 36,
#ifdef __SECONDARY_SLOT_SYSTEM__
	NEWQUICKSLOT_MAX_NUM	= 36,
#endif
	JOURNAL_MAX_NUM			= 2,

	QUERY_MAX_LEN			= 8192,

	FILE_MAX_LEN			= 128,

	PLAYER_EXP_TABLE_MAX	= 250,
	PLAYER_MAX_LEVEL_CONST	= 250,

	GUILD_MAX_LEVEL			= 20,
	MOB_MAX_LEVEL			= 100,

	ATTRIBUTE_MAX_VALUE		= 20,
	CHARACTER_PATH_MAX_NUM	= 64,
	SKILL_MAX_NUM			= 255,
	SKILLBOOK_DELAY_MIN		= 64800,
	SKILLBOOK_DELAY_MAX		= 108000, 
	SKILL_MAX_LEVEL			= 40,

	APPLY_NAME_MAX_LEN		= 32,
	EVENT_FLAG_NAME_MAX_LEN = 32,

	MOB_SKILL_MAX_NUM		= 5,

    POINT_MAX_NUM = 255,
	DRAGON_SOUL_BOX_SIZE = 32,
	DRAGON_SOUL_BOX_COLUMN_NUM = 8,
	DRAGON_SOUL_BOX_ROW_NUM = DRAGON_SOUL_BOX_SIZE / DRAGON_SOUL_BOX_COLUMN_NUM,
	DRAGON_SOUL_REFINE_GRID_SIZE = 15,
	MAX_AMOUNT_OF_MALL_BONUS	= 20,

	WEAR_MAX_NUM				= 32,
#ifdef __CHEQUE_SYSTEM__
	CHEQUE_MAX = 1000,
#endif

	//LIMIT_GOLD
	GOLD_MAX = 2000000000,
#ifdef __GAYA__
	GAYA_MAX = 999999,
#endif

	//END_LIMIT_GOLD

	SHOP_TAB_NAME_MAX = 32,
	SHOP_TAB_COUNT_MAX = 3,

#ifdef __SPECIAL_INVENTORY__
	SKILL_BOOK_INVENTORY_MAX_NUM	= 135,
	UPGRADE_ITEMS_INVENTORY_MAX_NUM	= 135,
	STONE_INVENTORY_MAX_NUM	= 135,
#endif

	BELT_INVENTORY_SLOT_WIDTH = 4,
	BELT_INVENTORY_SLOT_HEIGHT= 4,

	BELT_INVENTORY_SLOT_COUNT = BELT_INVENTORY_SLOT_WIDTH * BELT_INVENTORY_SLOT_HEIGHT,


/**
	 **** ÇöÀç±îÁö ÇÒ´ç µÈ ¾ÆÀÌÅÛ ¿µ¿ª Á¤¸® (DB»ó Item Position) ****
	+------------------------------------------------------+ 0
	| Ä³¸¯ÅÍ ±âº» ÀÎº¥Åä¸® (45Ä­ * 2ÆäÀÌÁö) 90Ä­           |
	+------------------------------------------------------+ 90 = INVENTORY_MAX_NUM(90)
	| Ä³¸¯ÅÍ Àåºñ Ã¢ (Âø¿ëÁßÀÎ ¾ÆÀÌÅÛ) 32Ä­                |
	+------------------------------------------------------+ 122 = INVENTORY_MAX_NUM(90) + WEAR_MAX_NUM(32)
	| ¿ëÈ¥¼® Àåºñ Ã¢ (Âø¿ëÁßÀÎ ¿ëÈ¥¼®) 12Ä­                |
	+------------------------------------------------------+ 134 = 122 + DS_SLOT_MAX(6) * DRAGON_SOUL_DECK_MAX_NUM(2)
	| ¿ëÈ¥¼® Àåºñ Ã¢ ¿¹¾à (¾ÆÁ÷ ¹Ì»ç¿ë) 18Ä­               |
	+------------------------------------------------------+ 152 = 134 + DS_SLOT_MAX(6) * DRAGON_SOUL_DECK_RESERVED_MAX_NUM(3)
	| º§Æ® ÀÎº¥Åä¸® (º§Æ® Âø¿ë½Ã¿¡¸¸ º§Æ® ·¹º§¿¡ µû¶ó È°¼º)|
	+------------------------------------------------------+ 168 = 152 + BELT_INVENTORY_SLOT_COUNT(16) = INVENTORY_AND_EQUIP_CELL_MAX
	| ¹Ì»ç¿ë                                               |
	+------------------------------------------------------+ ??
*/
};

#ifdef __TRANSMUTATION_SYSTEM__
enum EChangeLookInfo
{
	CL_WINDOW_MAX_MATERIALS = 3,
	CL_CLEAN_ATTR_VALUE0 = 8,
	CL_TRANSMUTATION_PRICE = 50000000,
	CL_SCROLL_VNUM = 72326,
};
#endif

enum EWearPositions
{
	WEAR_BODY,		// 0
	WEAR_HEAD,		// 1
	WEAR_FOOTS,		// 2
	WEAR_WRIST,		// 3
	WEAR_WEAPON,	// 4
	WEAR_NECK,		// 5
	WEAR_EAR,		// 6
	WEAR_UNIQUE1,	// 7
	WEAR_UNIQUE2,	// 8
	WEAR_ARROW,		// 9
	WEAR_SHIELD,	// 10
    WEAR_ABILITY1,  // 11
    WEAR_ABILITY2,  // 12
    WEAR_ABILITY3,  // 13
    WEAR_ABILITY4,  // 14
    WEAR_ABILITY5,  // 15
    WEAR_ABILITY6,  // 16
    WEAR_ABILITY7,  // 17
    WEAR_ABILITY8,  // 18
	WEAR_COSTUME_BODY,	// 19
	WEAR_COSTUME_HAIR,	// 20
	WEAR_COSTUME_ACCE,
#ifdef __WEAPON_COSTUME_SYSTEM__
	WEAR_COSTUME_WEAPON,
#endif
#ifdef __MOUNT_SYSTEM__
	WEAR_COSTUME_MOUNT,
#endif
	WEAR_RING1,			// 21	: ½Å±Ô ¹ÝÁö½½·Ô1 (¿ÞÂÊ)
	WEAR_RING2,			// 22	: ½Å±Ô ¹ÝÁö½½·Ô2 (¿À¸¥ÂÊ)
	WEAR_BELT,			// 23	: ½Å±Ô º§Æ®½½·Ô
#ifdef __PET_SYSTEM_LIKE_RUBINUM__
	WEAR_PET,			// 26
#endif

	WEAR_MAX = 32	// 
};

enum EDragonSoulDeckType
{
	DRAGON_SOUL_DECK_0,
	DRAGON_SOUL_DECK_1,
	DRAGON_SOUL_DECK_MAX_NUM = 2,

	DRAGON_SOUL_DECK_RESERVED_MAX_NUM = 3,	// NOTE: Áß¿ä! ¾ÆÁ÷ »ç¿ëÁßÀÌÁø ¾ÊÁö¸¸, 3ÆäÀÌÁö ºÐ·®À» ¿¹¾à ÇØ µÒ. DS DECKÀ» ´Ã¸± °æ¿ì ¹Ýµå½Ã ±× ¼ö¸¸Å­ RESERVED¿¡¼­ Â÷°¨ÇØ¾ß ÇÔ!
};

enum ESex
{
	SEX_MALE,
	SEX_FEMALE
};

enum EDirection
{
	DIR_NORTH,
	DIR_NORTHEAST,
	DIR_EAST,
	DIR_SOUTHEAST,
	DIR_SOUTH,
	DIR_SOUTHWEST,
	DIR_WEST,
	DIR_NORTHWEST,
	DIR_MAX_NUM
};

#define ABILITY_MAX_LEVEL	10  /* ±â¼ú ÃÖ´ë ·¹º§ */

enum EAbilityDifficulty
{
	DIFFICULTY_EASY,
	DIFFICULTY_NORMAL,
	DIFFICULTY_HARD,
	DIFFICULTY_VERY_HARD,
	DIFFICULTY_NUM_TYPES
};

enum EAbilityCategory
{
	CATEGORY_PHYSICAL,	/* ½ÅÃ¼Àû ¾îºô¸®Æ¼ */
	CATEGORY_MENTAL,	/* Á¤½ÅÀû ¾îºô¸®Æ¼ */
	CATEGORY_ATTRIBUTE,	/* ´É·Â ¾îºô¸®Æ¼ */
	CATEGORY_NUM_TYPES
};

enum EJobs
{
	JOB_WARRIOR,
	JOB_ASSASSIN,
	JOB_SURA,
	JOB_SHAMAN,
#ifdef __YMIR_UPDATE_WOLFMAN__
	JOB_WOLFMAN,		// ¼öÀÎÁ·. °³¹ßÄÚµå¸íÀÌ WOLFMAN. (±âÈ¹ÀÚ°¡ Á¤Çß¾¸. ³ªÁß¿¡ WOLF GIRL »ý°Üµµ ³ªÇÑÅ× ¸Ó¶óÇÏÁö¸¶¼À¤Ð¤Ð)
#endif
	JOB_MAX_NUM
};

#ifdef ENABLE_TRANSFORM_PLAYER_OX
enum ETypeInformationsEventOx
{ //@functionMe492
	OX_MOBNAME_PLAYER = 101, // MobVnum
	OX_MOBNAME_MODERATOR = 102, // MobVnum
	OX_MOBNAME_VIP = 103, // MobVnum
	
	OX_MAPINDEX = 113, // MapIndex 
	OX_TIME_AFFECT = 60*60*24*7, // Is not important time because when you will finish event ox, will be removed to 0.
};
#endif

enum ESkillGroups
{
	SKILL_GROUP_MAX_NUM = 2,
};

enum ERaceFlags
{
	RACE_FLAG_ANIMAL	= (1 << 0),
	RACE_FLAG_UNDEAD	= (1 << 1),
	RACE_FLAG_DEVIL		= (1 << 2),
	RACE_FLAG_HUMAN		= (1 << 3),
	RACE_FLAG_ORC		= (1 << 4),
	RACE_FLAG_MILGYO	= (1 << 5),
	RACE_FLAG_INSECT	= (1 << 6),
	RACE_FLAG_FIRE		= (1 << 7),
	RACE_FLAG_ICE		= (1 << 8),
	RACE_FLAG_DESERT	= (1 << 9),
	RACE_FLAG_TREE		= (1 << 10),
	RACE_FLAG_ATT_ELEC	= (1 << 11),
	RACE_FLAG_ATT_FIRE	= (1 << 12),
	RACE_FLAG_ATT_ICE	= (1 << 13),
	RACE_FLAG_ATT_WIND	= (1 << 14),
	RACE_FLAG_ATT_EARTH	= (1 << 15),
	RACE_FLAG_ATT_DARK	= (1 << 16),
};

enum EGuildWarFlags
{
	GWAR_FLAG_DUMMY			= (1 << 0),
	GWAR_FLAG_CAN_OBSERVER	= (1 << 1),
};

enum ELoads
{
	LOAD_NONE,
	LOAD_LIGHT,
	LOAD_NORMAL,
	LOAD_HEAVY,
	LOAD_MASSIVE
};

enum
{
	QUICKSLOT_TYPE_NONE,
	QUICKSLOT_TYPE_ITEM,
	QUICKSLOT_TYPE_SKILL,
	QUICKSLOT_TYPE_COMMAND,
	QUICKSLOT_TYPE_MAX_NUM,
};

enum EParts
{
	PART_MAIN,
	PART_WEAPON,
	PART_HEAD,
	PART_HAIR,
	PART_ACCE,
	PART_MAX_NUM,
};

enum EChatType
{
	CHAT_TYPE_TALKING,	/* ±×³É Ã¤ÆÃ */
	CHAT_TYPE_INFO,	/* Á¤º¸ (¾ÆÀÌÅÛÀ» Áý¾ú´Ù, °æÇèÄ¡¸¦ ¾ò¾ú´Ù. µî) */
	CHAT_TYPE_NOTICE,	/* °øÁö»çÇ× */
	CHAT_TYPE_PARTY,	/* ÆÄÆ¼¸» */
	CHAT_TYPE_GUILD,	/* ±æµå¸» */
	CHAT_TYPE_COMMAND,	/* ÀÏ¹Ý ¸í·É */
	CHAT_TYPE_SHOUT,	/* ¿ÜÄ¡±â */
	CHAT_TYPE_WHISPER,
	CHAT_TYPE_BIG_NOTICE,
	CHAT_TYPE_MONARCH_NOTICE,
#ifdef __ENABLE_DICE_SYSTEM__
	CHAT_TYPE_DICE_INFO, //11
#endif
	CHAT_TYPE_MAX_NUM
};

enum EWhisperType
{
	WHISPER_TYPE_NORMAL		= 0,
	WHISPER_TYPE_NOT_EXIST		= 1,
	WHISPER_TYPE_TARGET_BLOCKED	= 2,
	WHISPER_TYPE_SENDER_BLOCKED	= 3,
	WHISPER_TYPE_ERROR		= 4,
	WHISPER_TYPE_GM			= 5,
	WHISPER_TYPE_SYSTEM		= 0xFF
};

enum ECharacterPosition
{
	POSITION_GENERAL,
	POSITION_BATTLE,
	POSITION_DYING,
	POSITION_SITTING_CHAIR,
	POSITION_SITTING_GROUND,
	POSITION_INTRO,
	POSITION_MAX_NUM
};

enum EGMLevels
{
	GM_PLAYER,
#ifdef ENABLE_FUNCTION_VEGAS_VIP
	GM_VIP,
#endif
	GM_LOW_WIZARD,
	GM_WIZARD,
	GM_HIGH_WIZARD,
	GM_GOD,
	GM_IMPLEMENTOR
};

enum EMobRank
{
	MOB_RANK_PAWN,
	MOB_RANK_S_PAWN,
	MOB_RANK_KNIGHT,
	MOB_RANK_S_KNIGHT,
	MOB_RANK_BOSS,
	MOB_RANK_KING,
	MOB_RANK_MAX_NUM
};

enum ECharType
{
	CHAR_TYPE_MONSTER,
	CHAR_TYPE_NPC,
	CHAR_TYPE_STONE,
	CHAR_TYPE_WARP,
	CHAR_TYPE_DOOR,
	CHAR_TYPE_BUILDING,
	CHAR_TYPE_PC,
	CHAR_TYPE_POLYMORPH_PC,
	CHAR_TYPE_HORSE,
	CHAR_TYPE_GOTO
};

enum EBattleType
{
	BATTLE_TYPE_MELEE,
	BATTLE_TYPE_RANGE,
	BATTLE_TYPE_MAGIC,
	BATTLE_TYPE_SPECIAL,
	BATTLE_TYPE_POWER,
	BATTLE_TYPE_TANKER,
	BATTLE_TYPE_SUPER_POWER,
	BATTLE_TYPE_SUPER_TANKER,
	BATTLE_TYPE_MAX_NUM
};

enum EApplyTypes
{
	APPLY_NONE,			// 0
	APPLY_MAX_HP,		// 1
	APPLY_MAX_SP,		// 2
	APPLY_CON,			// 3
	APPLY_INT,			// 4
	APPLY_STR,			// 5
	APPLY_DEX,			// 6
	APPLY_ATT_SPEED,	// 7
	APPLY_MOV_SPEED,	// 8
	APPLY_CAST_SPEED,	// 9
	APPLY_HP_REGEN,		// 10
	APPLY_SP_REGEN,		// 11
	APPLY_POISON_PCT,	// 12
	APPLY_STUN_PCT,		// 13
	APPLY_SLOW_PCT,		// 14
	APPLY_CRITICAL_PCT,		// 15
	APPLY_PENETRATE_PCT,	// 16
	APPLY_ATTBONUS_HUMAN,	// 17
	APPLY_ATTBONUS_ANIMAL,	// 18
	APPLY_ATTBONUS_ORC,		// 19
	APPLY_ATTBONUS_MILGYO,	// 20
	APPLY_ATTBONUS_UNDEAD,	// 21
	APPLY_ATTBONUS_DEVIL,	// 22
	APPLY_STEAL_HP,			// 23
	APPLY_STEAL_SP,			// 24
	APPLY_MANA_BURN_PCT,	// 25
	APPLY_DAMAGE_SP_RECOVER,	// 26
	APPLY_BLOCK,			// 27
	APPLY_DODGE,			// 28
	APPLY_RESIST_SWORD,		// 29
	APPLY_RESIST_TWOHAND,	// 30
	APPLY_RESIST_DAGGER,	// 31
	APPLY_RESIST_BELL,		// 32
	APPLY_RESIST_FAN,		// 33
	APPLY_RESIST_BOW,		// 34
	APPLY_RESIST_FIRE,		// 35
	APPLY_RESIST_ELEC,		// 36
	APPLY_RESIST_MAGIC,		// 37
	APPLY_RESIST_WIND,		// 38
	APPLY_REFLECT_MELEE,	// 39
	APPLY_REFLECT_CURSE,	// 40
	APPLY_POISON_REDUCE,	// 41
	APPLY_KILL_SP_RECOVER,	// 42
	APPLY_EXP_DOUBLE_BONUS,	// 43
	APPLY_GOLD_DOUBLE_BONUS,	// 44
	APPLY_ITEM_DROP_BONUS,	// 45
	APPLY_POTION_BONUS,		// 46
	APPLY_KILL_HP_RECOVER,	// 47
	APPLY_IMMUNE_STUN,		// 48
	APPLY_IMMUNE_SLOW,		// 49
	APPLY_IMMUNE_FALL,		// 50
	APPLY_SKILL,			// 51
	APPLY_BOW_DISTANCE,		// 52
	APPLY_ATT_GRADE_BONUS,	// 53
	APPLY_DEF_GRADE_BONUS,	// 54
	APPLY_MAGIC_ATT_GRADE,	// 55
	APPLY_MAGIC_DEF_GRADE,	// 56
	APPLY_CURSE_PCT,		// 57
	APPLY_MAX_STAMINA,		// 58
	APPLY_ATTBONUS_WARRIOR,	// 59
	APPLY_ATTBONUS_ASSASSIN,	// 60
	APPLY_ATTBONUS_SURA,	// 61
	APPLY_ATTBONUS_SHAMAN,	// 62
	APPLY_ATTBONUS_MONSTER,	// 63
	APPLY_MALL_ATTBONUS,			// 64 °ø°Ý·Â +x%
	APPLY_MALL_DEFBONUS,			// 65 ¹æ¾î·Â +x%
	APPLY_MALL_EXPBONUS,			// 66 °æÇèÄ¡ +x%
	APPLY_MALL_ITEMBONUS,			// 67 ¾ÆÀÌÅÛ µå·ÓÀ² x/10¹è
	APPLY_MALL_GOLDBONUS,			// 68 µ· µå·ÓÀ² x/10¹è
	APPLY_MAX_HP_PCT,				// 69 ÃÖ´ë »ý¸í·Â +x%
	APPLY_MAX_SP_PCT,				// 70 ÃÖ´ë Á¤½Å·Â +x%
	APPLY_SKILL_DAMAGE_BONUS,		// 71 ½ºÅ³ µ¥¹ÌÁö * (100+x)%
	APPLY_NORMAL_HIT_DAMAGE_BONUS,	// 72 ÆòÅ¸ µ¥¹ÌÁö * (100+x)%
	APPLY_SKILL_DEFEND_BONUS,		// 73 ½ºÅ³ µ¥¹ÌÁö ¹æ¾î * (100-x)%
	APPLY_NORMAL_HIT_DEFEND_BONUS,	// 74 ÆòÅ¸ µ¥¹ÌÁö ¹æ¾î * (100-x)%
	APPLY_PC_BANG_EXP_BONUS,		// 75 PC¹æ ¾ÆÀÌÅÛ EXP º¸³Ê½º
	APPLY_PC_BANG_DROP_BONUS,		// 76 PC¹æ ¾ÆÀÌÅÛ µå·ÓÀ² º¸³Ê½º

	APPLY_EXTRACT_HP_PCT,			// 77 »ç¿ë½Ã HP ¼Ò¸ð

	APPLY_RESIST_WARRIOR,			// 78 ¹«»ç¿¡°Ô ÀúÇ×
	APPLY_RESIST_ASSASSIN,			// 79 ÀÚ°´¿¡°Ô ÀúÇ×
	APPLY_RESIST_SURA,				// 80 ¼ö¶ó¿¡°Ô ÀúÇ×
	APPLY_RESIST_SHAMAN,			// 81 ¹«´ç¿¡°Ô ÀúÇ×
	APPLY_ENERGY,					// 82 ±â·Â
	APPLY_DEF_GRADE,				// 83 ¹æ¾î·Â. DEF_GRADE_BONUS´Â Å¬¶ó¿¡¼­ µÎ¹è·Î º¸¿©Áö´Â ÀÇµµµÈ ¹ö±×(...)°¡ ÀÖ´Ù.
	APPLY_COSTUME_ATTR_BONUS,		// 84 ÄÚ½ºÆ¬ ¾ÆÀÌÅÛ¿¡ ºÙÀº ¼Ó¼ºÄ¡ º¸³Ê½º
	APPLY_MAGIC_ATTBONUS_PER,		// 85 ¸¶¹ý °ø°Ý·Â +x%
	APPLY_MELEE_MAGIC_ATTBONUS_PER,			// 86 ¸¶¹ý + ¹Ð¸® °ø°Ý·Â +x%

	APPLY_RESIST_ICE,		// 87 ³Ã±â ÀúÇ×
	APPLY_RESIST_EARTH,		// 88 ´ëÁö ÀúÇ×
	APPLY_RESIST_DARK,		// 89 ¾îµÒ ÀúÇ×

	APPLY_ANTI_CRITICAL_PCT,	//90 Å©¸®Æ¼ÄÃ ÀúÇ×
	APPLY_ANTI_PENETRATE_PCT,	//91 °üÅëÅ¸°Ý ÀúÇ×

#ifdef __YMIR_UPDATE_WOLFMAN__
	APPLY_BLEEDING_REDUCE,		//92
	APPLY_BLEEDING_PCT,			//93
	APPLY_ATTBONUS_WOLFMAN,		//94 ¼öÀÎÁ·¿¡°Ô °­ÇÔ
	APPLY_RESIST_WOLFMAN,		//95 ¼öÀÎÁ·¿¡°Ô ÀúÇ×
	APPLY_RESIST_CLAW,			//96 CLAW¹«±â¿¡ ÀúÇ×
#endif

#ifdef __MAGIC_STONE_SYSTEM__
	APPLY_RESIST_MAGIC_REDUCTION,	//97
#endif

	MAX_APPLY_NUM,              //
};

enum EOnClickEvents
{
	ON_CLICK_NONE,
	ON_CLICK_SHOP,
	ON_CLICK_TALK,
	ON_CLICK_MAX_NUM
};

enum EOnIdleEvents
{
	ON_IDLE_NONE,
	ON_IDLE_GENERAL,
	ON_IDLE_MAX_NUM
};

#ifdef ENABLE_HIDE_NAME_PLAYER
enum ETypeEventOX
{
	EVENT_OX_MAP = 113,
	EVENT_OX_FLAG = 0,
};
#endif

enum EWindows
{
	RESERVED_WINDOW,
	INVENTORY,
	EQUIPMENT,
	SAFEBOX,
	MALL,
	DRAGON_SOUL_INVENTORY,
	BELT_INVENTORY,
#ifdef __SPECIAL_INVENTORY__
	SKILL_BOOK_INVENTORY,
	UPGRADE_ITEMS_INVENTORY,
	STONE_INVENTORY,
#endif
	GROUND
};

enum EMobSizes
{
	MOBSIZE_RESERVED,
	MOBSIZE_SMALL,
	MOBSIZE_MEDIUM,
	MOBSIZE_BIG
};

enum EAIFlags
{
	AIFLAG_AGGRESSIVE	= (1 << 0),
	AIFLAG_NOMOVE	= (1 << 1),
	AIFLAG_COWARD	= (1 << 2),
	AIFLAG_NOATTACKSHINSU	= (1 << 3),
	AIFLAG_NOATTACKJINNO	= (1 << 4),
	AIFLAG_NOATTACKCHUNJO	= (1 << 5),
	AIFLAG_ATTACKMOB = (1 << 6 ),
	AIFLAG_BERSERK	= (1 << 7),
	AIFLAG_STONESKIN	= (1 << 8),
	AIFLAG_GODSPEED	= (1 << 9),
	AIFLAG_DEATHBLOW	= (1 << 10),
	AIFLAG_REVIVE		= (1 << 11),
};

enum EMobStatType
{
	MOB_STATTYPE_POWER,
	MOB_STATTYPE_TANKER,
	MOB_STATTYPE_SUPER_POWER,
	MOB_STATTYPE_SUPER_TANKER,
	MOB_STATTYPE_RANGE,
	MOB_STATTYPE_MAGIC,
	MOB_STATTYPE_MAX_NUM
};

enum EImmuneFlags
{
	IMMUNE_STUN		= (1 << 0),
	IMMUNE_SLOW		= (1 << 1),
	IMMUNE_FALL		= (1 << 2),
	IMMUNE_CURSE	= (1 << 3),
	IMMUNE_POISON	= (1 << 4),
	IMMUNE_TERROR	= (1 << 5),
	IMMUNE_REFLECT	= (1 << 6),
};

enum EMobEnchants
{
	MOB_ENCHANT_CURSE,
	MOB_ENCHANT_SLOW,
	MOB_ENCHANT_POISON,
	MOB_ENCHANT_STUN,
	MOB_ENCHANT_CRITICAL,
	MOB_ENCHANT_PENETRATE,
	MOB_ENCHANTS_MAX_NUM
};

enum EMobResists
{
	MOB_RESIST_SWORD,
	MOB_RESIST_TWOHAND,
	MOB_RESIST_DAGGER,
	MOB_RESIST_BELL,
	MOB_RESIST_FAN,
	MOB_RESIST_BOW,
	MOB_RESIST_FIRE,
	MOB_RESIST_ELECT,
	MOB_RESIST_MAGIC,
	MOB_RESIST_WIND,
	MOB_RESIST_POISON,
	MOB_RESIST_CLAW,
	MOB_RESISTS_MAX_NUM
};

enum
{
	SKILL_ATTR_TYPE_NORMAL = 1,
	SKILL_ATTR_TYPE_MELEE,
	SKILL_ATTR_TYPE_RANGE,
	SKILL_ATTR_TYPE_MAGIC
		/*
		   SKILL_ATTR_TYPE_FIRE,
		   SKILL_ATTR_TYPE_ICE,
		   SKILL_ATTR_TYPE_ELEC,
		   SKILL_ATTR_TYPE_DARK,
		   */
};

enum
{
	SKILL_NORMAL,
	SKILL_MASTER,
	SKILL_GRAND_MASTER,
	SKILL_PERFECT_MASTER,
};

enum EGuildWarType
{
	GUILD_WAR_TYPE_FIELD,
	GUILD_WAR_TYPE_BATTLE,
	GUILD_WAR_TYPE_FLAG,
	GUILD_WAR_TYPE_MAX_NUM
};

enum EGuildWarState
{
	GUILD_WAR_NONE,
	GUILD_WAR_SEND_DECLARE,
	GUILD_WAR_REFUSE,
	GUILD_WAR_RECV_DECLARE,
	GUILD_WAR_WAIT_START,
	GUILD_WAR_CANCEL,
	GUILD_WAR_ON_WAR,
	GUILD_WAR_END,
	GUILD_WAR_OVER,
	GUILD_WAR_RESERVE,

	GUILD_WAR_DURATION = 30*60, // 1½Ã°£
	GUILD_WAR_WIN_POINT = 1000,
	GUILD_WAR_LADDER_HALF_PENALTY_TIME = 12*60*60,
};

enum EAttributeSet
{
	ATTRIBUTE_SET_WEAPON,
	ATTRIBUTE_SET_BODY,
	ATTRIBUTE_SET_WRIST,
	ATTRIBUTE_SET_FOOTS,
	ATTRIBUTE_SET_NECK,
	ATTRIBUTE_SET_HEAD,
	ATTRIBUTE_SET_SHIELD,
	ATTRIBUTE_SET_EAR,
	ATTRIBUTE_SET_MAX_NUM
};

enum EPrivType
{
	PRIV_NONE,
	PRIV_ITEM_DROP,
	PRIV_GOLD_DROP,
	PRIV_GOLD10_DROP,
	PRIV_EXP_PCT,
	MAX_PRIV_NUM,
};

enum EMoneyLogType
{
	MONEY_LOG_RESERVED,
	MONEY_LOG_MONSTER,
	MONEY_LOG_SHOP,
	MONEY_LOG_REFINE,
	MONEY_LOG_QUEST,
	MONEY_LOG_GUILD,
	MONEY_LOG_MISC,
	MONEY_LOG_MONSTER_KILL,
	MONEY_LOG_DROP,
	MONEY_LOG_TYPE_MAX_NUM,
};

enum EPremiumTypes
{
	PREMIUM_EXP,		// °æÇèÄ¡°¡ 1.2¹è
	PREMIUM_ITEM,		// ¾ÆÀÌÅÛ µå·ÓÀ²ÀÌ 2¹è
	PREMIUM_SAFEBOX,		// Ã¢°í°¡ 1Ä­¿¡¼­ 3Ä­
	PREMIUM_AUTOLOOT,		// µ· ÀÚµ¿ ÁÝ±â
	PREMIUM_FISH_MIND,		// °í±Þ ¹°°í±â ³¬ÀÏ È®·ü »ó½Â
	PREMIUM_MARRIAGE_FAST,	// ±Ý½Ç Áõ°¡ ¾çÀ» ºü¸£°ÔÇÕ´Ï´Ù.
	PREMIUM_GOLD,		// µ· µå·ÓÀ²ÀÌ 1.5¹è
	PREMIUM_MAX_NUM = 9
};

enum SPECIAL_EFFECT
{
	SE_NONE,

	SE_HPUP_RED,
	SE_SPUP_BLUE,
	SE_SPEEDUP_GREEN,
	SE_DXUP_PURPLE,
	SE_CRITICAL,
	SE_PENETRATE,
	SE_BLOCK,
	SE_DODGE,
	SE_CHINA_FIREWORK,
	SE_SPIN_TOP,
	SE_SUCCESS,
	SE_FAIL,
	SE_FR_SUCCESS,
	SE_LEVELUP_ON_14_FOR_GERMANY,
	SE_LEVELUP_UNDER_15_FOR_GERMANY,
	SE_PERCENT_DAMAGE1,
	SE_PERCENT_DAMAGE2,
	SE_PERCENT_DAMAGE3,

	SE_AUTO_HPUP,
	SE_AUTO_SPUP,

	SE_EQUIP_RAMADAN_RING,		// ¶ó¸¶´Ü ÃÊ½Â´ÞÀÇ ¹ÝÁö(71135) Âø¿ëÇÒ ¶§ ÀÌÆåÆ® (¹ßµ¿ÀÌÆåÆ®ÀÓ, Áö¼ÓÀÌÆåÆ® ¾Æ´Ô)
	SE_EQUIP_HALLOWEEN_CANDY,		// ÇÒ·ÎÀ© »çÅÁÀ» Âø¿ë(-_-;)ÇÑ ¼ø°£¿¡ ¹ßµ¿ÇÏ´Â ÀÌÆåÆ®
	SE_EQUIP_HAPPINESS_RING,		// Å©¸®½º¸¶½º Çàº¹ÀÇ ¹ÝÁö(71143) Âø¿ëÇÒ ¶§ ÀÌÆåÆ® (¹ßµ¿ÀÌÆåÆ®ÀÓ, Áö¼ÓÀÌÆåÆ® ¾Æ´Ô)
	SE_EQUIP_LOVE_PENDANT,		// ¹ß·»Å¸ÀÎ »ç¶ûÀÇ ÆÒ´øÆ®(71145) Âø¿ëÇÒ ¶§ ÀÌÆåÆ® (¹ßµ¿ÀÌÆåÆ®ÀÓ, Áö¼ÓÀÌÆåÆ® ¾Æ´Ô)
#ifdef __VERSION_162__
	SE_EFFECT_HEALER,
#endif
	SE_EQUIP_RUZGAR,
	SE_ACCE_SUCCEDED_1,
	SE_EQUIP_ACCE_1,
	SE_EQUIP_ACCE_2,
	SE_EQUIP_ACCE_3,
	SE_EQUIP_ACCE_4,
} ;

enum ETeenFlags
{
	TEENFLAG_NONE = 0,
	TEENFLAG_1HOUR,
	TEENFLAG_2HOUR,
	TEENFLAG_3HOUR,
	TEENFLAG_4HOUR,
	TEENFLAG_5HOUR,
};

#include "item_length.h"

// inventoryÀÇ positionÀ» ³ªÅ¸³»´Â ±¸Á¶Ã¼
// int¿ÍÀÇ ¾Ï½ÃÀû Çüº¯È¯ÀÌ ÀÖ´Â ÀÌÀ¯´Â,
// ÀÎº¥ °ü·ÃµÈ ¸ðµç ÇÔ¼ö°¡ window_typeÀº ¹ÞÁö ¾Ê°í, cell ÇÏ³ª¸¸ ¹Þ¾Ò±â ¶§¹®¿¡,(±âÁ¸¿¡´Â ÀÎº¥ÀÌ ÇÏ³ª »ÓÀÌ¾î¼­ inventory typeÀÌ¶õ°Ô ÇÊ¿ä¾ø¾ú±â ¶§¹®¿¡,)
// ÀÎº¥ °ü·Ã ¸ðµç ÇÔ¼ö È£ÃâºÎºÐÀ» ¼öÁ¤ÇÏ´Â °ÍÀÌ ³­°¨ÇÏ±â ‹š¹®ÀÌ´Ù.

enum EDragonSoulRefineWindowSize
{
	DRAGON_SOUL_REFINE_GRID_MAX = 15,
};

enum EMisc2
{
	DRAGON_SOUL_EQUIP_SLOT_START = INVENTORY_MAX_NUM + WEAR_MAX_NUM,	// 180 + 32 (212)
	DRAGON_SOUL_EQUIP_SLOT_END = DRAGON_SOUL_EQUIP_SLOT_START + (DS_SLOT_MAX * DRAGON_SOUL_DECK_MAX_NUM),	// 212 + 12 ( 224 )
	DRAGON_SOUL_EQUIP_RESERVED_SLOT_END = DRAGON_SOUL_EQUIP_SLOT_END + (DS_SLOT_MAX * DRAGON_SOUL_DECK_RESERVED_MAX_NUM),	// 224 + 18 ( 242 )

	BELT_INVENTORY_SLOT_START = DRAGON_SOUL_EQUIP_RESERVED_SLOT_END,	// 242 
	BELT_INVENTORY_SLOT_END = BELT_INVENTORY_SLOT_START + BELT_INVENTORY_SLOT_COUNT,	// 242 + 16 ( 258 )

#ifdef __SPECIAL_INVENTORY__
	SKILL_BOOK_INVENTORY_SLOT_START	= BELT_INVENTORY_SLOT_END,
	SKILL_BOOK_INVENTORY_SLOT_END = SKILL_BOOK_INVENTORY_SLOT_START + SKILL_BOOK_INVENTORY_MAX_NUM,
	
	UPGRADE_ITEMS_INVENTORY_SLOT_START = SKILL_BOOK_INVENTORY_SLOT_END,
	UPGRADE_ITEMS_INVENTORY_SLOT_END = UPGRADE_ITEMS_INVENTORY_SLOT_START + UPGRADE_ITEMS_INVENTORY_MAX_NUM,

	STONE_INVENTORY_SLOT_START = UPGRADE_ITEMS_INVENTORY_SLOT_END,
	STONE_INVENTORY_SLOT_END = STONE_INVENTORY_SLOT_START + STONE_INVENTORY_MAX_NUM,
#endif

#ifdef __SPECIAL_INVENTORY__
	//INVENTORY_AND_EQUIP_SLOT_MAX = UPGRADE_ITEMS_INVENTORY_SLOT_END,
	INVENTORY_AND_EQUIP_SLOT_MAX = STONE_INVENTORY_SLOT_END,
#else
	INVENTORY_AND_EQUIP_SLOT_MAX = BELT_INVENTORY_SLOT_END,
#endif
};

#pragma pack(push, 1)

typedef struct SItemPos
{
	BYTE window_type;
	WORD cell;
	SItemPos ()
	{
		window_type = INVENTORY;
		cell = WORD_MAX;
	}
	SItemPos (BYTE _window_type, WORD _cell)
	{
		window_type = _window_type;
		cell = _cell;
	}
	bool IsValidItemPosition() const
	{
		switch (window_type)
		{
		case RESERVED_WINDOW:
			return false;
		case INVENTORY:
		case EQUIPMENT:
		case BELT_INVENTORY:
#ifdef __SPECIAL_INVENTORY__
		case SKILL_BOOK_INVENTORY:
		case UPGRADE_ITEMS_INVENTORY:
		case STONE_INVENTORY:
#endif
			return cell < INVENTORY_AND_EQUIP_SLOT_MAX;
		case DRAGON_SOUL_INVENTORY:
			return cell < (DRAGON_SOUL_INVENTORY_MAX_NUM);
		// µ¿ÀûÀ¸·Î Å©±â°¡ Á¤ÇØÁö´Â window´Â valid Ã¼Å©¸¦ ÇÒ ¼ö°¡ ¾ø´Ù.
		case SAFEBOX:
		case MALL:
			return false;
		default:
			return false;
		}
		return false;
	}
	bool IsEquipPosition() const
	{
		return (IsDefaultInventoryEquipPosition() || IsDragonSoulEquipPosition());
	}
	bool IsDefaultInventoryEquipPosition() const
	{
		return ((INVENTORY == window_type || EQUIPMENT == window_type) && cell >= INVENTORY_MAX_NUM && cell < INVENTORY_MAX_NUM + WEAR_MAX_NUM);
	}
	bool IsDragonSoulEquipPosition() const
	{
		return ((INVENTORY == window_type || EQUIPMENT == window_type) && cell >= DRAGON_SOUL_EQUIP_SLOT_START && cell < DRAGON_SOUL_EQUIP_SLOT_END);
	}
	bool IsBeltInventoryPosition() const
	{
		return ((INVENTORY == window_type) && cell >= BELT_INVENTORY_SLOT_START && cell < BELT_INVENTORY_SLOT_END);
	}
	bool IsDefaultInventoryPosition() const
	{
		return ((INVENTORY == window_type) && cell < INVENTORY_MAX_NUM);
	}
#ifdef __SPECIAL_INVENTORY__
	bool IsSkillBookInventoryPosition() const
	{
		return ((INVENTORY == window_type) && cell >= SKILL_BOOK_INVENTORY_SLOT_START && cell < SKILL_BOOK_INVENTORY_SLOT_END);
	}
	bool IsUpgradeItemsInventoryPosition() const
	{
		return ((INVENTORY == window_type) && cell >= UPGRADE_ITEMS_INVENTORY_SLOT_START && cell < UPGRADE_ITEMS_INVENTORY_SLOT_END);
	}
	bool IsStoneInventoryPosition() const
	{
		return ((INVENTORY == window_type) && cell >= STONE_INVENTORY_SLOT_START && cell < STONE_INVENTORY_SLOT_END);
	}
#endif
	bool operator==(const struct SItemPos& rhs) const
	{
		return (window_type == rhs.window_type) && (cell == rhs.cell);
	}
	bool operator<(const struct SItemPos& rhs) const
	{
		return (window_type < rhs.window_type) || ((window_type == rhs.window_type) && (cell < rhs.cell));
	}
} TItemPos;

const TItemPos NPOS (RESERVED_WINDOW, WORD_MAX);

typedef enum
{
	SHOP_COIN_TYPE_GOLD, // DEFAULT VALUE
	SHOP_COIN_TYPE_SECONDARY_COIN,
} EShopCoinType;

#pragma pack(pop)

#endif
