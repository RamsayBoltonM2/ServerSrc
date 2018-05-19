#ifndef __INC_SERVICE_H__
#define __INC_SERVICE_H__

#define _IMPROVED_PACKET_ENCRYPTION_
#define __PET_SYSTEM__
#define __UDP_BLOCK__

#define __ENABLE_CUBE_RENEWAL__
#define __GAYA__
#define __ENABLE_POLY_SHOP__
#define __KOSTUM_ENCHANT_SISTEM__
#define __SASH_SEEN_SYSTEM__
#define __ENABLE_DICE_SYSTEM__
#define __HIGHLIGHT_SYSTEM__
#define __MOUNT_SYSTEM__
#define __MS_BUG_WITH_MAPS__
#define __MAINTENANCE_SYSTEM__
#define __PRESTIGE_SYSTEM__
#define __GROWTH_PET_SYSTEM__
#define __BIOLOG_SYSTEM__
#define __VIEW_TARGET_PLAYER_HP__
#define __VIEW_TARGET_DECIMAL_HP__
#define __LOCALIZATION_SYSTEM__
#define __AFFECT_DUP_FIX__
#define __ANNONUNCEMENT_LEVELUP__
#define __ONLINE_COMMAND__
#define __SEND_TARGET_INFO__
#define __MINI_GAME_RUMI__
#define __7AND8TH_SKILLS__
#define __SHOPEX_SYSTEM__
#define __GF_UPDATE_STROKE_INFO__
#define __CHEQUE_SYSTEM__
#define __GF_SEAL_ITEM_UPDATE__
#define __COSTUME_ATTR_SYSTEM__
#define __ATTR_TRANSFER_SYSTEM__
#define __GF_UPDATE_HIGHLIGHT_ICON_SLOT__
#define __LOGIN_SECURITY_SYSTEM__
#define __SPECIAL_INVENTORY__
#define __PET_SYSTEM_LIKE_RUBINUM__
#define __GF_MESSENGER_UPDATE__
#define __WEAPON_COSTUME_SYSTEM__
#define __TRANSMUTATION_SYSTEM__
#define __SUPPORT_SYSTEM__
#define __OFFLINESHOP_SYSTEM__
#define __PRIVATESHOP_SEARCH_SYSTEM__
#define __SKILL_MANA_FIX_UPDATE__
#define __GF_USER_PANEL__
#define __INGAME_SUPPORT_TICKET_SYSTEM__
#define __EXTENDED_REFINE_WINDOW_SYSTEM__
#define __SECONDARY_SLOT_SYSTEM__
#define __EXTENDED_OXEVENT_SYSTEM__
#define __INGAME_FOREX_SYSTEM__
#define __GF_UPDATE_CHINESE_TEMPLE__
#define __ENABLE_BLOCK_PICKUP__
#define __GF_CHARM_SYSTEM__
#define __ACTIVATE_MYSQL_QUERY__
#define __INGAME_MASTER_BAN__
#define __PVP_TOURNAMENT_SYSTEM__
#define __OXEVENT_QUIZ_UPDATE__
#define __ACTIVATE_USER_CONTROL__
#define __DOJANG_UPDATE__
#define __MAP_LEVEL_LOGIN_BUGFIX__
#define __EXTRA_FUNCTION_OPEN_TELEPORT__
#define __TELEPORT_SYSTEM_FOR_PLAYER__
#define __AFFECT_POTION_UPDATE__
#define __ACTIVATE_KILL_COMMAND__

#define __ENABLE_PVP_ADVANCED__
#ifdef __ENABLE_PVP_ADVANCED__
	#define BLOCK_CHANGEITEM "pvp.BlockChangeItem"
	#define BLOCK_BUFF "pvp.BlockBuff"
	#define BLOCK_POTION "pvp.BlockPotion"
	#define BLOCK_RIDE "pvp.BlockRide"
	#define BLOCK_PET "pvp.BlockPet"
	#define BLOCK_POLY "pvp.BlockPoly"
	#define BLOCK_PARTY "pvp.BlockParty"
	#define BLOCK_EXCHANGE_ "pvp.BlockExchange"
	#define BLOCK_EQUIPMENT_ "pvp.BLOCK_VIEW_EQUIPMENT"
	#define BET_WINNER "pvp.BetMoney"
	#define CHECK_IS_FIGHT "pvp.IsFight"
#endif

#define __YMIR_UPDATE_WOLFMAN__
#ifdef __YMIR_UPDATE_WOLFMAN__
	#define USE_MOB_BLEEDING_AS_POISON
	#define USE_MOB_CLAW_AS_DAGGER
	#define USE_ITEM_BLEEDING_AS_POISON//activated
	#define USE_ITEM_CLAW_AS_DAGGER//activated
	#define USE_WOLFMAN_STONES
	#define USE_WOLFMAN_BOOKS
#endif

#define __MAGIC_STONE_SYSTEM__
#ifdef __MAGIC_STONE_SYSTEM__
	#define USE_MAGIC_REDUCTION_STONES
#endif

#define __DUNGEON_FOR_GUILD__
#ifdef __DUNGEON_FOR_GUILD__
	#define __MELEY_LAIR_DUNGEON__
	#ifdef __MELEY_LAIR_DUNGEON__
		#define __DESTROY_INFINITE_STATUES_GM__
		#define __LASER_EFFECT_ON_75HP__
		#define __LASER_EFFECT_ON_50HP__
	#endif
#endif

#define __NEW_EVENTS__
#ifdef __NEW_EVENTS__
	#define __KINGDOMS_WAR__
#endif

#define __VERSION_162__
#ifdef __VERSION_162__
	#define HEALING_SKILL_VNUM 265
#endif

#endif