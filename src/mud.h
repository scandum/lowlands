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

#ifndef FULL
#define FULL

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <zlib.h>



#define DEVELOPER_MAIL	"emud@sotmud.net"
#define ADMIN_MAIL		"emud@sotmud.net"

#define __GLIBC__2__

#if defined(TRADITIONAL)
	#define const
	#define args(list)              ()
	#define DECLARE_DO_FUN(fun)     void fun()
	#define DECLARE_SPEC_FUN(fun)   bool fun()
	#define DECLARE_SPELL_FUN(fun)  void fun()
#else
	#define args(list)              list
	#define DECLARE_DO_FUN(fun)     DO_FUN    fun
	#define DECLARE_SPEC_FUN(fun)   SPEC_FUN  fun
	#define DECLARE_SPELL_FUN(fun)  SPELL_FUN fun
#endif

#define BV00		(0   <<  0)
#define BV01		(1   <<  0)
#define BV02		(1   <<  1)
#define BV03		(1   <<  2)
#define BV04		(1   <<  3)
#define BV05		(1   <<  4)
#define BV06		(1   <<  5)
#define BV07		(1   <<  6)
#define BV08		(1   <<  7)
#define BV09		(1   <<  8)
#define BV10		(1   <<  9)
#define BV11		(1   << 10)
#define BV12		(1   << 11)
#define BV13		(1   << 12)
#define BV14		(1   << 13)
#define BV15		(1   << 14)
#define BV16		(1   << 15)
#define BV17		(1   << 16)
#define BV18		(1   << 17)
#define BV19		(1   << 18)
#define BV20		(1   << 19)
#define BV21		(1   << 20)
#define BV22		(1   << 21)
#define BV23		(1   << 22)
#define BV24		(1   << 23)
#define BV25		(1   << 24)
#define BV26		(1   << 25)
#define BV27		(1   << 26)
#define BV28		(1   << 27)
#define BV29		(1   << 28)
#define BV30		(1   << 29)
#define BV31		(1   << 30)
#define BV32		(1LL << 31)
#define BV33		(1LL << 32)
#define BV34		(1LL << 33)
#define BV35		(1LL << 34)
#define BV36		(1LL << 35)
#define BV37		(1LL << 36)
#define BV38		(1LL << 37)
#define BV39		(1LL << 38)


#define MAX_OBJ_REF                       20000
#define MAX_STRING_LENGTH                 60000
#define MAX_INPUT_LENGTH                   2000
#define COMPRESS_BUF_SIZE                 60000
#define MAX_QUEST_BYTES                      16
#define MAX_NAME_LENGTH                      12
#define MAX_HISCORE                          50
#define MAX_ALIAS                            60
#define MAX_KILL_TRACK                       10
#define MAX_MUDPROG_NEST                     10
#define MAX_LAST_LEFT                         5
#define MAX_BUFFER_LENGTH                 50000
#define MAX_OBJECTS_IN_CONTAINER             75
#define MAX_OBJECTS_IN_ROOM                 100
#define MAX_PK_ATTACKS                       10
#define MAX_STAT                             51
#define MAX_SCROLL_BUF                     1000
#define DESCRIPTOR_TIMEOUT   60 * 60 * 24 * 365
#define APPEAR_MAX                            6

#define COLOR_NO_CHANGE                      -1
#define COLOR_TEXT                            0
#define COLOR_TOP_STAT                        1
#define COLOR_BOT_STAT                        2
#define COLOR_SCORE                           3
#define COLOR_YOU_ARE_HIT                     4
#define COLOR_YOU_HIT                         5
#define COLOR_PROMPT                          6
#define COLOR_EXITS                           7
#define COLOR_PARTY_HIT                       8
#define COLOR_SPEACH                          9
#define COLOR_OBJECTS                        10
#define COLOR_MOBILES                        11
#define COLOR_TACTICAL                       12
#define COLOR_TACT_PARTY                     13
#define COLOR_TACT_ENEMY                     14
#define COLOR_TACT_NEUTRAL                   15
#define COLOR_CHAT                           16
#define COLOR_TALK                           17
#define COLOR_PLAN                           18
#define COLOR_MAX                            19

#define VT102_DIM                             0
#define VT102_BOLD                            1
#define VT102_UNDERLINE                       4
#define VT102_FLASHING                        5
#define VT102_REVERSE                         7

#define VT100_FAST                         BV01
#define VT100_BEEP                         BV02
#define VT100_HIGHLIGHT                    BV03
#define VT100_ECMA48                       BV04
#define VT100_BOLD                         BV05
#define VT100_UNDERLINE                    BV06
#define VT100_FLASH                        BV07
#define VT100_REVERSE                      BV08
#define VT100_ANSI                         BV09
#define VT100_DRAW                         BV10
#define VT100_INTERFACE                    BV11
#define VT100_REFRESH                      BV12

#define TACTICAL_TOP                       BV01
#define TACTICAL_INDEX                     BV02
#define TACTICAL_EXPTNL                    BV03

#define FALSE                                 0
#define TRUE                                  1
#define QUIET                                 2
#define VERBOSE                               3

/*
	Typedefs
*/

typedef short int                 sh_int;
typedef unsigned char             bool;

typedef struct mud_data           MUD_DATA;
typedef struct affect_data        AFFECT_DATA;
typedef struct area_data          AREA_DATA;
typedef struct char_data          CHAR_DATA;
typedef struct descriptor_data    DESCRIPTOR_DATA;
typedef struct exit_data          EXIT_DATA;
typedef struct extra_descr_data   EXTRA_DESCR_DATA;
typedef struct help_data          HELP_DATA;
typedef struct mob_index_data     MOB_INDEX_DATA;
typedef struct note_data          NOTE_DATA;
typedef struct topic_data         TOPIC_DATA;
typedef struct obj_data           OBJ_DATA;
typedef struct poison_data        POISON_DATA;
typedef struct bitvector_type     BITVECTOR_DATA;
typedef struct obj_index_data     OBJ_INDEX_DATA;
typedef struct pc_data            PC_DATA;
typedef struct crime_data         CRIME_DATA;
typedef struct file_data          FILE_DATA;
typedef struct bounty_data        BOUNTY_DATA;
typedef struct help_menu_data     HELP_MENU_DATA;
typedef struct obj_prog           OBJ_PROG;
typedef struct npc_data           NPC_DATA;
typedef struct mud_prog_token     MP_TOKEN;
typedef struct reset_data         RESET_DATA;
typedef struct room_index_data    ROOM_INDEX_DATA;
typedef struct shop_data          SHOP_DATA;
typedef struct time_info_data     TIME_INFO_DATA;
typedef struct weather_data       WEATHER_DATA;
typedef struct mud_prog_data      MUD_PROG;
typedef struct player_game        PLAYER_GAME;
typedef struct usage_data         USAGE_DATA;
typedef struct castle_data        CASTLE_DATA;
typedef struct pvnum_data         PVNUM_DATA;
typedef struct ban_data           BAN_DATA;
typedef struct trivia_data        TRIVIA_DATA;
typedef struct tactical_map       TACTICAL_MAP;
typedef struct object_reference   OBJ_REFERENCE;
typedef struct clan_data          CLAN_DATA;
typedef struct editor_data        EDITOR_DATA;
typedef struct fighting_data      FIGHT_DATA;
typedef struct pet_data           PET_DATA;
typedef struct hiscore_data       HISCORE_DATA;
typedef struct hiscore_list       HISCORE_LIST;
typedef struct obj_ref_hash       OBJ_REF_HASH;
typedef struct room_timer_data    ROOM_TIMER_DATA;
typedef struct junk_data          JUNK_DATA;
typedef struct math_data          MATH_DATA;
typedef struct mth_data           MTH_DATA;

/*
	Function typedefs.
*/

typedef void   DO_FUN             args((CHAR_DATA *ch, char *argument));
typedef bool   SPEC_FUN           args((CHAR_DATA *ch));
typedef void   SPELL_FUN          args((int sn, int level, CHAR_DATA *ch, void *vo, int target));


/*
	Game parameters.
	Increase the max'es if you add more of something.
	Adjust the pulse numbers to suit yourself.
*/

#define MAX_EXP_WORTH                   1000000
#define MIN_EXP_WORTH                        75

/*
	this number comes really exact, if you specify more than needed
	a crash is most likely. 	Manwe 07/10/1999
*/

#define MAX_SKILL                           269
#define MAX_BITVECTOR                       460
#define MAX_BODY                             21
#define MAX_CLASS                             8
#define MAX_RACE                             16
#define MAX_GOD                               3
#define MAX_LANGUAGE                         15
#define MAX_RACE_LANGUAGE                     8
#define MAX_LEVEL                            99
#define MAX_PVNUM                         12000
#define MAX_AREA                           1000
#define MAX_VNUM                 100 * MAX_AREA
#define MAX_TOPIC                             7
#define MAX_LINKPERPORT                     750
#define LEVEL_HERO                MAX_LEVEL - 3
#define LEVEL_IMMORTAL            MAX_LEVEL - 2

#define	PULSE_PER_SECOND                     10
#define	PULSE_AGGRESSIVE   1 * PULSE_PER_SECOND / 2
#define	PULSE_MOBILE       2 * PULSE_PER_SECOND
#define	PULSE_VIOLENCE     4 * PULSE_PER_SECOND
#define	PULSE_PROGRAM      4 * PULSE_PER_SECOND
#define	PULSE_CHARSAVE  1800 * PULSE_PER_SECOND
#define PULSE_MSDP         1 * PULSE_PER_SECOND
#define	PULSE_TICK         1 * PULSE_PER_SECOND * 60
#define	PULSE_AREA         1 * PULSE_PER_SECOND * 60
#define	PULSE_SHOPS       10 * PULSE_PER_SECOND * 60
#define	PULSE_AREASAVE    20 * PULSE_PER_SECOND * 60

#define	OBJ_SAC_TIME                         25

#define AREA_VERSION                          4
#define PLAYER_VERSION                        1

#define MOST_TRIVIA			 0
#define MOST_MORPH			 1
#define MOST_REINCARNATE		 2
#define MOST_KILL_EN		 3
#define MOST_KILL_PC		 4
#define MOST_KILL_NPC		 5
#define MOST_KILL_BY_NPC		 6
#define MOST_KILL_BY_EN		 7
#define MOST_KILL_BY_PC		 8
#define MOST_HP			 9
#define MOST_MANA			10
#define MOST_MOVE			11
#define MOST_AC			12
#define MOST_DR			13
#define MOST_HR			14
#define MOST_CASTLE			15
#define MOST_HOURS			16
#define MOST_SAVE			17
#define MOST_MOST			18


/*
	some main variables, initialized in main()
*/

#define MUD_EMUD_BOOTING		BV01
#define MUD_EMUD_DOWN		BV02
#define MUD_EMUD_REBOOT		BV03
#define MUD_EMUD_COPYOVER	BV04
#define MUD_EMUD_CRASH		BV05
#define MUD_EMUD_BOOTDB		BV06
#define MUD_EMUD_REALGAME	BV07
#define MUD_WIZLOCK			BV08
#define MUD_CLANRENT		BV09
#define MUD_DUALFLIP		BV10
#define MUD_SKIPOUTPUT		BV11
#define MUD_EQAFFECTING		BV12
#define MUD_LOGALL			BV13
#define MUD_PURGER			BV14
#define MUD_STRAIGHTNUMBERS	BV15

#define TIMER_AREA_SAVE		 0
#define TIMER_SHOP_UPD		 1
#define TIMER_CHAR_UPD		 2
#define TIMER_AREA_UPD		 3
#define TIMER_WEATHER_UPD	 4
#define TIMER_OBJ_UPD		 5
#define TIMER_CHAR_SAVE		 6
#define TIMER_VIOL_UPD		 7
#define TIMER_OBJ_PROG		 8
#define TIMER_MOB_PROG		 9
#define TIMER_MOB_UPD		10
#define TIMER_MSDP            11
#define TIMER_AGGR_UPD		12
#define TIMER_PURGE			13
#define TIMER_TACTICAL_UPD	14
#define TIMER_SCAN_DESC		15
#define TIMER_PROCESS_INPUT	16
#define TIMER_PROCESS_OUTPUT	17
#define TIMER_MAXTIMER		18

#define CMD_NONE              BV00
#define CMD_HIDE              BV01
#define CMD_IDLE              BV02
#define CMD_BERSERK           BV03
#define CMD_QUIT              BV04
#define CMD_COMM              BV05
#define CMD_EDIT              BV06

/*
	This structure holds most of the global variables - Scandum 28-04-2003
*/

struct mud_data
{
	DESCRIPTOR_DATA     * f_desc;
	DESCRIPTOR_DATA     * l_desc;
	PLAYER_GAME         * f_player;
	PLAYER_GAME         * l_player;
	AREA_DATA           * f_area;
	AREA_DATA           * l_area;
	CHAR_DATA           * f_char;
	CHAR_DATA           * l_char;
	SHOP_DATA           * f_shop;
	SHOP_DATA           * l_shop;
	OBJ_DATA            * f_obj;
	OBJ_DATA            * l_obj;
	CLAN_DATA           * f_clan;
	CLAN_DATA           * l_clan;
	JUNK_DATA           * f_junk;
	JUNK_DATA           * l_junk;
	ROOM_TIMER_DATA     * f_room_timer;
	ROOM_TIMER_DATA     * l_room_timer;
	FIGHT_DATA          * f_fight;
	FIGHT_DATA          * l_fight;
	PET_DATA            * f_pet;
	PET_DATA            * l_pet;
	NOTE_DATA           * f_note;
	NOTE_DATA           * l_note;
	BOUNTY_DATA         * f_bounty;
	BOUNTY_DATA         * l_bounty;
	BAN_DATA            * f_ban;
	BAN_DATA            * l_ban;
	BAN_DATA            * f_nban;
	BAN_DATA            * l_nban;
	TRIVIA_DATA         * f_trivia;
	TRIVIA_DATA         * l_trivia;
	FILE_DATA           * f_open_file;
	FILE_DATA           * l_open_file;
	MATH_DATA           * f_math;
	MATH_DATA           * l_math;
	DESCRIPTOR_DATA     * update_gld;
	CHAR_DATA           * update_wch;
	CHAR_DATA           * update_ich;
	CHAR_DATA           * update_rch;
	OBJ_DATA            * update_obj;
	CHAR_DATA           * mp_group_greeter;
	CHAR_DATA           * mp_group_greeted;
	HISCORE_LIST        * high_scores[MOST_MOST];
	TIME_INFO_DATA      * time_info;
	USAGE_DATA          * usage;
	TACTICAL_MAP        * tactical;
	struct tm             time;
	time_t                current_time;
	long long             total_io_bytes;
	long long             total_io_ticks;
	long long             total_io_delay;
	long long             total_io_exec;
	int                   bitvector_ref[26];
	int                   command_ref[26];
	int                   social_ref[26];
	time_t                boot_time;
	int                   port;
	int                   control;
	int                   flags;
	int                   msdp_table_size;
	int                   mudprog_nest;
	int                   total_mob;
	int                   total_obj;
	int                   total_plr;
	int                   total_desc;
	int                   top_exit;
	int                   top_affect;
	int                   top_area;
	int                   top_ed;
	int                   top_help;
	int                   top_mob_index;
	int                   top_obj_index;
	int                   top_room;
	int                   top_reset;
	int                   top_shop;
	int                   top_mprog;
	int                   top_oprog;
	int                   top_mtrig;
	bool                  sunlight;
	char                * last_player_cmd;
	unsigned char       * mccp_buf;
	int                   mccp_len;
};

/*
	Time and weather stuff.
*/

#define SUN_DARK			0
#define SUN_RISE			1
#define SUN_LIGHT			2
#define SUN_SET			3

#define SKY_CLOUDLESS		0
#define SKY_CLOUDY			1
#define SKY_RAINING			2
#define SKY_LIGHTNING		3

struct time_info_data
{
	int		hour;
	int		day;
	int		month;
	int		year;
};


struct weather_data
{
	bool		sky;
	sh_int	temperature;
	sh_int	change;
	bool		wind_dir;
	sh_int	wind_speed;

	sh_int	temp_summer;	/* Summer temperature			*/
	sh_int	temp_winter;	/* Winter temperature			*/
	sh_int	temp_daily;	/* Daily  temperature change		*/
	bool		wet_scale;	/* Wetness scale   0 - 10		*/
	bool		wind_scale;	/* Windyness scale 0 - 10		*/
};

struct player_game
{
	PLAYER_GAME * next;
	PLAYER_GAME * prev;
	CHAR_DATA   * ch;
};

/*
	Connected state for a channel.
*/

#define CON_GET_NAME                  -90
#define CON_GET_OLD_PASSWORD          -89

#define CON_GET_NEW_NAME              -50
#define CON_GET_NEW_PASSWORD          -49
#define CON_CONFIRM_NEW_PASSWORD      -48
#define CON_ANSI                      -47
#define CON_VT100                     -46
#define CON_GET_NEW_SEX               -45
#define CON_GET_NEW_RACE              -44
#define CON_GET_NEW_CLASS             -43

#define CON_APPEAR_LOOK               -30
#define CON_APPEAR_SKINCOLOR          -29
#define CON_APPEAR_EYECOLOR           -28
#define CON_APPEAR_HAIRCOLOR          -27
#define CON_APPEAR_HAIRTYPE           -26
#define CON_APPEAR_HAIRLENGTH         -25
#define CON_APPEAR_VERIFY             -24

#define CON_REROLL                    -20

#define CON_READ_MOTD                  -2
#define CON_COPYOVER_RECOVER           -1
#define CON_PLAYING                     0
#define CON_EDITING                     1

/*
	These keep track of what a person is editing
*/

#define MODE_NONE                       0
#define MODE_RESTRICTED                 1
#define MODE_REPEATCMD                  2
#define MODE_WRITING_NOTE               3
#define MODE_PERSONAL_DESC              4
#define MODE_ROOM_DESC                  5
#define MODE_ROOM_EXTRA                 6
#define MODE_ROOM_PROG                  7
#define MODE_MOB_DESC                   8
#define MODE_MOB_EXTRA                  9
#define MODE_MOB_PROG                  10
#define MODE_OBJ_DESC                  11
#define MODE_OBJ_EXTRA                 12
#define MODE_OBJ_PROG                  13
#define MODE_HELP_EDIT                 14
#define MODE_CRIME_RECORD              15



#define EDIT_TYPE_NONE                  0
#define EDIT_TYPE_ROOM                  1
#define EDIT_TYPE_MOB                   2
#define EDIT_TYPE_OBJ                   3
#define EDIT_TYPE_HELP                  4
#define EDIT_TYPE_RESET                 5
#define EDIT_TYPE_SHOP                  6
#define EDIT_TYPE_AREA                  7
#define EDIT_TYPE_MPROG                 8
#define EDIT_TYPE_OPROG                 9
#define EDIT_TYPE_RPROG                10

#define INTERP_ALIAS          BV01
#define INTERP_PROG               BV02
#define INTERP_FORCE          BV03
#define INTERP_DUMP               BV04
#define INTERP_SCROLL          BV05
#define INTERP_PAGE               BV06
#define INTERP_PAUSE          BV07
#define INTERP_AUTO               BV08
#define INTERP_HELP               BV09
#define INTERP_TACT_UPDATE     BV10

#define TRACK_NORTH               BV01
#define TRACK_EAST                BV02
#define TRACK_SOUTH               BV03
#define TRACK_WEST                BV04
#define TRACK_UP                  BV05
#define TRACK_DOWN                BV06
#define TRACK_FLY                 BV07
#define TRACK_BLOOD               BV08


#define TO_ROOM		0
#define TO_NOTVICT		1
#define TO_VICT		2
#define TO_CHAR		3


/*
	Descriptor (channel) structure.
*/

struct descriptor_data
{
	DESCRIPTOR_DATA   * next;
	DESCRIPTOR_DATA   * prev;
	DESCRIPTOR_DATA   * snoop_by;
	MTH_DATA          * mth;
	CHAR_DATA         * character;
	CHAR_DATA         * original;
	char              * host;
	sh_int              descriptor;
	sh_int              connected;
	char                inbuf [MAX_INPUT_LENGTH];
	char                incomm [MAX_INPUT_LENGTH];
	char              * back_buf;    /* Used for aliasses */
	char              * inlast;
	int                 repeat;
	char                outbuf [MAX_STRING_LENGTH];
	int                 outtop;
	int                 intop;
	int                 port_size;
	int                 connecting_time;
	bool                lookup;
	sh_int              timeout;
};


struct  editor_data
{
	sh_int              numlines;
	sh_int              on_line;
	int                 size;
	char              * line[600];
};


struct fighting_data
{
	FIGHT_DATA	*	next;
	FIGHT_DATA	*	prev;
	CHAR_DATA		*	who;
	CHAR_DATA		*	ch;
};


struct pet_data
{
	PET_DATA		*	next;
	PET_DATA		*	prev;
	CHAR_DATA		*	ch;
};


/*
	Attribute bonus structures.
*/

struct str_app_type
{
	sh_int	tohit;
	sh_int	todam;
	sh_int	carry;
	sh_int	wield;
};


struct int_app_type
{
	sh_int	learn;
	sh_int	manap;
};


struct wis_app_type
{
	sh_int practice;
	sh_int regen;
};


struct dex_app_type
{
	sh_int defensive;
	sh_int learn;
	sh_int movep;
	sh_int regen;
};


struct con_app_type
{
	sh_int	hitp;
	sh_int	regen;
};


struct obj_prog
{
	OBJ_PROG		*	next;
	OBJ_PROG		*	prev;
	OBJ_PROG		*	true;
	OBJ_PROG		*	false;
	char			*	unknown;
	char			*	argument;
	int				trigger;
	int				if_value;
	sh_int			cmd;
	bool				obj_command;
	bool				if_check;
	bool				if_symbol;
	bool				if_true;
	bool				if_false;
	bool				quest_offset;
	bool				quest_bits;
	bool				index;
	bool				percentage;
};

/*
	Help table types.
*/

struct help_data
{
	HELP_DATA		*	next;
	HELP_DATA 	*	prev;
	sh_int			level;
	char 		*	keyword;
	char 		*	text;
	AREA_DATA 	*	area;
	HELP_MENU_DATA *	first_menu;
	HELP_MENU_DATA *	last_menu;
};


struct help_menu_data
{
	HELP_MENU_DATA	*	next;
	HELP_MENU_DATA *	prev;
	char				option;
	HELP_DATA		*	help;
};


/*
	Shop types.
*/

#define MAX_TRADE	 5

struct shop_data
{
	SHOP_DATA		*	next;
	SHOP_DATA		*	prev;
	int				keeper;			/* Vnum of shop keeper mob	*/
	bool				buy_type[MAX_TRADE];/* Item types shop will buy	*/
	sh_int			profit_buy;		/* Cost multiplier for buying	*/
	sh_int			profit_sell;		/* Cost multiplier for selling*/
	bool				open_hour;		/* First opening hour		*/
	bool				close_hour;		/* First closing hour		*/
};

/*
	Per-class stuff.
*/

struct class_type
{
	char       who_name[4];          /* Three-letter name for 'who'	*/
	char       name[40];             /* Actual title  Chaos 11/7/93 */
	int        weapon;               /* First weapon */
	sh_int     skill_adept;          /* Minimum skill level */
	sh_int     skill_master;         /* Maximum skill level */
	sh_int     hp_min;               /* Min hp gained on leveling */
	sh_int     hp_max;               /* Max hp gained on leveling */
	sh_int     mana_min;             /* Min mana gained on leveling */
	sh_int     mana_max;             /* Max mana gained on leveling */
	char       desc[256];            /* Describes the class */
	long long  spec;                 /* bitvector for class specialties */
	sh_int     attr_prime;           /* Prime attribute */
	sh_int     attr_second;          /* Second attribute */
	sh_int     prime_mod;            /* Modifier for main attribute */
	sh_int     sec_mod;              /* Modifier for second attribute */
};


struct god_type
{
	char            name[13];
	char            logout_msg[81];
	char            finger_msg[81];
	char            exp_msg[81];
	char            level_msg[81];
	char            death_msg[81];
	char            recall_msg[81];
	char            pk_char_msg[81];
	char            pk_room_msg[81];
	char            pk_recall_msg[81];
	char            color[6];
	char            who_letter[2];
	int             resistance;
	int             bonus_hp;
	int             bonus_mana;
	int             bonus_move;
};

struct sector_type
{
	char            sector_name[20];
	int             movement;
	int             color;
	sh_int          flags;
	sh_int          light;
	sh_int          sight;
};

struct language_type
{
	char          * name;
	char          * who_name;
	sh_int          flags;
};

struct race_type
{
	char      race_name[15];           /* Race Names */
	sh_int    race_mod[5];             /* Str, Dex, Int, Wis, Con */
	sh_int    race_class[MAX_CLASS];   /* Ran, Gla, Mar, Nin, Ele, Ill, Mon, Nec */
	sh_int    max_speed;               /* 0-walk 1-normal 2-jog 3-run 4-haste */
	sh_int    move_rate;               /* .66 to 1.5  by 1 to 4 */
	sh_int    vision;                  /* 0-normal 1-night 2-dark */
	sh_int    hp_mod;                  /* bonus when advancing a level */
	sh_int    mana_mod;
	sh_int    move_mod;
	char      race_special[80];        /* info about abilities for start */
	char      race_description[512];
	sh_int    enter;                   /* first reincarnation allowed */
	sh_int    leave;                   /* last reincarnation allowed */
	sh_int    weight;                  /* weight in pounds */
	sh_int    height;                  /* height in feet */
	int       res_low;                 /* Resistances */
	int       res_high;
	int       vul_low;                 /* Vulnerabilities */
	int       vul_high;
	long long flags;                   /* specialties */
	int       attack_parts;            /* body parts race attacks with */
	int       language;
	sh_int    min_appearance[APPEAR_MAX];
	sh_int    max_appearance[APPEAR_MAX];
};

/*
	Data structures for notes.
*/

struct note_data
{
	NOTE_DATA *	next;
	NOTE_DATA *	prev;
	char 	*	sender;
	char		*	date;
	char 	*	to_list;
	char 	*	subject;
	int			topic;
	char 	*	text;
	int			time;		/* encoded format for dates */
	int			room_vnum;	/* Room based note boards */
};

struct topic_data
{
	char		*	name;
	int			min_level;
};

struct affect_data
{
	AFFECT_DATA    * next;
	AFFECT_DATA    * prev;
	sh_int           type;
	sh_int           duration;
	bool             location;
	sh_int           modifier;
	bool             bittype;
	long long        bitvector;
};

struct mth_data
{
	struct msdp_data ** msdp_data;
	char              * proxy;
	char              * terminal_type;
	char                telbuf[MAX_INPUT_LENGTH];
	int                 teltop;
	long long           mtts;
	int                 comm_flags;
	short               cols;
	short               rows;
	z_stream          * mccp2;
	z_stream          * mccp3;
};

/*
	Well known mob virtual numbers.
*/

#define MOB_VNUM_TOTEM                        4
#define MOB_VNUM_ILLUSION                  9905
#define MOB_VNUM_FIRE_ELEMENTAL            9906
#define MOB_VNUM_WATER_ELEMENTAL           9907
#define MOB_VNUM_AIR_ELEMENTAL             9908
#define MOB_VNUM_EARTH_ELEMENTAL           9909
#define MOB_VNUM_WINGED_CALL               9910

#define CLANFLAG_COMMLOG                   BV01
#define CLANFLAG_ORDER                     BV02
#define CLANFLAG_CHAOS                     BV03


#define AFLAG_NODEBUG                      BV01
#define AFLAG_NOTELEPORT                   BV02
#define AFLAG_NOGOHOME                     BV03
#define AFLAG_NORECALL                     BV04
#define AFLAG_NOCASTLE                     BV05
#define AFLAG_NORIP                        BV06
#define AFLAG_FREEQUIT                     BV07
#define AFLAG_NOSUMMON                     BV08
#define AFLAG_AUTOSAVE                     BV09
#define AFLAG_MODIFIED                     BV10
#define AFLAG_WEATHER                      BV11

#define SFLAG_NONE               BV00
#define SFLAG_INDOORS          BV01
#define SFLAG_NOWEATHER          BV02
#define SFLAG_SWIM               BV03

#define ACT_IS_NPC                         BV01          /* Old                         */
#define ACT_SENTINEL                       BV02          /* Stays in one room          */
#define ACT_SCAVENGER                      BV03          /* Picks up objects               */
#define ACT_DRUNK                          BV04          /* is drunk and talks so      */
#define ACT_MOUNT                          BV05          /* Can be mounted               */
#define ACT_AGGRESSIVE                     BV06          /* Attacks PC's               */
#define ACT_STAY_SECTOR                    BV07          /* Stays in same sector type  */
#define ACT_WIMPY                          BV08          /* Flees when hurt               */
#define ACT_PET                            BV09          /* Auto set for pets          */
#define ACT_TRAIN                          BV10          /* Can train PC's               */
#define ACT_PRACTICE                       BV11          /* Can practice PC's          */
#define ACT_WEAK                           BV12          /* Cannot carry anything      */
#define ACT_SMART                          BV13          /* Cannot say anything          */
#define ACT_ONE_FIGHT                      BV14          /* Disapears after fight          */
#define ACT_NO_ORDER                       BV15          /* Cannot be ordered          */
#define ACT_RIDE                           BV16          /* Creature can be ridden     */
#define ACT_BODY                           BV17          /* Body parts are used          */
#define ACT_RACE                           BV18          /* Race is defined after gold */
#define ACT_UNDEAD                         BV19          /* No corpse or body parts     */
#define ACT_ELEMENTAL                      BV20          /* Friendly Elemental type     */
#define ACT_CLAN_GUARD                     BV21          /* Clan Guards                    */
#define ACT_CLAN_HEALER                    BV22          /* Clan Healers               */
#define ACT_WILL_DIE                       BV23          /* Purely Internal               */
#define ACT_NO_DAMAGE                      BV24          /* Doesn't deal damage          */
#define ACT_CLASS                          BV25        /* Mimmics selected class */
#define ACT_NO_EXP                         BV26        /* Doesn't give exp */

#define AFF_BLIND                          BV01
#define AFF_INVISIBLE                      BV02
#define AFF_IMP_INVISIBLE                  BV03
#define AFF_DETECT_INVIS                   BV04
#define AFF_FLASH_POWDER                   BV05
#define AFF_DETECT_HIDDEN                  BV06
#define AFF_TRUESIGHT                      BV07
#define AFF_SANCTUARY                      BV08
#define AFF_RESERVED_2                     BV09
#define AFF_INFRARED                       BV10
#define AFF_CURSE                          BV11
#define AFF_UNDERSTAND                     BV12
#define AFF_POISON                         BV13
#define AFF_PROTECT_EVIL                   BV14
#define AFF_PROTECT_GOOD                   BV15
#define AFF_SNEAK                          BV16
#define AFF_HIDE                           BV17
#define AFF_SLEEP                          BV18
#define AFF_CHARM                          BV19
#define AFF_FLYING                         BV20
#define AFF_PASS_DOOR                      BV21
#define AFF_STEALTH                        BV22
#define AFF_CLEAR                          BV23
#define AFF_HUNT                           BV24
#define AFF_TONGUES                        BV25
#define AFF_ETHEREAL                       BV26
#define AFF_HASTE                          BV27
#define AFF_STABILITY                      BV28


#define AFF2_ENHANCED_REST                -BV01
#define AFF2_ENHANCED_HEAL                -BV02
#define AFF2_ENHANCED_REVIVE              -BV03
#define AFF2_DIVINE_INSPIRATION           -BV04
#define AFF2_CAMPING                      -BV05
#define AFF2_BERSERK                      -BV06
#define AFF2_MIRROR_IMAGE                 -BV07
#define AFF2_HALLUCINATE                  -BV08
#define AFF2_BLEEDING                     -BV09
#define AFF2_EARTHBIND                    -BV10
#define AFF2_ETHEREAL                     -BV11
#define AFF2_ASTRAL                       -BV12
#define AFF2_BREATH_WATER                 -BV13
#define AFF2_ICICLE_ARMOR                 -BV14
#define AFF2_QUICKEN                      -BV15
#define AFF2_CONFUSION                    -BV16
#define AFF2_HAS_FLASH                    -BV17
#define AFF2_POSSESS                      -BV18
#define AFF2_FIRESHIELD                   -BV19
#define AFF2_TORRID_BALM                  -BV20
#define AFF2_FIREWALK                     -BV21
#define AFF2_STEELHAND                    -BV22

#define DAM_NONE                    BV00
#define DAM_PIERCE                    BV01
#define DAM_SLICE                    BV02
#define DAM_BASH                    BV03
#define DAM_CHOP                    BV04
#define DAM_REND                    BV05
#define DAM_THRUST                    BV06
#define DAM_VIBE                    BV07
#define DAM_FIRE                    BV08
#define DAM_COLD                    BV09
#define DAM_LIGHTNING               BV10
#define DAM_LIFE                    BV11
#define DAM_LIGHT                    BV12
#define DAM_POISON                    BV13
#define DAM_HOLY                    BV14
#define DAM_UNHOLY                    BV15
#define DAM_PSIONIC                    BV16
#define DAM_EVIL                    BV17
#define DAM_GOOD                    BV18
#define DAM_MAGIC                    BV19
#define DAM_ACID                    BV20


#define ATTACK_DISARM               BV01
#define ATTACK_TRIP                    BV02
#define ATTACK_PARRY               BV03
#define ATTACK_DODGE               BV04
#define ATTACK_BLOCK               BV05
#define ATTACK_TUMBLE               BV06
#define ATTACK_MIRROR               BV07

#define ATTACK_
#define SEX_NEUTRAL                    0
#define SEX_MALE                    1
#define SEX_FEMALE                    2


#define TRIG_COMMAND               BV01
#define TRIG_VOID                    BV02
#define TRIG_UNKNOWN               BV03
#define TRIG_TICK                    BV04
#define TRIG_DAMAGE                    BV05
#define TRIG_HIT                    BV06
#define TRIG_WEAR                    BV07
#define TRIG_REMOVE                    BV08
#define TRIG_SACRIFICE               BV09
#define TRIG_DROP                    BV10
#define TRIG_GET                    BV11
#define TRIG_ROOM_COMMAND          BV12
#define TRIG_ROOM_UNKNOWN          BV13
#define TRIG_WEAR_COMMAND          BV14
#define TRIG_WEAR_UNKNOWN          BV15


#define OPROG_ECHO                     0
#define OPROG_GOD_COMMAND           1
#define OPROG_GOD_ARGUMENT           2
#define OPROG_COMMAND                3
#define OPROG_ARGUMENT                4
#define OPROG_IF_HAS_OBJECT           5
#define OPROG_IF                     6
#define OPROG_JUNK                     7
#define OPROG_QUEST_SET                8
#define OPROG_QUEST_ADD                9
#define OPROG_OBJECT_QUEST_IF          10
#define OPROG_PLAYER_QUEST_IF          11
#define OPROG_APPLY                    12

#define OIF_WEAR_LOC                8
#define OIF_TIME_OF_DAY                9
#define OIF_WEATHER                    10
#define OIF_RANDOM_PERCENT          11
#define OIF_USER_PERCENT_HITPT     12
#define OIF_USER_PERCENT_MANA          13
#define OIF_USER_PERCENT_MOVE          14
#define OIF_USER_SECTOR               15
#define OIF_USER_ALIGNMENT          16
#define OIF_USER_GOLD               17
#define OIF_USER_CLASS               18
#define OIF_USER_WHICH_GOD          19
#define OIF_USER_AREA               20
#define OIF_USER_LEVEL               21
#define OIF_USER_POSITION          22
#define OIF_USER_RACE               23
#define OIF_USER_SEX               24
#define OIF_USER_ROOM_NUM          25
#define OIF_USER_FIGHTING          26


#define OBJ_VNUM_MONEY_ONE           2
#define OBJ_VNUM_MONEY_SOME           3
#define OBJ_VNUM_TOTEM                4
#define OBJ_VNUM_CORPSE_NPC          10
#define OBJ_VNUM_CORPSE_PC          11
#define OBJ_VNUM_SEVERED_HEAD          12
#define OBJ_VNUM_TORN_HEART          13
#define OBJ_VNUM_SLICED_ARM          14
#define OBJ_VNUM_SLICED_LEG          15
#define OBJ_VNUM_FINAL_TURD          16
#define OBJ_VNUM_MUSHROOM          20
#define OBJ_VNUM_LIGHT_BALL          21
#define OBJ_VNUM_SPRING               22
#define OBJ_VNUM_GATE               23
#define OBJ_VNUM_ICE_ARROW          24
#define OBJ_VNUM_BOUNTY_HEAD          25

#define OBJ_VNUM_SCHOOL_MACE          3700
#define OBJ_VNUM_SCHOOL_DAGGER     3701
#define OBJ_VNUM_SCHOOL_SWORD          3702
#define OBJ_VNUM_SCHOOL_VEST          3703
#define OBJ_VNUM_SCHOOL_SHIELD     3704
#define OBJ_VNUM_SCHOOL_SPEAR          3750
#define OBJ_VNUM_SCHOOL_STAFF          3751
#define OBJ_VNUM_SCHOOL_WHIP          3752


#define ITEM_NOTHING                0
#define ITEM_LIGHT                     1
#define ITEM_SCROLL                     2
#define ITEM_WAND                     3
#define ITEM_STAFF                     4
#define ITEM_WEAPON                     5
#define ITEM_TREASURE                8
#define ITEM_ARMOR                     9
#define ITEM_POTION                    10
#define ITEM_FURNITURE               12
#define ITEM_TRASH                    13
#define ITEM_CONTAINER               15
#define ITEM_DRINK_CON               17
#define ITEM_KEY                    18
#define ITEM_FOOD                    19
#define ITEM_MONEY                    20
#define ITEM_BOAT                    22
#define ITEM_CORPSE_NPC               23
#define ITEM_CORPSE_PC               24
#define ITEM_FOUNTAIN               25
#define ITEM_PILL                    26
#define ITEM_PORTAL                    27
#define ITEM_ARTIFACT               28
#define ITEM_LOCKPICK               29
#define ITEM_AMMO                    30
#define ITEM_TOTEM                    31
#define ITEM_HERB                    32

#define ITEM_GLOW                  BV01
#define ITEM_HUM                   BV02
#define ITEM_DARK                  BV03
#define ITEM_LOCK                  BV04
#define ITEM_EVIL                  BV05
#define ITEM_INVIS                 BV06
#define ITEM_MAGIC                 BV07
#define ITEM_NODROP                BV08
#define ITEM_BLESS                 BV09
#define ITEM_ANTI_GOOD             BV10
#define ITEM_ANTI_EVIL             BV11
#define ITEM_ANTI_NEUTRAL          BV12
#define ITEM_NOREMOVE              BV13
#define ITEM_INVENTORY             BV14
#define ITEM_BURNING               BV15
#define ITEM_NOT_VALID             BV16
#define ITEM_AUTO_ENGRAVE          BV17
#define ITEM_FORGERY               BV18
#define ITEM_ETHEREAL              BV19
#define ITEM_MOUNT                 BV20
#define ITEM_MODIFIED              BV21
#define ITEM_MYTHICAL              BV22
#define ITEM_EPICAL                BV23

#define ITEM_TAKE                  BV01
#define ITEM_WEAR_FINGER           BV02
#define ITEM_WEAR_NECK             BV03
#define ITEM_WEAR_BODY             BV04
#define ITEM_WEAR_HEAD             BV05
#define ITEM_WEAR_LEGS             BV06
#define ITEM_WEAR_FEET             BV07
#define ITEM_WEAR_HANDS            BV08
#define ITEM_WEAR_ARMS             BV09
#define ITEM_WEAR_SHIELD           BV10
#define ITEM_WEAR_ABOUT            BV11
#define ITEM_WEAR_WAIST            BV12
#define ITEM_WEAR_WRIST            BV13
#define ITEM_WEAR_WIELD            BV14
#define ITEM_WEAR_HOLD             BV15
#define ITEM_WEAR_HEART            BV16


#define APPLY_NONE                    0
#define APPLY_STR                    1
#define APPLY_DEX                    2
#define APPLY_INT                    3
#define APPLY_WIS                    4
#define APPLY_CON                    5
#define APPLY_SEX                    6
#define APPLY_RACE                    7
#define APPLY_LEVEL                    8
#define APPLY_AGE                    9
#define APPLY_HEIGHT               10
#define APPLY_WEIGHT               11
#define APPLY_MANA                    12
#define APPLY_HIT                    13
#define APPLY_MOVE                    14
#define APPLY_EXP                    16
#define APPLY_AC                    17
#define APPLY_HITROLL               18
#define APPLY_DAMROLL               19
#define APPLY_SAVING_PARA          20
#define APPLY_SAVING_ROD               21
#define APPLY_SAVING_PETRI          22
#define APPLY_SAVING_BREATH          23
#define APPLY_SAVING_SPELL          24

#define AFFECT_TO_NONE                0
#define AFFECT_TO_CHAR                1
#define AFFECT_TO_OBJ                2

/*
	ITEM_CONTAINER value[1]
*/

#define CONT_CLOSEABLE          BV01
#define CONT_PICKPROOF          BV02
#define CONT_CLOSED               BV03
#define CONT_LOCKED               BV04
#define CONT_MAGICAL_LOCK     BV05
#define CONT_TRAPPED_LOCK     BV06
#define CONT_MECHANICAL_LOCK     BV07
#define CONT_SMALL_LOCK          BV08
#define CONT_BIG_LOCK          BV09
#define CONT_EASY_PICK          BV10
#define CONT_HARD_PICK          BV11

/*
	ITEM_PORTAL value[2]
*/

#define GATE_RANDOM               BV01
#define GATE_GOWITH               BV02
#define GATE_NOFLEE               BV03     /* Uncoded untill today */
#define GATE_STEP_THROUGH     BV04
#define GATE_STEP_INTO          BV05
#define GATE_RANDOM_AREA          BV06
/*
     ITEM_FURNITURE value[2]
*/

#define SLEEP_IN               1
#define SLEEP_ON               2
#define REST_IN               4
#define REST_ON               8
#define SIT_IN                    16
#define SIT_ON                    32
#define STAND_IN               64
#define STAND_ON               128


#define ROOM_VNUM_LIMBO               2
#define ROOM_VNUM_JUNK               3
#define ROOM_VNUM_TOTEM               4
#define ROOM_VNUM_SCHOOL               3700
#define ROOM_VNUM_BLACKSMITH          9704
#define ROOM_VNUM_TEMPLE               9755
#define ROOM_VNUM_IDENTIFY          9818
#define ROOM_VNUM_ARENA               10500
#define ROOM_VNUM_JAIL               12001
#define ROOM_VNUM_CASTLE               80000
#define ROOM_VNUM_RIFT               70000
#define ROOM_VNUM_WILDERNESS          99000

#define ROOM_DARK                    BV01
#define ROOM_SMOKE                    BV02
#define ROOM_NO_MOB                    BV03
#define ROOM_INDOORS               BV04
#define ROOM_GLOBE                    BV05
#define ROOM_NO_GOHOME               BV06
#define ROOM_CLAN_DONATION          BV07
#define ROOM_ALTAR                    BV08
#define ROOM_BANK                    BV09
#define ROOM_PRIVATE               BV10
#define ROOM_SAFE                    BV11
#define ROOM_SOLITARY               BV12
#define ROOM_PET_SHOP               BV13
#define ROOM_NO_RECALL               BV14
#define ROOM_RIP                    BV15
#define ROOM_BLOCK                    BV16
#define ROOM_NO_SAVE               BV17
#define ROOM_MORGUE                    BV18
#define ROOM_ALLOW_WAR               BV19
#define ROOM_ALLOW_GLA               BV20
#define ROOM_ALLOW_MAR               BV21
#define ROOM_ALLOW_NIN               BV22
#define ROOM_ALLOW_DRU               BV23
#define ROOM_ALLOW_SOR               BV24
#define ROOM_ALLOW_SHA               BV25
#define ROOM_ALLOW_WLC               BV26
#define ROOM_IS_CASTLE               BV27
#define ROOM_IS_ENTRANCE               BV28
#define ROOM_NOTE_BOARD               BV29
#define ROOM_NO_CASTLE               BV30
#define ROOM_NO_RIP                    BV31
#define ROOM_MAZE                    BV32
#define ROOM_ICE                    BV33
#define ROOM_DYNAMIC               BV34

#define DIR_NORTH                     0
#define DIR_EAST                      1
#define DIR_SOUTH                     2
#define DIR_WEST                      3
#define DIR_UP                        4
#define DIR_DOWN                      5


#define EX_ISDOOR                  BV01
#define EX_CLOSED                  BV02
#define EX_LOCKED                  BV03
#define EX_HIDDEN                  BV04
#define EX_RIP                     BV05
#define EX_PICKPROOF               BV06
#define EX_BASHPROOF               BV07
#define EX_MAGICPROOF              BV08
#define EX_BASHED                  BV09
#define EX_UNBARRED                BV10
#define EX_BACKDOOR                BV11
#define EX_CLAN_BACKDOOR           BV12
#define EX_PASSPROOF               BV13
#define EX_MAGICAL_LOCK            BV14
#define EX_TRAPPED_LOCK            BV15
#define EX_MECHANICAL_LOCK         BV16
#define EX_SMALL_LOCK              BV17
#define EX_BIG_LOCK                BV18
#define EX_EASY_PICK               BV19
#define EX_HARD_PICK               BV20
#define EX_RIFT                    BV21

#define PICK_MAGICAL_LOCK          BV01
#define PICK_TRAPPED_LOCK          BV02
#define PICK_MECHANICAL_LOCK          BV03
#define PICK_SMALL_LOCK               BV04
#define PICK_BIG_LOCK               BV05

#define SECT_INSIDE                   0
#define SECT_CITY                     1
#define SECT_FIELD                    2
#define SECT_FOREST                   3
#define SECT_HILLS                    4
#define SECT_MOUNTAIN                 5
#define SECT_LAKE                     6
#define SECT_RIVER                    7
#define SECT_OCEAN                    8
#define SECT_AIR                      9
#define SECT_DESERT                  10
#define SECT_LAVA                    11
#define SECT_ETHEREAL                12
#define SECT_ASTRAL                  13
#define SECT_UNDER_WATER             14
#define SECT_UNDER_GROUND            15
#define SECT_DEEP_EARTH              16
#define SECT_ROAD                    17
#define SECT_SWAMP                   18
#define SECT_BEACH                   19
#define SECT_TUNDRA                  20
#define SECT_EDGE                    21
#define SECT_MAX                     22


#define WEAR_NONE                    -1
#define WEAR_LIGHT                    0
#define WEAR_FINGER_L                 1
#define WEAR_FINGER_R                 2
#define WEAR_NECK_1                   3
#define WEAR_NECK_2                   4
#define WEAR_BODY                     5
#define WEAR_HEAD                     6
#define WEAR_LEGS                     7
#define WEAR_FEET                     8
#define WEAR_HANDS                    9
#define WEAR_ARMS                    10
#define WEAR_SHIELD                  11
#define WEAR_ABOUT                   12
#define WEAR_WAIST                   13
#define WEAR_WRIST_L                 14
#define WEAR_WRIST_R                 15
#define WEAR_WIELD                   16
#define WEAR_DUAL_WIELD              17
#define WEAR_HOLD                    18
#define WEAR_HEART                   19
#define MAX_WEAR                     20


#define COND_DRUNK                    0
#define COND_FULL                     1
#define COND_THIRST                   2
#define COND_AIR                      3

#define POS_DEAD                   0
#define POS_MORTAL                 1
#define POS_INCAP                  2
#define POS_STUNNED                3
#define POS_SLEEPING               4
#define POS_RESTING                5
#define POS_SITTING                6
#define POS_FIGHTING               7
#define POS_STANDING               8

#define PVNUM_SILENCED             BV01
#define PVNUM_DENIED               BV02
#define PVNUM_MUTED                BV03
#define PVNUM_FROZEN               BV04
#define PVNUM_LOGGED               BV05
#define PVNUM_ARRESTED             BV06
#define PVNUM_DELETED              BV07

#define PLR_IS_NPC                 BV01          /* Old */
#define PLR_TELOPTS                BV02
#define PLR_EXP_TO_LEVEL           BV03
#define PLR_AUTOEXIT               BV04
#define PLR_AUTOLOOT               BV05
#define PLR_AUTOSAC                BV06
#define PLR_BLANK                  BV07
#define PLR_BRIEF                  BV08
#define PLR_REPEAT                 BV09
#define PLR_EXITPOS                BV10
#define PLR_PROMPT                 BV11          /* Old */
#define PLR_TELNET_GA              BV12          /* Old */
#define PLR_HOLYLIGHT              BV13
#define PLR_WIZINVIS               BV14
#define PLR_WIZTIME                BV15
#define PLR_SILENCE                BV16          /* Old */
#define PLR_OUTCAST                BV17
#define PLR_MUTE                   BV18          /* Old */
#define PLR_WIZCLOAK               BV19
#define PLR_LOG                    BV20          /* Old */
#define PLR_DENY                   BV21          /* Old */
#define PLR_FREEZE                 BV22          /* Old */
#define PLR_THIEF                  BV23
#define PLR_KILLER                 BV24
#define PLR_DAMAGE                 BV25
#define PLR_AUTO_SPLIT             BV26
#define PLR_QUIET                  BV27
#define PLR_PAGER                  BV28
#define PLR_CHAT                   BV29          /* Old */
#define PLR_PLAN                   BV30          /* Old */
#define PLR_AFK                    BV31
#define PLR_BATTLE                 BV32          /* Old */
#define PLR_VICTIM_LIST            BV33          /* Old */
#define PLR_HEARLOG                BV34
#define PLR_BUILDLIGHT             BV35
#define PLR_FOS                    BV36          /* Old */
#define PLR_ARRESTED               BV37          /* Old */
#define PLR_NOTAG                  BV38          /* Don't want to participate in TAG */
#define PLR_TAGGED                 BV39          /* Player is tagged */

#define SPAM_YOU_HIT               BV01
#define SPAM_YOU_MISS              BV02
#define SPAM_THEY_HIT              BV03
#define SPAM_THEY_MISS             BV04
#define SPAM_PARTY_HIT             BV05
#define SPAM_PARTY_MISS            BV06
#define SPAM_THEY_HIT_PARTY        BV07
#define SPAM_THEY_MISS_PARTY       BV08
#define SPAM_OTHER_HIT             BV09
#define SPAM_OTHER_MISS            BV10
#define SPAM_PARTY_STATUS          BV11
#define SPAM_PARTY_MOVE            BV12
#define SPAM_HEAR_UTTER            BV13

/*
     Channel bits.
*/

#define CHANNEL_CHAT               BV01
#define CHANNEL_FOS                BV02
#define CHANNEL_PLAN               BV03
#define CHANNEL_TALK               BV04
#define CHANNEL_TRIVIA             BV05
#define CHANNEL_MORPH              BV06
#define CHANNEL_BATTLE             BV07

#define CLASS_RANGER                  0
#define CLASS_GLADIATOR               1
#define CLASS_MARAUDER                2
#define CLASS_NINJA                   3
#define CLASS_ELEMENTALIST            4
#define CLASS_ILLUSIONIST             5
#define CLASS_MONK                    6
#define CLASS_NECROMANCER             7
#define CLASS_MONSTER                99


#define FLAG_CLASS_RANGER          BV01
#define FLAG_CLASS_GLADIATOR       BV02
#define FLAG_CLASS_MARAUDER        BV03
#define FLAG_CLASS_NINJA           BV04
#define FLAG_CLASS_ELEMENTALIST    BV05
#define FLAG_CLASS_ILLUSIONIST     BV06
#define FLAG_CLASS_MONK            BV07
#define FLAG_CLASS_NECROMANCER     BV08


#define RACE_HUMAN                    0
#define RACE_ELF                      1
#define RACE_DROW                     2
#define RACE_DWARF                    3
#define RACE_GNOME                    4
#define RACE_ORC                      5
#define RACE_OGRE                     6
#define RACE_HOBBIT                   7
#define RACE_WEREWOLF                 8
#define RACE_DEMON                    9
#define RACE_WRAITH                  10
#define RACE_TROLL                   11
#define RACE_VAMPIRE                 12
#define RACE_TITAN                   13
#define RACE_ENT                     14
#define RACE_DRAGON                  15

#define BODY_HEAD                         BV01
#define BODY_MOUTH                         BV02
#define BODY_EYE                         BV03
#define BODY_TORSO                          BV04
#define BODY_HIP                         BV05
#define BODY_LEG                         BV06
#define BODY_ARM                         BV07
#define BODY_WING                         BV08
#define BODY_TAIL                         BV09
#define BODY_TENTACLE                    BV10
#define BODY_HORN                         BV11
#define BODY_CLAW                         BV12
#define BODY_HAND                         BV13
#define BODY_FOOT                         BV14
#define BODY_BRANCH                         BV15
#define BODY_ROOT                         BV16
#define BODY_TWIG                         BV17
#define BODY_TRUNK                         BV18
#define BODY_PINCERS                    BV19
#define BODY_SPINNARET                    BV20
#define BODY_BEAK                         BV21


#define RSKILL_NONE                    BV00
#define RSKILL_HUMAN               BV01
#define RSKILL_ELF                    BV02
#define RSKILL_DARKELF               BV03
#define RSKILL_DWARF               BV04
#define RSKILL_GNOME               BV05
#define RSKILL_ORC                    BV06
#define RSKILL_GRIFFON               BV07
#define RSKILL_HALFELF               BV08
#define RSKILL_OGRE                    BV09
#define RSKILL_HOBBIT               BV10

#define RSKILL_SPRITE               BV11
#define RSKILL_SHADE               BV12
#define RSKILL_ROC                    BV13
#define RSKILL_THREEKREEN          BV14
#define RSKILL_WEREWOLF               BV15
#define RSKILL_DEMON               BV16
#define RSKILL_GARGOYLE               BV17
#define RSKILL_WRAITH               BV18
#define RSKILL_TROLL               BV19
#define RSKILL_VAMPIRE               BV20
#define RSKILL_PHOENIX               BV21
#define RSKILL_DOPPELGANGER          BV22
#define RSKILL_ARACHNID               BV23
#define RSKILL_TITAN               BV24
#define RSKILL_EAGLE               BV25
#define RSKILL_KRAKEN               BV26
#define RSKILL_BALROG               BV27
#define RSKILL_DRAGON               BV28
#define RSKILL_ENT                    BV29


#define RSPEC_NONE                      BV00
#define RSPEC_FLYING                    BV01
#define RSPEC_ETHEREAL                  BV02
#define RSPEC_SWIM                      BV03
#define RSPEC_UNDEAD                    BV04
#define RSPEC_SEE_AGGR                    BV05
#define RSPEC_FORAGE                    BV06
#define RSPEC_INTIMIDATE                    BV07
#define RSPEC_MULTI_ARMS                    BV08
#define RSPEC_KNOW_ALIGN                    BV09
#define RSPEC_EASY_FLEE                    BV10
#define RSPEC_MAGIC_DEFENSE               BV11
#define RSPEC_BRAWLING                    BV12
#define RSPEC_FASTHEAL                    BV13
#define RSPEC_FASTREVIVE                    BV14
#define RSPEC_ASTRAL                    BV15
#define RSPEC_AGE_AC                    BV16
#define RSPEC_AGE_HR                    BV17
#define RSPEC_AGE_DR                    BV18
#define RSPEC_VAMPIRIC                    BV19
#define RSPEC_PASS_DOOR                    BV20
#define RSPEC_FIREWALK                     BV21
#define RSPEC_BREATH_WATER               BV22

#define CFLAG_NONE                      BV00
#define CFLAG_MANA                      BV01
#define CFLAG_EAT                       BV02
#define CFLAG_QUAFF                     BV03
#define CFLAG_QUICK_FLEE                BV04
#define CFLAG_SHIELD                    BV05

#define LANGFLAG_NONE                   BV00
#define LANGFLAG_CHAOS                  BV01
#define LANGFLAG_ORDER                  BV02
#define LANGFLAG_CLASS                  BV03

#define LANG_HUMAN                      BV01
#define LANG_ELVEN                      BV02
#define LANG_DROW                       BV03
#define LANG_DWARVEN                    BV04
#define LANG_GNOMISH                    BV05
#define LANG_ORCISH                     BV06
#define LANG_OGRISH                     BV07
#define LANG_HALFLING                   BV08
#define LANG_DEMONIC                    BV09
#define LANG_ARCHAIC                    BV10
#define LANG_RUNIC                      BV11
#define LANG_TEMPLAR                    BV12
#define LANG_DRACONIC                   BV13
#define LANG_ENTROPIAN                  BV14
#define LANG_EQUILIBRIUM                BV15

#define APPEAR_LOOK                        0
#define APPEAR_SKINCOLOR                   1
#define APPEAR_EYECOLOR                    2
#define APPEAR_HAIRCOLOR                   3
#define APPEAR_HAIRTYPE                    4
#define APPEAR_HAIRLENGTH                  5


#define WEAPON_SLICE                       1
#define WEAPON_STAB                        2
#define WEAPON_SLASH                       3
#define WEAPON_WHIP                        4
#define WEAPON_CLAW                        5
#define WEAPON_BLAST                       6
#define WEAPON_POUND                       7
#define WEAPON_CRUSH                       8
#define WEAPON_GREP                        9
#define WEAPON_BITE                       10
#define WEAPON_PIERCE                     11
#define WEAPON_CHOP                       12

#define WEAPON_TYPE_WEAPON                 0
#define WEAPON_TYPE_SWORD                  1
#define WEAPON_TYPE_DAGGER                 2
#define WEAPON_TYPE_AXE                    3
#define WEAPON_TYPE_MACE                   4
#define WEAPON_TYPE_STAFF                  5
#define WEAPON_TYPE_WHIP                   6
#define WEAPON_TYPE_FLAIL                  7
#define WEAPON_TYPE_SPEAR                  8
#define WEAPON_TYPE_CLAW                   9
#define WEAPON_TYPE_SHORTBOW              10
#define WEAPON_TYPE_LONGBOW               11
#define WEAPON_TYPE_CROSSBOW              12
#define WEAPON_TYPE_BLOWPIPE              13


#define SFOL_NONE                       BV00
#define SFOL_CHAOS                      BV01
#define SFOL_ORDER                      BV02
#define SFOL_ALL                        SFOL_ORDER|SFOL_CHAOS
#define SFOL_DUNNO                      BV03

#define FSKILL_NONE                     0
#define FSKILL_SPELL                    1
#define FSKILL_CMD                      2

#define SPEED_WALK                      0
#define SPEED_NORMAL                    1
#define SPEED_JOG                       2
#define SPEED_RUN                       3
#define SPEED_HASTE                     4
#define SPEED_BLAZE                     5

#define MOVE_LIGHT                      1
#define MOVE_MEDIUM                     2
#define MOVE_HEAVY                      3

#define VISION_NORMAL                   0
#define VISION_NIGHT                    1
#define VISION_DARK                     2

#define ROOM_TIMER_SANCTIFY             0
#define ROOM_TIMER_SMOKE                1
#define ROOM_TIMER_GLOBE                2
#define ROOM_TIMER_BLOCK                3
#define ROOM_TIMER_ICE                  4
#define ROOM_TIMER_UNBLOCK              5

/*
	Prototype for a mob.
*/

struct mob_index_data
{
	SPEC_FUN			*	spec_fun;
	SHOP_DATA			*	pShop;
	AREA_DATA			*   	area;
	MUD_PROG		*	first_prog;
	MUD_PROG		*	last_prog;
	CHAR_DATA			*	first_instance;
	CHAR_DATA			*	last_instance;
	EXTRA_DESCR_DATA	*	first_extradesc;
	EXTRA_DESCR_DATA	*	last_extradesc;
	int					progtypes;
	char				*	player_name;
	char				*	short_descr;
	char 			*	long_descr;
	char 			*	description;
	long long				act;
	long long				affected_by;
	int					vnum;
	int					gold;
	int					body_parts;
	int					attack_parts;
	int					creator_pvnum;
	sh_int				total_mobiles;
	sh_int				level;
	sh_int				alignment;
	sh_int				hitnodice;
	sh_int				hitsizedice;
	int					hitplus;
	sh_int				damnodice;
	sh_int				damsizedice;
	sh_int				damplus;
	bool					sex;
	bool					class;
	bool					position;
	bool					race;
};


/*
	One character (PC or NPC).
*/

struct char_data
{
	CHAR_DATA           * next;
	CHAR_DATA           * prev;
	CHAR_DATA           * next_in_room;
	CHAR_DATA           * prev_in_room;
	CHAR_DATA           * next_instance;
	CHAR_DATA           * prev_instance;
	AFFECT_DATA         * first_affect;
	AFFECT_DATA         * last_affect;
	OBJ_DATA            * first_carrying;
	OBJ_DATA            * last_carrying;
	CHAR_DATA           * master;
	CHAR_DATA           * leader;
	FIGHT_DATA          * fighting;
	RESET_DATA          * reset;
	MOB_INDEX_DATA      * pIndexData;
	DESCRIPTOR_DATA	* desc;
	ROOM_INDEX_DATA	* in_room;
	PC_DATA             * pcdata;
	NPC_DATA            * npcdata;
	POISON_DATA         * poison;
	char                * name;
	char                * short_descr;
	char                * long_descr;
	char                * description;
	long long             act;
	long long             affected_by;
	long long             affected2_by;
	int                   critical_hit_by;  /* The PVNUM of the hitter */
	int                   gold;
	int                   hit;
	int                   max_hit;
	sh_int                mana;
	sh_int                max_mana;
	sh_int                move;
	sh_int                max_move;
	sh_int                level;
	sh_int                trust;
	sh_int                timer;
	sh_int                wait;
	sh_int                carry_weight;
	sh_int                carry_number;
	sh_int                sex;
	bool                  class;
	bool                  race;
	bool                  position;
	sh_int                saving_throw;
	sh_int                alignment;
	sh_int                hitroll;
	sh_int                damroll;
	bool               	  speed;
	sh_int                distracted;
	sh_int                attack;      /* Used to allow 1 disarm, etc.*/
	OBJ_DATA            * furniture;   /* sit/sleep/stand on object */
	CHAR_DATA           * mounting;
};

/*
	Data which only PC's have.
*/

struct pc_data
{
	CLAN_DATA           * clan;
	HELP_DATA           * last_help;
	HELP_DATA           * prev_help;
	CRIME_DATA          * first_record;	/* the X-Files :P */
	CRIME_DATA          * last_record;
	OBJ_DATA            * corpse;          /* Pointer to player's corpse */
	OBJ_DATA            * asn_obj;          /* object assassinating for */
	CASTLE_DATA         * castle;
	TACTICAL_MAP        * tactical;          /* The recorded tactical map for comparison */
	ROOM_INDEX_DATA     * travel_from;	/* Traveling old room  */
	EDITOR_DATA         * editor;
	NOTE_DATA           * pnote;
	CHAR_DATA           * reply;
	CHAR_DATA           * shadowing;
	CHAR_DATA           * shadowed_by;
	DO_FUN              * last_cmd;
	void                * temp_ptr;
	void                * edit_ptr;
	char                * edit_buf;
	char                * host;
	char                * bamfin;
	char                * bamfout;
	char                * title;
	char                * alias          [MAX_ALIAS];
	char                * alias_c          [MAX_ALIAS];
	char                * mail_address;
	char                * html_address;
	char                * clan_name;
	char                * clan_pledge;
	char                * back_buf[26];	/* Change to pointer to string mode */
	char                * page_buf;
	char                * tracking;
	char                * ambush;
	char                * prompt_layout;	/* For reconfiguration */
	char                * subprompt;	/* subprompt for OLC*/
	char                * auto_command;
	char                * tactical_index;/* The two byte index for a player */
	char                * block_list;	/* list of characters to notell/nobeep */
	char                * last_command;
	bool                * quest[MAX_AREA];
	char                * last_pk_attack_name[MAX_PK_ATTACKS];
	int                   last_pk_attack_time[MAX_PK_ATTACKS];
	int                   last_pk_attack_pvnum[MAX_PK_ATTACKS];
	int                   color[COLOR_MAX];
	char                * scroll_buf[MAX_SCROLL_BUF];
	long long             password;
	int                   history[6];
	int                   version;
	int                   a_range_lo;          /* olc vnum range */
	int                   a_range_hi;
	int                   played;
	int                   previous_hours;
	int                   killer_played;
	int                   outcast_played;
	int                   last_connect;
	int                   last_time;
	int                   last_improve;
	int                   last_saved;
	int                   last_combat;
	int                   topic_stamp	[MAX_TOPIC];
	int                   last_note;
	int                   note_topic;
	int                   just_died_ctr;
	int                   time_of_death;
	int                   port_size;
	int                   pvnum;
	int                   corpse_room;
	int                   clan_position;
	int                   channel;
	int                   language;
	int                   speak;
	int                   death_room;
	int                   recall;
	int                   spam;
	int                   last_real_room;
	int                   was_in_room;
	int                   travel;
	int                   account;
	int                   jailtime;
	int                   jaildate;
	int                   morph_points;
	int                   trivia_points;
	int                   clock;
	int                   exp;
	int                   actual_max_hit;
	sh_int                actual_max_mana;
	sh_int                actual_max_move;
	sh_int                actual_sex;
	sh_int                vt100_flags;
	sh_int                screensize_h;
	sh_int                screensize_v;
	sh_int                tactical_flags;
	sh_int                tactical_size_v;
	sh_int                tactical_size_c;
	sh_int                interp;
	sh_int                scroll_beg;
	sh_int                scroll_end;
	sh_int                perm_str;
	sh_int                perm_int;
	sh_int                perm_wis;
	sh_int                perm_dex;
	sh_int                perm_con;
	sh_int                mod_str;
	sh_int                mod_int;
	sh_int                mod_wis;
	sh_int                mod_dex;
	sh_int                mod_con;
	sh_int                idle;
	sh_int                wimpy;
	sh_int                condition[4];
	sh_int                learned[MAX_SKILL];
	sh_int                practice;
	sh_int                request;
	sh_int                obj_version_number;
	sh_int                god;
	sh_int                reincarnation;
	sh_int                arrested;
	sh_int                armor;
	sh_int                eqdamroll;
	sh_int                eqhitroll;
	sh_int                eqsaves;
	sh_int                honorific;
	sh_int                mclass[MAX_CLASS];

	bool                  appearance[APPEAR_MAX];
	bool                  editmode;
	bool                  edittype;
	bool                  tempmode;
	bool                  auto_flags;

	bool                  switched;
	bool                  trivia_guesses;
};

/*
	a PC's CASTLE information
*/

struct	castle_data
{
	int               	entrance;          /* #vnum of first room in castle	*/
	int               	door_room;	/* #vnum of first door command	*/
	sh_int               door_dir;          /* direction of first door command */
	bool               	has_backdoor;	/* TRUE if the PC has already created a back door */
	int               	cost;          /* the total cost of the castle */
	sh_int               num_rooms;	/* number of rooms created must be < level/4 */
	sh_int               num_mobiles;	/* number of mobiles created must be < level/4 */
	sh_int               num_objects;	/* number of objects created must be < level/4 */
};


struct pvnum_data
{
	char               * name;
	int               	date;
	int               	flags;
	CHAR_DATA          * ch;
};

struct ban_data
{
	BAN_DATA		* next;
	BAN_DATA		* prev;
	char			* name;
	char			* banned_by;
	int				date;
};


struct trivia_data
{
	TRIVIA_DATA	* next;
	TRIVIA_DATA	* prev;
	char			* question;
	char			* answer;
};


struct hiscore_list
{
	HISCORE_DATA	* last;
	HISCORE_DATA	* first;
};

struct hiscore_data
{
	HISCORE_DATA	* next;
	HISCORE_DATA	* prev;
	char			* player;
	int				vnum;
	int				score;
};


struct obj_ref_hash
{
	OBJ_DATA		* first;
	OBJ_DATA		* last;
};


struct room_timer_data
{
	ROOM_TIMER_DATA	* next;
	ROOM_TIMER_DATA	* prev;
	int					vnum;
	int					type;
	int					timer;
};


struct junk_data
{
	JUNK_DATA			* next;
	JUNK_DATA			* prev;
	OBJ_DATA			* obj;
	CHAR_DATA			* mob;
};


struct math_data
{
	MATH_DATA			* next;
	MATH_DATA			* prev;
	char				* str1;
	char				* str2;
	char				* str3;
};

#define MAX_TACTICAL_ROW  20
#define MAX_TACTICAL_COL 160

struct tactical_map
{
	unsigned char   map[MAX_TACTICAL_ROW * MAX_TACTICAL_COL];
	unsigned int  color[MAX_TACTICAL_ROW * MAX_TACTICAL_COL];
};

struct crime_data
{
	CRIME_DATA     * next;
	CRIME_DATA     * prev;
	char           * crime_record;
	char           * arrester;
	int              date;
	char           * releaser;
	sh_int           level;
	int              jailtime;
	bool             released;
};


struct file_data
{
	FILE_DATA 	* next;
	FILE_DATA 	* prev;
	char 		* filename;
	char 		* opentype;
	FILE 		* fp;
};


struct clan_data
{
	CLAN_DATA * next;
	CLAN_DATA * prev;
	char      * name;
	char      * filename;
	char      * description;
	char      * motto;
	char      * leader[5];
	char      * email;
	int         god;
	int         flags;
	int         pkills[MAX_GOD];
	int         pdeaths[MAX_GOD];
	int         mkills;
	int         mdeaths;
	int         tax;
	long long   coffers;
	int         members;
	int         home;
	int         store;
	int         last_saved;
	int         founder_pvnum;
	int         num_guards;
	int         num_healers;
	int         num_trainers;
	int         num_banks;
	int         num_morgues;
	int         num_altars;
	int         num_backdoors;
	char      * bamfself;
	char      * bamfin;
	char      * bamfout;
};

#define HISTORY_KILL_PC			0
#define HISTORY_KILL_EN			1
#define HISTORY_KILL_BY_PC		2
#define HISTORY_KILL_BY_EN		3
#define HISTORY_KILL_BY_NPC		4
#define HISTORY_KILL_NPC			5


#define CLAN_NEUTRAL_KILL               0
#define CLAN_NEUTRAL_DEATH              0
#define CLAN_CHAOS_KILL                 1
#define CLAN_CHAOS_DEATH                1
#define CLAN_ORDER_KILL                 2
#define CLAN_ORDER_DEATH                2

#define AUTO_OFF				0
#define AUTO_AUTO				1
#define AUTO_QUICK				2

#define GOD_NEUTRAL				0
#define GOD_CHAOS				1
#define GOD_ORDER				2

#define FUNCTION_NONE			0
#define FUNCTION_AREA_CREATOR		1
#define FUNCTION_COP			2
#define FUNCTION_ENFORCER		4
#define FUNCTION_GOD			8
#define FUNCTION_BETA_TESTER		16
#define FUNCTION_DEVELOPER		32
#define FUNCTION_SECRET_AGENT		64 /* a cop, of which only the god can see it's one */


/*
	Data which only NPC's have.
*/


struct mud_prog_data
{
	MUD_PROG       * next;
	MUD_PROG       * prev;
	MP_TOKEN       * first_token;
	MP_TOKEN       * last_token;
	int              type;
	char             flags;
	char           * arglist;
	char           * comlist;
};


struct mud_prog_token
{
	MP_TOKEN			* next;
	MP_TOKEN			* prev;
	bool					type;
	char				* string;
	sh_int					value;
	bool					level;
};

/*
	The following structure is set up so that individual monsters may have
	different stats     -  Chaos 5/25/94
*/

struct npc_data
{
	unsigned char          * mob_quest;
	char                   * remember;
	char                   * hate_fear;
	int                      pvnum_last_hit;
	sh_int                   damnodice;
	sh_int                   damsizedice;
	sh_int                   damplus;
	bool                     delay_index;
};

/*
	costs in thousands
*/

#define COST_OF_ENTRANCE		 50000
#define COST_OF_ROOM		 25000
#define COST_OF_DOOR		  1000
#define COST_OF_BACKDOOR		 50000
#define COST_OF_SET				 1000
#define COST_OF_CREATE			25000
#define COST_OF_CLANDOOR          500000

#define CLANHALL_CONSTRUCTION  150000000
#define CLANHALL_FLAG		 100000000

#define RENT_BASIC_CLAN_HALL	  25000000
#define RENT_BASIC_ORDER_HALL	  25000000
#define RENT_PER_GUARD		   1000000
#define RENT_PER_HEALER		   2000000
#define RENT_PER_TRAINER		   1000000
#define RENT_PER_BANK		   2500000
#define RENT_PER_MORGUE		   1500000
#define RENT_PER_ALTAR		   2500000
#define RENT_PER_BACKDOOR	   5000000

#define CASTLE_M_ACT_LIST	ACT_SENTINEL|ACT_WIMPY|ACT_SMART|ACT_WEAK|ACT_DRUNK|ACT_BODY|ACT_RACE|ACT_UNDEAD
#define CASTLE_M_AFF_LIST	AFF_INVISIBLE|AFF_DETECT_INVIS|AFF_DETECT_HIDDEN|AFF_TRUESIGHT|AFF_SANCTUARY|AFF_UNDERSTAND|AFF_PROTECT_EVIL|AFF_PROTECT_GOOD|AFF_SNEAK|AFF_HIDE|AFF_FLYING|AFF_PASS_DOOR|AFF_STEALTH|AFF_CLEAR|AFF_TONGUES|AFF_HASTE
#define CASTLE_M_BODY_LIST	BODY_HEAD|BODY_MOUTH|BODY_EYE|BODY_TORSO|BODY_HIP|BODY_LEG|BODY_ARM|BODY_WING|BODY_TAIL|BODY_TENTACLE|BODY_HORN|BODY_CLAW|BODY_HAND|BODY_FOOT|BODY_BRANCH|BODY_ROOT|BODY_TWIG|BODY_TRUNK
#define CASTLE_R_FLAG_LIST	ROOM_DARK|ROOM_SMOKE|ROOM_NO_MOB|ROOM_INDOORS|ROOM_PRIVATE|ROOM_SAFE|ROOM_NO_RECALL
#define CASTLE_D_FLAG_LIST	EX_ISDOOR|EX_CLOSED|EX_LOCKED|EX_HIDDEN|EX_PICKPROOF|EX_BASHPROOF|EX_MAGICPROOF|EX_PASSPROOF

#define CLAN_M_ACT_LIST         ACT_TRAIN|ACT_PRACTICE|ACT_CLAN_GUARD|ACT_CLAN_HEALER
#define CLAN_R_FLAG_LIST        ROOM_ALTAR|ROOM_BANK|ROOM_MORGUE

#define CASTLE_O_FLAG_LIST	ITEM_GLOW|ITEM_HUM|ITEM_INVIS|ITEM_MAGIC|ITEM_EVIL|ITEM_BLESS
#define CASTLE_O_LID_LIST	CONT_CLOSEABLE|CONT_PICKPROOF|CONT_CLOSED|CONT_LOCKED
#define CASTLE_O_FURNIT_LIST	STAND_ON|STAND_IN|REST_ON|REST_IN|SIT_ON|SIT_IN|SLEEP_ON|SLEEP_IN

#define ERROR_PROG			-1
#define ACT_PROG			BV01
#define SPEECH_PROG			BV02
#define RAND_PROG			BV03
#define FIGHT_PROG			BV04
#define DEATH_PROG			BV05
#define HITPRCNT_PROG		BV06
#define ENTRY_PROG			BV07
#define GREET_PROG			BV08
#define ALL_GREET_PROG		BV09
#define GIVE_PROG			BV10
#define BRIBE_PROG			BV11
#define RANGE_PROG			BV12
#define SOCIAL_PROG			BV13
#define KILL_PROG			BV14
#define GROUP_GREET_PROG		BV15
#define TIME_PROG			BV16
#define REPOP_PROG			BV17
#define DELAY_PROG			BV18
#define EXIT_PROG			BV19
#define TRIGGER_PROG		BV20
#define DESC_PROG			BV21

#define COMMAND_OPROG         BV01
#define DAMAGE_OPROG          BV02
#define DROP_OPROG            BV03
#define GET_OPROG             BV04
#define HIT_OPROG             BV05
#define RAND_OPROG            BV06
#define REMOVE_OPROG          BV07
#define SACRIFICE_OPROG       BV08
#define UNKNOWN_OPROG         BV09
#define WEAR_OPROG            BV10

#define COMMAND_RPROG         BV01
#define UNKNOWN_RPROG         BV02

#define TOKENFLAG_NOT         BV01

#define MPTRIGGER_RAND_PLR    BV01


#define MPTOKEN_UNKNOWN        0
#define MPTOKEN_SOCIAL         1
#define MPTOKEN_COMMAND        2
#define MPTOKEN_IF             3
#define MPTOKEN_AND            4
#define MPTOKEN_OR             5
#define MPTOKEN_ELSE           6
#define MPTOKEN_ELSEIF         7
#define MPTOKEN_ENDIF          8
#define MPTOKEN_BREAK          9
#define MPTOKEN_SWITCH        10
#define MPTOKEN_CASE          11
#define MPTOKEN_DEFAULT       12
#define MPTOKEN_ENDSWITCH     13

/*
	Liquids.
*/

#define LIQ_WATER			0
#define LIQ_BEER			1
#define LIQ_WINE			2
#define LIQ_ALE			3
#define LIQ_DARKALE			4
#define LIQ_WHISKY			5
#define LIQ_LEMONADE		6
#define LIQ_FIREBRT			7
#define LIQ_LOCALSPC		8
#define LIQ_SLIME			9
#define LIQ_MILK			10
#define LIQ_TEA			11
#define LIQ_COFFEE			12
#define LIQ_BLOOD			13
#define LIQ_SALTWATER		14
#define LIQ_COKE			15
#define LIQ_ORANGEJUICE		16
#define LIQ_BRANDY			17
#define LIQ_ICEWINE			18
#define LIQ_RUM			19
#define LIQ_VODKA			20
#define LIQ_CHAMPAGNE		21
#define LIQ_MAX			22

struct liq_type
{
	char				* liq_name;
	char				* liq_color;
	sh_int				liq_affect[3];
};

#define FOOD_BREAD			0
#define FOOD_GARLIC			1
#define FOOD_MAX			2


struct bounty_data
{
	BOUNTY_DATA		* next;
	BOUNTY_DATA		* prev;
	char				* name;
	int					amount;
	int					expires;
};

/*
	Extra description data for a room or object.
*/

struct extra_descr_data
{
	EXTRA_DESCR_DATA	* next;		/* Next in list			*/
	EXTRA_DESCR_DATA	* prev;		/* Prev in list			*/
	char				* keyword;		/* Keyword in look/examine	*/
	char				* description;	/* What to see				*/
};

/*
	Prototype for an object.
*/

struct obj_index_data
{
	OBJ_DATA            * first_instance;
	OBJ_DATA            * last_instance;
	EXTRA_DESCR_DATA    * first_extradesc;
	EXTRA_DESCR_DATA    * last_extradesc;
	AFFECT_DATA         * first_affect;
	AFFECT_DATA         * last_affect;
	MUD_PROG            * first_prog;
	MUD_PROG            * last_prog;
	int                   progtypes;
	OBJ_PROG            * first_oprog;
	OBJ_PROG            * last_oprog;
	int                   oprogtypes;
	AREA_DATA           * area;
	char                * name;
	char                * short_descr;
	char                * long_descr;
	char                * description;

	int                   vnum;
	sh_int                item_type;
	int                   extra_flags;
	sh_int                wear_flags;
	sh_int                weight;
	int                   cost;
	int                   value[8];
	int                   level;
	int                   total_objects;
	char                * attack_string;
	int                   class_flags;
	int                   creator_pvnum;
};


struct poison_data
{
	POISON_DATA		* next;
	bool					for_npc;
	sh_int				poison_type;
	int					instant_damage_low;
	int					instant_damage_high;
	int					constant_duration;	/* In combat rounds */
	int					constant_damage_low;
	int					constant_damage_high;
	int					owner;			/* Pvnum of creator of poison */
	int					poisoner;			/* PVNUM of poisoner */
};

/*
	One object.
*/

struct obj_data
{
	OBJ_DATA            * next;
	OBJ_DATA            * prev;
	OBJ_DATA            * next_content;
	OBJ_DATA            * prev_content;
	OBJ_DATA            * next_instance;
	OBJ_DATA            * prev_instance;
	OBJ_DATA            * next_ref;
	OBJ_DATA            * prev_ref;
	OBJ_DATA            * first_content;
	OBJ_DATA            * last_content;
	OBJ_DATA            * in_obj;
	CHAR_DATA           * carried_by;
	RESET_DATA          * reset;
	AFFECT_DATA         * first_affect;
	AFFECT_DATA         * last_affect;
	OBJ_INDEX_DATA      * pIndexData;
	ROOM_INDEX_DATA     * in_room;
	POISON_DATA         *     poison;
	char                * name;
	char                * short_descr;
	char                * long_descr;
	char                * description;
	sh_int                item_type;
	int                   extra_flags;
	sh_int                wear_flags;
	sh_int                wear_loc;
	sh_int                weight;
	int                   cost;
	sh_int                level;
	sh_int                timer;
	sh_int                sac_timer;
	int                   value[8];
	unsigned char       * obj_quest;
	unsigned short        owned_by;          /*  PVNUM of owner */
	sh_int                content_weight;    /* total weight of container */
	long long             obj_ref_key;
};

/*
	Exit data.
*/

struct exit_data
{
	int                      to_room;
	int                      vnum;
	int                      flags;
	int                      key;
	char                   * keyword;
	char                   * description;
};

/*
	Reset commands:
	'*': comment
	'M': read a mobile
	'O': read an object
	'P': put object in object
	'G': give object to mobile
	'E': equip object to mobile
	'D': set state of door
	'R': randomize room exits
	'S': stop (end of list)
*/

/*
	Area-reset definition.
*/

struct reset_data
{
	RESET_DATA		* next;
	RESET_DATA		* prev;
	char					command;
	int					arg1;
	int					arg2;
	int					arg3;
	CHAR_DATA			* mob;
	OBJ_DATA			* obj;
	RESET_DATA		* container;
};

/*
	Area definition.
*/
struct area_data
{
	AREA_DATA			* next;
	AREA_DATA			* prev;
	RESET_DATA		* first_reset;
	RESET_DATA		* last_reset;
	HELP_DATA			* first_help;
	HELP_DATA			* last_help;
	char				* name;
	char				* filename;
	bool				* area_quest;
	WEATHER_DATA		* weather_info;
	int					average_level;
	sh_int				version;
	sh_int				age;
	sh_int				nplayer;
	sh_int				flags;
	sh_int				tmp;
	sh_int				count;
	sh_int				hi_hard_range;		/* Enforced range */
	sh_int				low_hard_range;
	sh_int				hi_soft_range;		/* Suggested range */
	sh_int				low_soft_range;
	char				* authors;
	char				* resetmsg;
	int					low_r_vnum;
	int					hi_r_vnum;
	int					low_o_vnum;
	int					hi_o_vnum;
	int					low_m_vnum;
	int					hi_m_vnum;
	int					olc_range_lo;
	int					olc_range_hi;
};

/*
	Room type.
*/

struct room_index_data
{
	CHAR_DATA              * first_person;
	CHAR_DATA              * last_person;
	OBJ_DATA               * first_content;
	OBJ_DATA               * last_content;
	EXTRA_DESCR_DATA       * first_extradesc;
	EXTRA_DESCR_DATA       * last_extradesc;
	AREA_DATA              * area;
	EXIT_DATA              * exit[6];
	MUD_PROG               * first_prog;
	MUD_PROG               * last_prog;
	int                      progtypes;
	char                   * name;
	char                   * description;
	int                      vnum;
	long long                room_flags;
	int                      light;
	int                      creator_pvnum;
	short                    sector_type;
	short                    stamp;
	short                    content_count;
	char                   * last_left_name[MAX_LAST_LEFT];
	sh_int                   last_left_bits[MAX_LAST_LEFT];
	time_t                   last_left_time[MAX_LAST_LEFT];
	CHAR_DATA              * sanctify_char;
};

/*
	System usage information
*/

struct usage_data
{
	int					players[24][7];
};

/*
	Types of attacks.
	Must be non-overlapping with spell/skill types,
	but may be arbitrary beyond that.
*/

#define TYPE_UNDEFINED		-1
#define TYPE_HIT			1000
#define TYPE_NOFIGHT		999

/*
	Target types.
*/

#define TAR_IGNORE				0
#define TAR_CHAR_OFFENSIVE		1
#define TAR_CHAR_DEFENSIVE		2
#define TAR_CHAR_SELF			3
#define TAR_OBJ_INV				4
#define TAR_OBJ_CHAR_OFF			5
#define TAR_OBJ_CHAR_DEF			6

struct bitvector_type	/* text of particular body parts */
{
	char				* name;	/* name of bit */
	long long				value;
};

struct body_type  /* text of particular body parts */
{
	char				* name;		/* names of body part when sliced */
	char				* short_descr;	/* short_descr of body part when sliced */
	char				* long_descr;	/* long_descr of body part when sliced */
	char 		* description;	/* description of sliced body part */
	char				* attack;		/* kind of attack the body part does */
	char				* sliced;		/* what it does when it is sliced off */
	int					dam_type;
};

/*
	Skills include spells as a particular case.
*/

struct skill_type
{
	char                * name;                   /* Name of skill    */
	sh_int                skill_level[MAX_CLASS]; /* Level needed by class    */
	SPELL_FUN           * spell_fun;              /* Spell pointer (for spells)    */
	sh_int              * pgsn;                   /* Pointer to associated gsn    */
	sh_int                flags;                  /* skill flags */
	int                   follower;               /* meant for which followers? */
	int                   race;
	int                   dam_type;
	sh_int                target;                 /* Legal targets */
	bool                  position;               /* Position for caster / user    */
	sh_int                slot;                   /* Slot for #OBJECT loading    */
	sh_int                cost;                   /* Minimum mana used        */
	sh_int                beats;                  /* Waiting time after use    */
	char                * noun_damage;            /* Damage message */
	char                * msg_off;                /* Wear off message    */
	char                * msg_obj_off;
};

extern char *telcmds[];

struct telnet_type
{
	char      *name;
	int       flags;
};

extern struct telnet_type telnet_table[];

typedef void MSDP_FUN (struct descriptor_data *d, int index);

struct msdp_type
{
	char     *name;
	int       flags;
	MSDP_FUN *fun;
};

struct msdp_data
{
	char     *value;
	int      flags;
};

extern struct msdp_type msdp_table[];

/*
	These are skill_lookup return values for common skills and spells.
*/

sh_int	gsn_acupunch;
sh_int	gsn_ambush;
sh_int	gsn_anatomy;
sh_int	gsn_anti_magic_shell;
sh_int	gsn_armor_usage;
sh_int	gsn_assassinate;
sh_int	gsn_attack;
sh_int	gsn_second_attack;
sh_int	gsn_third_attack;
sh_int	gsn_fourth_attack;
sh_int	gsn_fifth_attack;
sh_int	gsn_backstab;
sh_int	gsn_bargain;
sh_int	gsn_bash;
sh_int	gsn_berserk;
sh_int	gsn_black_aura;
sh_int	gsn_blindness;
sh_int	gsn_blood_frenzy;
sh_int	gsn_brandish;
sh_int	gsn_brawling;
sh_int	gsn_camp;
sh_int	gsn_charm_person;
sh_int	gsn_circle;
sh_int	gsn_clear_path;
sh_int	gsn_climb;
sh_int	gsn_cripple;
sh_int	gsn_critical_hit;
sh_int	gsn_curse;
sh_int	gsn_detect_evil;
sh_int	gsn_detect_forgery;
sh_int	gsn_detect_good;
sh_int	gsn_disarm;
sh_int	gsn_disguise;
sh_int	gsn_distract;
sh_int	gsn_divert;
sh_int	gsn_dodge;
sh_int	gsn_door_bash;
sh_int	gsn_drain;
sh_int	gsn_dual_wield;
sh_int	gsn_earthbind;
sh_int	gsn_endurance;
sh_int	gsn_enhanced_damage;
sh_int	gsn_faerie_fire;
sh_int	gsn_fear;
sh_int	gsn_fireshield;
sh_int	gsn_first_strike;
sh_int	gsn_fisticuffs;
sh_int	gsn_flame_blade;
sh_int	gsn_glance;
sh_int	gsn_flash_powder;
sh_int	gsn_fly;
sh_int	gsn_forge;
sh_int	gsn_gouge;
sh_int	gsn_greater_backstab;
sh_int	gsn_greater_hunt;
sh_int	gsn_greater_peek;
sh_int	gsn_greater_pick;
sh_int	gsn_greater_stealth;
sh_int	gsn_greater_track;
sh_int	gsn_guard;
sh_int	gsn_haste;
sh_int	gsn_head_butt;
sh_int	gsn_hide;
sh_int	gsn_hunt;
sh_int	gsn_icicle_armor;
sh_int	gsn_impale;
sh_int	gsn_improved_invis;
sh_int	gsn_invis;
sh_int	gsn_kick;
sh_int	gsn_knife;
sh_int	gsn_lightning_reflexes;
sh_int	gsn_lock_lock;
sh_int	gsn_mage_blast;
sh_int	gsn_mage_shield;
sh_int	gsn_magic_mirror;
sh_int	gsn_make_poison;
sh_int	gsn_mana_shackles;
sh_int	gsn_mana_shield;
sh_int	gsn_martial_arts;
sh_int	gsn_mass;
sh_int	gsn_mount;
sh_int	gsn_muffle;
sh_int	gsn_nightmare;
sh_int	gsn_notice;
sh_int	gsn_object_rape;
sh_int	gsn_parry;
sh_int	gsn_pass_without_trace;
sh_int	gsn_peek;
sh_int	gsn_perception;
sh_int	gsn_pick_lock;
sh_int	gsn_plant;
sh_int	gsn_poison;
sh_int	gsn_polymorph;
sh_int	gsn_precision;
sh_int	gsn_quick_draw;
sh_int	gsn_scroll_mastery;
sh_int	gsn_stone_fist;
sh_int	gsn_strangle;
sh_int	gsn_range_cast;
sh_int	gsn_rearm;
sh_int	gsn_recite;
sh_int	gsn_remove_fear;
sh_int	gsn_riposte;
sh_int	gsn_rescue;
sh_int	gsn_resilience;
sh_int	gsn_shadow;
sh_int	gsn_shield_block;
sh_int	gsn_shoot;
sh_int	gsn_sleep;
sh_int	gsn_slow;
sh_int	gsn_snatch;
sh_int	gsn_sneak;
sh_int	gsn_spy;
sh_int	gsn_steal;
sh_int	gsn_stealth;
sh_int	gsn_tame;
sh_int	gsn_throw;
sh_int	gsn_track;
sh_int	gsn_trip;
sh_int	gsn_tumble;
sh_int	gsn_vampiric_touch;
sh_int	gsn_voice;
sh_int	gsn_warmth;
sh_int	gsn_axe_fighting;
sh_int	gsn_bow_fighting;
sh_int	gsn_claw_fighting;
sh_int	gsn_dagger_fighting;
sh_int	gsn_flail_fighting;
sh_int	gsn_mace_fighting;
sh_int	gsn_spear_fighting;
sh_int	gsn_staff_fighting;
sh_int	gsn_sword_fighting;
sh_int	gsn_whip_fighting;
sh_int	gsn_whirl;
sh_int	gsn_withdraw;
sh_int	gsn_word_of_recall;
sh_int	gsn_write_spell;
sh_int	gsn_zap;
sh_int  gsn_steelhand;

/*
	Utility macros.
*/

#define UMIN(a, b)              ((a) < (b) ? (a) : (b))
#define UMAX(a, b)              ((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)         ((b) < (a) ? (a) : (b) > (c) ? (c) : (b))
#define LOWER(c)                ((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)                ((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))

#define HAS_BIT(var, bit)       ((var)  & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define DEL_BIT(var, bit)       ((var) &= (~(bit)))
#define TOG_BIT(var, bit)       ((var) ^= (bit))

#define IS_SET(flag, bit)	((flag)  & (bit))
#define REMOVE_BIT(var, bit)	((var)  &= (~(bit)))
#define TOGGLE_BIT(var, bit)	((var)  ^= (bit))

#define IS_SHIFT(val, bit)	((val)   & (1 << (bit)))
#define SHIFT(bit)              ((1)    << (bit))
#define UNSHIFT(bit)            ((ffs(bit) - 1))

#define get_room_index(vnum)    ((vnum) < 0 ? NULL : (vnum) >= MAX_VNUM ? NULL : room_index[vnum])
#define get_mob_index(vnum)     ((vnum) < 0 ? NULL : (vnum) >= MAX_VNUM ? NULL :  mob_index[vnum])
#define get_obj_index(vnum)     ((vnum) < 0 ? NULL : (vnum) >= MAX_VNUM ? NULL :  obj_index[vnum])

/*
	Character macros.
*/

#define CH(descriptor)                  (descriptor->original ? descriptor->original : descriptor->character)

#define MP_VALID_MOB(ch)                ((ch)->desc == NULL && IS_NPC(ch) && !IS_SET(ch->act, ACT_WILL_DIE) && !IS_AFFECTED(ch, AFF_CHARM) && !IS_AFFECTED(ch, AFF2_POSSESS))

#define IS_NPC(ch)                      ((ch)->npcdata != NULL)
#define IS_IMMORTAL(ch)                 (get_trust(ch) >= LEVEL_IMMORTAL)
#define IS_GOD(ch)                      (!IS_NPC((ch)) && (ch)->pcdata->pvnum < 100)
#define IS_HERO(ch)                     (get_trust(ch) >= LEVEL_HERO)
#define IS_UNDEAD(ch)                   ((IS_NPC(ch) && IS_SET(ch->act, ACT_UNDEAD)) || rspec_req(ch, RSPEC_UNDEAD))

#define IS_AFFECTED(ch, sn)             (sn > 0 ? IS_SET((ch)->affected_by, (sn)) : IS_SET((ch)->affected2_by, 0 - (sn)))

#define CAN_FLY(ch)                     ((IS_AFFECTED(ch, AFF_FLYING)       || rspec_req(ch, RSPEC_FLYING))  && !IS_AFFECTED(ch, AFF2_EARTHBIND))
#define CAN_SWIM(ch)                    (                                     rspec_req(ch, RSPEC_SWIM))
#define CAN_ETHEREAL_WALK(ch)           (IS_AFFECTED(ch, AFF2_ETHEREAL)     || rspec_req(ch, RSPEC_ETHEREAL))
#define CAN_ASTRAL_WALK(ch)             (IS_AFFECTED(ch, AFF2_ASTRAL)       || rspec_req(ch, RSPEC_ASTRAL))

#define CAN_BREATH_WATER(ch)            (IS_AFFECTED(ch, AFF2_BREATH_WATER) || IS_SET(race_table[(ch)->race].flags, RSPEC_BREATH_WATER) || IS_SET(race_table[(ch)->race].flags, RSPEC_UNDEAD))
#define CAN_FIREWALK(ch)                (IS_AFFECTED(ch, AFF2_FIREWALK)     || rspec_req(ch, RSPEC_FIREWALK))

#define IS_GOOD(ch)                     (ch->alignment >= 350)
#define IS_EVIL(ch)                     (ch->alignment <= -350)
#define IS_NEUTRAL(ch)                  (!IS_GOOD(ch) && !IS_EVIL(ch))

#define IS_AWAKE(ch)                    (ch->position > POS_SLEEPING)
#define GET_AC(ch)                      (IS_NPC(ch) ? 0 : ch->pcdata->armor + (IS_AWAKE(ch) ? dex_app[get_curr_dex(ch)].defensive : 0))

#define IS_OUTSIDE(ch)                  (!IS_SET(ch->in_room->room_flags, ROOM_INDOORS))

#define IS_DRUNK(ch, drunk)             (number_percent() < (ch->pcdata->condition[COND_DRUNK] * 2 / drunk))

#define NO_WEATHER_SECT(sect)           (IS_SET(sector_table[sect].flags, SFLAG_NOWEATHER))

#define WAIT_STATE(ch, npulse)          (ch->wait = UMAX(ch->wait, npulse*3/2))

/*
	Object macros.
*/
#define CAN_WEAR(obj, part)		(IS_SET(obj->wear_flags,  part))
#define IS_OBJ_STAT(obj, stat)	(IS_SET(obj->extra_flags, stat))

/*
	Description macros.
*/

#define PERS(ch, looker)			(can_see(looker, ch) ? get_name(ch) : "someone")

/*
	Structure for a command in the command lookup table.
*/

struct cmd_type
{
	char				* const name;
	DO_FUN			* do_fun;
	bool					position;
	sh_int				level;
	bool					log;
	int					flags;
};

/*
	Structure for a social in the socials table.
*/

struct social_type
{
	char				* name;
	char                            * type; 
	char				* self_no_vict;
	char				* room_no_vict;
	char				* vict_to_self;
	char				* vict_to_room;
	char				* vict_to_vict;
	char				* self_to_self;
	char				* self_to_room;
};



/*
	Global constants.
*/

extern	const	struct	str_app_type    str_app		[MAX_STAT];
extern	const	struct	int_app_type    int_app		[MAX_STAT];
extern	const	struct	wis_app_type    wis_app		[MAX_STAT];
extern	const	struct	dex_app_type    dex_app		[MAX_STAT];
extern	const	struct	con_app_type    con_app         [MAX_STAT];

extern    const   struct    bitvector_type  bitvector_table [MAX_BITVECTOR];
extern    const   struct    topic_data      topic_table     [MAX_TOPIC];
extern    const   struct    class_type      class_table     [MAX_CLASS];
extern    const   struct    race_type       race_table      [MAX_RACE];
extern    const   struct    god_type        god_table       [MAX_GOD];
extern    const   struct    sector_type     sector_table    [SECT_MAX];
extern    const   struct    language_type   language_table  [MAX_LANGUAGE];
extern    const   struct    cmd_type        cmd_table       [];

extern    const   struct    liq_type        liq_table       [LIQ_MAX];
extern    const   struct    skill_type      skill_table     [MAX_SKILL];
extern    const   struct    body_type       body_table      [];
extern    const   struct    social_type     social_table    [];

extern    const    float    class_eq_table  [MAX_CLASS]     [MAX_WEAR];
extern            sh_int    cmd_gsn         [];
extern              char *  dir_name        [];
extern              char *  dir_name_short  [];
extern            sh_int    rev_dir         [];
extern              char *  wind_dir_name   [];

extern  char *  const   class_flags             [];
extern  char *  const   race_specs              [];
extern	char * const	mprog_flags		[];
extern	char * const	area_flags		[];
extern	char * const	f_flags			[];
extern	char * const	cont_flags		[];
extern	char * const	exit_flags		[];
extern	char * const	reset_exit_types	[];
extern	char * const	reset_rand_types	[];
extern	char * const	r_flags 		[];
extern	char * const	o_flags 		[];
extern	char * const	act_flags 	[];
extern	char * const	a_flags 		[];
extern	char * const	plr_flags 	[];
extern	char * const	wear_locs 	[];
extern	char * const	part_flags 	[];
extern	char * const	o_types 		[];
extern	char * const	w_flags 		[];
extern	char * const	a_types 		[];
extern	char * const	p_types			[];
extern	char * const	sex_types			[];
extern	char * const	r_types			[];
extern	char * const	sect_types		[];
extern	char * const	bow_types			[];

#include "mth.h"



/*
	Global variables.
*/

MUD_DATA			* mud;

FILE				* fpReserve;
FILE				* fpAppend;

ROOM_INDEX_DATA	* room_index[MAX_VNUM];
MOB_INDEX_DATA		* mob_index[MAX_VNUM];
OBJ_INDEX_DATA		* obj_index[MAX_VNUM];
PVNUM_DATA		* pvnum_index[MAX_PVNUM];
char				* str_empty;
char					mpdamstring[MAX_INPUT_LENGTH];



/*
	Command functions.
	Defined in act_*.c (mostly).
*/

DECLARE_DO_FUN(do_advance);
DECLARE_DO_FUN(do_afk);
DECLARE_DO_FUN(do_appearance);
DECLARE_DO_FUN(do_areas);
DECLARE_DO_FUN(do_arrest);
DECLARE_DO_FUN(do_at);
DECLARE_DO_FUN(do_backstab);
DECLARE_DO_FUN(do_ban);
DECLARE_DO_FUN(do_bank);
DECLARE_DO_FUN(do_bounty);
DECLARE_DO_FUN(do_brandish);
DECLARE_DO_FUN(do_buy);
DECLARE_DO_FUN(do_identify);
DECLARE_DO_FUN(do_cast);
DECLARE_DO_FUN(do_mana);
DECLARE_DO_FUN(do_move);
DECLARE_DO_FUN(do_range_cast);
DECLARE_DO_FUN(do_channel_talk);
DECLARE_DO_FUN(do_chat);
DECLARE_DO_FUN(do_fos);
DECLARE_DO_FUN(do_beep);
DECLARE_DO_FUN(do_plan);
DECLARE_DO_FUN(do_close);
DECLARE_DO_FUN(do_commands);
DECLARE_DO_FUN(do_skills);
DECLARE_DO_FUN(do_compare);
DECLARE_DO_FUN(do_config);
DECLARE_DO_FUN(do_consider);
DECLARE_DO_FUN(do_credits);
DECLARE_DO_FUN(do_deny);
DECLARE_DO_FUN(do_description);
DECLARE_DO_FUN(do_disarm);
DECLARE_DO_FUN(do_disconnect);
DECLARE_DO_FUN(do_dismount);
DECLARE_DO_FUN(do_down);
DECLARE_DO_FUN(do_drink);
DECLARE_DO_FUN(do_drop);
DECLARE_DO_FUN(do_east);
DECLARE_DO_FUN(do_eat);
DECLARE_DO_FUN(do_echo);
DECLARE_DO_FUN(do_emote);
DECLARE_DO_FUN(do_enter);
DECLARE_DO_FUN(do_equipment);
DECLARE_DO_FUN(do_examine);
DECLARE_DO_FUN(do_exits);
DECLARE_DO_FUN(do_fill);
DECLARE_DO_FUN(do_flee);
DECLARE_DO_FUN(do_follow);
DECLARE_DO_FUN(do_force);
DECLARE_DO_FUN(do_forage);
DECLARE_DO_FUN(do_freeze);
DECLARE_DO_FUN(do_get);
DECLARE_DO_FUN(do_give);
DECLARE_DO_FUN(do_goto);
DECLARE_DO_FUN(do_group);
DECLARE_DO_FUN(do_gtell);
DECLARE_DO_FUN(do_help);
DECLARE_DO_FUN(do_hide);
DECLARE_DO_FUN(do_buildlight);
DECLARE_DO_FUN(do_holylight);
DECLARE_DO_FUN(do_timemode);
DECLARE_DO_FUN(do_initiate);

DECLARE_DO_FUN(do_morph);
DECLARE_DO_FUN(do_mudlist);
DECLARE_DO_FUN(do_mute);
DECLARE_DO_FUN(do_peek);
DECLARE_DO_FUN(do_pload);
DECLARE_DO_FUN(do_poof);
DECLARE_DO_FUN(do_pquit);
DECLARE_DO_FUN(do_prompt);
DECLARE_DO_FUN(do_victory_list);
DECLARE_DO_FUN(do_travel);
DECLARE_DO_FUN(do_trivia);
DECLARE_DO_FUN(do_tag);
DECLARE_DO_FUN(do_notag);
DECLARE_DO_FUN(do_most);
DECLARE_DO_FUN(do_dump);
DECLARE_DO_FUN(do_map);
DECLARE_DO_FUN(do_a);
DECLARE_DO_FUN(do_reincarnate);
DECLARE_DO_FUN(do_tactical);
DECLARE_DO_FUN(do_spam);
DECLARE_DO_FUN(do_resetarea);
DECLARE_DO_FUN(do_savearea);
DECLARE_DO_FUN(do_email);
DECLARE_DO_FUN(do_html);
DECLARE_DO_FUN(do_strangle);
DECLARE_DO_FUN(do_strength);
DECLARE_DO_FUN(do_intelligence);
DECLARE_DO_FUN(do_wisdom);
DECLARE_DO_FUN(do_constitution);
DECLARE_DO_FUN(do_dexterity);
DECLARE_DO_FUN(do_lookup);
DECLARE_DO_FUN(do_know);
DECLARE_DO_FUN(do_create);
DECLARE_DO_FUN(do_engrave);
DECLARE_DO_FUN(do_auto);
DECLARE_DO_FUN(do_repeat);
DECLARE_DO_FUN(do_release);
DECLARE_DO_FUN(do_file);
DECLARE_DO_FUN(do_openfiles);
DECLARE_DO_FUN(do_finger);
DECLARE_DO_FUN(do_history);
DECLARE_DO_FUN(do_doorset);
DECLARE_DO_FUN(do_rescale);
DECLARE_DO_FUN(do_connect);
DECLARE_DO_FUN(do_prog);
DECLARE_DO_FUN(do_fixpass);
DECLARE_DO_FUN(do_inventory);
DECLARE_DO_FUN(do_invis);
DECLARE_DO_FUN(do_cloak);
DECLARE_DO_FUN(do_head_butt);
DECLARE_DO_FUN(do_kick);
DECLARE_DO_FUN(do_mount);
DECLARE_DO_FUN(do_tame);
DECLARE_DO_FUN(do_bash);
DECLARE_DO_FUN(do_bashdoor);
DECLARE_DO_FUN(do_acupunch);
DECLARE_DO_FUN(do_whirl);
DECLARE_DO_FUN(do_kill);
DECLARE_DO_FUN(do_list);
DECLARE_DO_FUN(do_lock);
DECLARE_DO_FUN(do_log);
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_scan);
DECLARE_DO_FUN(do_memory);
DECLARE_DO_FUN(do_cpu);
DECLARE_DO_FUN(do_find);
DECLARE_DO_FUN(do_mpasound);
DECLARE_DO_FUN(do_mpareaecho);
DECLARE_DO_FUN(do_mpaset);
DECLARE_DO_FUN(do_mpat);
DECLARE_DO_FUN(do_mpcalculate);
DECLARE_DO_FUN(do_mpcommands);
DECLARE_DO_FUN(do_mpdamage);
DECLARE_DO_FUN(do_mpdelay);
DECLARE_DO_FUN(do_mpdo);
DECLARE_DO_FUN(do_mpecho);
DECLARE_DO_FUN(do_mpechoaround);
DECLARE_DO_FUN(do_mpechoat);
DECLARE_DO_FUN(do_mpforce);
DECLARE_DO_FUN(do_mplog);
DECLARE_DO_FUN(do_mpgoto);
DECLARE_DO_FUN(do_mpjunk);
DECLARE_DO_FUN(do_mpjunk_person);
DECLARE_DO_FUN(do_mpkill);
DECLARE_DO_FUN(do_mpmaze);
DECLARE_DO_FUN(do_mpmload);
DECLARE_DO_FUN(do_mpoload);
DECLARE_DO_FUN(do_mppurge);
DECLARE_DO_FUN(do_mpquiet);
DECLARE_DO_FUN(do_mptransfer);
DECLARE_DO_FUN(do_mptrigger);
DECLARE_DO_FUN(do_mpmset);
DECLARE_DO_FUN(do_mposet);
DECLARE_DO_FUN(do_mpmadd);
DECLARE_DO_FUN(do_mpoadd);
DECLARE_DO_FUN(do_mpseed);
DECLARE_DO_FUN(do_mpswap);
DECLARE_DO_FUN(do_mpgorand);
DECLARE_DO_FUN(do_mpzset);
DECLARE_DO_FUN(do_set);
DECLARE_DO_FUN(do_stat);
DECLARE_DO_FUN(do_show);
DECLARE_DO_FUN(do_murde);
DECLARE_DO_FUN(do_murder);
DECLARE_DO_FUN(do_nban);
DECLARE_DO_FUN(do_north);
DECLARE_DO_FUN(do_note);
DECLARE_DO_FUN(do_load);
DECLARE_DO_FUN(do_open);
DECLARE_DO_FUN(do_order);
DECLARE_DO_FUN(do_pardon);
DECLARE_DO_FUN(do_password);
DECLARE_DO_FUN(do_peace);
DECLARE_DO_FUN(do_pick);
DECLARE_DO_FUN(do_pose);
DECLARE_DO_FUN(do_practice);
DECLARE_DO_FUN(do_honorific);
DECLARE_DO_FUN(do_purge);
DECLARE_DO_FUN(do_put);
DECLARE_DO_FUN(do_quaff);
DECLARE_DO_FUN(do_qui);
DECLARE_DO_FUN(do_quit);
DECLARE_DO_FUN(do_reboo);
DECLARE_DO_FUN(do_reboot);
DECLARE_DO_FUN(do_rename);
DECLARE_DO_FUN(do_recall);
DECLARE_DO_FUN(do_death);
DECLARE_DO_FUN(do_recho);
DECLARE_DO_FUN(do_recite);
DECLARE_DO_FUN(do_relog);
DECLARE_DO_FUN(do_remove);
DECLARE_DO_FUN(do_rent);
DECLARE_DO_FUN(do_reply);
DECLARE_DO_FUN(do_report);
DECLARE_DO_FUN(do_rescue);
DECLARE_DO_FUN(do_rest);
DECLARE_DO_FUN(do_restore);
DECLARE_DO_FUN(do_return);
DECLARE_DO_FUN(do_revert);
DECLARE_DO_FUN(do_sacrifice);
DECLARE_DO_FUN(do_save);
DECLARE_DO_FUN(do_say);
DECLARE_DO_FUN(do_score);
DECLARE_DO_FUN(do_status);
DECLARE_DO_FUN(do_affects);
DECLARE_DO_FUN(do_search);
DECLARE_DO_FUN(do_sell);
DECLARE_DO_FUN(do_shout);
DECLARE_DO_FUN(do_sing);
DECLARE_DO_FUN(do_shutdow);
DECLARE_DO_FUN(do_shutdown);
DECLARE_DO_FUN(do_sit);
DECLARE_DO_FUN(do_silence);
DECLARE_DO_FUN(do_sla);
DECLARE_DO_FUN(do_slay);
DECLARE_DO_FUN(do_slaughter);
DECLARE_DO_FUN(do_sleep);
DECLARE_DO_FUN(do_sneak);
DECLARE_DO_FUN(do_snoop);
DECLARE_DO_FUN(do_socials);
DECLARE_DO_FUN(do_south);
DECLARE_DO_FUN(do_split);
DECLARE_DO_FUN(do_stand);
DECLARE_DO_FUN(do_steal);
DECLARE_DO_FUN(do_switch);
DECLARE_DO_FUN(do_tell);
DECLARE_DO_FUN(do_time);
DECLARE_DO_FUN(do_level);
DECLARE_DO_FUN(do_vt100);
DECLARE_DO_FUN(do_color);
DECLARE_DO_FUN(do_buffer);
DECLARE_DO_FUN(do_grep);
DECLARE_DO_FUN(do_port);
DECLARE_DO_FUN(do_speed);
DECLARE_DO_FUN(do_alias);
DECLARE_DO_FUN(do_unalias);
DECLARE_DO_FUN(do_block);
DECLARE_DO_FUN(do_class);
DECLARE_DO_FUN(do_multi);
DECLARE_DO_FUN(do_clock);
DECLARE_DO_FUN(do_refresh);
DECLARE_DO_FUN(do_tick);
DECLARE_DO_FUN(do_title);
DECLARE_DO_FUN(do_train);
DECLARE_DO_FUN(do_transfer);
DECLARE_DO_FUN(do_trust);
DECLARE_DO_FUN(do_unlock);
DECLARE_DO_FUN(do_up);
DECLARE_DO_FUN(do_value);
DECLARE_DO_FUN(do_visible);
DECLARE_DO_FUN(do_wake);
DECLARE_DO_FUN(do_wear);
DECLARE_DO_FUN(do_weather);
DECLARE_DO_FUN(do_west);
DECLARE_DO_FUN(do_where);
DECLARE_DO_FUN(do_who);
DECLARE_DO_FUN(do_language);
DECLARE_DO_FUN(do_wimpy);
DECLARE_DO_FUN(do_test);
DECLARE_DO_FUN(do_wizhelp);
DECLARE_DO_FUN(do_wizmap);
DECLARE_DO_FUN(do_wizlock);
DECLARE_DO_FUN(do_zap);
DECLARE_DO_FUN(do_shoot);
DECLARE_DO_FUN(do_track);
DECLARE_DO_FUN(do_ambush);
DECLARE_DO_FUN(do_notice);
DECLARE_DO_FUN(do_polymorph);
DECLARE_DO_FUN(do_delete);
DECLARE_DO_FUN(do_suicide);
DECLARE_DO_FUN(do_speak);
DECLARE_DO_FUN(do_throw);
DECLARE_DO_FUN(do_shadow);
DECLARE_DO_FUN(do_divert);
DECLARE_DO_FUN(do_voice);
DECLARE_DO_FUN(do_knife);
DECLARE_DO_FUN(do_make_poison);
DECLARE_DO_FUN(do_make_flash);
DECLARE_DO_FUN(do_stealth);
DECLARE_DO_FUN(do_disguise);
DECLARE_DO_FUN(do_spy);
DECLARE_DO_FUN(do_distract);
DECLARE_DO_FUN(do_snatch);
DECLARE_DO_FUN(do_clear_path);
DECLARE_DO_FUN(do_hunt);
DECLARE_DO_FUN(do_evaluate);
DECLARE_DO_FUN(do_usage);
DECLARE_DO_FUN(do_castle);
DECLARE_DO_FUN(do_assassin);
DECLARE_DO_FUN(do_assassinate);
DECLARE_DO_FUN(do_glance);
DECLARE_DO_FUN(do_plant);
DECLARE_DO_FUN(do_camp);
DECLARE_DO_FUN(do_mass);
DECLARE_DO_FUN(do_berserk);
DECLARE_DO_FUN(do_blood_frenzy);
DECLARE_DO_FUN(do_forge);
DECLARE_DO_FUN(do_destroy);
DECLARE_DO_FUN(do_gohome);
DECLARE_DO_FUN(do_copyover);
DECLARE_DO_FUN(do_trip);
DECLARE_DO_FUN(do_gouge);
DECLARE_DO_FUN(do_cripple);
DECLARE_DO_FUN(do_circle);
DECLARE_DO_FUN(do_hearlog);
DECLARE_DO_FUN(do_attack);
DECLARE_DO_FUN(do_clans);
DECLARE_DO_FUN(do_outcast);
DECLARE_DO_FUN(do_makeclan);
DECLARE_DO_FUN(do_clanset);
DECLARE_DO_FUN(do_coffer);
DECLARE_DO_FUN(do_pledge);
DECLARE_DO_FUN(do_clanwhere);
DECLARE_DO_FUN(do_renounce);
DECLARE_DO_FUN(do_nominate);
DECLARE_DO_FUN(do_heal);

DECLARE_DO_FUN(do_drain);

DECLARE_DO_FUN(do_area_assign);
DECLARE_DO_FUN(do_edit);
DECLARE_DO_FUN(do_forcerent);
DECLARE_DO_FUN(do_forceren);
DECLARE_DO_FUN(do_clan_message);
DECLARE_DO_FUN(do_donate);
DECLARE_DO_FUN(do_immtalk);
DECLARE_DO_FUN(do_devtalk);
DECLARE_DO_FUN(do_stack);
DECLARE_DO_FUN(do_version);
DECLARE_DO_FUN(do_showcurve);

/*
	Spell functions.
	Defined in magic.c.
*/

DECLARE_SPELL_FUN(spell_null);
DECLARE_SPELL_FUN(spell_acid_blast);
DECLARE_SPELL_FUN(spell_acid_breath);
DECLARE_SPELL_FUN(spell_animate_dead);
DECLARE_SPELL_FUN(spell_animate_object);
DECLARE_SPELL_FUN(spell_armor);
DECLARE_SPELL_FUN(spell_astral_projection);
DECLARE_SPELL_FUN(spell_banish);
DECLARE_SPELL_FUN(spell_black_aura);
DECLARE_SPELL_FUN(spell_bless);
DECLARE_SPELL_FUN(spell_blindness);
DECLARE_SPELL_FUN(spell_block_area);
DECLARE_SPELL_FUN(spell_burning_hands);
DECLARE_SPELL_FUN(spell_call_lightning);
DECLARE_SPELL_FUN(spell_cause_critical);
DECLARE_SPELL_FUN(spell_cause_light);
DECLARE_SPELL_FUN(spell_cause_serious);
DECLARE_SPELL_FUN(spell_chain_lightning);
DECLARE_SPELL_FUN(spell_change_sex);
DECLARE_SPELL_FUN(spell_charm_person);
DECLARE_SPELL_FUN(spell_chill_touch);
DECLARE_SPELL_FUN(spell_color_spray);
DECLARE_SPELL_FUN(spell_continual_light);
DECLARE_SPELL_FUN(spell_control_weather);
DECLARE_SPELL_FUN(spell_create_food);
DECLARE_SPELL_FUN(spell_create_spring);
DECLARE_SPELL_FUN(spell_cure_blindness);
DECLARE_SPELL_FUN(spell_cure_critical);
DECLARE_SPELL_FUN(spell_cure_light);
DECLARE_SPELL_FUN(spell_cure_poison);
DECLARE_SPELL_FUN(spell_cure_serious);
DECLARE_SPELL_FUN(spell_curse);
DECLARE_SPELL_FUN(spell_demon);
DECLARE_SPELL_FUN(spell_detect_evil);
DECLARE_SPELL_FUN(spell_detect_good);
DECLARE_SPELL_FUN(spell_detect_hidden);
DECLARE_SPELL_FUN(spell_detect_invis);
DECLARE_SPELL_FUN(spell_detect_poison);
DECLARE_SPELL_FUN(spell_dispel_evil);
DECLARE_SPELL_FUN(spell_dispel_good);
DECLARE_SPELL_FUN(spell_dispel_magic);
DECLARE_SPELL_FUN(spell_dispel_undead);
DECLARE_SPELL_FUN(spell_divine_inspiration);
DECLARE_SPELL_FUN(spell_earthbind);
DECLARE_SPELL_FUN(spell_earthen_spirit);
DECLARE_SPELL_FUN(spell_earthquake);
DECLARE_SPELL_FUN(spell_enchant_weapon);
DECLARE_SPELL_FUN(spell_energy_bolt);
DECLARE_SPELL_FUN(spell_energy_drain);
DECLARE_SPELL_FUN(spell_energy_shift);
DECLARE_SPELL_FUN(spell_enhanced_heal);
DECLARE_SPELL_FUN(spell_enhance_object);
DECLARE_SPELL_FUN(spell_enhanced_rest);
DECLARE_SPELL_FUN(spell_enhanced_revive);
DECLARE_SPELL_FUN(spell_envision);
DECLARE_SPELL_FUN(spell_ethereal_travel);
DECLARE_SPELL_FUN(spell_fear);
DECLARE_SPELL_FUN(spell_flame_blade);
DECLARE_SPELL_FUN(spell_flamewave);
DECLARE_SPELL_FUN(spell_faerie_fire);
DECLARE_SPELL_FUN(spell_faerie_fog);
DECLARE_SPELL_FUN(spell_feast	);
DECLARE_SPELL_FUN(spell_fireball);
DECLARE_SPELL_FUN(spell_fire_breath);
DECLARE_SPELL_FUN(spell_flamestrike);
DECLARE_SPELL_FUN(spell_fly	);
DECLARE_SPELL_FUN(spell_frost_breath);
DECLARE_SPELL_FUN(spell_gas_breath);
DECLARE_SPELL_FUN(spell_gate);
DECLARE_SPELL_FUN(spell_general_purpose);
DECLARE_SPELL_FUN(spell_giant_strength);
DECLARE_SPELL_FUN(spell_globe_of_darkness);
DECLARE_SPELL_FUN(spell_harm);
DECLARE_SPELL_FUN(spell_haste);
DECLARE_SPELL_FUN(spell_heal);
DECLARE_SPELL_FUN(spell_high_explosive);
DECLARE_SPELL_FUN(spell_ice_arrow);
DECLARE_SPELL_FUN(spell_ice_layer);
DECLARE_SPELL_FUN(spell_icicle_armor);
DECLARE_SPELL_FUN(spell_induction);
DECLARE_SPELL_FUN(spell_invis_obj);
DECLARE_SPELL_FUN(spell_homonculous);
DECLARE_SPELL_FUN(spell_identify);
DECLARE_SPELL_FUN(spell_infravision);
DECLARE_SPELL_FUN(spell_invis);
DECLARE_SPELL_FUN(spell_lightning_breath);
DECLARE_SPELL_FUN(spell_locate_object);
DECLARE_SPELL_FUN(spell_mage_shield);
DECLARE_SPELL_FUN(spell_magic_mirror);
DECLARE_SPELL_FUN(spell_magic_missile);
DECLARE_SPELL_FUN(spell_mana_shackles);
DECLARE_SPELL_FUN(spell_mana_shield);
DECLARE_SPELL_FUN(spell_mass_invis);
DECLARE_SPELL_FUN(spell_mindblast);
DECLARE_SPELL_FUN(spell_paralyzing_embrace);
DECLARE_SPELL_FUN(spell_pass_door);
DECLARE_SPELL_FUN(spell_petrifying_touch);
DECLARE_SPELL_FUN(spell_poison);
DECLARE_SPELL_FUN(spell_protection_evil);
DECLARE_SPELL_FUN(spell_protection_good);
DECLARE_SPELL_FUN(spell_psionic_shockwave);
DECLARE_SPELL_FUN(spell_quicken);
DECLARE_SPELL_FUN(spell_refresh);
DECLARE_SPELL_FUN(spell_remove_curse);
DECLARE_SPELL_FUN(spell_remove_fear);
DECLARE_SPELL_FUN(spell_restore	);
DECLARE_SPELL_FUN(spell_rift);
DECLARE_SPELL_FUN(spell_rip);
DECLARE_SPELL_FUN(spell_sanctuary);
DECLARE_SPELL_FUN(spell_sanctify);
DECLARE_SPELL_FUN(spell_shield);
DECLARE_SPELL_FUN(spell_shocking_grasp);
DECLARE_SPELL_FUN(spell_sleep);
DECLARE_SPELL_FUN(spell_snake_dart);
DECLARE_SPELL_FUN(spell_song_of_the_seas);
DECLARE_SPELL_FUN(spell_stability);
DECLARE_SPELL_FUN(spell_stone_fist);
DECLARE_SPELL_FUN(spell_stone_skin);
DECLARE_SPELL_FUN(spell_summon);
DECLARE_SPELL_FUN(spell_teleport);
DECLARE_SPELL_FUN(spell_totem);
DECLARE_SPELL_FUN(spell_tremor);
DECLARE_SPELL_FUN(spell_ventriloquate);
DECLARE_SPELL_FUN(spell_waterburst);
DECLARE_SPELL_FUN(spell_weaken);
DECLARE_SPELL_FUN(spell_windblast);
DECLARE_SPELL_FUN(spell_winged_call);
DECLARE_SPELL_FUN(spell_word_of_recall);
DECLARE_SPELL_FUN(spell_write_spell);


DECLARE_SPELL_FUN(spell_beast);
DECLARE_SPELL_FUN(spell_shade);
DECLARE_SPELL_FUN(spell_phantasm);
DECLARE_SPELL_FUN(spell_tongues);
DECLARE_SPELL_FUN(spell_understand);
DECLARE_SPELL_FUN(spell_illusion);
DECLARE_SPELL_FUN(spell_mirror_image);
DECLARE_SPELL_FUN(spell_hallucinate);
DECLARE_SPELL_FUN(spell_breath_water);
DECLARE_SPELL_FUN(spell_mage_blast);
DECLARE_SPELL_FUN(spell_confusion);
DECLARE_SPELL_FUN(spell_benediction);
DECLARE_SPELL_FUN(spell_righteous_fury);
DECLARE_SPELL_FUN(spell_soothing_touch);
DECLARE_SPELL_FUN(spell_farheal);
DECLARE_SPELL_FUN(spell_farrevive);
DECLARE_SPELL_FUN(spell_holy_word);
DECLARE_SPELL_FUN(spell_unholy_word);
DECLARE_SPELL_FUN(spell_invigorate);
DECLARE_SPELL_FUN(spell_improved_invis);
DECLARE_SPELL_FUN(spell_truesight);
DECLARE_SPELL_FUN(spell_hallucinatory_terrain);
DECLARE_SPELL_FUN(spell_nightmare);
DECLARE_SPELL_FUN(spell_smoke);
DECLARE_SPELL_FUN(spell_recharge);
DECLARE_SPELL_FUN(spell_torrid_balm);
DECLARE_SPELL_FUN(spell_transport);
DECLARE_SPELL_FUN(spell_anti_magic_shell);
DECLARE_SPELL_FUN(spell_possess);
DECLARE_SPELL_FUN(spell_vampiric_touch);
DECLARE_SPELL_FUN(spell_slow);
DECLARE_SPELL_FUN(spell_elemental);
DECLARE_SPELL_FUN(spell_unbarring_ways);
DECLARE_SPELL_FUN(spell_brew_potion);
DECLARE_SPELL_FUN(spell_fireshield);
DECLARE_SPELL_FUN(spell_metamorphose_liquids);
DECLARE_SPELL_FUN(spell_firewalk);
DECLARE_SPELL_FUN(spell_steelhand);
DECLARE_SPELL_FUN(spell_sloth);
DECLARE_SPELL_FUN(spell_touch_of_idiocy);

/*
	OS-dependent declarations.
	These are all very standard library functions,
	but some systems have incomplete or non-ansi header files.
*/

#if defined(_AIX)
char	* crypt     args((const char *key, const char *salt));
#endif

#if defined(apollo)
int         atoi          args((const char *string));
void * calloc     args((unsigned nelem, size_t size));
char * crypt     args((const char *key, const char *salt));
#endif

#if defined(hpux)
char * crypt     args((const char *key, const char *salt));
#endif

#if	defined(interactive)
#endif

#if defined(linux)
char * crypt     args((const char *key, const char *salt));
#endif

#if	defined(MIPS_OS)
char * crypt     args((const char *key, const char *salt));
#endif

#if	defined(NeXT)
char * crypt     args((const char *key, const char *salt));
#endif

#if	defined(sequent)
char * crypt     args((const char *key, const char *salt));
int	fclose          args((FILE *stream));
int	fprintf          args((FILE *stream, const char *format, ...));
int	fread          args((void *ptr, int size, int n, FILE *stream));
int	fseek          args((FILE *stream, long offset, int ptrname));
void	perror          args((const char *s));
int	ungetc          args((int c, FILE *stream));
#endif

/*
	The crypt(3) function is not available on some operating systems.
	In particular, the U.S. Government prohibits its export from the
	United States to foreign countries.
	Turn on NOCRYPT to keep passwords in plain text.
*/

#if defined(NOCRYPT)
#define crypt(s1, s2)	(s1)
#endif

#define PLAYER_DIR		"../player"
#define CLAN_DIR		"../clans"
#define NULL_FILE		"/dev/null"

#define AREA_LIST		"data/area.lst"
#define TRIVIA_LIST		"data/trivia.lst"
#define COPYOVER_FILE	"data/copyover.dat"
#define SHUTDOWN_FILE	"shutdown.txt"
#define VERSION_FILE	"../src/VERSION"

/*
	redefenitions of the executables. We now have emud and current_emud - Manwe
*/

#define REAL_FILE "../bin/current_lola" /* executable path */

/*
	Our function prototypes.
	One big lump ... this is every function in Merc.
*/

#define CD  CHAR_DATA
#define MID MOB_INDEX_DATA
#define OD  OBJ_DATA
#define OID OBJ_INDEX_DATA
#define RID ROOM_INDEX_DATA
#define RD  RESET_DATA
#define AD  AREA_DATA
#define TM  TACTICAL_MAP
#define HD  HELP_DATA
#define BD  BOUNTY_DATA
#define ED  EXIT_DATA

/*
	color.c
*/

int substitute_color             args((char *input, char *output, int colors));

/*
	act_comm.c
*/


void        add_follower         args((CHAR_DATA *ch, CHAR_DATA *master));
void        stop_follower        args((CHAR_DATA *ch));
void        die_follower         args((CHAR_DATA *ch));
void        add_shadow           args((CHAR_DATA *ch, CHAR_DATA *master));
void        stop_shadow          args((CHAR_DATA *ch));
bool        is_valid_password    args((char *pass));
bool        is_same_group        args((CHAR_DATA *ach, CHAR_DATA *bch));
bool        vnum_in_group        args((CHAR_DATA *ach, int mobvnum));
bool        pvnum_in_group       args((CHAR_DATA *ch, int pvnum));
void        do_battle	         args((const char * fmt, ...));
CD        * start_partial_load   args((CHAR_DATA *ch, char *argument));
void        clear_partial_load   args((CHAR_DATA *ch));
void        wait_state           args((CHAR_DATA *ch, int npulse));
char      * ansi_strip           args((char *txi));
void        do_llog	         args((CHAR_DATA *ch, char *argument));
char      * center	         args((char *inp, int length));
char      * justify	         args((char *inp, int length));
char      * ansi_justify         args((char *inp, int length));
void        send_goodbye         args((DESCRIPTOR_DATA *d));
bool        check_parse_name     args((char * , bool));
void        add_obj_ref_hash     args((OBJ_DATA *obj));
void        rem_obj_ref_hash     args((OBJ_DATA *obj));
void        enter_game           args((CHAR_DATA *ch));
void        display_empty_screen args((DESCRIPTOR_DATA *d));
void        arachnos_devel       args((char *fmt, ...));
void        arachnos_mudlist     args((char *fmt, ...));

/*
	vt100.c
*/

TM        * get_tactical_map            args((CHAR_DATA *ch));
void        clear_tactical_map          args((TACTICAL_MAP *tact));
TM        * get_diff_tactical           args((CHAR_DATA *ch));
char      * get_tactical_string         args((CHAR_DATA *ch, TACTICAL_MAP *tact));
void        vt100on                     args((CHAR_DATA *));
void        vt100off                    args((CHAR_DATA *));
void        vt100prompt                 args((CHAR_DATA *));
void        vt100prompter               args((CHAR_DATA *));
char      * ansi_translate              args((char *txt));
char      * ansi_translate_def          args((CHAR_DATA *ch, char *txt, char *def));
char      * ansi_translate_text         args((CHAR_DATA *ch, char *txt));
char      * ansi_compress               args((CHAR_DATA *ch, char *txt, int color, int code));
char      * get_color_diff              args((CHAR_DATA *, int, int, int, int, int, int));
int         ansi_strlen                 args((char *txt));
void        process_naws                args((DESCRIPTOR_DATA *d, int cols, int rows));
/*
	act_info.c
*/

bool			check_blind          args((CHAR_DATA *ch));
void			delete_player          args((CHAR_DATA *ch));
void			sort_alias          args((CHAR_DATA *ch));
void			set_title	          args((CHAR_DATA *ch, char *title));
HD		* get_help	          args((CHAR_DATA *ch, char *argument));
AD		* lookup_area          args((char *arg));
int         lookup_god          args((char *arg));
int         lookup_class          args((char *arg));
int         lookup_race          args((char *arg));
CD		* find_char_dir          args((CHAR_DATA *ch, bool dir, char *name));
void			check_most          args((CHAR_DATA *ch));
void			update_hiscores     args((CHAR_DATA *ch, int rank, int score, bool cptype));
char		* get_color_string     args((CHAR_DATA *ch, int regist, int vt_code));
char		* get_color_code          args((CHAR_DATA *ch, int regist, int vt_code));

/*
	act_move.c
*/

void 	show_who_can_see     args((CHAR_DATA *ch, char *txt));
bool			move_char	          args((CHAR_DATA *ch, int door, bool forreal));
bool 	can_move_char          args((CHAR_DATA *ch, int door));
int         find_door	          args((CHAR_DATA *ch, char *arg));
int         get_max_speed          args((CHAR_DATA *ch));

/*
	act_obj.c
*/


int         get_cost	           args((CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy));
OD        * find_char_corpse       args((CHAR_DATA *ch, bool method));
int         bargain	           args((CHAR_DATA *ch, int cost));
int         give_gold	           args((CHAR_DATA *ch, int amount));
int         reinc_eq_level         args((CHAR_DATA *ch));
int         obj_eq_level           args((CHAR_DATA *ch, OBJ_DATA *obj));
bool        wear_obj	           args((CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace, int location , bool fDisplay));
int         has_wear_location      args((OBJ_DATA *obj, int location));
/*
	bounty.c
*/

int         gold_transaction       args((CHAR_DATA *ch, int amount));
void        post_bounty            args((char *name, int amount));
void        sort_bounty            args((BOUNTY_DATA * bptr));
void        remove_bounty          args((BOUNTY_DATA * bptr));
BD        * get_bounty             args((char *name));
void        collect_bounty         args((CHAR_DATA *ch, CHAR_DATA *victim));


/*
	act_wiz.c
*/

int         unique_players         args((void));
int         find_location          args((CHAR_DATA *ch, char *arg));
char      * broken_bits            args((long long number, char *vector, bool linear));
void        auto_release           args((CHAR_DATA *ch));
void        add_crime              args((CHAR_DATA *ch, PVNUM_DATA *pvnum));
void        set_quest_bits         args((unsigned char **,unsigned int, unsigned int, unsigned int));
int         get_quest_bits         args((unsigned char *, unsigned int, unsigned int));
char      * quest_bits_to_string   args((unsigned char *));
void        mpdebug                args((CHAR_DATA *ch, MP_TOKEN *token));

/*
	castle.c
*/

bool        get_bitvector_value    args((char *name,int *number,char *allowed));
void        list_bitvectors        args((CHAR_DATA *ch,char *prefix));
void        del_castle             args((CHAR_DATA *victim));

/*
	comm.c
*/

void        close_socket           args((DESCRIPTOR_DATA *dclose));
void        write_to_buffer        args((DESCRIPTOR_DATA *d, char *txt, int length));
void        send_to_room           args((char *txt, ROOM_INDEX_DATA *room));
void        send_to_char           args((const char *txt, CHAR_DATA *ch));
void        send_to_char_color     args((char *txt, CHAR_DATA *ch));
void        act                    args((const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type));
void        log_printf             args((char *fmt, ...));
void        log_god_printf         args((char *fmt, ...));
void        log_build_printf       args((int vnum, char *fmt, ...));
int         cat_sprintf            args((char *dest, const char *fmt, ...));
int         cat_snprintf           args((char *dest, size_t leng, char *fmt, ...));
void        ch_printf              args((CHAR_DATA *ch, const char *fmt, ...));
void        ch_printf_color        args((CHAR_DATA *ch, const char *fmt, ...));
void        check_bad_desc         args((CHAR_DATA *ch));
void        remove_bad_desc_name   args((char *name));
void        add_player             args((CHAR_DATA *ch));
void        sub_player             args((CHAR_DATA *ch));

/*
	db.c
*/

int         find_command           args((char *cmd_str, int trust));
int         find_social            args((char *social_str));
void        boot_db                args((bool fCopyOver));
void        area_update            args((void));
CD        * create_mobile          args((MOB_INDEX_DATA *pMobIndex));
OD        * create_object          args((OBJ_INDEX_DATA *pObjIndex, bool hashed));
void        clear_char             args((CHAR_DATA *ch));
void        copyover_recover       args((void));
void        free_char              args((CHAR_DATA *ch));
char      * get_extra_descr        args((const char *name, EXTRA_DESCR_DATA *ed));
void        fread_area             args((FILE *fp, int segment));
char        fread_letter           args((FILE *fp));
long long   fread_number           args((FILE *fp));
char      * fread_string           args((FILE *fp));
void        fread_to_eol           args((FILE *fp));
char      * fread_line             args((FILE *fp));
char      * fread_word             args((FILE *fp));
int         str_cat_max            args((const char *, const char *, int));
int         str_apd_max            args((const char *, const char *, int, int));
int         str_cpy_max            args((const char *, const char *, int));
void        number_seed            args((int number));
int         number_fuzzy           args((int number));
int         number_dizzy           args((int number));
int         number_range           args((int from, int to));
int         number_percent         args((void));
int         number_door            args((void));
int         number_bits            args((int width));
int         dice                   args((int number, int size));
void        smash_tilde            args((char *str));
bool        str_cmp                args((const char *astr, const char *bstr));
bool        str_prefix             args((const char *astr, const char *bstr));
bool        str_infix              args((const char *astr, const char *bstr));
bool        str_suffix             args((const char *astr, const char *bstr));
char      * strlower               args((const char *str));
char      * strupper               args((const char *str));
char      * str_resize             args((const char *st,char *buf,int length));
char      * get_prefix             args((const char *argument));
char      * get_time_string        args((time_t time));
char      * numbersuf              args((int number));
char      * tocivilian             args((int hour));
char      * tomoney                args((int money));
char      * get_domain             args((const char *str));
char      * short_to_name          args((const char *str, int amount));
char      * capitalize             args((const char *str));
char      * capitalize_all         args((const char *str));
char      * lower_all              args((const char *str));
char      * capitalize_name        args((const char *str));
void        append_file            args((CHAR_DATA *, char *, FILE *, char *));
void        bug                    args((const char *str, ...));
void        log_string             args((char *str));
void        log_god_string         args((char *str));
void        log_build_string       args((int vnum, char *str));
bool        is_valid_save          args((char *file_name, char *text_crc));
void        create_shop            args((MOB_INDEX_DATA *mob));
void        delete_shop            args((MOB_INDEX_DATA *mob));
void        create_exit            args((ROOM_INDEX_DATA *room, int door));
void        delete_exit            args((ROOM_INDEX_DATA *room, int door));
RID       * create_room            args((int vnum));
bool        delete_room            args((ROOM_INDEX_DATA *room));
bool        delete_obj             args((OBJ_INDEX_DATA *obj));
bool        delete_mob             args((MOB_INDEX_DATA *mob));
bool        delete_help            args((HELP_DATA *help));
RID       * make_room              args((int vnum));
OID       * make_object            args((int vnum));
MID       * make_mobile            args((int vnum));
HD        * make_helpfile          args((char *argument, AREA_DATA *area));
ED        * make_exit              args((ROOM_INDEX_DATA *pRoomIndex, ROOM_INDEX_DATA *to_room, bool door));
void        start_timer            args((int timer));
void        close_timer            args((int timer));
long long   display_timer          args((CHAR_DATA *ch, int timer));
void        add_char               args((CHAR_DATA *ch));
AD        * get_area_from_vnum     args((int vnum));


/*
	data.c
*/

void        save_notes             args((void));
void        load_notes             args((void));
void        load_bounties          args((void));
void        save_bounties          args((void));
void        load_sites             args((void));
void        save_sites             args((void));
void        load_nsites            args((void));
void        save_nsites            args((void));
void        load_victors           args((void));
void        save_victors           args((void));
void        load_players           args((void));
void        save_players           args((void));
void        load_timeinfo          args((void));
void        save_timeinfo          args((void));
void        load_usage             args((void));
void        save_usage             args((void));
void        load_hiscores          args((void));
void        save_hiscores          args((void));
void        del_data               args((CHAR_DATA *ch));


/*
	edit.c
*/

void        start_editing        args((CHAR_DATA *ch, char *data));
void        stop_editing            args((CHAR_DATA *ch));
void        edit_buffer             args((CHAR_DATA *ch, char *argument));
char      * copy_buffer            args((CHAR_DATA *ch));


/*
	reset.c
*/

RD        * make_reset     args((char letter, int arg1, int arg2, int arg3));
void        reset_area      args((AREA_DATA  *pArea));
void        delete_reset     args((AREA_DATA  *pArea, RESET_DATA * pReset));

bool        is_room_reset     args((RESET_DATA *reset, ROOM_INDEX_DATA *room));
bool        is_door_reset     args((RESET_DATA *reset, int door));
bool        is_obj_reset     args((RESET_DATA *reset, OBJ_INDEX_DATA  *obj));
bool        is_mob_reset     args((RESET_DATA *reset, MOB_INDEX_DATA  *mob));

/*
	str_hash.c
*/
char *str_alloc( char *str );
char *str_dupe( char *str );
void str_free( char *str );

/*
	fight.c
*/

void  violence_update          args((void));
bool  can_mass_attack          args((CHAR_DATA *ch, CHAR_DATA *victim));
bool  can_reincarnate_attack     args((CHAR_DATA *ch, CHAR_DATA *victim));
void  multi_hit                  args((CHAR_DATA *ch, CHAR_DATA *victim, int dt));
void        damage_hurt           args((CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt));
void        damage                  args((CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt));
bool        check_hit                  args((CHAR_DATA *ch, CHAR_DATA *victim, int hit, int dt));
int         get_body_part           args((CHAR_DATA *ch, CHAR_DATA *victim, int dt, int type));
int         damage_modify           args((CHAR_DATA *ch, CHAR_DATA *victim, int dt, int dam));
void        check_killer             args((CHAR_DATA *ch, CHAR_DATA *victim));
bool        can_attack             args((CHAR_DATA *ch, CHAR_DATA *victim));
void        found_hating            args((CHAR_DATA *ch, CHAR_DATA *victim));
bool        valid_fight           args((CHAR_DATA *ch, CHAR_DATA *victim));
void        get_attack_string          args((CHAR_DATA *ch, CHAR_DATA *viewer, char *buf));
int         get_damnodice           args((CHAR_DATA *ch));
int         get_damsizedice          args((CHAR_DATA *ch));
void        update_pos           args((CHAR_DATA *ch));
void        death_cry                  args((CHAR_DATA *ch));
void        stop_hate_fear           args((CHAR_DATA *ch));
void        free_fight             args((CHAR_DATA *ch));
CD        * who_fighting            args((CHAR_DATA *ch));
void        stop_fighting           args((CHAR_DATA *ch, bool fBoth));
bool        check_recently_fought     args((CHAR_DATA *ch, CHAR_DATA *victim));

/*
	handler.c
*/

char      * get_name                    args((CHAR_DATA *ch));
char      * get_god_name                args((int god));
int         get_trust                   args((CHAR_DATA *ch));
int         get_pager_breakpt           args((CHAR_DATA *ch));
int         get_page_width              args((CHAR_DATA *ch));
int         get_age                     args((CHAR_DATA *ch));
int         get_max_str                 args((CHAR_DATA *ch));
int         get_max_int                 args((CHAR_DATA *ch));
int         get_max_wis                 args((CHAR_DATA *ch));
int         get_max_dex                 args((CHAR_DATA *ch));
int         get_max_con                 args((CHAR_DATA *ch));
int         get_curr_str                args((CHAR_DATA *ch));
int         get_curr_int                args((CHAR_DATA *ch));
int         get_curr_wis                args((CHAR_DATA *ch));
int         get_curr_dex                args((CHAR_DATA *ch));
int         get_curr_con                args((CHAR_DATA *ch));
int         can_carry_n                 args((CHAR_DATA *ch));
int         can_carry_w                 args((CHAR_DATA *ch));
int         get_pets                    args((CHAR_DATA *ch));
bool        in_camp                     args((CHAR_DATA *ch));
bool        is_name                     args((const char *str, char *namelist));
bool        is_name_short               args((const char *str, char *namelist));
bool        is_name_list                args((char *str, char *namelist));
bool        is_multi_name_list          args((char *str, char *namelist));
bool        is_multi_name_list_short    args((char *str, char *namelist));
bool        is_multi_name_short         args((char *str, char *namelist));
void        affect_modify               args((CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd));
void        affect_to_char              args((CHAR_DATA *ch, AFFECT_DATA *paf));
void        affect_from_char            args((CHAR_DATA *ch, AFFECT_DATA *paf));
void        affect_to_obj               args((OBJ_DATA *obj, AFFECT_DATA *paf));
void        affect_from_obj             args((OBJ_DATA *obj, AFFECT_DATA *paf));
void        affect_strip                args((CHAR_DATA *ch, int sn));
bool        is_affected                 args((CHAR_DATA *ch, int sn));
void        affect_join                 args((CHAR_DATA *ch, AFFECT_DATA *paf));
void        affect2_join                args((CHAR_DATA *ch, AFFECT_DATA *paf));
void        char_from_room              args((CHAR_DATA *ch));
void        char_to_room                args((CHAR_DATA *ch, int vnum));
void        obj_to_char                 args((OBJ_DATA *obj, CHAR_DATA *ch));
void        obj_from_char               args((OBJ_DATA *obj));
int         apply_ac                    args((OBJ_DATA *obj, int iWear));
OD        * get_eq_char                 args((CHAR_DATA *ch, int iWear));
void        check_zap                   args((CHAR_DATA *ch, bool fDisplay));
int         objref                      args((char *argument, OBJ_DATA *obj));
void        equip_char                  args((CHAR_DATA *ch, OBJ_DATA *obj, int iWear));
void        unequip_char                args((CHAR_DATA *ch, OBJ_DATA *obj));
int         count_users                 args((OBJ_DATA * obj));
int         count_mob_list              args((MOB_INDEX_DATA *obj, OBJ_DATA *list));
int         char_exists                 args((char *arg));
void        obj_from_room               args((OBJ_DATA *obj));
void        obj_to_room                 args((OBJ_DATA *obj, int vnum));
void        obj_to_obj                  args((OBJ_DATA *obj, OBJ_DATA *obj_to));
void        obj_from_obj                args((OBJ_DATA *obj));
void        junk_obj                    args((OBJ_DATA *obj));
void        extract_obj                 args((OBJ_DATA *obj));
void        junk_mob                    args((CHAR_DATA *ch));
void        extract_char                args((CHAR_DATA *ch));
void        extract_exit                args((ROOM_INDEX_DATA *room, EXIT_DATA * pexit, int door));
CD        * get_char_room               args((CHAR_DATA *ch, char *argument));
CD        * get_char_area               args((CHAR_DATA *ch, char *argument));
CD        * get_char_world              args((CHAR_DATA *ch, char *argument));
CD        * get_player_world            args((CHAR_DATA *ch, char *argument));
CD        * get_char_pvnum              args((int pvnum));
char      * get_name_pvnum              args((int pvnum));
int         get_pvnum_name              args((char *name));
OD        * get_obj_list                args((CHAR_DATA *ch, char *argument, OBJ_DATA *list));
OD        * get_obj_carry_even_invis    args((CHAR_DATA *ch, char *argument));
OD        * get_obj_carry               args((CHAR_DATA *ch, char *argument));
OD        * get_obj_carry_keeper        args((CHAR_DATA *keeper, char *argument, CHAR_DATA *ch));
OD        * get_obj_wear                args((CHAR_DATA *ch, char *argument));
OD        * get_obj_wear_vnum           args((CHAR_DATA *ch, int vnum));
OD        * get_obj_here                args((CHAR_DATA *ch, char *argument));
OD        * get_obj_world               args((CHAR_DATA *ch, char *argument));
CD        * get_mob_vnum                args((CHAR_DATA *ch, char *argument));
OD        * get_obj_vnum                args((CHAR_DATA *ch, char *argument));
RID       * get_random_room_index       args((CHAR_DATA *ch, int lo_room, int hi_room));
int         multi                       args((CHAR_DATA *ch, int sn));
int         multi_pick                  args((CHAR_DATA *ch, int sn));
int         multi_skill_level           args((CHAR_DATA *ch, int sn));
int         learned                     args((CHAR_DATA *ch, int sn));
void        check_improve               args((CHAR_DATA *ch, int sn));
int         weapon_skill                args((CHAR_DATA *ch, int type));

/*
	mob_handler.c
*/

OD        * get_obj_here_even_blinded      args((CHAR_DATA *ch, char *argument));
OD        * get_obj_wear_even_blinded      args((CHAR_DATA *ch, char *argument));
OD        * get_obj_carry_even_blinded     args((CHAR_DATA *ch, char *argument));
OD        * get_obj_list_even_blinded      args((CHAR_DATA *ch, char *argument, OBJ_DATA *list));
OD        * get_obj_area_even_blinded      args((CHAR_DATA *ch, char *argument));
OD        * get_obj_world_even_blinded     args((CHAR_DATA *ch, char *argument));
CD        * get_char_room_even_blinded     args((CHAR_DATA *ch, char *argument));
CD        * get_char_area_even_blinded     args((CHAR_DATA *ch, char *argument));
CD        * get_char_world_even_blinded    args((CHAR_DATA *ch, char *argument));
CD        * get_player_world_even_blinded  args((CHAR_DATA *ch, char *argument));
int         find_mp_location            args((CHAR_DATA *ch, char *arg));
int         is_mp_affected                 args((CHAR_DATA *ch, char *arg));
bool        is_room_good_for_teleport      args((CHAR_DATA *victim, int rvnum));
void        set_exit                       args((int room, int door, int dest));
long long   translate_bits                 args((char *string));

/*
	handler.c
*/

OD        * create_money                   args((int amount));
int         get_obj_weight                 args((OBJ_DATA *obj));
bool        room_is_dark                   args((ROOM_INDEX_DATA *pRoomIndex));
bool        room_is_private                args((ROOM_INDEX_DATA *pRoomIndex));
bool        can_see                        args((CHAR_DATA *ch, CHAR_DATA *victim));
bool        can_see_world                  args((CHAR_DATA *ch, CHAR_DATA *victim));
bool        can_see_infra                  args((CHAR_DATA *ch, CHAR_DATA *victim));
bool        can_see_smoke                  args((CHAR_DATA *ch));
bool        can_see_hidden                 args((CHAR_DATA *ch));
bool        can_see_in_room                args((CHAR_DATA *ch, ROOM_INDEX_DATA *room));
int         get_vision                     args((CHAR_DATA *ch));
int         get_sight                      args((CHAR_DATA *ch));
int         get_room_light                 args((CHAR_DATA *ch, ROOM_INDEX_DATA *room, bool door));
bool        can_hear                       args((CHAR_DATA *ch, CHAR_DATA *victim));
bool        can_see_obj                    args((CHAR_DATA *ch, OBJ_DATA *obj));
bool        can_drop_obj                   args((CHAR_DATA *ch, OBJ_DATA *obj));
char      * item_type_name                 args((OBJ_DATA *obj));
char      * affect_loc_name                args((int location));
char      * affect_bit_name                args((int vector));
char      * affect2_bit_name               args((int vector));
void        char_reset                     args((CHAR_DATA *ch));
int         obj_level_estimate             args((OID *objIndex));
int         obj_level                      args((OID *objIndex, int *lvl_neg, int *lvl_pos));
int         obj_utility_level              args((CHAR_DATA *ch, OBJ_DATA *obj));
bool        blocking                       args((CHAR_DATA *victim, CHAR_DATA *ch));
bool        login_allowed                  args((DESCRIPTOR_DATA *d, CHAR_DATA *ch));
ED        * get_exit                       args((int vnum, bool door));
bool        can_use_exit                   args((CD *ch, EXIT_DATA *exit));
bool        is_valid_exit                  args((CD *ch, RID *room, bool dir));


/*
	interp.c
*/

void        interpret                      args((CHAR_DATA *ch, char *argue));
bool        is_number                      args((char *argument));
bool        is_number_argument             args((char *argument));
int         number_argument                args((char *argument, char *arg));
char      * one_argument            args((char *argument, char *arg_first));
char      * one_argument_nolower           args((char *argument, char *arg_first));


/*
	language.c
*/

void        add_language                   args((CHAR_DATA *ch));
int         total_language                 args((CHAR_DATA *ch));
void        fix_language                   args((CHAR_DATA *ch));
bool        can_understand                 args((CHAR_DATA *victim, CHAR_DATA *ch, bool fDisplay));
void        say_spell                      args((CHAR_DATA *ch, int sn));
char      * translate                      args((CHAR_DATA *ch, char *text));


/*
	magic.c
*/

int         skill_lookup                   args((char *name));
int         slot_lookup                    args((int slot));
bool        saves_spell                    args((int level, CHAR_DATA *ch, CHAR_DATA *victim));
void        obj_cast_spell                 args((int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj));
void        totem_cast_spell               args((OBJ_DATA *obj));

/*
	mob_prog.c
*/

int         mprog_name_to_type     args ((char *name));
int         oprog_name_to_type     args ((char *name));
int         rprog_name_to_type     args ((char *name));
char      * mprog_type_to_name     args ((int type));
char      * oprog_type_to_name     args ((int type));
char      * rprog_type_to_name     args ((int type));

void        mprog_wordlist_check	args ((char * arg, CHAR_DATA *mob, CHAR_DATA* actor, OBJ_DATA* object, void* vo, int type));
bool        oprog_wordlist_check	args ((char * cmd, char * arg, CHAR_DATA *mob, CHAR_DATA* actor, OBJ_DATA* object, void* vo, int type));
void        mudprog_percent_check       args ((MUD_PROG *mprg, CHAR_DATA *mob, CHAR_DATA* actor, OBJ_DATA* object, void* vo, int type, int vnum));
void        mprog_time_check 	args ((CHAR_DATA *mob, CHAR_DATA* actor, OBJ_DATA* object, void* vo, int type));
void        mprog_act_trigger           args ((char* buf, CHAR_DATA *mob, CHAR_DATA* ch, OBJ_DATA* obj, void* vo));
bool        mprog_social_trigger        args ((char *social, CHAR_DATA *mob, CHAR_DATA *ch));
void        mprog_trigger_trigger       args ((char *txt, CHAR_DATA *mob, CHAR_DATA *ch));
void        mprog_bribe_trigger         args ((CHAR_DATA *mob, CHAR_DATA *ch, int amount));
void        mprog_delay_trigger         args ((CHAR_DATA *mob, bool index));
void        mprog_entry_trigger         args ((CHAR_DATA *mob));
void        mprog_give_trigger          args ((CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA* obj));
void        mprog_greet_trigger         args ((CHAR_DATA *ch));
void        mprog_fight_trigger         args ((CHAR_DATA *mob, CHAR_DATA *ch));
void        mprog_hitprcnt_trigger      args ((CHAR_DATA *mob, CHAR_DATA *ch));
void        mprog_death_trigger         args ((CHAR_DATA *mob));
void        mprog_repop_trigger	        args ((CHAR_DATA *mob));
void        mprog_speech_trigger        args ((char *txt, CHAR_DATA *mob));
void        mprog_range_trigger         args ((CHAR_DATA *mob, CHAR_DATA *ch, bool shot_from));
void        mprog_kill_trigger          args ((CHAR_DATA *mob));
void        mprog_exit_trigger		args ((CHAR_DATA *mob, int door));
bool        mprog_desc_trigger		args ((CHAR_DATA *mob, CHAR_DATA *ch, char *txt));


bool        oprog_command_trigger       args((CHAR_DATA *ch, char *cmd, char *arg));
void        oprog_damage_trigger        args((CHAR_DATA *ch, OBJ_DATA *obj, CHAR_DATA *victim));
void        oprog_drop_trigger          args((CHAR_DATA *ch, OBJ_DATA *obj));
void        oprog_get_trigger           args((CHAR_DATA *ch, OBJ_DATA *obj));
void        oprog_hit_trigger           args((CHAR_DATA *ch, CHAR_DATA *victim));
void        oprog_rand_trigger          args((CHAR_DATA *ch));
void        oprog_remove_trigger        args((CHAR_DATA *ch, OBJ_DATA *obj));
void        oprog_sacrifice_trigger     args((CHAR_DATA *ch, OBJ_DATA *obj));
bool        oprog_unknown_trigger       args((CHAR_DATA *ch, char *cmd, char *arg));
void        oprog_wear_trigger          args((CHAR_DATA *ch, OBJ_DATA *obj));

bool        rprog_command_trigger       args((CHAR_DATA *ch, char *cmd, char *arg));
bool        rprog_unknown_trigger       args((CHAR_DATA *ch, char *cmd, char *arg));

/*
	mount.c
*/

bool        is_mounting           args((CHAR_DATA *ch));
bool        is_mounted           args((CHAR_DATA *ch));


/*
	olc.c
*/

void        stop_olc_editing     args((CHAR_DATA *ch, AREA_DATA *area));
void        assign_area           args((CHAR_DATA *ch));
bool        can_olc_modify          args((CHAR_DATA *ch, int vnum));
bool        show_build_vnum     args((CHAR_DATA *ch, int vnum));
char      * flag_string          args((long long bitvector, char * const flagarray[]));
char      * type_string          args((long long bitvector, char * const flagarray[]));
long long   get_flag           args((char *flag, char *const array[]));
char      * give_flags          args((char *const array[]));
char      * fixer           args((char *arg));
void        save_area           args((AREA_DATA *area, bool forreal));

/*
	save.c
*/

long long   encrypt                     args((char *str));
long long   encrypt64                   args((char *str));
void        add_pvnum                   args((CHAR_DATA *ch));
int         find_free_pvnum             args((void));
void        roll_race                   args((CHAR_DATA *ch));
void        save_char_obj               args((CHAR_DATA *ch, int which_type));
bool        load_char_obj               args((DESCRIPTOR_DATA *d, char *name));
void        load_clan_pit               args((FILE *fp));
void        save_clan_pit               args((CLAN_DATA *clan, FILE *fp));
void        save_world                  args((void));
int         check_dirs                  args((void));

/*
	socials.c
*/

void        interp_social               args((CHAR_DATA *ch, int cmd, char *argument));
bool        check_social                args((CHAR_DATA *ch, char *command, char *argument));


/*
	special.c
*/

SPEC_FUN *  spec_lookup         args((const char *name));
char  *  spec_name_lookup       args((SPEC_FUN *fun));


/*
	update.c
*/
int         hit_gain                 args((CHAR_DATA * ch, int quiet));
int         mana_gain                args((CHAR_DATA * ch, int quiet));
int         move_gain                args((CHAR_DATA * ch, int quiet));
int         exp_level                args((int class, int level));
void        advance_level            args((CHAR_DATA *ch, bool fSave));
void        demote_level             args((CHAR_DATA *ch, bool fSave));
void        gain_exp                 args((CHAR_DATA *ch, int gain));
void        gain_condition           args((CHAR_DATA *ch, int iCond, int value));
void        update_handler           args((void));
void        set_room_timer           args((int vnum, int type, int timer));
bool        del_room_timer           args((int vnum, int type));


/*
	clans.c
*/

bool         is_clan_leader          args((CHAR_DATA *ch));
CLAN_DATA  * get_clan                args((char *name));
CLAN_DATA  * get_clan_from_vnum      args((int vnum));
void         load_clans              args((void));
void         save_all_clans          args((void));
void         save_clan               args((CLAN_DATA *clan));
void         destroy_clan            args((CLAN_DATA *clan));
void         send_clan_message       args((CLAN_DATA *clan, char *message));
long long    max_coffer              args((CLAN_DATA *clan));
bool         is_own_clanhall         args((CHAR_DATA *ch));
bool         is_in_clanhall          args((CHAR_DATA *ch));

/*
	race.c
*/

bool           rspec_req           args((CHAR_DATA *ch, long long attr));
bool           cflag               args((CHAR_DATA *ch, long long attr));
int            get_age_bonus       args((CHAR_DATA *ch, int type));

/*
	drunkify.c
*/

char * drunkify          args((const char *text));
bool        is_drunk(CHAR_DATA *ch);

/*
	debug.c
*/

void        push_call(char *f, ...);
void        push_call_format(char *f, ...);
void        pop_call(void);

void        dump_stack(void);
void        build_dump_stack(int vnum);
void  dump_desc_characters(void);
void  print_errno(int number);
char * cut_at_eol(char *string);

void        open_reserve(void);
void        close_reserve(void);
FILE * my_fopen(char *filename, char *opentype, bool silent);
void        my_fclose(FILE *which);
void        my_fclose_all();

/*
	games.c
*/

void        update_trivia();
void        morph_update(CHAR_DATA *ch, CHAR_DATA *victim);

/*
	purger.c
*/

void        start_purger();
void        update_purger();

/*
	dynamic.c
*/

char * get_dynamic_description        (CHAR_DATA *ch);
char * get_dynamic_player_description (CHAR_DATA *ch, CHAR_DATA *victim, char *string);
char * get_char_description           (CHAR_DATA *ch);
void   display_appearance_selections  (CHAR_DATA *ch, int appearance);
int    process_appearance_selections  (CHAR_DATA *ch, int appearance, char *arg);

/*
	pathfind.c
*/

int         findpath_search_victim(CHAR_DATA *ch, CHAR_DATA *victim, int range);
CD        * findpath_search_name  (CHAR_DATA *ch, char *name, int range);

/*
	math.c
*/

long long mathexp(CHAR_DATA *ch, const char *str);
long long mathexp_tokenize(CHAR_DATA *ch, const char *str);
void      mathexp_level(CHAR_DATA *ch, MATH_DATA *node);
void      mathexp_compute(CHAR_DATA *ch, MATH_DATA *node);
long long tintoi(const char *str);
long long tincmp(const char *left, const char *right);
long long tindice(const char *left, const char *right);


/*
	mob_commands.c
*/

void      mazegen              args((CHAR_DATA *ch, int cx, int cy, int cz, int x, int y, int z, int size, int count, int seed, int last_door));
void      transference         args((CHAR_DATA *victim, int location));

/*
	economy.c
*/

int       obj_worth            args((OBJ_INDEX_DATA *objIndex));
int       mob_worth            args((MOB_INDEX_DATA *mobIndex));

#undef CD
#undef MID
#undef OD
#undef OID
#undef RID
#undef RD
#undef AD
#undef TM
#undef HD
#undef BD
#undef ED


void raw_kill(CHAR_DATA *);
void make_corpse     args((CHAR_DATA *ch));
void show_char_to_char_1(CHAR_DATA *, CHAR_DATA *);
int which_god(CHAR_DATA *);
bool new_notes(CHAR_DATA *);

bool remove_obj     args((CHAR_DATA *ch, int iWear, bool fReplace, bool fDisplay));

void force_help(DESCRIPTOR_DATA *, char *);
bool write_to_descriptor args((DESCRIPTOR_DATA *d, char *txt, int length));

void write_to_port args((DESCRIPTOR_DATA *d));
bool is_desc_valid(CHAR_DATA *);


void set_fighting     args((CHAR_DATA *ch, CHAR_DATA *victim));
void reset_color(CHAR_DATA *);

int mob_armor (CHAR_DATA *);

CHAR_DATA   *find_keeper(CHAR_DATA *);

int direction_door(char *);  /* Returns door# from direction text */
CHAR_DATA *lookup_char(char *);


bool is_spell(int);
int get_mana(CHAR_DATA *, int);

long long get_game_usec(void);

int compare_obj(CHAR_DATA *, OBJ_DATA *, OBJ_DATA *); /* difference of strength if same */

int has_wear_location(OBJ_DATA *, int);
int count_total_objects(OBJ_DATA *);/* determines total objects in cont */



void start_object_program (CHAR_DATA *, OBJ_DATA *, OBJ_PROG *, char *);
void object_program (CHAR_DATA *, OBJ_DATA *, OBJ_PROG *, char *);



void knight_adjust_hpmnmv(CHAR_DATA *);

void check_most(CHAR_DATA *);

int character_expiration(CHAR_DATA *);

#define VICTORY_LIST_SIZE 20

char *victory_list[VICTORY_LIST_SIZE];


int GET_DAMROLL(CHAR_DATA *);
int GET_HITROLL(CHAR_DATA *);
int GET_SAVING_THROW(CHAR_DATA *);

/*
	Presto 8/3/98
*/

#define NORMAL_SAVE 0
#define BACKUP_SAVE 1

/*
	Hypnos 20020326: Directory mask for player dirs
*/

#define XX_DIRMASK 0770


#define ALLOCMEM(result, type, number) \
{ \
	result = calloc(1, sizeof(type) * (number)); \
}

#define FREEMEM(point) \
{ \
	if (point == NULL) \
	{ \
		log_printf("FREEMEM: Freeing NULL pointer in file %s on line %d", __FILE__, __LINE__); \
	} \
	free(point); \
	point = NULL; \
}


#define CHECK_AUTOSAVE(area) \
{ \
	if (IS_SET((area)->flags, AFLAG_AUTOSAVE)) \
	{ \
		SET_BIT((area)->flags, AFLAG_MODIFIED); \
	} \
}


#define NKEY(literal, field, value) \
{ \
	if (!strcmp(word, literal)) \
	{ \
		field			= value; \
		fMatch			= TRUE; \
		break; \
	} \
}

#define SKEY(literal, field, value) \
{ \
	if (!strcmp(word, literal)) \
	{ \
		STRFREE(field); \
		field  = value; \
		fMatch = TRUE; \
		break; \
	} \
}

#define AKEY(literal, field, value) \
{ \
	if (!strcmp(word, literal)) \
	{ \
		cnt = value; \
		field[cnt] = value; \
		fMatch = TRUE; \
		break; \
	} \
}

#define RESTRING(point, value) \
{ \
	STRFREE(point); \
	point = str_alloc((value)); \
}

#define STRALLOC(point)			str_alloc((point))

#define STRDUPE(point)			str_dupe((point))


/*
#define STRFREE(point) \
{ \
	if (point == NULL) \
	{ \
		log_printf("STRFREE: Freeing NULL pointer in file %s on line %d", __FILE__, __LINE__); \
	} \
	str_free((point)); \
	point = NULL; \
}
*/

#define STRFREE(point) \
{ \
	str_free((point)); \
	point = NULL; \
}

/*
	New link added to the last link
*/

#define LINK(link, first, last, next, prev) \
{ \
	if (!(first)) \
	{ \
		(first)				= (link); \
	} \
	else \
	{ \
		(last)->next			= (link); \
	} \
	(link)->next				= NULL; \
	(link)->prev				= (last); \
	(last)					= (link); \
}


#define INSERT_LEFT(link, rght, first, next, prev) \
{ \
	(link)->prev				= (rght)->prev; \
	(rght)->prev				= (link); \
	(link)->next				= (rght); \
	\
	if ((link)->prev) \
	{ \
		(link)->prev->next		= (link); \
	} \
	else \
	{ \
		(first)				= (link); \
	} \
}


#define INSERT_RIGHT(link, left, last, next, prev) \
{ \
	(link)->next				= (left)->next; \
	(left)->next				= (link); \
	(link)->prev				= (left); \
	\
	if ((link)->next) \
	{ \
		(link)->next->prev		= (link); \
	} \
	else \
	{ \
		(last)				= (link); \
	} \
}


#define UNLINK(link, first, last, next, prev) \
{ \
	if (!(link)->prev) \
	{ \
		(first)				= (link)->next; \
	} \
	else \
	{ \
		(link)->prev->next		= (link)->next; \
	} \
	if (!(link)->next) \
	{ \
		(last)				= (link)->prev; \
	} \
	else \
	{ \
		(link)->next->prev		= (link)->prev; \
	} \
	(link)->next = NULL; \
	(link)->prev = NULL; \
}

#endif
