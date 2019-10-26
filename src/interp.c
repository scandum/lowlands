/***************************************************************************
 * Lola  1.0 by Igor van den Hoven.                                        *
 *                                                                         *
 * Emud  2.2 by Igor van den Hoven, Michiel Lange.                         *
 *                                                                         *
 * MrMud 1.4 by David Bills and Dug Michael.                               *
 *                                                                         *
 * Merc  2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael      *
 * Chastain, Michael Quan, and Mitchell Tse.                               *
 *                                                                         *
 * Original Diku Mud copyright (C) 1990 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeld, Tom Madsen, and Katje Nyboe.     *
 ***************************************************************************/

#include <sys/time.h>
#include "mud.h"

void    process_command( CHAR_DATA *, char *);
void preprocess_command( CHAR_DATA *, char *);

/*
	Command logging types.
*/

#define LOG_NORMAL      0
#define LOG_ALWAYS      1
#define LOG_NEVER       2

/*
	Command table.
*/

sh_int cmd_gsn[512];

/*
	Alphabetic order for binary search - Scandum
*/

const struct cmd_type cmd_table [] =
{
	{ ",",			do_emote,			POS_RESTING,	-9,			LOG_NORMAL,	CMD_HIDE},
	{ "/",			do_recall,		POS_FIGHTING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ ".",			do_chat,			POS_SLEEPING,	 1,			LOG_NORMAL,	CMD_HIDE|CMD_BERSERK},
	{ "'",			do_say,			POS_RESTING,	-9,			LOG_NORMAL,	CMD_HIDE},
	{ "?",			do_grep,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_HIDE|CMD_BERSERK},
	{ ":",			do_channel_talk,	POS_DEAD,		 1,			LOG_NORMAL,	CMD_HIDE|CMD_COMM|CMD_BERSERK},
	{ ";",			do_gtell,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_HIDE|CMD_BERSERK},
	{ "]",			do_immtalk,		POS_SLEEPING,	MAX_LEVEL-3,	LOG_NORMAL,	CMD_HIDE|CMD_BERSERK},

	{ "a",			do_a,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_HIDE},
	{ "acupunch",		do_acupunch,		POS_FIGHTING,	-3,			LOG_NORMAL,	CMD_HIDE},
	{ "advance",		do_advance,		POS_DEAD,		MAX_LEVEL,	LOG_ALWAYS,	CMD_NONE},
	{ "affects",		do_affects,		POS_DEAD,		-8,			LOG_NORMAL,	CMD_BERSERK},
	{ "afk",			do_afk,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "alias",		do_alias,			POS_DEAD,		-3,			LOG_NORMAL,	CMD_NONE},
	{ "ambush",		do_ambush,		POS_STANDING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "appearance",     do_appearance,      POS_DEAD,      MAX_LEVEL-3,   LOG_NORMAL,    CMD_NONE},
	{ "areas",		do_areas,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "arrest",		do_arrest,		POS_DEAD,		MAX_LEVEL,	LOG_NORMAL,	CMD_EDIT},
	{ "assassin",		do_assassin,		POS_FIGHTING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "assassinate",	do_assassinate,	POS_FIGHTING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "at",			do_at,			POS_DEAD,		MAX_LEVEL-1,	LOG_NORMAL,	CMD_NONE},
	{ "attack",		do_attack,		POS_DEAD,		MAX_LEVEL,	LOG_NORMAL,	CMD_NONE},
	{ "auto",			do_auto,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_BERSERK},

	{ "buy",			do_buy,			POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "backstab",		do_backstab,		POS_STANDING,	-3,			LOG_NORMAL,	CMD_HIDE},
	{ "banish",		do_ban,			POS_DEAD,		MAX_LEVEL,	LOG_NORMAL,	CMD_NONE},
	{ "bank",			do_bank,			POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "bash",			do_bash,			POS_FIGHTING,	-3,			LOG_NORMAL,	CMD_HIDE},
	{ "beep",			do_beep,			POS_SLEEPING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "berserk",		do_berserk,		POS_FIGHTING,	-3,			LOG_NORMAL,	CMD_HIDE},
	{ "block",		do_block,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "bloodfrenzy",	do_blood_frenzy,	POS_FIGHTING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "bounty",		do_bounty,		POS_RESTING,	 1,			LOG_NORMAL, 	CMD_NONE},
	{ "brandish",		do_brandish,		POS_FIGHTING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "buffer",		do_buffer,		POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "buildlight",	do_buildlight, 	POS_DEAD,		MAX_LEVEL-2,	LOG_NORMAL,	CMD_NONE},


	{ "cast",			do_cast,			POS_FIGHTING,	-3,			LOG_NORMAL,	CMD_NONE},
	{ "camp",			do_camp,			POS_RESTING,	-3,			LOG_NORMAL,	CMD_HIDE},
	{ "castle",		do_castle,		POS_RESTING,	 1,			LOG_NORMAL,	CMD_EDIT},
	{ "chat",			do_chat,			POS_SLEEPING,	 1,			LOG_NORMAL,	CMD_NONE|CMD_BERSERK},
	{ "clans",		do_clans,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE|CMD_BERSERK},
	{ "clanmessage",	do_clan_message,	POS_DEAD,		MAX_LEVEL,	LOG_ALWAYS,	CMD_NONE|CMD_BERSERK},
	{ "clanset",		do_clanset,		POS_RESTING,	10,			LOG_NORMAL,	CMD_HIDE|CMD_EDIT},
	{ "clanwhere",		do_clanwhere,		POS_RESTING,	10,			LOG_NORMAL,	CMD_HIDE},
	{ "class",		do_class,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "clearpath",		do_clear_path,		POS_STANDING,	-3,			LOG_NORMAL,	CMD_HIDE},
	{ "cloak",		do_cloak,			POS_DEAD,		MAX_LEVEL-1,	LOG_NORMAL,	CMD_NONE},
	{ "close",		do_close,			POS_RESTING,	-8,			LOG_NORMAL,	CMD_NONE},
	{ "clock",		do_clock,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "circle",		do_circle,		POS_FIGHTING,	-3,			LOG_NORMAL,	CMD_HIDE},
	{ "coffer",		do_coffer,		POS_RESTING,	10,  		LOG_ALWAYS,	CMD_HIDE},
	{ "color",		do_color,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "commands",		do_commands,		POS_DEAD,		-8,			LOG_NORMAL,	CMD_HIDE},
	{ "compare",		do_compare,		POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "connect",		do_connect,		POS_DEAD,		MAX_LEVEL-1,	LOG_NORMAL,	CMD_NONE},
	{ "consider",		do_consider,		POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "constitution",	do_constitution,	POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "config",		do_config,		POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "copyover",		do_copyover,		POS_DEAD,		MAX_LEVEL,	LOG_ALWAYS,	CMD_QUIT},
	{ "credits",		do_credits,		POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "cripple",		do_cripple,		POS_FIGHTING,	-3,			LOG_NORMAL,	CMD_HIDE},


	{ "down",			do_down,			POS_STANDING,	-8,			LOG_NORMAL,	CMD_IDLE|CMD_BERSERK},
	{ "death",		do_death,			POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "delete",		do_delete,		POS_DEAD,		 1,			LOG_NEVER,	CMD_NONE},
	{ "deny",			do_deny,			POS_DEAD,		MAX_LEVEL,	LOG_NORMAL,	CMD_NONE},
	{ "description",	do_description,	POS_DEAD,		 1,			LOG_NORMAL,	CMD_EDIT},
	{ "destroy",		do_destroy,		POS_DEAD,		MAX_LEVEL,	LOG_ALWAYS,	CMD_NONE},
	{ "devtalk",		do_devtalk,		POS_DEAD,		MAX_LEVEL,	LOG_NORMAL,	CMD_NONE|CMD_BERSERK},
	{ "dexterity",		do_dexterity,		POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "disarm",		do_disarm,		POS_FIGHTING,	-3,			LOG_NORMAL,	CMD_HIDE},
	{ "disconnect",	do_disconnect,		POS_DEAD,		MAX_LEVEL,	LOG_ALWAYS,	CMD_NONE},
	{ "disguise",		do_disguise,		POS_RESTING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "dismount",		do_dismount,		POS_FIGHTING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "distract",		do_distract,		POS_STANDING,	-3,			LOG_NORMAL,	CMD_HIDE},
	{ "divert",		do_divert,		POS_FIGHTING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "donate",		do_donate,		POS_RESTING,	10,			LOG_NORMAL,	CMD_HIDE},
	{ "doorbash",		do_bashdoor,		POS_FIGHTING,	-3,			LOG_NORMAL,	CMD_HIDE},
	{ "doorset",		do_doorset,		POS_DEAD,		MAX_LEVEL-1,	LOG_NORMAL,	CMD_NONE},
	{ "drink",		do_drink,			POS_RESTING,	~ 1,			LOG_NORMAL,	CMD_NONE},
	{ "drain",		do_drain,			POS_FIGHTING,	-3,			LOG_NORMAL,	CMD_HIDE},
	{ "drop",			do_drop,			POS_RESTING,	-9,			LOG_NORMAL,	CMD_NONE},
	{ "dump",			do_dump,			POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},


	{ "east",			do_east,			POS_STANDING,	-8,			LOG_NORMAL,	CMD_IDLE|CMD_BERSERK},
	{ "eat",			do_eat,			POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "echo",			do_echo,			POS_DEAD,		MAX_LEVEL-1,	LOG_NORMAL,	CMD_NONE},
	{ "edit",			do_edit,			POS_DEAD,		MAX_LEVEL-2,	LOG_NORMAL,	CMD_EDIT},
/*	{ "email",		do_email,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE}, */
	{ "emote",		do_emote,			POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "engrave",		do_engrave,		POS_STANDING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "enter",		do_enter,			POS_STANDING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "equipment",		do_equipment,		POS_DEAD,		-8,			LOG_NORMAL,	CMD_NONE},
	{ "evaluate",		do_evaluate,		POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "examine",		do_examine,		POS_RESTING,	-8,			LOG_NORMAL,	CMD_NONE},
	{ "exits",		do_exits,			POS_RESTING,	-8,			LOG_NORMAL,	CMD_NONE},


	{ "flee",			do_flee,			POS_FIGHTING,	-8,			LOG_NORMAL,	CMD_NONE},
	{ "fill",			do_fill,			POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "finger",		do_finger,		POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE|CMD_BERSERK},
	{ "find",			do_find,			POS_DEAD,		MAX_LEVEL-2,	LOG_NORMAL,	CMD_NONE},
	{ "fixpass",		do_fixpass,		POS_DEAD,		MAX_LEVEL-3,	LOG_ALWAYS,	CMD_NONE},
	{ "follow",		do_follow,		POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "forage",		do_forage,		POS_STANDING,	1,			LOG_NORMAL,	CMD_HIDE},
	{ "force",		do_force,			POS_DEAD,		MAX_LEVEL-1,	LOG_ALWAYS,	CMD_NONE},
	{ "forceren",		do_forceren,		POS_DEAD,		MAX_LEVEL,	LOG_NORMAL,	CMD_HIDE},
	{ "forcerent",		do_forcerent,		POS_DEAD,		MAX_LEVEL,	LOG_ALWAYS,	CMD_NONE},
	{ "forge",		do_forge,			POS_RESTING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "fos",			do_fos,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE|CMD_BERSERK},
	{ "freeze",		do_freeze,		POS_DEAD,		MAX_LEVEL,	LOG_NORMAL,	CMD_NONE},


	{ "get",			do_get,			POS_RESTING,	-9,			LOG_NORMAL,	CMD_NONE},
	{ "give",			do_give,			POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "glance",		do_glance,		POS_RESTING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "gohome",		do_gohome,		POS_RESTING,	10,  		LOG_NORMAL,	CMD_HIDE},
	{ "goto",			do_goto,			POS_DEAD,		MAX_LEVEL-3,	LOG_NORMAL,	CMD_NONE},
	{ "gouge",		do_gouge,			POS_FIGHTING,	-3,			LOG_NORMAL,	CMD_HIDE},
	{ "grep",			do_grep,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE|CMD_BERSERK},
	{ "group",		do_group,			POS_SLEEPING,	 1,			LOG_NORMAL,	CMD_NONE|CMD_BERSERK},
	{ "gtell",		do_gtell,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE|CMD_BERSERK},


	{ "help",			do_help,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "headbutt",		do_head_butt,		POS_FIGHTING,	-3,			LOG_NORMAL,	CMD_HIDE},
	{ "heal",			do_heal,			POS_RESTING,	10,			LOG_NORMAL,	CMD_HIDE},
	{ "hearlog",		do_hearlog,		POS_DEAD,		MAX_LEVEL-2,	LOG_NORMAL,	CMD_NONE},
	{ "hide",			do_hide,			POS_RESTING,	-3,			LOG_NORMAL,	CMD_HIDE},
	{ "hold",			do_wear,			POS_RESTING,	-8,			LOG_NORMAL,	CMD_HIDE},
	{ "holylight",		do_holylight,		POS_DEAD,		MAX_LEVEL-3,	LOG_NORMAL,	CMD_NONE},
	{ "honorific",		do_honorific,		POS_SLEEPING,  90,			LOG_NORMAL,	CMD_NONE},
/*	{ "html",			do_html,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE}, */
	{ "hunt",			do_hunt,			POS_STANDING,	-3,			LOG_NORMAL,	CMD_HIDE},


	{ "inventory",      do_inventory,		POS_DEAD,		-8,			LOG_NORMAL,	CMD_NONE},
	{ "identify",       do_identify,		POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "immtalk",        do_immtalk,		POS_DEAD,		MAX_LEVEL-3,	LOG_NORMAL,	CMD_NONE},
	{ "initiate",       do_initiate,		POS_RESTING,	10,			LOG_NORMAL,	CMD_HIDE},
	{ "intelligence",   do_intelligence,	POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "invis",          do_invis,			POS_DEAD,		MAX_LEVEL-1,	LOG_NORMAL,	CMD_NONE},


	{ "kill",           do_kill,			POS_FIGHTING,	-9,			LOG_NORMAL,	CMD_BERSERK},
	{ "kick",			do_kick,			POS_FIGHTING,	-3,			LOG_NORMAL,	CMD_HIDE|CMD_BERSERK},
	{ "knife",		do_knife,			POS_FIGHTING,	-3,			LOG_NORMAL,	CMD_HIDE},
	{ "know",			do_know,			POS_RESTING,	1,			LOG_NORMAL,	CMD_HIDE},


	{ "look",			do_look,			POS_RESTING,	-8,			LOG_NORMAL,	CMD_NONE|CMD_BERSERK},
	{ "language",		do_language,		POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "level",		do_level,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "list",			do_list,			POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "load",			do_load,			POS_DEAD,		MAX_LEVEL-2,	LOG_NORMAL,	CMD_NONE},
	{ "lock",			do_lock,			POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "log",			do_log,			POS_DEAD,		MAX_LEVEL,	LOG_NORMAL,	CMD_NONE},


	{ "map",			do_map,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "makeclan",		do_makeclan,		POS_RESTING,	10,  		LOG_NORMAL,	CMD_HIDE},
	{ "makeflash",		do_make_flash,		POS_STANDING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "makepoison",	do_make_poison,	POS_STANDING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "mana",			do_mana,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "mass",			do_mass,			POS_FIGHTING,	-3,			LOG_NORMAL,	CMD_HIDE},

	{ "morph",		do_morph,			POS_DEAD,		-7,			LOG_NORMAL,	CMD_NONE},
	{ "most",			do_most,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "mount",		do_mount,			POS_FIGHTING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "move",			do_move,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "mudlist",		do_mudlist,         POS_DEAD,      MAX_LEVEL,	LOG_NORMAL,	CMD_NONE},
	{ "multiclass",	do_multi,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "murde",		do_murde,			POS_STANDING,	 5,			LOG_NORMAL,	CMD_HIDE},
	{ "murder",		do_murder,		POS_STANDING,	 5,			LOG_NORMAL,	CMD_BERSERK},
	{ "mute",			do_mute,			POS_DEAD,		MAX_LEVEL,	LOG_NORMAL,	CMD_NONE},
	{ "mpaset",		do_mpaset,		POS_DEAD,		97,			LOG_ALWAYS,	CMD_HIDE},
	{ "mpasound",		do_mpasound,		POS_DEAD,		97,			LOG_NORMAL,	CMD_HIDE},
	{ "mpareaecho",	do_mpareaecho,		POS_DEAD,		97,			LOG_NORMAL,	CMD_HIDE},
	{ "mpdamage",		do_mpdamage,		POS_DEAD,		97,			LOG_NORMAL,	CMD_HIDE},
	{ "mpdelay",		do_mpdelay,		POS_DEAD,		98,			LOG_NORMAL,	CMD_HIDE},
	{ "mpjunk",		do_mpjunk,		POS_DEAD,		97,			LOG_NORMAL,	CMD_HIDE},
	{ "mpjunkperson",	do_mpjunk_person,	POS_DEAD,		97,			LOG_NORMAL,	CMD_HIDE},
	{ "mpecho",		do_mpecho,		POS_DEAD,		97,			LOG_NORMAL,	CMD_HIDE},
	{ "mpechoat",		do_mpechoat,		POS_DEAD,		97,			LOG_NORMAL,	CMD_HIDE},
	{ "mpechoaround",	do_mpechoaround,	POS_DEAD,		97,			LOG_NORMAL,	CMD_HIDE},
	{ "mpgorand",		do_mpgorand,		POS_DEAD,		97,			LOG_NORMAL,	CMD_HIDE},
	{ "mpkill",		do_mpkill,		POS_DEAD,		97,			LOG_NORMAL,	CMD_HIDE},
	{ "mpzset",		do_mpzset,		POS_DEAD,		97,			LOG_NORMAL,	CMD_HIDE},
	{ "mpcalculate",	do_mpcalculate,	POS_DEAD,		98,			LOG_NORMAL,	CMD_HIDE},
	{ "mpdo",			do_mpdo,			POS_DEAD,		98,			LOG_NORMAL,	CMD_HIDE},
	{ "mpmaze",		do_mpmaze,		POS_DEAD,		MAX_LEVEL-1,	LOG_ALWAYS,	CMD_NONE},
	{ "mpmload",		do_mpmload, 		POS_DEAD,		98,			LOG_NORMAL,	CMD_HIDE},
	{ "mpoload",		do_mpoload, 		POS_DEAD,		98,			LOG_NORMAL,	CMD_HIDE},
	{ "mppurge",		do_mppurge, 		POS_DEAD,		98,			LOG_NORMAL,	CMD_HIDE},
	{ "mpquiet",		do_mpquiet, 		POS_DEAD,		98,			LOG_NORMAL,	CMD_HIDE},
	{ "mpgoto",		do_mpgoto,		POS_DEAD,		98,			LOG_NORMAL,	CMD_HIDE},
	{ "mpat",			do_mpat,			POS_DEAD,		98,			LOG_NORMAL,	CMD_HIDE},
	{ "mptransfer",	do_mptransfer,		POS_DEAD,		98,			LOG_NORMAL,	CMD_HIDE},
	{ "mptrigger",		do_mptrigger,		POS_DEAD,		97,			LOG_NORMAL,	CMD_HIDE},
	{ "mpforce",		do_mpforce, 		POS_DEAD,		98,			LOG_NORMAL,	CMD_HIDE},
	{ "mplog",		do_mplog,			POS_DEAD,		98,			LOG_NORMAL,	CMD_HIDE},
	{ "mpmset",		do_mpmset,		POS_DEAD,		98,			LOG_NORMAL,	CMD_HIDE},
	{ "mposet",		do_mposet,		POS_DEAD,		98,			LOG_NORMAL,	CMD_HIDE},
	{ "mpmadd",		do_mpmadd,		POS_DEAD,		98,			LOG_NORMAL,	CMD_HIDE},
	{ "mpoadd",		do_mpoadd,		POS_DEAD,		98,			LOG_NORMAL,	CMD_HIDE},
	{ "mpseed",		do_mpseed,		POS_DEAD,		98,			LOG_NORMAL,	CMD_HIDE},
	{ "mpswap",		do_mpswap,		POS_DEAD,		98,			LOG_NORMAL,	CMD_HIDE},


	{ "north",		do_north,			POS_STANDING,	-8,			LOG_NORMAL,	CMD_IDLE|CMD_BERSERK},
	{ "notice",		do_notice,		POS_STANDING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "nbanish",		do_nban,			POS_DEAD,		MAX_LEVEL,	LOG_NORMAL,	CMD_NONE},
	{ "nominate",		do_nominate,		POS_RESTING,	10,			LOG_NORMAL,	CMD_HIDE},
	{ "note",			do_note,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_EDIT|CMD_BERSERK},


	{ "open",			do_open,			POS_RESTING,	-8,			LOG_NORMAL,	CMD_NONE},
	{ "order",		do_order,			POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "outcast",		do_outcast,		POS_RESTING,	10,			LOG_NORMAL,	CMD_HIDE},


	{ "put",			do_put,			POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "pardon",		do_pardon,		POS_DEAD,		MAX_LEVEL-2,	LOG_NORMAL,	CMD_NONE},
	{ "password",		do_password,		POS_DEAD,		 1,			LOG_NEVER, 	CMD_NONE},
	{ "peace",		do_peace,			POS_DEAD,		MAX_LEVEL-3,	LOG_NORMAL,	CMD_NONE},
	{ "peek",			do_peek,			POS_RESTING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "pick",			do_pick,			POS_RESTING,	-3,			LOG_NORMAL,	CMD_HIDE},
	{ "plan",			do_plan,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "plant",		do_plant,	 		POS_RESTING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "pledge",		do_pledge,		POS_RESTING,	20,  		LOG_NORMAL,	CMD_HIDE},
	{ "pload",		do_pload,			POS_DEAD,		MAX_LEVEL,	LOG_ALWAYS,	CMD_NONE},
	{ "pquit",		do_pquit,			POS_DEAD,		MAX_LEVEL,	LOG_ALWAYS,	CMD_NONE},
	{ "polymorph",		do_polymorph,		POS_STANDING,  -3,			LOG_NORMAL,	CMD_HIDE},
	{ "poof",			do_poof,			POS_DEAD,		MAX_LEVEL-3,	LOG_NORMAL,	CMD_NONE},
	{ "pose",			do_pose,			POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "practice",		do_practice,		POS_SLEEPING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "prog",			do_prog,			POS_DEAD,		MAX_LEVEL-2,	LOG_NORMAL,	CMD_NONE},
	{ "prompt",		do_prompt,		POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "purge",		do_purge,			POS_DEAD,		MAX_LEVEL-3,	LOG_NORMAL,	CMD_NONE},


	{ "quaff",		do_quaff,			POS_FIGHTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "qui",			do_qui,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_HIDE},
	{ "quit",			do_quit,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_BERSERK|CMD_QUIT},


	{ "reply",		do_reply,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE|CMD_COMM|CMD_BERSERK},
	{ "rangecast",		do_range_cast,		POS_FIGHTING,	-3,			LOG_NORMAL,	CMD_HIDE},
	{ "reboo",		do_reboo,			POS_DEAD,		MAX_LEVEL,	LOG_NORMAL,	CMD_HIDE},
	{ "reboot",		do_reboot,		POS_DEAD,		MAX_LEVEL,	LOG_ALWAYS,	CMD_QUIT},
	{ "recall",		do_recall,		POS_FIGHTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "recite",		do_recite,		POS_FIGHTING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "refresh",		do_refresh,		POS_DEAD,		-8,			LOG_NORMAL,	CMD_BERSERK},
	{ "reincarnate",	do_reincarnate,	POS_STANDING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "relog",		do_relog,			POS_STANDING,   1,  		LOG_NORMAL,	CMD_BERSERK|CMD_QUIT},
	{ "release",		do_release,		POS_DEAD,		MAX_LEVEL,	LOG_ALWAYS,	CMD_NONE},
	{ "rename",		do_rename,		POS_DEAD,		MAX_LEVEL-3,	LOG_ALWAYS,	CMD_NONE},
	{ "renounce",		do_renounce,		POS_RESTING,	10,			LOG_NORMAL,	CMD_HIDE},
	{ "rent",			do_rent,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "remove",		do_remove,		POS_RESTING,	-8,			LOG_NORMAL,	CMD_NONE},
	{ "rescale",		do_rescale,		POS_DEAD,		MAX_LEVEL-2,	LOG_NORMAL,	CMD_NONE},
	{ "rescue",		do_rescue,		POS_FIGHTING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "resetarea",		do_resetarea,		POS_DEAD,		MAX_LEVEL-2,	LOG_NORMAL,	CMD_NONE},
	{ "rest",			do_rest,			POS_SLEEPING,	-8,			LOG_NORMAL,	CMD_NONE},
	{ "restore",		do_restore,		POS_DEAD,		MAX_LEVEL-2,	LOG_NORMAL,	CMD_NONE},
	{ "return",		do_return,		POS_DEAD,		-8,			LOG_NORMAL,	CMD_NONE},
	{ "revert",		do_revert,		POS_DEAD,		MAX_LEVEL,	LOG_ALWAYS,	CMD_NONE},


	{ "south",		do_south,			POS_STANDING,	-8,			LOG_NORMAL,	CMD_IDLE|CMD_BERSERK},
	{ "say",			do_say,			POS_RESTING,	-9,			LOG_NORMAL,	CMD_NONE},
	{ "sacrifice",		do_sacrifice,		POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "save",			do_save,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_HIDE},
	{ "savearea",		do_savearea,		POS_DEAD,		MAX_LEVEL-2,	LOG_NORMAL,	CMD_NONE},
	{ "scan",			do_scan,			POS_RESTING,	-8,			LOG_NORMAL,	CMD_BERSERK},
	{ "score",		do_score,			POS_DEAD,		-7,			LOG_NORMAL,	CMD_BERSERK},
	{ "search",		do_search,		POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "sell",			do_sell,			POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "set",			do_set,			POS_DEAD,		MAX_LEVEL-2,	LOG_NORMAL,	CMD_NONE},
	{ "shadow",		do_shadow,		POS_RESTING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "shoot",		do_shoot,	 		POS_FIGHTING,	-3,			LOG_NORMAL,	CMD_HIDE},
	{ "shout",		do_shout,			POS_RESTING,	-8,			LOG_NORMAL,	CMD_NONE},
	{ "show",			do_show,			POS_DEAD,      MAX_LEVEL-2,	LOG_NORMAL,	CMD_NONE},
	{ "shutdow",		do_shutdow,		POS_DEAD,		MAX_LEVEL,	LOG_NORMAL,	CMD_HIDE},
	{ "shutdown",		do_shutdown,		POS_DEAD,		MAX_LEVEL,	LOG_ALWAYS,	CMD_QUIT},
	{ "silence",		do_silence,		POS_DEAD,		MAX_LEVEL,	LOG_NORMAL,	CMD_NONE},
	{ "sing",			do_sing,			POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "sit",			do_sit,			POS_SLEEPING,	-8,			LOG_NORMAL,	CMD_NONE},
	{ "skills",		do_skills,		POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "sla",			do_sla,			POS_DEAD,		MAX_LEVEL-3,	LOG_NORMAL,	CMD_HIDE},
	{ "slaughter",      do_slaughter,		POS_DEAD,		MAX_LEVEL-1,	LOG_NORMAL,	CMD_NONE},
	{ "slay",			do_slay,			POS_DEAD,		MAX_LEVEL-3,	LOG_ALWAYS,	CMD_NONE},
	{ "sleep",		do_sleep,			POS_SLEEPING,	-8,			LOG_NORMAL,	CMD_NONE},
	{ "socials",		do_socials,		POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "snatch",		do_snatch,		POS_RESTING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "sneak",		do_sneak,			POS_STANDING,	-3,			LOG_NORMAL,	CMD_HIDE},
	{ "snoop",		do_snoop,			POS_DEAD,		MAX_LEVEL,	LOG_NORMAL,	CMD_NONE},
	{ "spam",			do_spam,			POS_DEAD,		-7,			LOG_NORMAL,	CMD_NONE},
	{ "speak",		do_speak,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "speed",		do_speed,			POS_DEAD,		-9,			LOG_NORMAL,	CMD_NONE},
	{ "split",		do_split,			POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "spy",			do_spy,			POS_STANDING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "stand",		do_stand,			POS_SLEEPING,	-9,			LOG_NORMAL,	CMD_NONE},
	{ "stat",			do_stat,			POS_DEAD,		MAX_LEVEL-2,	LOG_NORMAL,	CMD_NONE},
	{ "status",		do_status,		POS_DEAD,		-7,			LOG_NORMAL,	CMD_BERSERK},
	{ "steal",		do_steal,			POS_STANDING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "stealth",		do_stealth,		POS_STANDING,	-3,			LOG_NORMAL,	CMD_HIDE},
	{ "stop",			do_visible,		POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "strength",		do_strength,		POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "strangle",		do_strangle,		POS_STANDING,  -3,			LOG_NORMAL,	CMD_HIDE},
	{ "suicide",		do_suicide,		POS_DEAD,		 1,			LOG_NEVER,	CMD_NONE},
	{ "switch",		do_switch,		POS_DEAD,		MAX_LEVEL,	LOG_NORMAL,	CMD_NONE},


	{ "tell",			do_tell,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE|CMD_COMM},
	{ "take",			do_get,			POS_RESTING,	-9,			LOG_NORMAL,	CMD_HIDE},
	{ "talk",			do_channel_talk,	POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE|CMD_COMM},
	{ "tame",			do_tame,			POS_FIGHTING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "tactical",		do_tactical,		POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "test",			do_test,			POS_DEAD,		MAX_LEVEL,	LOG_ALWAYS,	CMD_HIDE},
	{ "throw",		do_throw,			POS_FIGHTING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "tick",			do_tick,			POS_DEAD,		MAX_LEVEL,	LOG_ALWAYS,	CMD_NONE},
	{ "title",		do_title,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "time",			do_time,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "timemode",		do_timemode,		POS_DEAD,		MAX_LEVEL-1,	LOG_NORMAL,	CMD_NONE},
	{ "train",		do_train,			POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "track",		do_track,			POS_STANDING,	 1,			LOG_NORMAL,	CMD_HIDE},
	{ "trip",			do_trip,			POS_FIGHTING,	-3,			LOG_NORMAL,	CMD_HIDE},
	{ "transfer",		do_transfer,		POS_DEAD,		MAX_LEVEL-2,	LOG_NORMAL,	CMD_NONE},
	{ "travel",		do_travel,		POS_STANDING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "trivia",		do_trivia,		POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},


	{ "up",			do_up,			POS_STANDING,	-8,			LOG_NORMAL,	CMD_IDLE|CMD_BERSERK},
	{ "unalias",		do_unalias,		POS_DEAD,		-3,			LOG_NORMAL,	CMD_NONE},
	{ "unlock",		do_unlock,		POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "usage",		do_usage,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},


	{ "vt100",		do_vt100,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "value",		do_value,			POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "version",		do_version,		POS_DEAD,		MAX_LEVEL,	LOG_NORMAL,	CMD_NONE},
	{ "victory",		do_victory_list,	POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "voice",		do_voice,			POS_RESTING,	 1,			LOG_NORMAL,	CMD_HIDE},


	{ "west",			do_west,			POS_STANDING,	-8,			LOG_NORMAL,	CMD_IDLE|CMD_BERSERK},
	{ "wake",			do_wake,			POS_SLEEPING,	-9,			LOG_NORMAL,	CMD_NONE},
	{ "wear",			do_wear,			POS_RESTING,	-8,			LOG_NORMAL,	CMD_NONE},
	{ "weather",		do_weather,		POS_RESTING,	 1,			LOG_NORMAL,	CMD_NONE},
	{ "where",		do_where,			POS_RESTING,	-8,			LOG_NORMAL,	CMD_NONE|CMD_BERSERK},
	{ "whirl",		do_whirl,			POS_FIGHTING,	-3,			LOG_NORMAL,	CMD_HIDE},
	{ "who",			do_who,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE|CMD_BERSERK},
	{ "wield",		do_wear,			POS_RESTING,	-8,			LOG_NORMAL,	CMD_HIDE},
	{ "wimpy",		do_wimpy,			POS_DEAD,		 1,			LOG_NORMAL,	CMD_BERSERK},
	{ "wisdom",		do_wisdom,		POS_DEAD,		 1,			LOG_NORMAL,	CMD_NONE},
	{ "wizhelp",		do_wizhelp,		POS_DEAD,      MAX_LEVEL-3,	LOG_NORMAL,	CMD_NONE},
	{ "wizlock",		do_wizlock,		POS_DEAD,		MAX_LEVEL,	LOG_ALWAYS,	CMD_NONE},
	{ "wizmap",		do_wizmap,		POS_DEAD,      MAX_LEVEL-2,	LOG_NORMAL,	CMD_NONE},


	{ "zap",			do_zap,			POS_STANDING,	 1,			LOG_NORMAL,	CMD_HIDE},


	{ "",			NULL,			POS_DEAD,	 	 0,			LOG_NORMAL,	CMD_HIDE}
};

long long get_game_usec( void )
{
	struct timeval last_time;

	push_call("get_game_usec()");

	gettimeofday(&last_time, NULL);

	pop_call();
	return((long long) last_time.tv_usec + 1000000LL * (long long) last_time.tv_sec);
}

/*
	The main entry point for executing commands.
	Can be recursively called from 'at', 'order', 'force'.
*/


void interpret( CHAR_DATA *ch, char *argument )
{
	char command[MAX_INPUT_LENGTH];
	char logline[MAX_INPUT_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	char pag_buf[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];

	int cmd = -1;
	int index;
	int leng;
	int trust;
	bool found = FALSE;
	long long last_time, delta;
	char *pt, *pti, *pto;
	bool auto_command;

	push_call_format("interpret(%s,%s)", ch->name, argument);

	strcpy(arg, argument);

	argument = arg;

	last_time = get_game_usec();

	if (!IS_NPC(ch) && IS_SET(ch->pcdata->interp, INTERP_AUTO))
	{
		REMOVE_BIT(ch->pcdata->interp, INTERP_AUTO);
		auto_command = TRUE;
	}
	else
	{
		auto_command = FALSE;
	}

	/*
		OLC stuff
	*/

	if (!IS_NPC(ch) && (ch->pcdata->editmode == MODE_REPEATCMD || ch->pcdata->tempmode == MODE_REPEATCMD))
	{
		(ch->pcdata->last_cmd) (ch, argument);

		pop_call();
		return;
	}

	if (cmd == -1)
	{
		if (ch->desc)
		{
			if (ch->desc->character == ch)
			{
				*ch->desc->incomm = '\0';
			}

			/*
				Let's not display commands in obj_progs, and doesn't hurt either
			*/

			if (!IS_NPC(ch) && IS_SET(ch->act, PLR_REPEAT))
			{
				if (ch->trust <= ch->level && !IS_SET(ch->pcdata->interp, INTERP_FORCE))
				{
					ch_printf_color(ch, "{078}[{178}%s{078}]\n\r", argument);
				}
			}

			if (!IS_NPC(ch)
			&& IS_SET(ch->pcdata->interp, INTERP_DUMP)
			&& auto_command == FALSE
			&& !IS_SET(ch->pcdata->interp, INTERP_FORCE)
			&& !IS_SET(ch->pcdata->interp, INTERP_PROG))
			{
				char jbuf[MAX_STRING_LENGTH];
				CHAR_DATA *fch;

				if (!strcasecmp(argument, "stop" ) )
				{
					send_to_char( "Dump stopped.\n\r", ch );
					REMOVE_BIT(ch->pcdata->interp, INTERP_DUMP);

					pop_call();
					return;
				}
				for (fch = ch->in_room->first_person ; fch ; fch = fch->next_in_room)
				{
					if (fch->position <= POS_SLEEPING)
					{
						continue;
					}
					if (ch != fch && blocking(fch, ch))
					{
						continue;
					}
					if (!can_understand(fch, ch, FALSE))
					{
						continue;
					}

					sprintf( jbuf, "%s dumps '%s'", ch->name, argument);
					ch_printf(fch, "%s%s\n\r", get_color_string(fch, COLOR_SPEACH, VT102_DIM), jbuf);
				}
				pop_call();
				return;
			}
		}

		/*
			Strip leading/tailing spaces.
		*/

		while (isspace(*argument) && *argument != '\0')
		{
			argument++;
		}

		if (*argument != '\0')
		{
			for (pt = argument + strlen(argument) - 1 ; *pt == ' ' ; pt--)
			{
				*pt = '\0';
			}
		}

		/*
			Send the scroll prompter out only if no inputs  -  Chaos 8/22/97
		*/

		if (ch->desc && IS_SET(CH(ch->desc)->pcdata->interp, INTERP_PAGE) && CH(ch->desc)->pcdata->page_buf != NULL)
		{
			REMOVE_BIT(CH(ch->desc)->pcdata->interp, INTERP_PAGE);

			leng = str_cpy_max( pag_buf, ansi_translate_text( ch, "{300}" ), MAX_BUFFER_LENGTH );
			leng = str_apd_max( pag_buf, CH(ch->desc)->pcdata->page_buf, leng, MAX_BUFFER_LENGTH);

			STRFREE (CH(ch->desc)->pcdata->page_buf );

			if (*argument == '\0')
			{
				send_to_char(pag_buf, ch);
			}
			else
			{
				send_to_char((const char*)ansi_translate_text(ch, "{088}{128}-----------------------------[Page Buffer Canceled]-----------------------------\n\r"), ch);
			}
		}

		if (*argument == '\0')
		{
			pop_call();
			return;
		}

		/*
			Add links to hyper-text helps
		*/

		if (!IS_NPC(ch) && IS_SET(ch->pcdata->interp, INTERP_HELP))
		{
			REMOVE_BIT(ch->pcdata->interp, INTERP_HELP);

			if (argument[1] == '\0' && argument[0] != '\0')
			{
				HELP_MENU_DATA *menu;
				HELP_MENU_DATA *prev_menu=NULL;
				HELP_MENU_DATA *menu2;
				HELP_DATA      *help;
				bool   foundh, foundh2;

				foundh	= FALSE;
				help		= ch->pcdata->last_help;
				for (menu = help->first_menu ; menu ; menu = menu->next)
				{
					if (menu->option == '-')
					{
						foundh = TRUE;
					}
					prev_menu = menu;
					if (prev_menu == NULL)
					{
						break;
					}
				}
				if (argument[0] == '-' && !foundh && ch->pcdata->prev_help != NULL)
				{
					send_to_char_color(ch->pcdata->prev_help->text, ch);
					ch->pcdata->last_help = ch->pcdata->prev_help;
					ch->pcdata->prev_help = NULL;
					SET_BIT(ch->pcdata->interp, INTERP_HELP);
					pop_call();
					return;
				}
				for (menu = help->first_menu ; menu ; menu = menu->next)
				{
					if (tolower(argument[0]) == menu->option)
					{
						send_to_char_color( menu->help->text, ch);
						foundh2	= FALSE;

						for (menu2 = menu->help->first_menu ; menu2 ; menu2 = menu2->next)
						{
							if (menu2->option == '-')
							{
								foundh2 = TRUE;
							}
						}
						if (!foundh2 && argument[0] != '-')
						{
							if (ch->pcdata->page_buf == NULL)
							{
								send_to_char_color("\n\r  {128}(-) {058}Return\n\r", ch);
							}
							else
							{
								leng = str_cpy_max(pag_buf, ch->pcdata->page_buf, MAX_BUFFER_LENGTH);
								leng = str_apd_max(pag_buf, ansi_translate_text(ch, "\n\r  {128}(-) {058}Return\n\r"), leng, MAX_BUFFER_LENGTH);
								ch->pcdata->page_buf = STRALLOC(pag_buf);
							}
						}
						if (argument[0] == '-')
						{
							ch->pcdata->prev_help = NULL;
						}
						else
						{
							ch->pcdata->prev_help = ch->pcdata->last_help;
						}
						ch->pcdata->last_help = menu->help;
						SET_BIT(ch->pcdata->interp, INTERP_HELP);

						pop_call();
						return;
					}
				}
				/* assume a menu option not listed is normal command and continue */
			}
		}

		/*
			Last Command  - Chaos 5/6/99
		*/

		if (!IS_NPC(ch) && !IS_SET(ch->pcdata->interp, INTERP_FORCE) && auto_command == FALSE && ch->pcdata->last_command != NULL)
		{
			char jbuf[MAX_STRING_LENGTH];

			if (!strcasecmp(argument, "stop") || !strcasecmp(argument, "quit"))
			{
				send_to_char( "Command stopped.\n\r", ch );
				STRFREE( ch->pcdata->last_command );
				ch->pcdata->last_command = NULL;

				pop_call();
				return;
			}
			strcpy( jbuf, ch->pcdata->last_command);
			strcat( jbuf, argument );
			strcpy( argument, jbuf );
			STRFREE( ch->pcdata->last_command );
			ch->pcdata->last_command = NULL;
		}
 
		/*
			Any command starting with numbers is a repeat command
		*/

		if (*argument >= '0' && *argument <= '9')
		{
			do_repeat( ch, argument);

			pop_call();
			return;
		}

		/*
			Added the '&' to multi-line commands  Chaos 12/7/93
		*/
		pti = argument;
		pto = command;

		for (pti = argument, pto = command ; *pti != ' ' && *pti != '\0' ; pto++, pti++)
		{
			*pto = *pti;
		}
		*pto = '\0';

		if (ch->desc && ch->desc->intop < MAX_INPUT_LENGTH-2)
		{
			if (strchr(argument, '&') != NULL && strcasecmp(command, "alias"))
			{
				process_command(ch, argument);

				pop_call();
				return;
			}
		}

		/*
			Implement freeze command.
		*/

		if (!IS_NPC(ch) && IS_SET(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_FROZEN))
		{
			send_to_char("You're totally frozen!\n\r", ch);
			pop_call();
			return;
		}

		/*
			Grab the command word.
			Special parsing so ' can be a command,
			also no spaces needed after punctuation.
		*/

		strcpy(logline, argument );

		if (!isalpha(*argument) && !isdigit(*argument))
		{
			*command = *argument;
			*(command+1) = '\0';
			argument++;
			while (isspace(*argument))
			{
				argument++;
			}
		}
		else
		{
			argument = one_argument(argument, command);
		}

		/*
			Look for command in command table.
		*/

		found = FALSE;
		trust = get_trust( ch );

		/*
			Look for alias first - Chaos 10/11/93
		*/

		if (ch->desc && auto_command == FALSE)
		{
			if (!IS_SET(CH(ch->desc)->pcdata->interp, INTERP_ALIAS)
			&&  !IS_SET(CH(ch->desc)->pcdata->interp, INTERP_PROG)
			&&  !IS_SET(CH(ch->desc)->pcdata->interp, INTERP_FORCE))
			{
				for (index = 0 ; index < MAX_ALIAS ; index++)
				{
					if (CH(ch->desc)->pcdata->alias[index] != NULL && CH(ch->desc)->pcdata->alias[index][0] != 0)
					{
						if (!strcasecmp(CH(ch->desc)->pcdata->alias_c[index], command))
						{
							SET_BIT(CH(ch->desc)->pcdata->interp, INTERP_ALIAS);

							buf[0] = 0;
							pti    = CH(ch->desc)->pcdata->alias[index];
							pto    = buf;

							while (*pti)
							{
								if (*pti == '%')
								{
									if (pto - buf + strlen(argument) >= MAX_INPUT_LENGTH - 2)
									{
										break;
									}
									pto += sprintf(pto, "%s", argument);
								}
								else
								{
									if (pto - buf >= MAX_INPUT_LENGTH - 2)
									{
										break;
									}
									*pto++ = *pti;
								}
								pti++;
							}
							*pto++ = 0;

							if (ch->desc && ch->desc->character == ch)
							{
								if (ch->desc->back_buf != NULL)
								{
									char buf2[MAX_INPUT_LENGTH];

									str_cpy_max( buf2, ch->desc->back_buf, MAX_INPUT_LENGTH );
									STRFREE(ch->desc->back_buf );
									str_cat_max( buf2, ch->desc->inbuf, MAX_INPUT_LENGTH );
									ch->desc->back_buf = STRALLOC( buf2 );
								}
								else
								{
									ch->desc->back_buf = STRALLOC(ch->desc->inbuf);
								}
								ch->desc->intop = 0;
								*ch->desc->inbuf = '\0';
							}
							preprocess_command(ch, buf);

							pop_call();
							return;
						}
					}
				}
			}
		}

		if ((cmd = find_command(command, trust)) != -1)
		{
			found = TRUE;
		}

		/*
			Berserk players can only "kill" or "murder"
		*/

		if (IS_AFFECTED(ch, AFF2_BERSERK) && !IS_IMMORTAL(ch))
		{
			if (!IS_NPC(ch) && ch->desc && !IS_SET(ch->pcdata->interp, INTERP_PROG) && !IS_SET(ch->pcdata->interp, INTERP_FORCE))
			{
				if (!found || !IS_SET(cmd_table[cmd].flags, CMD_BERSERK))
				{
					send_to_char("You're Berserk!  Kill!  Kill!  Kill!\n\r", ch);
					pop_call();
					return;
				}
			}
		}

		/*
			Arrested players are very limited as well. (Presto 02/97)
		*/

		if (!IS_NPC(ch) && IS_SET(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_ARRESTED))
		{
			if ((ch->pcdata->jailtime - (mud->current_time - ch->pcdata->jaildate)) <= 0)
			{
				auto_release(ch);
				pop_call();
				return;
			}

			if (!found)
			{
				send_to_char("You're in jail!\n\r", ch);
				pop_call();
				return;
			}

			if (strcasecmp(cmd_table[cmd].name, "say")
			&&  strcasecmp(cmd_table[cmd].name, "look")
			&&  strcasecmp(cmd_table[cmd].name, "quit")
			&&  strcasecmp(cmd_table[cmd].name, "refresh"))
			{
				send_to_char("You're in jail!\n\r", ch);
				pop_call();
				return;
			}
		}

		if (!IS_NPC(ch))
		{
			if (IS_SET(ch->act, PLR_AFK))
			{
				REMOVE_BIT(ch->act, PLR_AFK);
				act( "$n is no longer afk.", ch, NULL, NULL, TO_ROOM );
			}
		}
	}

	/*
		Log and snoop.
	*/

	if (cmd_table[cmd].log == LOG_NEVER)
	{
		strcpy(logline, cmd_table[cmd].name);
	}

	sprintf(mud->last_player_cmd, "%s: %s", ch->name, logline);

	if ((!IS_NPC(ch) && IS_SET(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_LOGGED)) || IS_SET(mud->flags, MUD_LOGALL) || cmd_table[cmd].log == LOG_ALWAYS)
	{
		log_printf("Log %s: %s", IS_NPC(ch) ? ch->short_descr : ch->name, logline);
	}
	else if (!IS_NPC(ch) && ch->pcdata && ch->pcdata->clan && IS_SET(ch->pcdata->clan->flags, CLANFLAG_COMMLOG) && IS_SET(cmd_table[cmd].flags, CMD_COMM))
	{
		log_printf("CLog %s: %s", IS_NPC(ch) ? ch->short_descr : ch->name, logline);
	}

	if (!found)
	{
		/* Check for object programs of unknown commands*/

		if (rprog_unknown_trigger(ch, command, argument))
		{
			pop_call();
			return;
		}

		if (oprog_unknown_trigger(ch, command, argument))
		{
			pop_call();
			return;
		}

		if (!IS_NPC(ch) && ch->desc && !IS_SET(ch->pcdata->interp, INTERP_PROG))
		{
			OBJ_DATA *obj;
			OBJ_PROG *prg;

			for (obj = ch->in_room->first_content ; obj ; obj = obj->next_content)
			{
				if (IS_SET(obj->pIndexData->oprogtypes, TRIG_ROOM_UNKNOWN))
				{
					for (prg = obj->pIndexData->first_oprog ; prg ; prg = prg->next)
					{
						if (prg->trigger == TRIG_ROOM_UNKNOWN  && !str_prefix(command, prg->unknown))
						{
							if (number_percent() <= prg->percentage)
							{
								start_object_program( ch, obj, prg, argument);
								pop_call();
								return;
							}
						}
					}
				}
			}

			for (obj = ch->first_carrying ; obj ; obj = obj->next_content)
			{
				if (obj->wear_loc != WEAR_NONE && IS_SET(obj->pIndexData->oprogtypes, TRIG_WEAR_UNKNOWN))
				{
					for (prg = obj->pIndexData->first_oprog ; prg ; prg = prg->next)
					{
						if (prg->trigger == TRIG_WEAR_UNKNOWN && !str_prefix(command, prg->unknown))
						{
							if (number_percent() <= prg->percentage)
							{
								start_object_program( ch, obj, prg, argument);
								pop_call();
								return;
							}
						}
					}
				}
				if (IS_SET(obj->pIndexData->oprogtypes, TRIG_UNKNOWN))
				{
					for (prg = obj->pIndexData->first_oprog ; prg ; prg = prg->next)
					{
						if (prg->trigger == TRIG_UNKNOWN && !str_prefix(command, prg->unknown))
						{
							if (number_percent() <= prg->percentage)
							{
								start_object_program( ch, obj, prg, argument);
								pop_call();
								return;
							}
						}
					}
				}
			}
		}

		if (!check_social(ch, command, argument) && strcasecmp(command, "!") && strcasecmp( command, "."))
		{
			ch_printf(ch, "Huh?  '%s' is not a command.\n\r", command);

			/*
				Pets limitations - Scandum
			*/

			if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM) && ch->master)
			{
				send_to_char( "Your pet can't do that.\n\r", ch->master);
				pop_call();
				return;
			}
		}
		pop_call();
		return;
	}

	/*
		Check for invalid combat
	*/

	if (ch->fighting != NULL)
	{
		if (ch->fighting->who->in_room != ch->in_room)
		{
			stop_fighting(ch,FALSE);
			update_pos( ch );
		}
	}

	/*
		Character not in position for command?
	*/

	if (ch->position < cmd_table[cmd].position)
	{
		switch( ch->position )
		{
			case POS_DEAD:
				send_to_char( "Lie still; you are DEAD.\n\r", ch );
				break;

			case POS_MORTAL:
			case POS_INCAP:
				send_to_char( "You are hurt far too bad for that.\n\r", ch );
				break;

			case POS_STUNNED:
				send_to_char( "You are too stunned to do that.\n\r", ch );
				break;

			case POS_SLEEPING:
				send_to_char( "In your dreams, or what?\n\r", ch );
				break;

			case POS_RESTING:
				send_to_char( "Nah... You feel too relaxed...\n\r", ch);
				break;

			case POS_SITTING:
				send_to_char( "Nah... You are too comfortable...\n\r", ch);
				break;

			case POS_FIGHTING:
				send_to_char( "No way!  You are still fighting!\n\r", ch);
				break;
		}
		pop_call();
		return;
	}

	/*
		Check for objects to respond to command in room
	*/

	if (!IS_NPC(ch) && ch->desc && !IS_SET(ch->pcdata->interp, INTERP_PROG) && !IS_SET(ch->pcdata->interp, INTERP_FORCE))
	{
		OBJ_DATA *obj;
		OBJ_PROG *prg;

		for (obj = ch->in_room->first_content ; obj ; obj = obj->next_content)
		{
			if (IS_SET(obj->pIndexData->oprogtypes, TRIG_ROOM_COMMAND))
			{
				for (prg = obj->pIndexData->first_oprog ; prg ; prg = prg->next)
				{
					if (prg->trigger == TRIG_ROOM_COMMAND && prg->cmd == cmd)
					{
						if (number_percent() <= prg->percentage)
						{
							start_object_program(ch, obj, prg, argument);
							pop_call();
							return;
						}
					}
				}
			}
		}
	}

	/*
		Decrement movement for exertion cmds
	*/

	if (cmd_gsn[cmd] != -1 && !IS_IMMORTAL(ch))
	{
		if (ch->move < skill_table[cmd_gsn[cmd]].cost)
		{
			send_to_char( "You are too tired to do that!\n\r", ch );

			pop_call();
			return;
		}
		else
		{
			ch->move -= skill_table[cmd_gsn[cmd]].cost;
		}
	}

	/* Check for object programs */

	if (!IS_NPC(ch) && ch->desc && !IS_SET(ch->pcdata->interp, INTERP_PROG) && !IS_SET(ch->pcdata->interp, INTERP_FORCE))
	{
		OBJ_DATA *obj;
		OBJ_PROG *prg;

		for (obj = ch->first_carrying ; obj ; obj = obj->next_content)
		{
			if (obj->wear_loc != WEAR_NONE && IS_SET(obj->pIndexData->oprogtypes, TRIG_WEAR_COMMAND))
			{
				for (prg = obj->pIndexData->first_oprog ; prg ; prg = prg->next)
				{
					if (prg->trigger == TRIG_WEAR_COMMAND && prg->cmd == cmd)
					{
						if (number_percent() <= prg->percentage)
						{
							start_object_program( ch, obj, prg, argument);
							pop_call();
							return;
						}
					}
				}
			}

			if (IS_SET(obj->pIndexData->oprogtypes, TRIG_COMMAND))
			{
				for (prg = obj->pIndexData->first_oprog ; prg ; prg = prg->next)
				{
					if (prg->trigger == TRIG_COMMAND && prg->cmd == cmd)
					{
						if (number_percent() <= prg->percentage)
						{
							start_object_program( ch, obj, prg, argument);
							pop_call();
							return;
						}
					}
				}
			}
		}
	}

	if (!IS_NPC(ch))
	{
		if (ch->pcdata->editmode != MODE_RESTRICTED)
		{
			ch->pcdata->last_cmd = *cmd_table[cmd].do_fun;
		}

		if (IS_SET(cmd_table[cmd].flags, CMD_IDLE))
		{
			ch->pcdata->idle = 0;
		}

		if (ch->pcdata->editmode == MODE_RESTRICTED)
		{
			if (IS_SET(cmd_table[cmd].flags, CMD_QUIT))
			{
				ch_printf(ch, "You cannot use quit commands while editing.\n\r");
				pop_call();
				return;
			}

			if (IS_SET(cmd_table[cmd].flags, CMD_EDIT))
			{
				ch_printf(ch, "You cannot use edit commands while editing.\n\r");
				pop_call();
				return;
			}
		}
	}

	if (rprog_command_trigger(ch, cmd_table[cmd].name, argument) == FALSE)
	{
		if (oprog_command_trigger(ch, cmd_table[cmd].name, argument) == FALSE)
		{
			(*cmd_table[cmd].do_fun) (ch, argument);
		}
	}

	if (!IS_SET(cmd_table[cmd].flags, CMD_QUIT) && ch != NULL && !IS_NPC(ch))
	{
		if (IS_SET(ch->act, PLR_WIZTIME))
		{
			delta = get_game_usec() - last_time;
			ch_printf(ch, "(%lld usec to execute command)\n\r", delta);
		}
	}

	pop_call();
	return;
}



/*
	Return true if an argument is completely numeric.
*/

bool is_number( char *argument )
{
	push_call("is_number(%p)",argument);

	if (*argument == '\0')
	{
		pop_call();
		return FALSE;
	}

	for ( ; *argument != '\0'; argument++ )
	{
		if (!isdigit(*argument) && (*argument != '-') && (*argument != '+'))
		{
			pop_call();
			return FALSE;
		}
	}
	pop_call();
	return TRUE;
}


bool is_number_argument( char *argument )
{
	push_call("is_number_argument(%p)",argument);

	if (*argument != '\0')
	{
		for ( ; *argument != '\0'; argument++ )
		{
			if (!isdigit(*argument) && *argument != '-' && *argument != '+' && *argument != '.')
			{
				pop_call();
				return FALSE;
			}
		}
	}
	pop_call();
	return TRUE;
}


/*
	Given a string like 14.foo, Return 14 and 'foo'
*/

int number_argument( char *argument, char *arg )
{
	char *pdot;
	int number;

	push_call("number_argument(%p,%p)",argument,arg);

	if ((pdot = strstr(argument, ".")) != NULL)
	{
		*pdot	= '\0';
		number	= atol(argument);
		*pdot	= '.';
		strcpy(arg, pdot+1);

		pop_call();
		return number;
	}
	strcpy(arg, argument);

	pop_call();
	return 1;
}

/*
	Pick off one argument from a string and Return the rest.
	Understands quotes, lowercases.
*/

char *one_argument( char *argument, char *arg_first )
{
	char cEnd;

	push_call("one_argument(%p,%p)",argument,arg_first);

	if (argument == NULL)
	{
		log_string("one_argument: argument == NULL");
		dump_stack();
		pop_call();
		return NULL;
	}

	while (*argument == ' ' || *argument == '\n' || *argument == '\r' || *argument == 27)
	{
    		argument++;
	}

	cEnd = ' ';

	if (*argument == '\'' || *argument == '"')
	{
		cEnd = *argument++;
	}

	while (*argument != '\0' && *argument != '\n' && *argument != '\r')
	{
		if (*argument == cEnd)
		{
			argument++;
			break;
		}

		*arg_first = tolower(*argument);

		arg_first++;
		argument++;
	}
	*arg_first = '\0';

	while (*argument == ' ' || *argument == '\n' || *argument == '\r' || *argument == 27)
	{
		argument++;
	}
	pop_call();
	return argument;
}


char *one_argument_nolower( char *argument, char *arg_first )
{
	char cEnd;

	push_call("one_argument(%p,%p)",argument,arg_first);

	if (argument == NULL)
	{
		log_string("one_argument_nolower(), argument is NULL");
		pop_call();
		return NULL;
	}

	while ( *argument==' ' || *argument=='\n' || *argument=='\r' || *argument==27)
	{
    		argument++;
	}
	cEnd = ' ';
	if ( *argument == '\'' || *argument == '"' )
	{
		cEnd = *argument++;
	}

	while ( *argument != '\0' && *argument != '\n' && *argument != '\r' )
	{
		if ( *argument == cEnd )
		{
			argument++;
			break;
		}

		*arg_first = *argument;

		arg_first++;
		argument++;
	}
	*arg_first = '\0';

	while ( *argument==' ' || *argument=='\n' || *argument=='\r' || *argument==27)
	{
		argument++;
	}
	pop_call();
	return argument;
}

void process_command( CHAR_DATA *ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH], command[MAX_INPUT_LENGTH];
	char *pti, *pto;
	bool found;

	push_call("process_command(%p,%p)",ch,argument);

	found = FALSE;

	pti = argument;
	pto = command;

	while (*pti != 0)
	{
		if (*pti == '&')
		{
			if (found == FALSE)
			{
				found = TRUE;
				*pto  = 0;
				pto   = buf;   /* Switch recording medium */
			}
			else
			{
				*pto++ = '\n';
			}
			pti++;
		}
		else
		{
			*pto++ = *pti++;
		}
	}

	if (found)
	{
		*pto++ = '\n';
	}

	*pto = 0;

	if (found)
	{
		/* Add buf to the inbuf */

		if (*ch->desc->inbuf == 0)
		{
			ch->desc->intop = str_cpy_max(ch->desc->inbuf, buf, MAX_INPUT_LENGTH);
		}
		else
		{
			if (ch->desc->inbuf[ch->desc->intop-1] != '\n')
			{
				ch->desc->intop = str_apd_max(ch->desc->inbuf, "\n", ch->desc->intop, MAX_INPUT_LENGTH);
				ch->desc->intop = str_apd_max(ch->desc->inbuf, buf, ch->desc->intop, MAX_INPUT_LENGTH);
			}
			else
			{
				ch->desc->intop = str_apd_max(ch->desc->inbuf, buf, ch->desc->intop, MAX_INPUT_LENGTH);
			}
		}
	}

	interpret(ch, command);

	pop_call();
	return;
}

void preprocess_command( CHAR_DATA *ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH], command[MAX_INPUT_LENGTH], tail[MAX_INPUT_LENGTH];
	char *pti, *pto;
	bool found;

	push_call("preprocess_command(%p,%p)",ch,argument);

	found = FALSE;
	pti   = argument;
	pto   = command;

	while (*pti)
	{
		if (*pti == '&')
		{
			if (found == FALSE)
			{
				found = TRUE;
				*pto = 0;
				pto = buf;   /* Switch recording medium */
			}
			else
			{
				*pto++ = '\n';
			}
			pti++;
		}
		else
		{
			*pto++ = *pti++;
		}
	}

	if (found)
	{
		*pto++ = '\n';
	}
	*pto = 0;

	if (found)
	{
		if (*ch->desc->inbuf == 0)
		{
			ch->desc->intop = str_cpy_max(ch->desc->inbuf, buf, MAX_INPUT_LENGTH);
		}
		else
		{
			strcpy(tail, ch->desc->inbuf);
			ch->desc->intop = str_cpy_max(ch->desc->inbuf, buf, MAX_INPUT_LENGTH);
			ch->desc->intop = str_apd_max(ch->desc->inbuf, "\n", ch->desc->intop, MAX_INPUT_LENGTH);
			ch->desc->intop = str_apd_max(ch->desc->inbuf, tail, ch->desc->intop, MAX_INPUT_LENGTH);
		}
	}
	interpret(ch, command);

	pop_call();
	return;
}


void start_object_program ( CHAR_DATA * ch, OBJ_DATA *obj, OBJ_PROG *prg, char *argument)
{
	push_call("start_object_program(%p,%p,%p,%p)",ch,obj,prg,argument);

	if (ch->desc == NULL || ch->desc->character != ch || IS_NPC(ch))
	{
		pop_call();
		return;
	}

	/*
		No lock-ups here including wait oriented char lock-ups
	*/

	if (IS_SET(ch->pcdata->interp, INTERP_PROG) || ch->wait > 0)
	{
		pop_call();
		return;
	}

	if (ch->desc->connected == CON_EDITING || IS_SET(ch->pcdata->interp, INTERP_DUMP))
	{
		pop_call();
		return;
	}

	SET_BIT(ch->pcdata->interp, INTERP_PROG);

	object_program( ch, obj, prg, argument );

	REMOVE_BIT(ch->pcdata->interp, INTERP_PROG);

	pop_call();
	return;
}

void check_object_prog( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_PROG *prg, char *argument, int if_val )
{
	push_call("check_object_prog(%p,%p,%p,%p,%p)",ch,obj,prg,argument,if_val);

	switch( prg->if_symbol )
	{
		case '<':
			if (if_val < prg->if_value)
			{
				object_program( ch, obj, prg->true, argument );
			}
			else
			{
				object_program( ch, obj, prg->false , argument);
			}
			pop_call();
			return;

		case '>':
			if (if_val > prg->if_value)
			{
				object_program( ch, obj, prg->true, argument );
			}
			else
			{
				object_program( ch, obj, prg->false , argument);
			}
			pop_call();
			return;

		case '=':
			if (if_val == prg->if_value)
			{
				object_program( ch, obj, prg->true, argument );
			}
			else
			{
				object_program( ch, obj, prg->false , argument);
			}
			pop_call();
			return;

		case '!':
			if (if_val != prg->if_value)
			{
				object_program( ch, obj, prg->true, argument );
			}
			else
			{
				object_program( ch, obj, prg->false , argument);
			}
			pop_call();
			return;
	}
	pop_call();
	return;
}

void object_program ( CHAR_DATA * ch, OBJ_DATA *obj, OBJ_PROG *prg, char *argument)
{
	OBJ_DATA *obj2;
	int if_val;
	int cnt;
	char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
	char *pt, pb[2];

	char * const he_she		[] = { "it",  "he",  "she" };
	char * const him_her	[] = { "it",  "him", "her" };
	char * const his_her	[] = { "its", "his", "her" };

	push_call("object_program(%p,%p,%p,%p)",ch,obj,prg,argument);

	if (prg == NULL || obj == NULL || ch == NULL)
	{
		pop_call();
		return;
	}

	if (ch->desc == NULL || ch->desc->character != ch)
	{
		pop_call();
		return;
	}

	if_val = 0;

	switch (prg->obj_command)
	{
		case OPROG_ECHO:
			if (!str_suffix("\n\r", prg->argument))
			{
				ch_printf_color(ch, "%s", ansi_justify(prg->argument, get_page_width(ch)));
			}
			else
			{
				ch_printf_color(ch, "%s\n\r", ansi_justify(prg->argument, get_page_width(ch)));
			}
			object_program(ch, obj, prg->true , argument);
			pop_call();
			return;

		case OPROG_IF_HAS_OBJECT:
			for (obj2 = ch->first_carrying ; obj2 ; obj2 = obj2->next_content)
			{
				if (obj2->pIndexData->vnum == prg->if_value)
				{
					break;
				}
			}
			if (obj2)
			{
				object_program(ch, obj, prg->true, argument);
			}
			else
			{
				object_program(ch, obj, prg->false, argument);
			}
			pop_call();
			return;

		case OPROG_JUNK:
			junk_obj(obj);
			pop_call();
			return;

		case OPROG_GOD_COMMAND:
		case OPROG_GOD_ARGUMENT:
		case OPROG_COMMAND:
		case OPROG_ARGUMENT:
			pb[1]	= '\0';
			buf[0]	= '\0';

			for (pt = prg->argument ; *pt != '\0' ; pt++)
			{
				if (*pt != '$')
				{
					*pb = *pt;
					strcat( buf, pb );
				}
				else
				{
					pt++;
					switch (*pt)
					{
						case 'f':
							if (who_fighting(ch))
							{
								one_argument_nolower(ch->fighting->who->name, buf2);
								strcat(buf, buf2);
							}
							break;
						case 'F':
							if (who_fighting(ch))
							{
								strcat(buf, get_name(ch->fighting->who));
							}
							break;
						case 'i': strcat(buf, ch->name);					break;
						case ' ':	cat_sprintf(buf, "%s ", ch->name);			break;
						case 'I':	strcat(buf, get_name(ch));				break;
						case 'e':	strcat(buf, he_she [URANGE(0, ch->sex, 2)]);	break;
						case 'm': strcat(buf, him_her[URANGE(0, ch->sex, 2)]);	break;
						case 's':	strcat(buf, his_her[URANGE(0, ch->sex, 2)]);	break;
						case 'o': strcat(buf, obj->short_descr);			break;
						case '/': strcat(buf, "\n\r");					break;
						case '$':	strcat(buf, "$" );						break;
						default:
							cat_sprintf(buf, "%s ", ch->name);
							log_printf( "[%u] Bad Oprog $var: %c", obj->pIndexData->vnum, *pt);
							break;
					}
				}
 			}

			switch (prg->obj_command)
			{
				case OPROG_GOD_COMMAND:
				case OPROG_GOD_ARGUMENT:
					ch->trust = MAX_LEVEL-1;
					break;
			}

			for (buf2[0] = '\0', pt = buf ; *pt != '\0' ; pt++)
			{
				if (*pt == '\n')
				{
					continue;
				}
				if (*pt != '&' && *pt != '\r')
				{
					*pb = *pt;
					strcat( buf2, pb);
				}
				else if (*(pt+1) != '\0')
				{
					interpret(ch, buf2);
					buf2[0] = '\0';
				}
			}

			switch (prg->obj_command)
			{
				case OPROG_GOD_ARGUMENT:
				case OPROG_ARGUMENT:
					cat_sprintf(buf2, " %s", argument);
					break;
			}

			interpret(ch, buf2);

			ch->trust = ch->level;

			object_program(ch, obj, prg->true, argument);

			pop_call();
			return;

		case OPROG_APPLY:
			switch (prg->if_check)
			{
				case 1:
					ch->hit = UMIN(ch->max_hit, ch->hit + prg->if_value);

					if (ch->hit < 0 && ch->fighting == NULL)
					{
						ch->trust = ch->level;
						raw_kill( ch );
					}
					break;
				case 2:
					ch->move = URANGE(0, ch->move + prg->if_value, ch->max_move);
					break;
				case 3:
					ch->mana = URANGE(0, ch->mana + prg->if_value, ch->max_mana);
					break;
				case 4:
					ch->alignment = URANGE(-1000, ch->alignment + prg->if_value, 1000);

					check_zap(ch, TRUE);
			}
			object_program(ch, obj, prg->true, argument);
			break;

		case OPROG_QUEST_ADD:
			cnt  = get_quest_bits( obj->obj_quest, prg->quest_offset, prg->quest_bits);
			cnt += prg->if_value;
			set_quest_bits( &obj->obj_quest, prg->quest_offset, prg->quest_bits, cnt);

			SET_BIT(obj->extra_flags, ITEM_MODIFIED);

			object_program( ch, obj, prg->true, argument );
			break;

		case OPROG_QUEST_SET:
			set_quest_bits( &obj->obj_quest, prg->quest_offset, prg->quest_bits, prg->if_value);

			SET_BIT(obj->extra_flags, ITEM_MODIFIED);

			object_program( ch, obj, prg->true, argument );
			break;

		case OPROG_IF:
			switch( prg->if_check )
			{
				case OIF_USER_CLASS:
					if_val = ch->class;
					break;
				case OIF_USER_POSITION:
					if_val = ch->position;
					break;
				case OIF_USER_GOLD:
					if_val = ch->gold;
					break;
				case OIF_USER_AREA:
					if_val = ch->in_room->area->low_r_vnum;
					break;
				case OIF_USER_ROOM_NUM:
					if_val = ch->in_room->vnum;
					break;
				case OIF_RANDOM_PERCENT:
					if_val = number_percent();
					break;
				case OIF_USER_WHICH_GOD:
					if_val = which_god(ch);
					break;
				case OIF_USER_ALIGNMENT:
					if_val = ch->alignment;
					break;
				case OIF_USER_LEVEL:
					if_val = ch->level;
					break;
				case OIF_USER_RACE:
					if_val = ch->race;
					break;
				case OIF_USER_SEX:
					if_val = URANGE(0, ch->sex, 2);
					break;
				case OIF_WEAR_LOC:
					if_val = obj->wear_loc;
					break;
				case OIF_USER_PERCENT_MOVE:
					if_val = ch->move*100/UMAX(1, ch->max_move);
					break;
				case OIF_USER_PERCENT_HITPT:
					if_val = ch->hit*100/UMAX(1, ch->max_hit);
					break;
				case OIF_USER_PERCENT_MANA:
					if_val = ch->mana*100/UMAX(1, ch->max_mana);
					break;
				case OIF_USER_SECTOR:
					if_val = ch->in_room->sector_type;
					break;
				case OIF_TIME_OF_DAY:
					if_val = mud->time_info->hour;
					break;
				case OIF_WEATHER:
					if_val = (!IS_OUTSIDE(ch) || NO_WEATHER_SECT(ch->in_room->sector_type)) ? -1 : ch->in_room->area->weather_info->sky;
					break;
				case OIF_USER_FIGHTING:
					if_val = (who_fighting(ch) != NULL);
					break;
			}
			check_object_prog(ch, obj, prg, argument, if_val);
			break;

		case OPROG_OBJECT_QUEST_IF:
			if_val = get_quest_bits( obj->obj_quest, prg->quest_offset, prg->quest_bits );

			check_object_prog(ch, obj, prg, argument, if_val);
			break;

		case OPROG_PLAYER_QUEST_IF:
			if_val = get_quest_bits(ch->pcdata->quest[obj->pIndexData->area->low_r_vnum/100], prg->quest_offset, prg->quest_bits );

			check_object_prog(ch, obj, prg, argument, if_val);
			break;

		default:
			log_printf("object_progam, unknown trigger: %d", prg->obj_command);
			break;
	}
	pop_call();
	return;
}


void do_a( CHAR_DATA *ch, char *argument )
{
	push_call("do_a(%p,%p)",ch,argument);

	send_to_char( "Ah, what?\n\r", ch );

	pop_call();
	return;
}

