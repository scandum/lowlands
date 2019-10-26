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
#include <stdarg.h>
#include "mud.h"

/*
	Locals.
*/

AREA_DATA *area_load;
int tot_memory_warning = 0;

void create_menu_tree();

/*
	Cpu management
*/

long long timers[TIMER_MAXTIMER][5];

const char timer_strings[TIMER_MAXTIMER][40] =
{
	"Area Save Time:",
	"Update Shops:",
	"Update Time:",
	"Update Areas:",
	"Update Weather:",
	"Update Objects:",
	"Char Save Time:",
	"Update Violence:",
	"Update Obj Progs:",
	"Update Mob Progs:",
	"Update Mobiles:",
	"Update MSDP:",
	"Update Aggressive:",
	"Update Purger:",
	"Update Tactical:",
	"Scan Descriptor:",
	"Process Input:",
	"Process Output:",
};

/*
	Memory management
*/

#define	MAX_PERM_BLOCK	1000000
#define	MAX_MEM_LIST	48

const int  rgSizeList   [MAX_MEM_LIST]  =
{
	8,		16,		24,		32,
	40,		48,		56,		64,
	80,		96,		112,		128,
	144,		160,		176,		192,
	224,		256,		288,		320,
	352,		384,		416,		448,
	512,		576,		640,		704,
	768,		832,		896,		960,
	1088,	1216,	1344,	1472,
	1728,	1984,	2240,	2496,
	3072,	4096,	6144,	8192,
	16384,	32768,	65536,	72016
};

void *  alloc_perm      ( bool iList );

typedef struct  perm_block_list    PERM_BLOCK_LIST;

struct perm_block_list
{
	int               iMemPerm;
};

PERM_BLOCK_LIST *perm_block_index[ 255 ];  /* Maximum perm blocks of 256 */

typedef struct  free_mem_list    FREE_MEM_LIST;

struct free_mem_list
{
	FREE_MEM_LIST * next;
};

FREE_MEM_LIST *rgFreeList       [MAX_MEM_LIST];

int rgFreeCount[MAX_MEM_LIST];  /* count of freed memory */
int rgUsedCount[MAX_MEM_LIST];  /* count of used memory */

/*
	Semi-locals.
*/

FILE *                  fpArea;
char                    strArea[MAX_INPUT_LENGTH];


/*
	Local booting procedures.
*/

void         load_area                  args( ( FILE *fp, int segment ) );
void         load_helps                 args( ( FILE *fp ) );
void         load_mobiles               args( ( FILE *fp ) );
void         load_objects               args( ( FILE *fp ) );
void         load_resets                args( ( FILE *fp ) );
void         load_rooms                 args( ( FILE *fp ) );
void         load_shops                 args( ( FILE *fp ) );
void         load_specials              args( ( FILE *fp ) );
void         load_ranges                args( ( FILE *fp ) );
void         load_version               args( ( FILE *fp ) );
void         load_olc_ranges            args( ( FILE *fp ) );
void         load_resetmsg              args( ( FILE *fp ) );
void         load_authors               args( ( FILE *fp ) );
void         load_flags			args( ( FILE *fp ) );
void         load_temperature		args( ( FILE *fp ) );
void         load_weather               args( ( FILE *fp ) );
OBJ_PROG   * load_object_program        args( ( FILE *fp ) );
void         obj_prog_if_dest           args( ( OBJ_INDEX_DATA * ) );
void         expand_mud_prog            args( ( int vnum, MUD_PROG * ) );
char         expand_line_mprog          args( ( int vnum, MUD_PROG *, char *, bool, char ) );
RESET_DATA * get_reset_from_obj         args( ( RESET_DATA *lReset ) );
RESET_DATA * get_reset_from_mob         args( ( RESET_DATA *lReset ) );

/*
	MUDprogram locals
*/
        
void           mprog_read_programs     args ( ( FILE *fp, MOB_INDEX_DATA *pMobIndex ) );
void           oprog_read_programs     args ( ( FILE *fp, OBJ_INDEX_DATA *pObjIndex ) );
void           rprog_read_programs     args ( ( FILE *fp, ROOM_INDEX_DATA *pRoomIndex ) );

/*
	Big mama top level function.
*/

void boot_db( bool fCopyOver )
{
	long long boot_start, boot_end;

 	push_call("boot_db(%p)",fCopyOver);

	boot_start = get_game_usec();

	SET_BIT(mud->flags, MUD_EMUD_BOOTING);
	SET_BIT(mud->flags, MUD_EMUD_BOOTDB);

	srand48(mud->current_time);

	load_sites();
	load_nsites();
	load_usage();
	load_players();
	load_timeinfo();
	load_bounties();
	load_victors();
	load_hiscores();
	load_notes();


	/*
		Assign table indexes - Scandum
	*/

	{
		int index, cnt;

		for (index = 0 ; index < 26 ; index++)
		{
			for (cnt = 0 ; cnt < MAX_BITVECTOR ; cnt++)
			{
				if (bitvector_table[cnt].name[0] == 'A' + index)
				{
					/*
					log_printf("bitvector_ref[%c] = %d", 'A' + index, cnt);
					*/
					mud->bitvector_ref[index] = cnt;
					break;
				}
			}
		}

		for (index = 0 ; index < 26 ; index++)
		{
			for (cnt = 0 ; *cmd_table[cnt].name != '\0' ; cnt++)
			{
				if (*cmd_table[cnt].name == 'a' + index)
				{
					/*
					log_printf("command_ref[%c] = %d", 'a' + index, cnt);
					*/
					mud->command_ref[index] = cnt;
					break;
				}
			}
		}

		for (index = 0 ; index < 26 ; index++)
		{
			for (cnt = 0 ; *social_table[cnt].name != '\0' ; cnt++)
			{
				if (*social_table[cnt].name == 'a' + index)
				{
					/*
					log_printf("social_ref[%c] = %d", 'a' + index, cnt);
					*/
					mud->social_ref[index] = cnt;
					break;
				}
			}
		}

		for (cnt = 0 ; cnt < MAX_SKILL ; cnt++)
		{
			if (skill_table[cnt].pgsn != NULL)
			{
				/*
				log_printf("skill_table[%03d].name = %s", cnt, skill_table[cnt].name);
				*/
				*skill_table[cnt].pgsn = cnt;
			}
		}
	}

	/*
		give gsn's to cmds that need them (fatigy cmds like "kick")
	*/

	{
		int sn, cmd;

		for (cmd = 0 ; *cmd_table[cmd].name ; cmd++)
		{
			cmd_gsn[cmd] = -1;
		}

		for (sn = 0 ; sn < MAX_SKILL ; sn++)
		{
			if (IS_SET(skill_table[sn].flags, FSKILL_CMD))
			{
				cmd = find_command(skill_table[sn].name, MAX_LEVEL);

				if (cmd != -1)
				{
					cmd_gsn[cmd] = sn;
				}
			}
		}
	}

	/*
		Read in all the area files.
	*/
	{
		FILE *fpList;

		log_printf("Loading Areas");

		if ((fpList = my_fopen(AREA_LIST, "r", FALSE)) == NULL)
		{
			perror( AREA_LIST );
			abort();
		}

		while (TRUE)
		{
			strcpy(strArea, fread_word(fpList));
			log_string(strArea);

			if (strArea[0] == '$')
			{
				break;
			}
			if ((fpArea = my_fopen(strArea, "r", FALSE)) == NULL)
			{
				continue;
			}

			fread_area(fpArea, 0);

			my_fclose(fpArea);

			fpArea = NULL;
		}

		if (fpList)
		{
			my_fclose( fpList );
		}
	}


	log_string("Fixing Exits (skipped)");
/*
	fix_exits();
*/

	if (mud->time_info->hour < 5)
	{
		mud->sunlight = SUN_DARK;
	}
	else if (mud->time_info->hour <  6)
	{
		mud->sunlight = SUN_RISE;
	}
	else if ( mud->time_info->hour < 19 )
	{
		mud->sunlight = SUN_LIGHT;
	}
	else if ( mud->time_info->hour < 20 )
	{
		mud->sunlight = SUN_SET;
	}
	else
	{
		mud->sunlight = SUN_DARK;
	}

	SET_BIT(mud->flags, MUD_EMUD_BOOTDB);

	log_string("Updating Areas");
	area_update();

	REMOVE_BIT(mud->flags, MUD_EMUD_BOOTDB);

	log_string("Creating Help Menu Tree");
	create_menu_tree();

	load_clans();

	REMOVE_BIT(mud->flags, MUD_EMUD_BOOTING);

	{
		/*
			Clear out timers
		*/

		int cnt;

		for (cnt = 0 ; cnt < TIMER_MAXTIMER ; cnt++)
		{
			timers[cnt][0] = 0;
			timers[cnt][1] = 0;
			timers[cnt][2] = 0;
			timers[cnt][3] = 0;
			timers[cnt][4] = 0;
		}
	}
	if (fCopyOver)
	{
		copyover_recover();
	}

	boot_end = get_game_usec();

	log_printf("Database booted in %.2f seconds.", (boot_end - boot_start) / (float) 1000000);
	pop_call();
	return;
}

void fread_area(FILE *fp, int segment)
{
	char *word;

	push_call("fread_area(%p,%p)",fp, segment);

	while (TRUE)
	{
		if (fread_letter(fp) != '#')
		{
			bug( "Boot_db: # not found.", 0 );
			abort( );
		}

		word = fread_word(fp);

		switch (word[0])
		{
			case '$':
				pop_call();
				return;
			case 'A':
				if (!strcmp(word, "AREA"))
				{
					load_area(fp, segment);
					break;
				}
				if (!strcmp(word, "AUTHORS"))
				{
					load_authors(fp);
					break;
				}
			case 'F':
				if (!strcmp(word, "FLAGS"))
				{
					load_flags(fp);
					break;
				}
			case 'H':
				if (!strcmp(word, "HELPS"))
				{
					load_helps(fp);
					break;
				}
			case 'M':
				if (!strcmp(word, "MOBILES"))
				{
					load_mobiles(fp);
					break;
				}
			case 'O':
				if (!strcmp(word, "OLC_RANGES"))
				{
					load_olc_ranges(fp);
					break;
				}
				if (!strcmp(word, "OBJECTS"))
				{
					load_objects (fp);
					break;
				}
			case 'R':
				if (!strcmp(word, "RANGES"))
				{
					load_ranges(fp);
					break;
				}
				if (!strcmp(word, "RESETMSG"))
				{
					load_resetmsg(fp);
					break;
				}
				if (!strcmp(word, "RESETS"))
				{
					load_resets(fp);
					break;
				}
				if (!strcmp(word, "ROOMS"))
				{
					load_rooms(fp);
					break;
				}
			case 'S':
				if (!strcmp(word, "SHOPS"))
				{
					load_shops(fp);
					break;
				}
				if (!strcmp(word, "SPECIALS"))
				{
					load_specials(fp);
					break;
				}
			case 'T':
				if (!strcmp(word, "TEMPERATURE"))
				{
					load_temperature(fp);
					break;
				}
			case 'V':
				if (!strcmp(word, "VERSION"))
				{
					load_version(fp);
					break;
				}
			case 'W':
				if (!strcmp(word, "WEATHER"))
				{
					load_weather(fp);
					break;
				}
			default:
				bug("boot_db: bad section name: %s", word);
				abort( );
		}
	}


}

/*
	Snarf an 'area' header line.
*/

void load_area ( FILE *fp, int segment )
{
	AREA_DATA 	*pArea, *temp_area;

	push_call("load_area(%p,%p)",fp,segment);

	ALLOCMEM(pArea, AREA_DATA, 1);
	pArea->name					= fread_string( fp );
	pArea->age					= 99;
	pArea->low_r_vnum				= MAX_VNUM-1;
	pArea->low_o_vnum				= MAX_VNUM-1;
	pArea->low_m_vnum				= MAX_VNUM-1;
	pArea->olc_range_lo				= MAX_VNUM-1;
	pArea->resetmsg				= STRDUPE(str_empty);
	pArea->hi_soft_range			= MAX_LEVEL;
	pArea->hi_hard_range			= MAX_LEVEL;
	pArea->authors					= STRDUPE(str_empty);

	ALLOCMEM(pArea->weather_info, WEATHER_DATA, 1);

	pArea->weather_info->temp_winter	= -10;
	pArea->weather_info->temp_summer	=  20;
	pArea->weather_info->temp_daily	=  10;
	pArea->weather_info->wind_scale	=   4;
	pArea->weather_info->wet_scale	=   4;

	{
		for (temp_area = mud->f_area ; temp_area ; temp_area = temp_area->next)
		{
			if (strcmp(pArea->name, temp_area->name) < 0)
			{
				INSERT_LEFT(pArea, temp_area, mud->f_area, next, prev);
				break;
			}
		}
		if (!temp_area)
		{
			LINK(pArea, mud->f_area, mud->l_area, next, prev);
		}
		mud->top_area++;
	}
	pArea->filename	= STRALLOC(strArea);
	area_load = pArea;
	pop_call();
	return;
}


void load_temperature( FILE * fp )
{
	push_call("load_temperature(%p)",fp);

	fread_number(fp);
	fread_number(fp);
	fread_number(fp);
	fread_number(fp);

	pop_call();
	return;
}


void load_weather( FILE * fp )
{
	push_call("load_weather(%p)",fp);

	area_load->weather_info->temp_winter	= fread_number(fp);
	area_load->weather_info->temp_summer	= fread_number(fp);
	area_load->weather_info->temp_daily	= fread_number(fp);
	area_load->weather_info->wet_scale		= fread_number(fp);
	area_load->weather_info->wind_scale	= fread_number(fp);

	pop_call();
	return;
}


void load_flags( FILE *fp )
{
	area_load->flags = fread_number(fp);
}


void load_authors( FILE *fp )
{
	push_call("load_authors(%p)",fp);

	STRFREE(area_load->authors);
	area_load->authors = fread_string(fp);
	pop_call();
	return;
}


void load_ranges( FILE *fp )
{
	push_call("load_ranges(%p)",fp);
	int l_soft, h_soft, l_hard, h_hard;

	l_soft = fread_number(fp);
	h_soft = fread_number(fp);
	l_hard = fread_number(fp);
	h_hard = fread_number(fp);

	area_load->low_soft_range = URANGE(1, l_soft, 95);
	area_load->hi_soft_range  = URANGE(1, h_soft, 95);
	area_load->low_hard_range = URANGE(1, l_hard, 95);
	area_load->hi_hard_range  = URANGE(1, h_hard, 95);

	if (area_load->low_soft_range > area_load->hi_soft_range)
	{
		log_printf("Low soft range is greater than high soft range.");
	}
	if (area_load->low_hard_range > area_load->hi_hard_range)
	{
		log_printf("Low hard range is greater than high hard range.");
	}
	pop_call();
	return;
}

void load_version( FILE *fp )
{
	push_call("load_version(%p)",fp);

	area_load->version = fread_number(fp);

	pop_call();
	return;
}

void load_olc_ranges( FILE *fp )
{
	push_call("load_olc_ranges(%p)",fp);

	area_load->olc_range_lo = fread_number( fp );
	area_load->olc_range_hi = fread_number( fp );

	pop_call();
	return;
}


void load_resetmsg( FILE *fp )
{
	push_call("load_resetmsg(%p)",fp);

	STRFREE( area_load->resetmsg );
	area_load->resetmsg = fread_string( fp );

	pop_call();
	return;
}


/*
	Snarf a help section.
*/

void load_helps( FILE *fp )
{
	HELP_DATA *pHelp;

	push_call("load_helps(%p)",fp);

	while (TRUE)
	{
		ALLOCMEM(pHelp, HELP_DATA, 1);
		pHelp->level	= fread_number( fp );
		pHelp->keyword	= fread_string( fp );
		if (pHelp->keyword[0] == '$')
		{
			FREEMEM(pHelp);
			break;
		}
		pHelp->text	= fread_string( fp );
		pHelp->area	= area_load;

		LINK( pHelp, area_load->first_help, area_load->last_help, next, prev );
		mud->top_help++;
	}
	pop_call();
	return;
}


/*
	Add a character to the list of all characters
*/

void add_char( CHAR_DATA *ch )
{
	push_call("add_char(%p)",ch);

	if (!IS_NPC(ch))
	{
		if (learned(ch, gsn_first_strike) && mud->f_char)
		{
			INSERT_LEFT(ch, mud->f_char, mud->f_char, next, prev);
		}
		else
		{
			LINK(ch, mud->f_char, mud->l_char, next, prev);
		}
	}
	else
	{
		LINK(ch, mud->f_char, mud->l_char, next, prev);
		LINK(ch, ch->pIndexData->first_instance, ch->pIndexData->last_instance, next_instance, prev_instance);
	}
	pop_call();
	return;
}

/*
	Snarf a mob section.
*/

void load_mobiles( FILE *fp )
{
	MOB_INDEX_DATA *pMobIndex;
	EXTRA_DESCR_DATA *ed;
	int vnum;
	char letter;

	push_call("load_mobiles(%p)",fp);

	while (TRUE)
	{
		letter = fread_letter( fp );
		if (letter != '#')
		{
			bug( "Load_mobiles: # not found.", 0 );
			abort( );
		}

		vnum = fread_number( fp );
		if (vnum == 0)
		{
			break;
		}

		if (vnum < 1 || vnum >= MAX_VNUM)
		{
			bug("load_mobiles: vnum %u out of range.", vnum);
			abort( );
		}
		if (get_mob_index(vnum) != NULL)
		{
			bug("load_mobiles: vnum %u duplicated.", vnum);
			abort( );
		}

		ALLOCMEM(pMobIndex, MOB_INDEX_DATA, 1);

		pMobIndex->vnum			= vnum;
		mob_index[vnum]			= pMobIndex;
		pMobIndex->area			= area_load;

		if (IS_SET(mud->flags, MUD_EMUD_BOOTDB))
		{
			if (area_load->low_m_vnum > vnum)
			{
				area_load->low_m_vnum = vnum;
			}
			if (vnum > area_load->hi_m_vnum)
			{
				area_load->hi_m_vnum = vnum;
			}
		}

		pMobIndex->player_name	= fread_string( fp );
		pMobIndex->short_descr	= fread_string( fp );
		pMobIndex->long_descr	= fread_string( fp );
		pMobIndex->description	= fread_string( fp );
		pMobIndex->act			= fread_number( fp );
		pMobIndex->affected_by	= fread_number( fp );
		pMobIndex->alignment	= fread_number( fp );
		letter				= fread_letter( fp );
		pMobIndex->level		= fread_number( fp );

		pMobIndex->level 		= URANGE(1, pMobIndex->level, 300);

		pMobIndex->body_parts	= fread_number( fp );
		pMobIndex->attack_parts	= fread_number( fp );
		pMobIndex->hitnodice	= fread_number( fp );
							  fread_letter( fp );	/* 'd' */
		pMobIndex->hitsizedice	= fread_number( fp );
							  fread_letter( fp );	/* '+' */
		pMobIndex->hitplus		= fread_number( fp );
		pMobIndex->damnodice	= fread_number( fp );
							  fread_letter( fp );	/* 'd' */
		pMobIndex->damsizedice	= fread_number( fp );
							  fread_letter( fp );	/* '+' */
		pMobIndex->damplus		= fread_number( fp );
		pMobIndex->gold		= fread_number( fp );
		pMobIndex->race		= fread_number( fp );
		pMobIndex->position		= fread_number( fp );
							  fread_number( fp );	/* Unused */
		pMobIndex->sex			= fread_number( fp );

		if (letter != 'S')
		{
			bug( "Load_mobiles: vnum %u non-S.", vnum );
			abort( );
		}

		while (letter)
		{
			switch (letter = fread_letter(fp))
			{
				case 'E':
					ALLOCMEM(ed, EXTRA_DESCR_DATA, 1);
					ed->keyword		= fread_string( fp );
					ed->description	= fread_string( fp );
					LINK(ed, pMobIndex->first_extradesc, pMobIndex->last_extradesc, next, prev);
					mud->top_ed++;
					break;
				case 'C':
					pMobIndex->class	= fread_number( fp );
					break;
				case '>':
					ungetc(letter, fp);
					mprog_read_programs(fp, pMobIndex);
					letter = FALSE;
					break;

				default:
					ungetc(letter, fp);
					letter = FALSE;
					break;
			}
		}
		mud->top_mob_index++;
	}
	pop_call();
	return;
}

/*
	Snarf an obj section.
*/

void load_objects( FILE *fp )
{
	OBJ_PROG *prg;
	OBJ_INDEX_DATA *pObjIndex;
	EXTRA_DESCR_DATA *ed;
	AFFECT_DATA *paf;
	char buf[256], *pt;
	int vnum, index;
	char letter;

	push_call("load_object(%p)",fp);

	while (TRUE)
	{
		letter = fread_letter( fp );

		if ( letter != '#' )
		{
			bug( "Load_objects: # not found.", 0 );
			abort( );
		}

		vnum = fread_number( fp );

		if ( vnum == 0 )
		{
			break;
		}

		if (vnum < 1 || vnum >= MAX_VNUM)
		{
			bug("load_objects: vnum %u out of range.", vnum);
			abort( );
		}

		if (get_obj_index(vnum) != NULL)
		{
			bug("load_objects: vnum %u duplicated.", vnum);
			abort();
		}

		ALLOCMEM(pObjIndex, OBJ_INDEX_DATA, 1);
		pObjIndex->vnum = vnum;

		if (area_load->low_o_vnum > vnum)
		{
			area_load->low_o_vnum = vnum;
		}
		if (vnum > area_load->hi_o_vnum)
		{
			area_load->hi_o_vnum = vnum;
		}
		obj_index[ vnum ] = pObjIndex;
		pObjIndex->area = area_load;

		pt = fread_string( fp );  /* Add I### system to names */
		sprintf( buf, "i%u", vnum );
		if (strstr(pt, buf) == NULL)
		{
			sprintf( buf, "%s i%u", pt, vnum );
		}
		else
		{
			strcpy( buf, pt );
		}
		STRFREE (pt );

		pObjIndex->name               = STRALLOC(buf);
		pObjIndex->short_descr        = fread_string( fp );
		pObjIndex->long_descr         = fread_string( fp );
		pObjIndex->description        = fread_string( fp );
		pObjIndex->item_type          = fread_number( fp );
		pObjIndex->extra_flags        = fread_number( fp );
		pObjIndex->wear_flags         = fread_number( fp );
		pObjIndex->value[0]           = fread_number( fp );
		pObjIndex->value[1]           = fread_number( fp );
		pObjIndex->value[2]           = fread_number( fp );
		pObjIndex->value[3]           = fread_number( fp );
		pObjIndex->value[4]           = fread_number( fp );
		pObjIndex->value[5]           = fread_number( fp );
		pObjIndex->value[6]           = fread_number( fp );
		pObjIndex->value[7]           = fread_number( fp );
		pObjIndex->weight             = abs(fread_number( fp ));
		pObjIndex->cost               = abs(fread_number( fp ));
		pObjIndex->level              = fread_number( fp );
		pObjIndex->attack_string      = STRDUPE(str_empty);

		while (letter)
		{
			letter = fread_letter(fp);



			switch (letter)
			{
				case 'A':
					ALLOCMEM(paf, AFFECT_DATA, 1);
					paf->type			= 0;
					paf->duration		= -1;
					paf->location		= fread_number( fp );
					paf->modifier		= fread_number( fp );
					paf->bitvector 	= fread_number( fp );
					paf->bittype		= paf->bitvector ? AFFECT_TO_CHAR : AFFECT_TO_NONE;

					LINK(paf, pObjIndex->first_affect, pObjIndex->last_affect, next, prev);
					SET_BIT(pObjIndex->extra_flags, ITEM_MAGIC);
					mud->top_affect++;
					break;

				case 'C':
					STRFREE(pObjIndex->attack_string);
					pObjIndex->attack_string	= fread_string( fp );
					pObjIndex->class_flags	= fread_number( fp );
					break;

				case 'E':
					ALLOCMEM(ed, EXTRA_DESCR_DATA, 1);
					ed->keyword			= fread_string( fp );
					ed->description		= fread_string( fp );
					LINK(ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc, next, prev);
					mud->top_ed++;
					break;

				case 'P':
					prg = load_object_program(fp);
					LINK(prg, pObjIndex->first_oprog, pObjIndex->last_oprog, next, prev);
					SET_BIT(pObjIndex->oprogtypes, prg->trigger);
					break;

				case 'S':
					index = fread_number(fp);
					pObjIndex->value[index] = skill_lookup(fread_word(fp));
					break;

				case '>':
					ungetc(letter, fp);
					oprog_read_programs(fp, pObjIndex);
					letter = FALSE;
					break;
				default:
					ungetc(letter, fp);
					letter = FALSE;
					break;
			}
		}


		if (pObjIndex->first_oprog != NULL)
		{
			obj_prog_if_dest( pObjIndex );
		}

		mud->top_obj_index++;
	}
	pop_call();
	return;
}

/*
	Snarf a reset section.
*/

void load_resets( FILE *fp )
{
	RESET_DATA *pReset;
	char letter;

	push_call("load_resets(%p)",fp);

	if (area_load == NULL)
	{
		bug( "Load_resets: no #AREA seen yet.", 0 );
		abort( );
	}

	while (TRUE)
	{
		if ((letter = fread_letter(fp)) == 'S')
		{
			break;
		}

		ALLOCMEM(pReset, RESET_DATA, 1);

		pReset->command	= letter;
		pReset->arg1		= fread_number( fp );
		pReset->arg2		= fread_number( fp );
		pReset->arg3		= fread_number( fp );

		LINK(pReset, area_load->first_reset, area_load->last_reset, next, prev);
		mud->top_reset++;

		/*
			Add container references while at it - Scandum
		*/

		switch ( letter )
		{
			case 'P':
				pReset->container = get_reset_from_obj(pReset);
				break;

			case 'G':
			case 'E':
				pReset->container = get_reset_from_mob(pReset);
				break;
		}
	}
	pop_call();
	return;
}

/*
	Snarf a room section.
	And counting occurances of things.
*/

void load_rooms( FILE *fp )
{
	ROOM_INDEX_DATA *pRoomIndex;
	EXIT_DATA *pexit;
	EXTRA_DESCR_DATA *ed;
	int vnum;
	bool door;
	char letter;

	push_call("load_rooms(%p)",fp);

	if (area_load == NULL)
	{
		bug( "Load_resets: no #AREA seen yet.", 0 );
		abort( );
	}

	while (TRUE)
	{
		letter = fread_letter( fp );

		if (letter != '#')
		{
		    bug( "Load_rooms: # not found.", 0 );
		    abort( );
		}

		vnum	= fread_number(fp);

		if (vnum == 0)
		{
			break;
		}

		if (vnum < 0 || vnum >= MAX_VNUM)
		{
			bug("load_rooms: vnum %u out of range.", vnum);
			abort( );
		}

		if (room_index[vnum] != NULL && vnum < MAX_VNUM)
		{
			bug("load_rooms: vnum %u duplicated.", vnum);
			abort( );
		}

		ALLOCMEM(pRoomIndex, ROOM_INDEX_DATA, 1);

		if (vnum < area_load->low_r_vnum)
		{
			area_load->low_r_vnum = vnum;
		}
		if (vnum > area_load->hi_r_vnum)
		{
			area_load->hi_r_vnum = vnum;
		}

		pRoomIndex->area			= area_load;
		pRoomIndex->vnum			= vnum;
		room_index[vnum]			= pRoomIndex;

		pRoomIndex->name			= fread_string( fp );
		pRoomIndex->description		= fread_string( fp );
		pRoomIndex->creator_pvnum	= fread_number( fp );
		pRoomIndex->room_flags		= fread_number( fp );
		pRoomIndex->sector_type		= fread_number( fp );

		for (door = 0 ; door < MAX_LAST_LEFT ; door++)
		{
			pRoomIndex->last_left_name[door] = STRDUPE(str_empty);
		}

		while (letter)
		{
			letter = fread_letter(fp);

			switch (letter)
			{
				case 'S':
					letter = FALSE;
					break;

				case 'D':
					door = fread_number( fp );
					if (door > 5)
					{
						bug("fread_rooms: vnum %u has bad door number.", vnum);
						abort( );
					}
					ALLOCMEM(pexit, EXIT_DATA, 1);
					pexit->description		= fread_string( fp );
					pexit->keyword			= fread_string( fp );
					pexit->flags		= fread_number( fp );
					pexit->key			= fread_number( fp );
					pexit->vnum			= fread_number( fp );
					pexit->to_room			= pexit->vnum;
					pRoomIndex->exit[door]	= pexit;
					mud->top_exit++;
					break;

				case 'F':	/* Old fall room stuff */
					fread_number( fp );
					fread_number( fp );
					fread_number( fp );
					break;

				case 'E':
					ALLOCMEM(ed, EXTRA_DESCR_DATA, 1);
					ed->keyword				= fread_string( fp );
					ed->description			= fread_string( fp );
					LINK(ed, pRoomIndex->first_extradesc, pRoomIndex->last_extradesc, next, prev);
					mud->top_ed++;
					break;

				case '>':
					ungetc(letter, fp);
					rprog_read_programs(fp, pRoomIndex);
					break;

				default:
					ungetc(letter, fp);
					letter = FALSE;
					break;
			}
		}
		mud->top_room++;
	}
	pop_call();
	return;
}

/*
	Snarf a shop section.
*/

void load_shops( FILE *fp )
{
	SHOP_DATA *pShop;
	MOB_INDEX_DATA *pMobIndex;
	bool iTrade;

	push_call("load_shops(%p)",fp);

	while (TRUE)
	{
		ALLOCMEM( pShop, SHOP_DATA, 1);

		pShop->keeper = fread_number( fp );

		if (pShop->keeper == 0)
		{
			FREEMEM(pShop);
			break;
		}
		if (get_mob_index(pShop->keeper) == NULL)
		{
			bug("load_shop: Mobile %d does not exist", pShop->keeper);
			abort();
		}
		for (iTrade = 0 ; iTrade < MAX_TRADE ; iTrade++)
		{
			pShop->buy_type[iTrade] = fread_number( fp );
		}
		pShop->profit_buy	= fread_number( fp );
		pShop->profit_buy	= UMAX(pShop->profit_buy, 100);
		pShop->profit_sell	= fread_number( fp );
		pShop->profit_sell	= UMIN(pShop->profit_sell, 50);
		pShop->open_hour	= fread_number( fp );
		pShop->close_hour	= fread_number( fp );
		fread_to_eol( fp );
		pMobIndex 		= get_mob_index( pShop->keeper );

		pMobIndex->pShop	= pShop;

		LINK(pShop, mud->f_shop, mud->l_shop, next, prev);

		mud->top_shop++;
	}
	pop_call();
	return;
}

/*
	Snarf spec proc declarations.
*/

void load_specials( FILE *fp )
{
	int vnum;
	MOB_INDEX_DATA *pMobIndex;
	char letter;

	push_call("load_special(%p)",fp);

	while (TRUE)
	{
		switch (letter = fread_letter(fp))
		{
			default:
				bug("load_specials: letter '%c' not MSO.", letter);
				abort( );

			case 'S':
				pop_call();
				return;

			case 'M':
				vnum				= fread_number  ( fp );
				pMobIndex			= get_mob_index ( vnum );
				pMobIndex->spec_fun = spec_lookup   ( fread_word   ( fp ) );

				if (pMobIndex->spec_fun == NULL)
				{
					log_printf("load_specials: 'M': vnum %u.", pMobIndex->vnum );
				}
				break;
		}
		fread_to_eol( fp );
	}
	pop_call();
	return;
}

/*
	Connect castles to the real word, unused - Scandum
*/


void fix_exits( void )
{
	ROOM_INDEX_DATA *pRoomIndex;
	EXIT_DATA *pexit;
	int vnum, max_vnum;
	bool door;

	push_call("fix_exits(void)");

	max_vnum = room_index[ROOM_VNUM_CASTLE]->area->hi_r_vnum;

	for (vnum = ROOM_VNUM_CASTLE ; vnum <= max_vnum ; vnum++)
	{
		if ((pRoomIndex = room_index[vnum]) == NULL)
		{
			continue;
		}

		if (IS_SET(pRoomIndex->room_flags, ROOM_IS_ENTRANCE))
		{
			for (door = 0 ; door <= 5 ; door++)
			{
				if (pRoomIndex->exit[door] == NULL)
				{
					continue;
				}

				if (room_index[pRoomIndex->exit[door]->to_room] == NULL)
				{
					continue;
				}

				if (!IS_SET(pRoomIndex->room_flags, ROOM_IS_CASTLE))
				{
					continue;
				}

				if (room_index[pRoomIndex->exit[door]->to_room]->area == pRoomIndex->area)
				{
					continue;
				}

				if (room_index[pRoomIndex->exit[door]->to_room]->exit[rev_dir[door]] != NULL)
				{
					log_printf("Failure linking entrance of castle room: %u", pRoomIndex->vnum);
				}
				else
				{
					ALLOCMEM(pexit, EXIT_DATA, 1);
					mud->top_exit++;
					pexit->description 		= STRDUPE(pRoomIndex->exit[door]->description);
					pexit->keyword 		= STRDUPE(pRoomIndex->exit[door]->keyword);
					pexit->flags		= pRoomIndex->exit[door]->flags;
					pexit->key			= pRoomIndex->exit[door]->key;
					pexit->to_room			= pRoomIndex->vnum;

					room_index[pRoomIndex->exit[door]->to_room]->exit[rev_dir[door]] = pexit;
				}
			}
		}
	}
	pop_call();
	return;
}


/*
	Repopulate areas periodically.
*/

void repop_area ( AREA_DATA *pArea )
{
	PLAYER_GAME *gpl;

	pArea->age++;

	if (pArea->nplayer > 0 && pArea->age == 39 && (!pArea->resetmsg || strcasecmp(pArea->resetmsg, "off")))
	{
		int rnd = number_range(1, 4);

		for (gpl = mud->f_player ; gpl ; gpl = gpl->next)
		{
			if (IS_AWAKE(gpl->ch) && gpl->ch->in_room && gpl->ch->in_room->area == pArea)
			{
				if (pArea->resetmsg && pArea->resetmsg[0] != '\0')
				{
					ch_printf(gpl->ch, "%s\n\r", pArea->resetmsg);
				}
				else
				{
					switch (gpl->ch->in_room->sector_type)
					{
						case SECT_INSIDE:
							switch (rnd)
							{
								case 1: ch_printf(gpl->ch, "You hear the boards creaking.\n\r");				break;
								case 2: ch_printf(gpl->ch, "You hear growling echoing in the distance.\n\r");	break;
								case 3: ch_printf(gpl->ch, "You hear wax dripping off a candle.\n\r" );		break;
								case 4: ch_printf(gpl->ch, "You hear foot steps in the distance.\n\r");		break;
							}
							break;
						case SECT_CITY:
							switch (rnd)
							{
								case 1: ch_printf(gpl->ch, "You hear the shuffle of feet out in the street.\n\r");	break;
								case 2: ch_printf(gpl->ch, "You hear someone calling for the guards.\n\r");		break;
								case 3: ch_printf(gpl->ch, "You hear shopkeepers announcing their wares.\n\r");		break;
								case 4: ch_printf(gpl->ch, "You hear a dog barking in the distance.\n\r");			break;
							}
							break;
						case SECT_FIELD:
							switch (rnd)
							{
								case 1: ch_printf(gpl->ch, "You hear small creatures rustling the grass.\n\r");			break;
								case 2: ch_printf(gpl->ch, "You hear large creatures rustling the grass.\n\r");			break;
								case 3: ch_printf(gpl->ch, "You hear some field mice scurrying about.\n\r");			break;
								case 4: ch_printf(gpl->ch, "You hear a wolf barking to itself in the distance.\n\r");	break;
							}
							break;
						case SECT_FOREST:
							switch (rnd)
							{
								case 1: ch_printf(gpl->ch, "You hear the trees creaking in the wind.\n\r");							break;
								case 2: ch_printf(gpl->ch, "You hear wolves howling in the distance.\n\r");							break;
								case 3: ch_printf(gpl->ch, "You hear some lumberjacks calling timber.\n\r");							break;
								case 4: ch_printf(gpl->ch, "You hear the distinct sound of trees grooming one another in the wind.\n\r");	break;
							}
							break;
						case SECT_HILLS:
							switch (rnd)
							{
								case 1: ch_printf(gpl->ch, "You hear quiet, unintelligible muttering from no particular direction.\n\r");	break;
								case 2: ch_printf(gpl->ch, "You hear the wind whistling past your face.\n\r");							break;
								case 3: ch_printf(gpl->ch, "You hear green grass growing around you.\n\r");							break;
								case 4: ch_printf(gpl->ch, "You hear the bleating of a goat carried far across the hillside.\n\r"); 		break;
							}
							break;
						case SECT_MOUNTAIN:
							switch (rnd)
							{
								case 1: ch_printf(gpl->ch, "You hear rocks tumbling down the mountain.\n\r");					break;
								case 2: ch_printf(gpl->ch, "You hear echos of large hooved animals wandering the mountain.\n\r");	break;
								case 3: ch_printf(gpl->ch, "You hear small avalanches crashing down the mountain.\n\r");			break;
								case 4: ch_printf(gpl->ch, "You hear a rock falling down the side of a peak close by.\n\r");		break;
							}
							break;
						case SECT_LAKE:
							switch (rnd)
							{
								case 1: ch_printf(gpl->ch, "You hear fish splashing in the water.\n\r");					break;
								case 2: ch_printf(gpl->ch, "You hear something large fall into the water.\n\r");			break;
								case 3: ch_printf(gpl->ch, "You hear some old fisherman cast their lines about you.\n\r");	break;
								case 4: ch_printf(gpl->ch, "You hear water lapping timidly against the shore.\n\r");			break;
							}
							break;
						case SECT_RIVER:
							switch (rnd)
							{
								case 1: ch_printf(gpl->ch, "The river's waters gurgle softly as they flow.\n\r");				break;
								case 2: ch_printf(gpl->ch, "Water splashes up into the air as it runs over some rocks.\n\r");		break;
								case 3: ch_printf(gpl->ch, "You hear fish splashing in the water.\n\r");						break;
								case 4: ch_printf(gpl->ch, "You hear something large splash through the river down stream.\n\r");	break;
							}
							break;
						case SECT_OCEAN:
							switch (rnd)
							{
								case 1: ch_printf(gpl->ch, "You hear the waves cresting loudly.\n\r");				break;
								case 2: ch_printf(gpl->ch, "You hear a large fish splash in the water.\n\r");			break;
								case 3: ch_printf(gpl->ch, "You hear a fog horn echoing through the darkness.\n\r");		break;
								case 4: ch_printf(gpl->ch, "You hear the echoes of sea creatures deep below you.\n\r");	break;
							}
							break;
						case SECT_AIR:
							switch (rnd)
							{
								case 1: ch_printf(gpl->ch, "You hear the wind blowing harshly.\n\r");						break;
								case 2: ch_printf(gpl->ch, "You hear a hawk screeching loudly.\n\r");						break;
								case 3: ch_printf(gpl->ch, "You hear a storm forming around you.\n\r");					break;
								case 4: ch_printf(gpl->ch, "You hear air rushing at you from all directions at once.\n\r");	break;
							}
							break;
						case SECT_DESERT:
							switch (rnd)
							{
								case 1: ch_printf(gpl->ch, "You hear the sand whipping fiercely.\n\r");							break;
								case 2: ch_printf(gpl->ch, "You hear the hollow cries of something just barely alive.\n\r");			break;
								case 3: ch_printf(gpl->ch, "You hear a sandstorm growing in the distance.\n\r");					break;
								case 4: ch_printf(gpl->ch, "You hear the rumbling noise of a sand avalanche in the distance.\n\r");		break;
							}
							break;
						case SECT_LAVA:
							switch (rnd)
							{
								case 1: ch_printf(gpl->ch, "You hear the bubbling of solid stone.\n\r");						break;
								case 2: ch_printf(gpl->ch, "You hear grinding of rocks.\n\r");								break;
								case 3: ch_printf(gpl->ch, "You hear lava sizzling on cold stone.\n\r");						break;
								case 4: ch_printf(gpl->ch, "You hear the rush of swiftly flowing lava somewhere close by.\n\r");	break;
							}
							break;
						case SECT_ETHEREAL:
							switch (rnd)
							{
								case 1: ch_printf(gpl->ch, "You hear echos of an unknown creature.\n\r");							break;
								case 2: ch_printf(gpl->ch, "You hear the distant explosions of power.\n\r");						break;
								case 3: ch_printf(gpl->ch, "You hear an indescribable sound.\n\r");								break;
								case 4: ch_printf(gpl->ch, "You feel a transparent mist appear and thicken about you momentarily.\n\r");	break;
							}
							break;
						case SECT_ASTRAL:
							switch (rnd)
							{
								case 1: ch_printf(gpl->ch, "You hear silence echoing in your head.\n\r");					break;
								case 2: ch_printf(gpl->ch, "You hear the pain of a lost soul.\n\r");						break;
								case 3: ch_printf(gpl->ch, "You hear the beating of your own heart.\n\r");					break;
								case 4: ch_printf(gpl->ch, "You hear high pitched voices whispering quietly around you.\n\r");	break;
							}
							break;
						case SECT_UNDER_WATER:
							switch (rnd)
							{
								case 1: ch_printf(gpl->ch, "You hear the gurgling of water.\n\r");					break;
								case 2: ch_printf(gpl->ch, "You hear the clicking of a large marine animal.\n\r");		break;
								case 3: ch_printf(gpl->ch, "You hear the squeal of a dying fish.\n\r");				break;
								case 4: ch_printf(gpl->ch, "You hear the echoing roar of a giant sea creature.\n\r");	break;
							}
							break;
						case SECT_UNDER_GROUND:
							switch (rnd)
							{
								case 1: ch_printf(gpl->ch, "You hear a loud crash!\n\r");									break;
								case 2: ch_printf(gpl->ch, "Some cracks appear in the rock above your head.\n\r");				break;
								case 3: ch_printf(gpl->ch, "You hear someone kick some loose rumble and a mumbled apology.\n\r");	break;
								case 4: ch_printf(gpl->ch, "You hear nothing but the sound of your own breathing.\n\r");			break;
							}
							break;
						case SECT_DEEP_EARTH:
							switch (rnd)
							{
								case 1: ch_printf(gpl->ch, "You hear a distant cry echoing all around you.\n\r");		break;
								case 2: ch_printf(gpl->ch, "You hear the groan of the immense weight above you.\n\r");	break;
								case 3: ch_printf(gpl->ch, "You smell ozone.\n\r");							break;
								case 4: ch_printf(gpl->ch, "You hear the rocks around you shifting and settling.\n\r");	break;
							}
							break;
						case SECT_ROAD:
							switch (rnd)
							{
								case 1: ch_printf(gpl->ch, "You hear trade caravans riding off in the distance.\n\r");				break;
								case 2: ch_printf(gpl->ch, "You hear horsemen yelling in the distance.\n\r");					break;
								case 3: ch_printf(gpl->ch, "You hear crickets hidden alongside the road.\n\r");					break;
								case 4: ch_printf(gpl->ch, "You feel a gust of wind and streams of dust are blown from the road.\n\r");	break;
							}
							break;
						case SECT_SWAMP:
							switch (rnd)
							{
								case 1: ch_printf(gpl->ch, "Gurgling bubbles belch forth from the calm surface of the water.\n\r");				break;
								case 2: ch_printf(gpl->ch, "You hear frogs croaking in the distance.\n\r");								break;
								case 3: ch_printf(gpl->ch, "You hear mosquitos buzzing around you.\n\r");									break;
								case 4: ch_printf(gpl->ch, "The wind carries the terrified cries of something pulled into the murky depths.\n\r");	break;
							}
							break;
						case SECT_BEACH:
							switch (rnd)
							{
								case 1: ch_printf(gpl->ch, "You hear a faint clicking of claws as large crabs scurry across the sand.\n\r");		break;
								case 2: ch_printf(gpl->ch, "You hear the grating squawk of a seagull as it nabs a starfish on the sand.\n\r");	break;
								case 3: ch_printf(gpl->ch, "You hear the soft roar of waves crashing against the beach.\n\r");				break;
								case 4: ch_printf(gpl->ch, "The clean scent of seaweed fills the air as it washes up on the beach.\n\r");		break;
							}
							break;
						case SECT_TUNDRA:
							switch (rnd)
							{
								case 1: ch_printf(gpl->ch, "The distant barking of cavorting seals can be heard in the distance.\n\r");
								case 2: ch_printf(gpl->ch, "The wind whistles as snows dance and spiral over the tundra.\n\r");
								case 3: ch_printf(gpl->ch, "The icy silence is broken by the thunderous tread of a woolly mammoth\n\r");
								case 4: ch_printf(gpl->ch, "You hear the powerful roar of a sabretooth tiger capturing its prey.\n\r");
							}
							break;
						case SECT_EDGE:
							switch (rnd)
							{
								case 1: ch_printf(gpl->ch, "You hear thousands of creatures rushing at you from all directions.\n\r");	break;
								case 2: ch_printf(gpl->ch, "You hear nothing at all.\n\r");										break;
								case 3: ch_printf(gpl->ch, "You hear the sound of infinity.\n\r");								break;
								case 4: ch_printf(gpl->ch, "You feel the sands of time swirl around you.\n\r");						break;
							}
							break;
						default:
							ch_printf(gpl->ch, "You hear the patter of little feet.\n\r");
							break;
					}
				}	
			}
		}
	}

	/*
		Check age and reset.
	*/

	if (pArea->age >= 40)
	{
		reset_area( pArea );
	}
}

void area_update( void )
{
	AREA_DATA *pArea;

	push_call("area_update(void)");

	for (pArea = mud->f_area ; pArea ; pArea = pArea->next)
	{
		repop_area(pArea);
	}
	pop_call();
	return;
}

/*
	Locate last instance of obj reset
*/

RESET_DATA *get_reset_from_obj(RESET_DATA *lReset)
{
	RESET_DATA *pReset;

	push_call("get_reset_from_obj(%p)",lReset);

	for (pReset = lReset ; pReset ; pReset = pReset->prev)
	{
		if (pReset->arg1 == lReset->arg3)
		{
			if (pReset->command == 'O' || pReset->command == 'G' || pReset->command == 'E')
			{
				pop_call();
				return pReset;
			}
		}
	}
	log_printf("get_reset_from_obj: bad '%c' reset", lReset->command);
	pop_call();
	return NULL;
}

/*
	Locate last instance of mobile reset
*/

RESET_DATA *get_reset_from_mob( RESET_DATA *lReset )
{
	RESET_DATA *pReset;

	push_call("get_reset_from_mob(%p)",lReset);

	for (pReset = lReset->prev ; pReset ; pReset = pReset->prev)
	{
		if (pReset->command == 'M')
		{
			pop_call();
			return pReset;
		}
	}
	log_printf("get_reset_from_mob: bad '%c' reset", lReset->command);

	pop_call();
	return NULL;
}

void do_resetarea( CHAR_DATA *ch, char *arg)
{
	push_call("do_resetarea(%p,%p)",ch,arg);

	if (IS_NPC(ch))
	{
		log_printf("mob using area reset.");
		dump_stack();
		pop_call();
		return;
	}

	if (!can_olc_modify(ch, ch->in_room->vnum))
	{
		ch_printf(ch, "This area is not in your allocated range.\n\r");
		pop_call();
		return;
	}

	SET_BIT(mud->flags, MUD_EMUD_BOOTDB);

	if (IS_SET(mud->flags, MUD_EMUD_REALGAME))
	{
		log_printf("%s resetted area %s", get_name(ch), ch->in_room->area->name);
	}

	send_to_char("Resetting area.\n\r", ch);
	reset_area(ch->in_room->area);

	REMOVE_BIT(mud->flags, MUD_EMUD_BOOTDB);

	pop_call();
	return;
}

/*
	Create an instance of a mobile.
*/

CHAR_DATA *create_mobile( MOB_INDEX_DATA *pMobIndex )
{
	CHAR_DATA *mob;
	NPC_DATA *np;

	push_call("create_mobile(%p)",pMobIndex);

	if (pMobIndex == NULL)
	{
		bug("create_mobile: NULL pMobIndex.");
		abort( );
	}

	if (pMobIndex->total_mobiles > 1000)
	{
		log_build_printf(pMobIndex->vnum, "More than 1000 mobs created: %d.\n\r", pMobIndex->total_mobiles);
	}

	mud->total_mob++;

	ALLOCMEM(mob, CHAR_DATA, 1);
	ALLOCMEM(np,  NPC_DATA,  1);


	clear_char( mob );

	mob->npcdata              = np;
	mob->pIndexData           = pMobIndex;
	mob->name                 = STRDUPE(pMobIndex->player_name);
	mob->short_descr          = STRDUPE(pMobIndex->short_descr);
	mob->long_descr           = STRDUPE(pMobIndex->long_descr);
	mob->description          = STRDUPE(pMobIndex->description);
	mob->act                  = pMobIndex->act;
	mob->affected_by          = pMobIndex->affected_by;
	mob->alignment            = pMobIndex->alignment;
	mob->sex                  = pMobIndex->sex;
	mob->max_hit              = dice(pMobIndex->hitnodice, pMobIndex->hitsizedice) + pMobIndex->hitplus;
	mob->hit                  = mob->max_hit;
	mob->gold                 = mob_worth(pMobIndex);
	mob->level                = number_fuzzy(pMobIndex->level);

	mob->npcdata->damnodice	  = pMobIndex->damnodice;
	mob->npcdata->damsizedice = pMobIndex->damsizedice;
	mob->npcdata->damplus     = pMobIndex->damplus;
	mob->npcdata->remember    = STRDUPE(str_empty);
	mob->npcdata->hate_fear   = STRDUPE(str_empty);

	mob->race                 = pMobIndex->race;
	mob->class                = pMobIndex->class;
	mob->speed                = get_max_speed(mob);
	mob->position             = pMobIndex->position;

	add_char( mob );

	pMobIndex->total_mobiles++;

	pop_call();
	return mob;
}

/*
	Create an instance of an object.
*/

OBJ_DATA *create_object( OBJ_INDEX_DATA *pObjIndex, bool hashed )
{
	OBJ_DATA *obj;

	push_call("create_object(%p,%p)",pObjIndex,hashed);

	if (pObjIndex == NULL)
	{
		log_string("Create_object: NULL pObjIndex.");
		pop_call();
		return NULL;
	}

	ALLOCMEM(obj, OBJ_DATA, 1);

	obj->pIndexData     = pObjIndex;
	obj->level          = pObjIndex->level;
	obj->wear_loc       = WEAR_NONE;

	if (!hashed)
	{
		add_obj_ref_hash(obj);
	}

	obj->name           = STRDUPE(pObjIndex->name);
	obj->short_descr    = STRDUPE(pObjIndex->short_descr);
	obj->long_descr     = STRDUPE(pObjIndex->long_descr);
	obj->description    = STRDUPE(pObjIndex->description);
	obj->item_type      = pObjIndex->item_type;
	obj->extra_flags    = pObjIndex->extra_flags;
	obj->wear_flags     = pObjIndex->wear_flags;
	obj->value[0]       = pObjIndex->value[0];
	obj->value[1]       = pObjIndex->value[1];
	obj->value[2]       = pObjIndex->value[2];
	obj->value[3]       = pObjIndex->value[3];
	obj->value[4]       = pObjIndex->value[4];
	obj->value[5]       = pObjIndex->value[5];
	obj->value[6]       = pObjIndex->value[6];
	obj->value[7]       = pObjIndex->value[7];
	obj->weight         = pObjIndex->weight;
	obj->cost           = obj_worth(pObjIndex);
	obj->level          = pObjIndex->level;

	/*
		Mess with object properties.
	*/

	switch (obj->item_type)
	{
		default:
			log_printf("create_object: vnum %u has bad item_type", pObjIndex->vnum);
			break;

		case ITEM_LIGHT:
		case ITEM_FURNITURE:
		case ITEM_TRASH:
		case ITEM_CONTAINER:
		case ITEM_DRINK_CON:
		case ITEM_KEY:
		case ITEM_FOOD:
		case ITEM_BOAT:
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
		case ITEM_FOUNTAIN:
		case ITEM_PORTAL:
		case ITEM_ARTIFACT:
		case ITEM_LOCKPICK:
		case ITEM_TOTEM:
			break;

		case ITEM_SCROLL:
		case ITEM_POTION:
		case ITEM_PILL:
			obj->value[0] = number_fuzzy(obj->value[0]);
			break;

		case ITEM_WAND:
		case ITEM_STAFF:
			obj->value[0] = number_fuzzy(obj->value[0]);
			obj->value[1] = number_fuzzy(obj->value[1]);
			break;

		case ITEM_WEAPON:
			break;

		case ITEM_ARMOR:
			break;

		case ITEM_TREASURE:
			obj->value[0] = URANGE(1, obj->value[0], obj->level * 100);
			break;

		case ITEM_MONEY:
			obj->value[0] = URANGE(1, obj->value[0], obj->level * 100);
			break;

		case ITEM_AMMO:
			obj->value[1] = number_fuzzy(obj->value[1]);
			break;
	}

	LINK(obj, mud->f_obj, mud->l_obj, next, prev);
	LINK(obj, obj->pIndexData->first_instance, obj->pIndexData->last_instance, next_instance, prev_instance);
	mud->total_obj++;
	pObjIndex->total_objects++;
	pop_call();
	return obj;
}

/*
	Clear a new character.
*/

void clear_char( CHAR_DATA *ch )
{
	push_call("clear_char(%p)",ch);

	ch->position = POS_STANDING;
	ch->hit      = 20;
	ch->max_hit  = 20;
	ch->mana     = 100 + ch->level * 10;
	ch->max_mana = 100 + ch->level * 10;
	ch->move     = 100 + ch->level * 10;
	ch->max_move = 100 + ch->level * 10;

	pop_call();
	return;
}

/*
	Free a character.
*/

void free_char( CHAR_DATA *ch )
{
	POISON_DATA *pd;
	CRIME_DATA  *crime;
	int cnt;

	push_call("free_char(%p)",ch);

	while (ch->first_carrying)
	{
		extract_obj(ch->first_carrying);
	}

	while (ch->first_affect)
	{
		affect_from_char(ch, ch->first_affect);
	}

	while ((pd = ch->poison) != NULL)
	{
		ch->poison = ch->poison->next;
		FREEMEM(pd);
	}

	STRFREE(ch->name);
	STRFREE(ch->short_descr);
	STRFREE(ch->long_descr);
	STRFREE(ch->description);


	if (ch->fighting)
	{
		stop_fighting(ch, TRUE);
	}

	if (ch->pcdata)
	{
		STRFREE(ch->pcdata->host			);
		STRFREE(ch->pcdata->mail_address	);
		STRFREE(ch->pcdata->html_address	);
		STRFREE(ch->pcdata->block_list	);
		STRFREE(ch->pcdata->title		);
		STRFREE(ch->pcdata->auto_command	);
		STRFREE(ch->pcdata->bamfin		);
		STRFREE(ch->pcdata->bamfout		);
		STRFREE(ch->pcdata->tactical_index	);
		STRFREE(ch->pcdata->edit_buf		);
		STRFREE(ch->pcdata->prompt_layout	);
		STRFREE(ch->pcdata->subprompt		);
		STRFREE(ch->pcdata->tracking		);
		STRFREE(ch->pcdata->ambush		);
		STRFREE(ch->pcdata->clan_name		);
		STRFREE(ch->pcdata->clan_pledge	);

		if (ch->pcdata->last_command)
		{
			STRFREE(ch->pcdata->last_command);
		}

		if (ch->pcdata->page_buf)
		{
			STRFREE(ch->pcdata->page_buf);
		}

		while ((crime = ch->pcdata->first_record) != NULL)
		{
			ch->pcdata->first_record = crime->next;

			STRFREE(crime->crime_record);
			STRFREE(crime->arrester);
			STRFREE(crime->releaser);
			FREEMEM(crime);
		}


		if (ch->pcdata->pnote != NULL)
		{
			STRFREE(ch->pcdata->pnote->text);
			STRFREE(ch->pcdata->pnote->subject);
			STRFREE(ch->pcdata->pnote->to_list);
			STRFREE(ch->pcdata->pnote->date);
			STRFREE(ch->pcdata->pnote->sender);
			FREEMEM(ch->pcdata->pnote);
		}

		if (ch->pcdata->editor)
		{
			stop_editing(ch);
		}

		for (cnt = 0 ; cnt < MAX_PK_ATTACKS ; cnt++)
		{
			STRFREE(ch->pcdata->last_pk_attack_name[cnt]);
		}

		for (cnt = 0 ; cnt < MAX_ALIAS ; cnt++)
		{
			STRFREE (ch->pcdata->alias[cnt] );
			STRFREE (ch->pcdata->alias_c[cnt] );
		}

		for (cnt = 0 ; cnt < 26 ; cnt++)
		{
			STRFREE(ch->pcdata->back_buf[cnt] );
		}

		for (cnt = 0 ; cnt < MAX_AREA ; cnt++)
		{
			if (ch->pcdata->quest[cnt] != NULL)
			{
				FREEMEM(ch->pcdata->quest[cnt]);
			}
		}

		if (ch->pcdata->tactical)
		{
			FREEMEM(ch->pcdata->tactical);
		}

		if (ch->pcdata->castle)
		{
			FREEMEM(ch->pcdata->castle );
		}
		FREEMEM(ch->pcdata);
	}

	if (ch->npcdata)
	{
		if (ch->npcdata->mob_quest)
		{
			FREEMEM(ch->npcdata->mob_quest);
		}
		STRFREE(ch->npcdata->remember);
		STRFREE(ch->npcdata->hate_fear);

		FREEMEM(ch->npcdata);
	}

	FREEMEM(ch);

	pop_call();
	return;
}

/*
	Get an extra description from a list.
*/

char *get_extra_descr( const char *name, EXTRA_DESCR_DATA *ed )
{
	push_call("get_extra_descr(%p,%p)",name,ed);

	for ( ; ed ; ed = ed->next)
	{
		if (is_name(name, ed->keyword))
		{
			pop_call();
			return ed->description;
		}
	}
	pop_call();
	return NULL;
}


/*
	Read a letter from a file.
*/

char fread_letter( FILE *fp )
{
	char c;

	push_call("fread_letter(%p)",fp);

	do
	{
		if (feof(fp))
		{
			bug("fread_letter: EOF encountered on read.\n\r");
			if (IS_SET(mud->flags, MUD_EMUD_BOOTDB))
			{
				exit(1);
			}
			pop_call();
			return '\0';
		}
		c = getc(fp);
	}
	while (isspace(c));

	pop_call();
	return c;
}

/*
	Read a number from a file.
*/

long long fread_number( FILE *fp )
{
	int cnt;
	long long number;
	bool sign;
	char c;
	char buf[100];

	push_call("fread_number(%p)",fp);

	buf[0] = '\0';

	do
	{
		if (feof(fp))
		{
			bug("fread_number: EOF encountered on read.\n\r");
			if (IS_SET(mud->flags, MUD_EMUD_BOOTDB))
			{
				exit(1);
			}
			pop_call();
			return 0;
		}
		c = getc(fp);
	}
	while (isspace(c));

	number = 0;

	sign = (c == '-');

	if (c == '+' || c == '-')
	{
		c = getc(fp);
	}

	for (cnt = 0 ; isdigit(c) || isupper(c) || c == '_' ; cnt++)
	{
		buf[cnt] = c;

		c = getc(fp);
	}
	buf[cnt] = '\0';

	if (isupper(buf[0]))
	{
		for (cnt = mud->bitvector_ref[*buf - 'A'] ; cnt < MAX_BITVECTOR ; cnt++)
		{
			if (!strcmp(bitvector_table[cnt].name, buf))
			{
				number = bitvector_table[cnt].value;
				break;
			}
		}
		if (cnt == MAX_BITVECTOR)
		{
			if (HAS_BIT(mud->flags, MUD_EMUD_BOOTDB))
			{
				log_printf("fread_number: bad format1 '%s'.", buf);
			}
			else
			{
				log_printf("fread_number: bad format1 '%s'.", buf);
			}
		}
	}
	else
	{
		if (!isdigit(buf[0]))
		{
			log_printf("fread_number: bad format2 '%c%s'", c, fread_word(fp));
			pop_call();
			return number;
		}

		number = atoll(buf);
	}
	if (sign)
	{
		number = 0 - number;
	}
	if (c == '|')
	{
		number += fread_number( fp );
	}
	else if (c != ' ')
	{
		ungetc( c, fp );
	}
	pop_call();
	return number;
}

/*
	Read and allocate space for a string from a file.
	Strings are created and placed in Dynamic Memory.
*/

char *fread_string( FILE *fp )
{
	char buf[MAX_STRING_LENGTH];
	char *plast;
	char c;
	int ln;

	push_call("fread_string(%p)",fp);

	plast = buf;
	buf[0] = '\0';
	ln = 0;

	/*
		Skip blanks.
		Read first char.
	*/

	do
	{
		if (feof(fp))
		{
			bug("fread_string: EOF encountered on read.\n\r");
			if (IS_SET(mud->flags, MUD_EMUD_BOOTDB))
			{
				exit(1);
			}
			pop_call();
			return STRDUPE(str_empty);
		}
		c = getc( fp );
	}
	while (isspace(c));

	if ((*plast++ = c) == '~')
	{
		pop_call();
		return STRDUPE(str_empty);
	}

	for (ln = 0 ; ln < MAX_STRING_LENGTH ; ln++, plast++)
	{
		switch (*plast = getc(fp))
		{
			case EOF:
				bug( "Fread_string: EOF" );
				if (IS_SET(mud->flags, MUD_EMUD_BOOTDB))
				{
					exit( 1 );
				}
				*plast = '\0';
				pop_call();
				return STRALLOC(buf);

			case '\n':
				plast++;
				ln++;
				*plast = '\r';
				break;

			case '\r':
				plast--;
				ln--;
				break;

			case '~':
				*plast = '\0';
				pop_call();
				return STRALLOC( buf );
		}
	}
	bug("fread_string: string too long" );

	if (IS_SET(mud->flags, MUD_EMUD_BOOTDB))
	{
		abort();
	}
	*plast = '\0';

	pop_call();
	return STRALLOC(buf);
}


/*
	Read to end of line (for comments).
*/

void fread_to_eol( FILE *fp )
{
	int c;

	push_call("fread_to_eol(%p)",fp);

	do
	{
		if (feof(fp))
		{
			bug("fread_to_eol: EOF encountered on read.\n\r", 0);
			if (IS_SET(mud->flags, MUD_EMUD_BOOTDB))
			{
				abort();
			}
			pop_call();
			return;
		}
		c = getc(fp);
	}
	while (c != '\n' && c != '\r');

	do
	{
		c = getc( fp );
	}
	while (c == '\n' || c == '\r');

	ungetc(c, fp);
	pop_call();
	return;
}


char *fread_line( FILE *fp )
{
	static char line[MAX_STRING_LENGTH];
	int c, i;

	push_call("fread_line(%p)",fp);

	i = 0;

	do
	{
		if (feof(fp))
		{
			bug("fread_line: EOF encountered on read.\n\r",0);

			if (IS_SET(mud->flags, MUD_EMUD_BOOTDB))
			{
				abort();
			}
			line[i] = 0;

			pop_call();
			return line;
		}
		c = getc(fp);
	}
	while (isspace(c));

	ungetc(c, fp);

	while (c != '\n' && c != '\r')
	{
		if (feof(fp))
		{
			bug("fread_line: EOF encountered on read.\n\r",0);

			if (IS_SET(mud->flags, MUD_EMUD_BOOTDB))
			{
				abort();
			}

			line[i] = 0;

			pop_call();
			return line;
		}

		c = getc(fp);

		line[i++] = c;

		if (i >= MAX_STRING_LENGTH)
		{
			bug( "fread_line: line too long");

			dump_stack();

			abort();
		}
	}
	while (c != '\n' && c != '\r');

	line[i] = 0;

	do
	{
		c = getc(fp);
	}
	while (c == '\n' || c == '\r');

	ungetc(c, fp);

	pop_call();
	return line;
}

/*
	Read one word (into static buffer).
*/

char *fread_word( FILE *fp )
{
	static char word[MAX_INPUT_LENGTH];
	int c, i, x;

	push_call("fread_word(%p)",fp);

	i = 0;

	do
	{
		if (feof(fp))
		{
			bug("fread_word: EOF encountered on read.\n\r");

			if (IS_SET(mud->flags, MUD_EMUD_BOOTDB))
			{
				abort();
			}
			word[i] = 0;

			pop_call();
			return word;
		}
		c = getc(fp);
	}
	while (isspace(c));

	if (c == '\'' || c == '"')
	{
		x = c;
	}
	else
	{
		x = ' ';
		word[i++] = c;
	}

	while (i < MAX_INPUT_LENGTH)
	{
		if (feof(fp))
		{
			bug("fread_word: EOF encountered on read.\n\r");

			if (IS_SET(mud->flags, MUD_EMUD_BOOTDB))
			{
				abort();
			}
			word[i] = 0;

			pop_call();
			return word;
		}

		c = getc(fp);

		if (c == x || c == EOF || c == '\n')
		{
			if (c == EOF || x == ' ')
			{
				ungetc(c, fp);
			}
			word[i] = 0;

			pop_call();
			return word;
		}
		word[i++] = c;
	}
	word[60] = 0;

	bug("fread_word: word '%s' is too long.");

	abort();
}

void do_areas( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	char mainauth[32];
	AREA_DATA *pArea;
	int pcnt, mcnt, rcnt;
	CHAR_DATA *rch;
	int vnum;
	int lower_bound = 0;
	int upper_bound = MAX_LEVEL;

	push_call("do_areas(%p,%p)",ch,argument);

	if (argument != NULL && *argument != '\0')
	{
		if (isalpha(argument[0]))
		{
			for (pArea = mud->f_area ; pArea ; pArea = pArea->next)
			{
				if (!str_prefix(argument, pArea->name))
				{
					ch_printf(ch, "%s\n\r", pArea->name );
					ch_printf(ch, "Suggested levels:          %2d-%2d\n\r", pArea->low_soft_range, pArea->hi_soft_range);
					ch_printf(ch, "Min/Max levels:            %2d-%2d\n\r", pArea->low_hard_range, pArea->hi_hard_range);

					pcnt=0;
					mcnt=0;
					rcnt=0;

					for (vnum = pArea->low_r_vnum ; vnum <= pArea->hi_r_vnum ; vnum++)
					{
						if (room_index[vnum] && room_index[vnum]->area == pArea)
						{
							rcnt++;

							for (rch = room_index[vnum]->first_person ; rch ; rch = rch->next_in_room)
							{
								if (IS_NPC(rch) && can_see(ch, rch))
								{
									mcnt++;
								}
								else if (!IS_NPC(rch) && can_see_world(ch, rch))
								{
									pcnt++;
								}
							}
						}
					}
					ch_printf(ch, "Rooms in area:              %d\n\r", rcnt );
					ch_printf(ch, "Creatures in area:          %d\n\r", mcnt );
/*
					ch_printf(ch, "Players in area:            %d\n\r", pcnt );
*/
					ch_printf(ch, "Authors of area:            %s\n\r", pArea->authors ? pArea->authors : "none");

					pop_call();
					return;
				}
			}
			send_to_char( "That area was not found.\n\r", ch);
			pop_call();
			return;
		}
		else
		{
			argument = one_argument(argument, arg);

			if (arg[0] != '\0' && is_number(arg))
			{
				lower_bound = URANGE(0, atoi(arg), MAX_LEVEL);
			}
			else
			{
				send_to_char("That area wasn't found.\n\r", ch);
				pop_call();
				return;
			}

			if (argument[0] != '\0' && is_number(argument))
			{
				upper_bound = URANGE(0, atoi(argument), MAX_LEVEL);
			}
			else
			{
				send_to_char("Only two level numbers allowed.\n\r", ch);
				pop_call();
				return;
			}
			if (lower_bound > upper_bound)
			{
				upper_bound = upper_bound + lower_bound;
				lower_bound = upper_bound - lower_bound;
				upper_bound = upper_bound - lower_bound;
			}
		}
	}

	sprintf(buf, "{168} %-12s {138}%-30s {128}%10s {118}%10s\n\r",
		"Author",
		"Area Name",
		"Suggested",
		"Enforced");
	strcat (buf,  "{178}-------------------------------------------------------------------------------\n\r");

	for (pArea = mud->f_area ; pArea ; pArea = pArea->next)
	{
		if (pArea->low_soft_range >= lower_bound
		&&  pArea->hi_soft_range  <= upper_bound)
		{
			one_argument_nolower(pArea->authors, mainauth);

			if (!IS_GOD(ch))
			{
				cat_sprintf(buf, "{068} %-12s {038}%-30s{028} %7d-%2d {018}%7d-%2d\n\r",
					mainauth,
					pArea->name,
					pArea->low_soft_range,
					pArea->hi_soft_range,
					pArea->low_hard_range,
					pArea->hi_hard_range);
			}
			else
			{
				cat_sprintf(buf, "{068} %-12s {038}%-30s{028} %7d-%2d {018}%7d-%2d  {068}[%5d]\n\r",
					mainauth,
					pArea->name,
					pArea->low_soft_range,
					pArea->hi_soft_range,
					pArea->low_hard_range,
					pArea->hi_hard_range,
					pArea->low_r_vnum);
			}
		}
	}
	send_to_char_color(buf, ch);
	pop_call();
	return;
}

void do_cpu( CHAR_DATA *ch, char *arg)
{
	long long tot_cpu;
	int tim;

	push_call("do_cpu(%p,%p)",ch,arg);

	send_to_char("Section                           Time (usec)    Freq (msec)  %Prog         %CPU\n\r" , ch );

	for (tot_cpu = tim = 0 ; tim < TIMER_MAXTIMER ; tim++)
	{
		tot_cpu += display_timer(ch, tim);
	}

	ch_printf(ch, "\n\rAverage IO bandwidth:       %10lld MB per month\n\r", mud->total_io_bytes * PULSE_PER_SECOND * 60 * 60 * 24 * 30 / 1000000 / UMAX(1, mud->total_io_ticks));
	ch_printf(ch, "Unknown CPU Usage:              %6.2f percent\n\r", (mud->total_io_exec - tot_cpu) * 100.0 / (mud->total_io_delay + mud->total_io_exec));
	ch_printf(ch, "Average CPU Usage:              %6.2f percent\n\r",  mud->total_io_exec            * 100.0 / (mud->total_io_delay + mud->total_io_exec));

	pop_call();
	return;
}

/*
	Returns 1000 times normal percentage value
*/

long long display_timer( CHAR_DATA *ch, int timer )
{
	long long tot_usage, ind_usage;

	push_call("display_timer(%p,%p)",ch,timer);

	tot_usage = mud->total_io_exec + mud->total_io_delay;

	if (tot_usage == 0)
	{
		pop_call();
		return(0);
	}

	if (timers[timer][1] == 0 || timers[timer][4] == 0)
	{
		pop_call();
		return(0);
	}
	ind_usage = timers[timer][0] / timers[timer][1] * timers[timer][4];

	ch_printf(ch, "%-30s%8lld       %8lld      %8.2f     %8.2f\n\r",
		timer_strings[timer],
		timers[timer][0] / timers[timer][1],
		timers[timer][3] / timers[timer][4] / 1000,
		100.0 * (double) ind_usage / (double) mud->total_io_exec,
		100.0 * (double) ind_usage / (double) tot_usage);

	pop_call();
	return ind_usage;
}


void start_timer( int timer )
{
	struct timeval last_time;
	long long cur_time;

	gettimeofday(&last_time, NULL);

	cur_time = (long long) last_time.tv_usec + 1000000LL * (long long) last_time.tv_sec;

	if (timers[timer][2] == 0)
	{
		timers[timer][2] = cur_time ;
		return;
	}

	timers[timer][3] += cur_time - timers[timer][2];
	timers[timer][2] = cur_time;
	timers[timer][4] ++;

	return;
}


void close_timer( int timer )
{
	struct timeval last_time;
	long long cur_time;

	gettimeofday(&last_time, NULL);

	cur_time = (long long) last_time.tv_usec + 1000000LL * (long long) last_time.tv_sec;

/*
	if (cur_time - timers[timer][2] > 1000000 / PULSE_PER_SECOND)
	{
		bug("%s heartbeat violation of %lld usec", timer_strings[timer], cur_time - timers[timer][2] - (1000000LL / PULSE_PER_SECOND));
	}
*/
	timers[timer][0] += (cur_time - timers[timer][2]);
	timers[timer][1] ++;

	return;
}


void number_seed( int number )
{
	srand48(number);
}

/*
	randomize number by -10 to 10 percent
*/

int number_fuzzy( int number )
{
	if (number <= 2)
	{
		return number;
	}

	return number - (1 + number / 10) + number_range(0, 2 * (1 + number / 10));
}

/*
	Like fuzzy, but only return the change which can be negative.
*/

int number_dizzy( int number )
{
	number = 1 + (number * number / number) / 10; // always positive

	return 0 - number + number_range(0, 2 * number);
}

/*
	Generate a random number.
*/

int number_range( int from, int to )
{
	int val;

	push_call("number_range(%p,%p)",from,to);

	if (from > to || from < 0 || to < 0)
	{
		log_printf("number_range: invalid range (%d,%d)", from, to);
		dump_stack();

		if (from > to)
		{
			val	= from;
			from = to;
			to	= val;
		}
	}

	pop_call();
	return (lrand48() % (to - from + 1)) + from;
}

/*
	Roll some dice.
*/

int dice( int number, int size )
{
	int idice, sum;

	push_call("dice(%p,%p)",number,size);

	if (size <= 0)
	{
		pop_call();
		return 0;
	}

	for (idice = sum = 0 ; idice < number ; idice++)
	{
		sum += (lrand48() % size) + 1;
	}
	pop_call();
	return sum;
}

/*
	Return a value between 1 and 100
*/

int number_percent( void )
{
	return ((lrand48() % 100) + 1);
}


/*
	Generate a random door.
*/

int number_door( void )
{
	return (lrand48() % 6);
}

/*
	return a value between 0 and given power of 2
*/

int number_bits( int width )
{
	return (lrand48() % (1 << width));
}


/*
	Removes the tildes from a string.
	Used for player-entered strings that go into disk files.
*/

void smash_tilde( char *str )
{
	push_call("smash_tilde(%p)",str);

	for ( ; *str != '\0' ; str++)
	{
		if (*str == '~')
		{
			*str = '-';
		}
	}
	pop_call();
	return;
}


bool str_prefix( const char *astr, const char *bstr )
{
	return strncasecmp(astr, bstr, strlen(astr));
}

/*
	Compare strings, case insensitive, for match anywhere.
	Returns TRUE is astr not part of bstr.
	(compatibility with historical functions).
*/

bool str_infix( const char *astr, const char *bstr )
{
	int sstr1;
	int sstr2;
	int ichar;
	char c0;

	push_call("str_infix(%p,%p)",astr,bstr);

	if ((c0 = LOWER(astr[0])) == '\0')
	{
		pop_call();
		return FALSE;
	}

	sstr1 = strlen(astr);
	sstr2 = strlen(bstr);

	for (ichar = 0 ; ichar <= sstr2 - sstr1 ; ichar++)
	{
		if (c0 == LOWER(bstr[ichar]) && !str_prefix(astr, bstr + ichar))
		{
			pop_call();
			return FALSE;
		}
	}
	pop_call();
	return TRUE;
}

/*
	Compare strings, case insensitive, for suffix matching.
	Return TRUE if astr not a suffix of bstr
	(compatibility with historical functions).
*/

bool str_suffix( const char *astr, const char *bstr )
{
	int sstr1;
	int sstr2;

	push_call("str_suffix(%p,%p)",astr,bstr);

	sstr1 = strlen(astr);
	sstr2 = strlen(bstr);

	if (sstr1 <= sstr2 && !strcasecmp((char *)astr, (char *) (bstr + sstr2 - sstr1)))
	{
		pop_call();
		return FALSE;
	}
	else
	{
		pop_call();
		return TRUE;
	}
}

/*
	Returns a string of the specified length, containing at most the first
	length number of characters of st, will pad if necessary
	if length is positive, string will be right justified, if length is
	negative it will left justify
*/

char *str_resize(const char *st, char *buf, int length)
{
	const char *f;
	char *t;
	int  stLen;
	bool rightJustified;

	push_call("str_resize(%p,%p,%p)",st,buf,length);

	buf[0] = '\0';
	rightJustified = (length >= 0);

	if (!rightJustified)
	{
		length =- length;
	}
	stLen = strlen(st);

	if ((!rightJustified) || (stLen >= length))
	{
		for (f = st, t = buf ; (*f != 0) && (length > 0) ; f++, t++, length--)
		{
			*t = *f;
		}
		if (length > 0)
		{
			for ( ; length > 0 ; length--, t++)
			{
				*t = ' ';
			}
		}
		*t = '\0';
	}
	else if (length != 0)
	{
		for (t = buf ; length > stLen ; t++, length--)
		{
			*t = ' ';
		}
		if (length != 0)
		{
			for (f = st ; (*f != 0) && (length > 0) ; f++, t++, length--)
			{
				*t = *f;
			}
		}
		*t = '\0';
	}
	pop_call();
	return buf;
}

/*
	Get a or an depending on input
*/

char *get_prefix( const char *argument )
{
	static char prefix[3];

	push_call("get_prefix(%p)",argument);

	switch (tolower(argument[0]))
	{
		case 'a':
		case 'e':
		case 'i':
		case 'o':
		case 'u':
			strcpy(prefix, "an");
			break;
		default:
			strcpy(prefix, "a");
			break;
	}
	pop_call();
	return prefix;
}

/*
	Return first part of an IP address
*/

char *get_domain( const char *str )
{
	static char strbuf[MAX_INPUT_LENGTH];
	int i;

	push_call("get_domain(%p)", str);

	strcpy(strbuf, str);

	for (i = 0 ; strbuf[i] != 0 ; i++)
	{
		if (strbuf[i] == '.')
		{
			break;
		}
	}

	for (i++ ; strbuf[i] != 0 ; i++)
	{
		if (strbuf[i] == '.')
		{
			break;
		}
	}

	strbuf[i] = 0;

	pop_call();
	return strbuf;
}

/*
	Convert a short description into a name - Scandum
*/

char *short_to_name( const char *str, int amount )
{
	static char strstn[MAX_INPUT_LENGTH];
	char strbuf[MAX_INPUT_LENGTH];
	char *strpt;

	push_call("short_to_name(%p)", str);

	if (str == NULL)
	{
		pop_call();
		return amount == 1 ? "trash" : "pieces of trash";
	}

	if (!str_prefix("an ", str))
	{
		strcpy(strstn, &str[3]);
	}
	else if (!str_prefix("a ", str))
	{
		strcpy(strstn, &str[2]);
	}
	else if (!str_prefix("the ", str))
	{
		strcpy(strstn, &str[4]);
	}
	else if (!str_prefix("some ", str))
	{
		strcpy(strstn, &str[5]);
	}
	else
	{
		strcpy(strstn, str);
	}

	if (amount == 1)
	{
		pop_call();
		return strstn;
	}

	if ((strpt = strstr(strstn, " of ")) != NULL)
	{
		strcpy(strbuf, strpt);
		*strpt = '\0';
	}

	if (!str_suffix("ife", strstn))
	{
		strstn[strlen(strstn)-2] = 'v';
	}

	if (!str_suffix("ff", strstn))
	{
		strstn[strlen(strstn)-2] = 'v';
		strstn[strlen(strstn)-1] = 'e';
	}

	if (!str_suffix("f", strstn))
	{
		strstn[strlen(strstn)-1] = 'v';

		strcat(strstn, "e");
	}

	if (!str_suffix("ch", strstn))
	{
		strcat(strstn, "es");
	}

	if (!str_suffix("x", strstn))
	{
		strcat(strstn, "es");
	}

	if (str_suffix("s", strstn))
	{
		strcat(strstn, "s");
	}

	if (strpt)
	{
		strcat(strstn, strbuf);
	}

	pop_call();
	return strstn;
}


/*
	Turn given integer into a time string - Scandum
*/

char *get_time_string( time_t time )
{
	char *time_string;

	push_call("get_time_string(%p)",time);

	time_string = ctime( (const time_t *) &time);

	time_string[strlen(time_string)-1] = '\0';

	pop_call();
	return time_string;
}

/*
	Add th,st,nd,rd to numbers - Scandum
*/

char *numbersuf( int number )
{
	static char time[100];

	push_call("numbersuf(%d)",number);

	if (number > 4 && number <  20)
	{
		sprintf(time, "%dth", number);
	}
	else if (number % 10 ==  1)
	{
		sprintf(time, "%dst", number);
	}
	else if (number % 10 ==  2)
	{
		sprintf(time, "%dnd", number);
	}
	else if (number % 10 ==  3)
	{
		sprintf(time, "%drd", number);
	}
	else
	{
		sprintf(time, "%dth", number);
	}
	pop_call();
	return time;
}

/*
	Turns military time into civilian time - Scandum
*/

char *tocivilian( int hour )
{
	static char time[10];

	push_call("tocivilian(%d)",hour);

	sprintf(time, "%d %s", hour % 12 != 0 ? hour % 12 : 12, hour >= 12 ? "pm" : "am");

	pop_call();
	return time;
}

/*
	Turns an integer into a money string - Scandum
*/

char *tomoney( int money )
{
	static char strcap[25];
	static char strold[25];
	int cnt1, cnt2, cnt3;

	push_call("tomoney(%p)", money);

	sprintf(strold, "%d", money);

	cnt1 = strlen(strold);
	cnt2 = strlen(strold) + (strlen(strold) - 1 - (money < 0)) / 3;

	for (cnt3 = 0 ; cnt2 >= 0 ; cnt1--, cnt2--, cnt3++)
	{
		strcap[cnt2] = strold[cnt1];

		if (cnt3 && cnt3 % 3 == 0)
		{
			strcap[--cnt2] = ',';
		}
	}
	if (money < 0)
	{
		strcap[0] = '-';
	}
	pop_call();
	return strcap;
}

char *capitalize_all( const char *str )
{
	static char strcap[MAX_STRING_LENGTH];
	char *pti, *pto;

	push_call("capitalize(%p)",str);

	if (str == NULL)
	{
		*strcap = '\0';
		pop_call();
		return strcap;
	}
	pti = (char *)str;
	pto = (char *)strcap;
	for ( ; *pti != '\0' ; pti++, pto++)
	{
		*pto = UPPER(*pti);
	}
	*pto = '\0';
	pop_call();
	return strcap;
}

char *lower_all( const char *str )
{
	static char strcap[MAX_STRING_LENGTH];
	char *pti, *pto;

	push_call("capitalize(%p)",str);

	if (str == NULL)
	{
		*strcap = '\0';
		pop_call();
		return strcap;
	}
	pti = (char *)str;
	pto = (char *)strcap;
	for ( ; *pti != '\0' ; pti++, pto++)
	{
		*pto = LOWER(*pti);
	}
	*pto = '\0';
	pop_call();
	return strcap;
}

/*
	Returns an initial-capped string.
	!!!!!  Do not use more than one capitalize in a sprintf or other
	similar command.  Only the last one is read for all values,
	since the static variables are all set before the sprintf puts
	everything together.         Chaos 12/26/94
*/


char *capitalize( const char *str )
{
	static char strcap[2][MAX_STRING_LENGTH];
	char *pti, *pto;
	static int cnt;
	int x;

	push_call("capitalize(%p)",str);

	cnt = (cnt + 1) % 2;

	pti = (char *)str;
	pto = (char *)strcap[cnt];

	while (*pti)
	{
		*pto++ = *pti++;
	}
	*pto = '\0';

	for (x = 0 ; strcap[cnt][x] != '\0' ; x++)
	{
		if (isalpha(strcap[cnt][x]))
		{
			strcap[cnt][x] = UPPER(strcap[cnt][x]);
			break;
		}
	}
	pop_call();
	return strcap[cnt];
}

char *capitalize_name( const char *str )
{
	static char strcap[MAX_STRING_LENGTH];
	char *pti, *pto;

	push_call("capitalize_name(%p)",str);

	if (str == NULL)
	{
		*strcap = '\0';
		pop_call();
		return strcap;
	}
	pti=(char *)str;
	pto=(char *)strcap;
	for ( ; *pti != '\0'; pti++, pto++ )
	{
		*pto=LOWER(*pti);
	}
	*pto = '\0';
	*strcap = UPPER(*strcap);
	pop_call();
	return strcap;
}

/*
	Append a string to a file.
*/

void append_file( CHAR_DATA *ch, char *fname, FILE *fp, char *str )
{
	push_call("append_file(%p,%p,%p,%p)",ch,fname,fp,str);

	if ((ch != NULL && IS_NPC(ch)) || str == NULL || str[0] == '\0')
	{
		pop_call();
		return;
	}

	if (fpAppend)
	{
		my_fclose( fpAppend );
	}

	if (fp == NULL)
	{
		fp = my_fopen( fname, "a",FALSE);
	}

	if (fp == NULL)
	{
		perror( fname );
		fpAppend = my_fopen( NULL_FILE, "r",FALSE);
		pop_call();
		return;
	}

	if (ch == NULL)
	{
		fprintf( fp, "%s\n", str );
		fflush( fp );
		fpAppend = my_fopen( NULL_FILE, "r",FALSE);
		pop_call();
		return;
	}

	fprintf( fp, "[%5u] %s: %s\n", ch->in_room ? ch->in_room->vnum : 0, ch->name, str );

	fflush( fp );

	fpAppend = my_fopen( NULL_FILE, "r",FALSE);

	pop_call();
	return;
}

/*
	Reports a bug.        -1 param does not put in logs.
*/

void bug( const char *str, ... )
{
	char buf[MAX_STRING_LENGTH];

	push_call("bug(%p)",str);

	if (fpArea != NULL)
	{
		int iLine;
		int iChar;

		if (fpArea == stdin)
		{
			iLine = 0;
		}
		else
		{
			iChar = ftell(fpArea);
			fseek(fpArea, 0, 0);

			for (iLine = 0 ; ftell(fpArea) < iChar ; iLine++)
			{
				while (getc(fpArea) != '\n')
				{
					;
				}
			}
			fseek(fpArea, iChar, 0);
		}
		log_printf("[*****] FILE: %s LINE: %d", strArea, iLine);
	}

	strcpy(buf, "[+++++] ");
	{
		va_list param;

		va_start(param, str);
		vsprintf( buf + strlen(buf), str, param );
		va_end(param);
	}
	log_string( buf );
	log_printf("[+++++] %s", mud->last_player_cmd);

	pop_call();
	return;
}

/*
	Writes a string to the log.
*/

void log_string( char *str )
{
	char *strtime;
	struct timeval log_time;
	CHAR_DATA  *fch;
	PLAYER_GAME *fpl;

	push_call("log_string(%p)",str);

	for (fpl = mud->f_player ; fpl ; fpl = fpl->next)
	{
		fch = fpl->ch;
		if (IS_NPC(fch))
		{
			continue;
		}
		if (!IS_SET(fch->act, PLR_HEARLOG) || !IS_IMMORTAL(fch))
		{
			continue;
		}
		ch_printf_color(fch, "{138}Log: %s\n\r", justify(str, get_page_width(fch)));
	}

	gettimeofday(&log_time, NULL);
	strtime                    = ctime(&log_time.tv_sec);
	strtime[strlen(strtime)-6] = '\0';
	fprintf( stderr, "%s- %s\n", strtime, str );

	pop_call();
	return;
}

void log_build_string(int vnum, char *str )
{
	struct timeval log_time;
	char *strtime;
	CHAR_DATA  *fch;
	PLAYER_GAME *fpl;

	push_call("log_build_string(%p)",str);

	for (fpl = mud->f_player ; fpl ; fpl = fpl->next)
	{
		fch = fpl->ch;

		if (!IS_SET(fpl->ch->act, PLR_HEARLOG) || !can_olc_modify(fpl->ch, vnum))
		{
			continue;
		}
		ch_printf_color(fch, "{138}Log: [%u] %s\n\r", vnum, str);
	}
	gettimeofday(&log_time, NULL);
	strtime                    = ctime(&log_time.tv_sec);
	strtime[strlen(strtime)-6] = '\0';
	fprintf( stderr, "%s- [%u] %s\n", strtime, vnum, str );

	pop_call();
	return;
}

void log_god_string( char *str )
{
	char *strtime;
	struct timeval log_time;
	CHAR_DATA  *fch;
	PLAYER_GAME *fpl;

	push_call("log_god_string(%p)",str);

	for (fpl = mud->f_player ; fpl ; fpl = fpl->next)
	{
		fch = fpl->ch;
		if (IS_NPC(fch))
		{
			continue;
		}
		if (!IS_SET(fch->act, PLR_HEARLOG) || !IS_GOD(fch))
		{
			continue;
		}
		ch_printf_color(fch, "{138}Log: %s\n\r", justify(str, get_page_width(fch)));
	}
	gettimeofday(&log_time, NULL);
	strtime                    = ctime(&log_time.tv_sec);
	strtime[strlen(strtime)-6] = '\0';
	fprintf( stderr, "%s- %s\n", strtime, str );

	pop_call();
	return;
}

/*
	This procedure is responsible for reading any in_file MOBprograms.
*/

void mprog_read_programs( FILE *fp, MOB_INDEX_DATA *pMobIndex)
{
	MUD_PROG *mprg;
	bool        done = FALSE;

	push_call("mprog_read_programs(%p,%p)",fp,pMobIndex);

	if (fread_letter(fp) != '>')
	{
		bug("[%d] mprog_read_programs: did not find '>'", pMobIndex->vnum);
		abort( );
	}

	ALLOCMEM(mprg, MUD_PROG, 1);

	while (!done)
	{
		mprg->type = mprog_name_to_type(fread_word(fp));

		switch ( mprg->type )
		{
			case ERROR_PROG:
				bug("[%d] mprog_read_programs: ERROR_PROG.", pMobIndex->vnum);
				abort( );
				break;
			default:
				SET_BIT(pMobIndex->progtypes, mprg->type);

				mprg->arglist = fread_string(fp);
				fread_to_eol(fp);

				switch (mprg->type)
				{
					case SOCIAL_PROG:
						if (find_social(mprg->arglist) == -1)
						{
							log_printf("[%d] mprog_read_programs: unknown social: %s", pMobIndex->vnum, mprg->arglist);
						}
						break;

					case ACT_PROG:
					case EXIT_PROG:
					case GIVE_PROG:
					case SPEECH_PROG:
					case TRIGGER_PROG:
						break;

					default:
						if (!is_number(mprg->arglist))
						{
							log_printf("[%d] mprog_read_programs: invalid arglist: %s", pMobIndex->vnum, mprg->arglist);
						}
						break;
				}

				mprg->comlist = fread_string(fp);
				fread_to_eol(fp);

				if (strstr(mprg->comlist, "$r") != NULL)
				{
					SET_BIT(mprg->flags, MPTRIGGER_RAND_PLR);
				}
				expand_mud_prog( pMobIndex->vnum, mprg );  /* Tokenize */

				LINK(mprg, pMobIndex->first_prog, pMobIndex->last_prog, next, prev);

				switch (fread_letter( fp ) )
				{
					case '>':
						ALLOCMEM(mprg, MUD_PROG, 1);
						break;
					case '|':
						fread_to_eol( fp );
						done = TRUE;
						break;
					default:
						bug( "Load_mobiles: vnum %u bad MOBPROG.", pMobIndex->vnum );
						abort();
						break;
				}
				break;
		}
	}
	pop_call();
	return;
}

/*
	This procedure is responsible for reading any in_file OBJprograms.
*/

void oprog_read_programs( FILE *fp, OBJ_INDEX_DATA *pObjIndex)
{
	MUD_PROG *mprg;
	bool        done = FALSE;

	push_call("oprog_read_programs(%p,%p)",fp,pObjIndex);

	if (fread_letter(fp) != '>')
	{
		bug("load_objects: vnum %d OBJPROG char", pObjIndex->vnum);
		abort( );
	}

	ALLOCMEM(mprg, MUD_PROG, 1);

	while (!done)
	{
		mprg->type = oprog_name_to_type(fread_word(fp));

		switch ( mprg->type )
		{
			case ERROR_PROG:
				bug("load_objects: vnum %u ERROR_PROG type.", pObjIndex->vnum);
				abort( );
				break;
			default:
				SET_BIT(pObjIndex->progtypes, mprg->type);

				mprg->arglist		= fread_string( fp );
				fread_to_eol( fp );

				mprg->comlist		= fread_string( fp );
				fread_to_eol( fp );

				if (strstr(mprg->comlist, "$r") != NULL)
				{
					SET_BIT(mprg->flags, MPTRIGGER_RAND_PLR);
				}
				expand_mud_prog( pObjIndex->vnum, mprg );  /* Tokenize */

				LINK(mprg, pObjIndex->first_prog, pObjIndex->last_prog, next, prev);

				switch (fread_letter( fp ) )
				{
					case '>':
						ALLOCMEM(mprg, MUD_PROG, 1);
						break;
					case '|':
						fread_to_eol( fp );
						done = TRUE;
						break;
					default:
						bug( "Load_objects: vnum %u bad MUDPROG.", pObjIndex->vnum );
						abort();
						break;
				}
				break;
		}
	}
	pop_call();
	return;
}

void rprog_read_programs( FILE *fp, ROOM_INDEX_DATA *pRoomIndex)
{
	MUD_PROG *mprg;
	bool done = FALSE;

	push_call("rprog_read_programs(%p,%p)",fp,pRoomIndex);

	if (fread_letter(fp) != '>')
	{
		bug("load_rooms: vnum %d MUDPROG char", pRoomIndex->vnum);
		abort( );
	}

	ALLOCMEM(mprg, MUD_PROG, 1);

	while (done == FALSE)
	{
		mprg->type = rprog_name_to_type(fread_word(fp));

		switch ( mprg->type )
		{
			case ERROR_PROG:
				bug("load_objects: vnum %u ERROR_PROG type.", pRoomIndex->vnum);
				abort( );
				break;
			default:
				SET_BIT(pRoomIndex->progtypes, mprg->type);

				mprg->arglist = fread_string( fp );
				fread_to_eol( fp );

				mprg->comlist = fread_string( fp );
				fread_to_eol( fp );

				if (strstr(mprg->comlist, "$r") != NULL)
				{
					SET_BIT(mprg->flags, MPTRIGGER_RAND_PLR);
				}
				expand_mud_prog( pRoomIndex->vnum, mprg );  /* Tokenize */

				LINK(mprg, pRoomIndex->first_prog, pRoomIndex->last_prog, next, prev);

				switch (fread_letter( fp ) )
				{
					case '>':
						ALLOCMEM(mprg, MUD_PROG, 1);
						break;
					case '|':
						fread_to_eol( fp );
						done = TRUE;
						break;
					default:
						bug( "Load_rooms: vnum %u bad MUDPROG.", pRoomIndex->vnum );
						abort();
						break;
				}
				break;
		}
	}
	pop_call();
	return;
}

void create_menu_tree()
{
	char *pt;
	int cnt;
	char line[200];
	AREA_DATA *pArea, *tArea;
	HELP_DATA *pHelp, *tHelp;
	HELP_MENU_DATA *menu;
	char buf[MAX_STRING_LENGTH];

	push_call("create_menu_tree()");

	for (pArea = mud->f_area ; pArea ; pArea = pArea->next)
	{
		for (pHelp = pArea->first_help ; pHelp ; pHelp = pHelp->next)
		{
			strcpy(buf, pHelp->text);
			pt = buf;
			pt += strlen(buf) / 4;
			while (*pt != '\0')
			{
				if (*(pt+1) != '{' || *(pt+2) == '\0' || *(pt+3) != '}')
				{
					pt++;
					continue;
				}
				pt++;
				*pt = '\0';

				if (pHelp->first_menu == NULL)
				{
					STRFREE(pHelp->text);
					pHelp->text = STRALLOC(buf);
				}

				*pt = '{';
				/*
					find menu items here
				*/

				cnt = 0;
				while (*pt != '\r' && *pt != '\n' && *pt != '\0')
				{
					line[cnt] = *pt;
					cnt++;
					pt++;
				}
				line[cnt] = '\0';

				for (tHelp = pHelp->area->first_help ; tHelp ; tHelp = tHelp->next)
				{
					if (line[3] == tHelp->keyword[0] && line[4] == tHelp->keyword[1] && !strcmp(&line[3], tHelp->keyword))
					{
						ALLOCMEM(menu, HELP_MENU_DATA, 1);
						menu->option = tolower(line[1]);
						menu->help   = tHelp;
						LINK(menu, pHelp->first_menu, pHelp->last_menu, next, prev);
						goto end_of_loop;
					}
				}
				/*
				log_printf("create_menu: inter area help menu: %s", line);
				*/
				for (tArea = mud->f_area ; tArea ; tArea = tArea->next)
				{
					for (tHelp = tArea->first_help ; tHelp ; tHelp = tHelp->next)
					{
						if (line[3] == tHelp->keyword[0] && line[4] == tHelp->keyword[1] && !strcmp(&line[3], tHelp->keyword))
						{
							ALLOCMEM(menu, HELP_MENU_DATA, 1);
							menu->option = tolower(line[1]);
							menu->help   = tHelp;
							LINK(menu, pHelp->first_menu, pHelp->last_menu, next, prev);
							goto end_of_loop;
						}
					}
				}

				log_printf("create_menu: failed perfect match: %s", line);

				for (tArea = mud->f_area ; tArea ; tArea = tArea->next)
				{
					for (tHelp = tArea->first_help ; tHelp ; tHelp = tHelp->next)
					{
						if (is_name(&line[3], tHelp->keyword))
						{
							ALLOCMEM(menu, HELP_MENU_DATA, 1);
							menu->option = tolower(line[1]);
							menu->help   = tHelp;
							LINK(menu, pHelp->first_menu, pHelp->last_menu, next, prev);
							goto end_of_loop;
						}
					}
				}
				end_of_loop:
				continue;
			}
		}
	}
	pop_call();
	return;
}


int find_command( char *cmd_str, int trust )
{
	int cmd;

	push_call("find_command(%p)",cmd_str);

	if (isalpha(*cmd_str))
	{
		for (cmd = mud->command_ref[tolower(*cmd_str) - 'a'] ; *cmd_table[cmd].name == *cmd_str ; cmd++)
		{
			if (cmd_table[cmd].level <= trust && !str_prefix(cmd_str, cmd_table[cmd].name))
			{
				pop_call();
				return cmd;
			}
		}
	}
	else
	{
		for (cmd = 0 ; !isalpha(*cmd_table[cmd].name) ; cmd++)
		{
			if (cmd_table[cmd].level <= trust && !str_prefix(cmd_str, cmd_table[cmd].name))
			{
				pop_call();
				return cmd;
			}
		}
	}
	pop_call();
	return -1;
}

int find_social( char *social_str )
{
	int cmd;

	push_call("find_social(%p)",social_str);

	if (isalpha(*social_str))
	{
		for (cmd = mud->social_ref[tolower(*social_str) - 'a'] ; *social_table[cmd].name == *social_str ; cmd++)
		{
			if (!str_prefix(social_str, social_table[cmd].name))
			{
				pop_call();
				return cmd;
			}
		}
	}
	pop_call();
	return -1;
}

		
OBJ_PROG * load_object_program( FILE *fp)
{
	OBJ_PROG *prg;
	char buf[MAX_INPUT_LENGTH];

	push_call("load_object_program(%p)",fp);

	ALLOCMEM(prg, OBJ_PROG, 1);

	prg->unknown   = STRDUPE(str_empty);
	prg->if_symbol = '=';
	prg->argument  = STRDUPE(str_empty);

	prg->index	= fread_number( fp );	/* get index number */
	prg->trigger	= fread_number( fp );	/* get trigger command type */

	mud->top_oprog++;

	switch (prg->trigger)
	{
		case TRIG_VOID:
			break;

		case TRIG_COMMAND:
		case TRIG_ROOM_COMMAND:
		case TRIG_WEAR_COMMAND:
			prg->percentage = fread_number( fp );
			if ((prg->cmd = find_command(fread_word(fp), MAX_LEVEL - 1)) == -1)
			{
				bug("Bad obj_command command name: %s", buf);
				abort();
			}
			break;

		case TRIG_UNKNOWN:
		case TRIG_ROOM_UNKNOWN:
		case TRIG_WEAR_UNKNOWN:
			prg->percentage = fread_number( fp );
			STRFREE(prg->unknown);
			prg->unknown = STRALLOC(fread_word( fp )) ;
			break;

		case TRIG_TICK:
		case TRIG_DAMAGE:
		case TRIG_HIT:
		case TRIG_WEAR:
		case TRIG_REMOVE:
		case TRIG_SACRIFICE:
			prg->percentage = fread_number( fp );
			break;

		default:
			log_string( "Bad obj_command type");
	}

	prg->obj_command = fread_number( fp );

	switch (prg->obj_command)
	{
		case OPROG_ECHO:
		case OPROG_GOD_COMMAND:
		case OPROG_GOD_ARGUMENT:
		case OPROG_COMMAND:
		case OPROG_ARGUMENT:
			STRFREE(prg->argument);
			prg->argument		= fread_string( fp ) ;
			break;
		case OPROG_QUEST_SET:
			prg->quest_offset	= fread_number( fp );
			prg->quest_bits	= fread_number( fp );
			prg->if_value		= fread_number( fp );
			break;
		case OPROG_QUEST_ADD:
			prg->quest_offset	= fread_number( fp );
			prg->quest_bits	= fread_number( fp );
			prg->if_value		= fread_number( fp );
			break;
		case OPROG_PLAYER_QUEST_IF:
			prg->quest_offset	= fread_number( fp );
			prg->quest_bits	= fread_number( fp );
			prg->if_symbol		= fread_letter( fp );
			prg->if_value		= fread_number( fp );
			prg->if_true		= fread_number( fp );
			prg->if_false		= fread_number( fp );
			break;
		case OPROG_OBJECT_QUEST_IF:
			prg->quest_offset	= fread_number( fp );
			prg->quest_bits	= fread_number( fp );
			prg->if_symbol		= fread_letter( fp );
			prg->if_value		= fread_number( fp );
			prg->if_true		= fread_number( fp );
			prg->if_false		= fread_number( fp );
			break;
		case OPROG_IF_HAS_OBJECT:
			prg->if_value		= fread_number( fp );
			prg->if_true		= fread_number( fp );
			prg->if_false		= fread_number( fp );
			break;
		case OPROG_IF:
			prg->if_check		= fread_number( fp );
			prg->if_symbol		= fread_letter( fp );
			prg->if_value		= fread_number( fp );
			prg->if_true		= fread_number( fp );
			prg->if_false		= fread_number( fp );
			break;
		case OPROG_APPLY:
			prg->if_check		= fread_number( fp );
			prg->if_value		= fread_number( fp );
			break;
		case OPROG_JUNK:
			break;
		default:
			log_string( "Bad obj_command reaction type");
	}
	pop_call();
	return( prg );
}

void obj_prog_if_dest( OBJ_INDEX_DATA *obj )
{
	OBJ_PROG *prg, *dest;

	push_call("obj_prog_if_dest(%p)",obj);

	for (prg = obj->first_oprog ; prg ; prg = prg->next)
	{
		prg->true		= NULL;
		prg->false	= NULL;

		switch(prg->obj_command)
		{
			case OPROG_IF_HAS_OBJECT:
			case OPROG_IF:
			case OPROG_OBJECT_QUEST_IF:
			case OPROG_PLAYER_QUEST_IF:
				for (dest = prg->next ; prg->true == NULL && dest != NULL ; dest = dest->next)
				{
					if (prg->if_true == dest->index)
					{
						prg->true = dest;
					}
				}

				for (dest = prg->next ; prg->false == NULL && dest != NULL ; dest = dest->next)
				{
					if (prg->if_false == dest->index)
					{
						prg->false = dest;
					}
				}
				break;
			default:
				for (dest = prg->next ; prg->true == NULL && dest != NULL ; dest = dest->next)
				{
					if (prg->index == dest->index)
					{
						prg->true = dest;
					}
				}
				break;
		}
	}
	pop_call();
	return;
}


/*
	Returns a lowercase string.
*/

char *strlower( const char *str )
{
	static char strlow[MAX_STRING_LENGTH];
	int i;

	push_call("strlower(%p)",str);

	for (i = 0 ; str[i] != '\0' ; i++)
	{
		strlow[i] = tolower(str[i]);
	}
	strlow[i] = '\0';

	pop_call();
	return strlow;
}

/*
	Returns an uppercase string.
*/

char *strupper( const char *str )
{
	static char strup[MAX_STRING_LENGTH];
	int i;

	push_call("strupper(%p)",str);

	for (i = 0 ; str[i] != '\0' ; i++)
	{
		strup[i] = toupper(str[i]);
	}
	strup[i] = '\0';

	pop_call();
	return strup;
}

int str_cat_max( const char *out_str, const char *add_str, int max_size )
{
	register char *str_pt1, *str_pt2;
	register int str_actual, str_size;

	push_call("str_cat_max(%p,%p,%p)",out_str,add_str,max_size);

	/*
		Set limits of size, and subtract for the terminator
	*/
	str_actual = UMIN(UMAX(1, max_size ), MAX_STRING_LENGTH) - 1;

	for (str_size = 0, str_pt1 = (char *)out_str ; *str_pt1 != '\0' ; str_pt1++, str_size++)
	{
		if (str_size == str_actual)
		{
			*str_pt1 = '\0';
			pop_call();
			return( str_size );
		}
	}
	for (str_pt2 = (char *)add_str ; *str_pt2 != '\0' ; str_pt1++, str_pt2++, str_size++)
	{
		if (str_size == str_actual)
		{
			*str_pt1= '\0';
			pop_call();
			return( str_size );
		}
		else
		{
			*str_pt1=*str_pt2;
		}
	}
	*str_pt1='\0';
	pop_call();
	return( str_size );
}


int str_apd_max( const char *out_str, const char *add_str, int start, int max_size )
{
	register char *str_pt1, *str_pt2;
	register int str_actual, str_size;

	push_call("str_apd_max(%p,%p,%d,%d)",out_str,add_str,start,max_size);

	if (out_str == NULL)
	{
		log_printf("str_apd_max(): out_str = NULL.");
		pop_call();
		return 0;
	}

	str_actual = URANGE(1, max_size - 1, MAX_STRING_LENGTH - 1);

	if (str_actual <= start)
	{
		*((char *) out_str + str_actual) = '\0';

		pop_call();
		return( str_actual );
	}

	str_pt1	= (char *) (out_str + start);
	str_size	= start;

	for (str_pt2 = (char *) add_str ; *str_pt2 != '\0' ; str_pt1++, str_pt2++, str_size++)
	{
		if (str_size == str_actual)
		{
			break;
		}
		else
		{
			*str_pt1 = *str_pt2;
		}
	}
	*str_pt1 = '\0';

	pop_call();
	return( str_size );
}

int str_cpy_max( const char *out_str, const char *add_str, int max_size )
{
	register char *str_pt1, *str_pt2;
	register int str_actual, str_size;

	push_call("str_cpy_max(%p,%p,%p)",out_str,add_str,max_size);

	/*
		Set limits of size, and subtract for the terminator
	*/

	str_actual = URANGE(0, max_size, MAX_STRING_LENGTH-1);

	str_pt1 = (char *)out_str;
	str_pt2 = (char *)add_str;

	for (str_size = 0 ; *str_pt2 != '\0' ; str_pt1++, str_pt2++, str_size++)
	{
		if (str_size == str_actual)
		{
			*str_pt1 = '\0';
			pop_call();
			return( str_size );
		}
		else
		{
			*str_pt1 = *str_pt2;
		}
	}
	*str_pt1 = '\0';

	pop_call();
	return( str_size );
}

void expand_mud_prog( int vnum, MUD_PROG *mprog )
{
	char buf[MAX_INPUT_LENGTH];
	char *pti, *pto;
	char level;

	push_call("expand_mud_prog(%d,%p)",vnum,mprog);

	mud->top_mtrig++;

	pti = mprog->comlist;
	level = 0;

	while (*pti != '\0')
	{
		while (*pti == '\n' || *pti == '\r')
		{
			pti++;
		}
		if (*pti != '\0')
		{
			for (pto = buf ; *pti != '\r' && *pti != '\n' && *pti != '\0' ; pti++, pto++)
			{
				*pto = *pti;
			}
			*pto = '\0';
			level = expand_line_mprog( vnum, mprog, buf, FALSE, level );
		}
	}

	/*
		fixer_mprog will rebuild the old comlist - Scandum
	*/
	STRFREE(mprog->comlist );
	pop_call();
	return;
}

char expand_line_mprog( int vnum, MUD_PROG *mprog, char *line, bool iForce, char level )
{
	char *pti, *pto;
	char buf[MAX_INPUT_LENGTH];
	char string[MAX_INPUT_LENGTH];
	int type;
//	void *func;
	int value;
	MP_TOKEN *token;
	char achange, bchange;

	push_call("expand_line_mprog(%d,%p,%p,%p,%p)",vnum,mprog,line,iForce,level);

//	func = NULL;

	for (pti = line ; *pti == ' ' ; pti++);

	for (pto = buf ; isalnum(*pti) ; pti++, pto++)
	{
		*pto = *pti;
	}
	*pto = '\0';

	if (*buf == '\0')
	{
		pop_call();
		return(level);
	}
	type		= 0;   /* Error type */
	value	= 0;
	achange	= 0;
	bchange	= 0;

	switch (buf[0])
	{
		case 'a':
			if (!strcmp(buf, "and"))
			{
				type    = MPTOKEN_AND;
				bchange	= -1;
				achange	= +1;
			}
			break;
		case 'b':
			if (!strcmp(buf, "break"))
			{
				type    = MPTOKEN_BREAK;
			}
			break;
		case 'c':
			if (!strcmp(buf, "case"))
			{
				type = MPTOKEN_CASE;
				bchange	= -1;
				achange	= +1;
			}
			break;
		case 'd':
			if (!strcmp(buf, "default"))
			{	
				type    = MPTOKEN_DEFAULT;
				bchange	= -1;
				achange	= +1;
			}
			break;
		case 'e':
			if (!strcmp(buf, "else"))
			{
				type    = MPTOKEN_ELSE;
				bchange	= -1;
				achange	= +1;
			}
			if (!strcmp(buf, "elseif"))
			{
				type    = MPTOKEN_ELSEIF;
				bchange = -1;
				achange = +1;
			}
			else if (!strcmp(buf, "endif"))
			{
				type    = MPTOKEN_ENDIF;
				bchange	= -1;
			}
			else if (!strcmp(buf, "endswitch"))
			{
				type    = MPTOKEN_ENDSWITCH;
				bchange	= -2;
			}
			break;
		case 'i':
			if (!strcmp(buf, "if"))
			{
				type    = MPTOKEN_IF;
				achange	= +1;
			}
			break;
		case 'o':
			if (!strcmp(buf, "or"))
			{
				type    = MPTOKEN_OR;
				bchange	= -1;
				achange	= +1;
			}
			break;
		case 's':
			if (!strcmp(buf, "switch"))
			{
				type    = MPTOKEN_SWITCH;
				achange	= 2;
			}
			break;
	}

	if (type == 0)
	{
		if ((value = find_command(buf, MAX_LEVEL - 1)) >= 0)
		{
			type = MPTOKEN_COMMAND;
		}
		else if ((value = find_social(buf)) >= 0)
		{
			type = MPTOKEN_SOCIAL;
		}
		else
		{
			log_build_printf(vnum, "expand_line_mprog: unknown command: %s", buf);
			value = 0;
			type  = 0;
		}
	}

	ALLOCMEM(token, MP_TOKEN, 1);

	token->type     = type;
	token->value    = value;

	mud->top_mprog++;

	LINK(token, mprog->first_token, mprog->last_token, next, prev);

	level += bchange;      /* before change */

	if (level < 0)
	{
		level = 0;
	}
	token->level = level;

	level += achange;      /* after change */

	while (*pti == ' ')
	{
		pti++;
	}

	switch (type)
	{
		case MPTOKEN_IF:
		case MPTOKEN_AND:
		case MPTOKEN_OR:
		case MPTOKEN_ELSEIF:
		case MPTOKEN_SWITCH:
			pto = string;

			while (*pti != '(')
			{
				if (*pti == '\0')
				{
					log_build_printf(vnum, "ifchck syntax error");

					token->type = 0;
					break;
				}
				else if (*pti == ' ')
				{
					pti++;
				}
				else if (*pti == '!')
				{
					*pto++ = *pti++;
					SET_BIT(token->value, TOKENFLAG_NOT);
				}
				else
				{
					*pto++ = *pti++;
				}
			}
			*pto++ = ' ';

			while (*pti != ')')
			{
				if (*pti == '\0')
				{
					log_build_printf(vnum, "ifchck syntax error");

					token->type = 0;
					break;
				}
				else if (*pti == ' ')
				{
					pti++;
				}
				else
				{
					*pto++ = *pti++;
				}
			}
			*pto++ = *pti++;

			while (*pti == ' ')
			{
				pti++;
			}

			if (*pti == '\0')
			{
				*pto = '\0';
				token->string = STRALLOC(string);
				break;
			}

			*pto++ = ' ';

			while (*pti != ' ' && !isalnum(*pti))
			{
				if (*pti == '\0')
				{
					log_build_printf(vnum, "ifchck operator without value");
					token->string = STRDUPE(str_empty);

					pop_call();
					return level;
				}
				else
				{
					*pto++ = *pti++;
				}
			}

			while (*pti == ' ')
			{
				pti++;
			}

			*pto++ = ' ';

			while (*pti != ' ' && *pti != '\0')
			{
				*pto++ = *pti++;
			}
			*pto = '\0';

			token->string = STRALLOC(string);
			break;

		default:
			for (pto = string ; *pti != '\0' && *pti != '\r' && *pti != '\n' ; pti++, pto++)
			{
				*pto = *pti;
			}
			*pto = '\0';

			token->string = STRALLOC(string);
	}
	pop_call();
	return(level);
}

/*
	creation and deletion for OLC - Scandum 22-06-2002
*/


void create_shop( MOB_INDEX_DATA *mob )
{
	SHOP_DATA *pShop;
	int iTrade;

	push_call("create_shop(%p)",mob);

	ALLOCMEM(pShop, SHOP_DATA, 1);

	pShop->keeper = mob->vnum;

	for (iTrade = 0 ; iTrade < MAX_TRADE ; iTrade++)
	{
		pShop->buy_type[iTrade] = ITEM_NOTHING;
	}

	pShop->profit_buy	= 100;
	pShop->profit_sell	=  50;
	pShop->open_hour	=   0;
	pShop->close_hour	=  23;
	mob->pShop		= pShop;

	LINK(pShop, mud->f_shop, mud->l_shop, next, prev);

	mud->top_shop++;

	pop_call();
	return;
}


void delete_shop( MOB_INDEX_DATA *mob )
{
	push_call("delete_shop(%p)",mob);

	if (mob->pShop)
	{
		UNLINK(mob->pShop, mud->f_shop, mud->l_shop, next, prev);
		FREEMEM(mob->pShop);
		mob->pShop = NULL;
		--mud->top_shop;
	}
	pop_call();
	return;
}


void create_exit( ROOM_INDEX_DATA *room, int door)
{
	EXIT_DATA *pexit;

	push_call("create_exit(%p,%d)",room,door);

	if (room->exit[door] != NULL)
	{
		pop_call();
		return;
	}

	ALLOCMEM(pexit, EXIT_DATA, 1);

	pexit->description		= STRDUPE(str_empty);
	pexit->keyword			= STRDUPE(str_empty);
	pexit->vnum			= -1;
	pexit->key			= -1;

	room->exit[door]		= pexit;

	mud->top_exit++;

	pop_call();
	return;
}


void delete_exit( ROOM_INDEX_DATA *room, int door)
{
	EXIT_DATA *pexit;

	push_call("delete_exit(%p,%d)",room,door);

	if ((pexit = room->exit[door]) == NULL)
	{
		pop_call();
		return;
	}

	STRFREE(pexit->keyword);
	STRFREE(pexit->description);
	FREEMEM(pexit);
	mud->top_exit--;

	room->exit[door] = NULL;

	pop_call();
	return;
}


ROOM_INDEX_DATA *create_room( int vnum )
{
	ROOM_INDEX_DATA *room;

	push_call("create_room(%d)",vnum);

	ALLOCMEM(room, ROOM_INDEX_DATA, 1);

	room_index[vnum] = room;
	mud->top_room++;

	pop_call();
	return room;
}


bool delete_room( ROOM_INDEX_DATA *room )
{
	bool door;
	int  vnum;
	ROOM_INDEX_DATA *prev;
	OBJ_DATA *o;
	CHAR_DATA *ch;
	EXTRA_DESCR_DATA *ed;
	RESET_DATA *pReset;

	push_call("delete_room(%p)",room);

	switch (room->vnum)
	{
		case ROOM_VNUM_LIMBO:
		case ROOM_VNUM_JUNK:
		case ROOM_VNUM_TEMPLE:
		case ROOM_VNUM_SCHOOL:
		case ROOM_VNUM_ARENA:
		case ROOM_VNUM_CASTLE:
			log_printf("Refusing to delete hardcore vnum: %d", room->vnum);
			dump_stack();
			pop_call();
			return FALSE;
	}

	del_room_timer(room->vnum, -1);

	while ((ch = room->first_person) != NULL)
	{
		if (!IS_NPC(ch))
		{
			char_from_room(ch);
			char_to_room(ch, ROOM_VNUM_TEMPLE);
		}
		else
		{
			log_printf("junking mob %s", ch->name);
			junk_mob(ch);
		}
	}

	while ((o = room->first_content) != NULL)
	{
		junk_obj(o);
	}


	while ((ed = room->first_extradesc) != NULL)
	{
		room->first_extradesc = ed->next;
		STRFREE(ed->keyword);
		STRFREE(ed->description);
		FREEMEM(ed );
		--mud->top_ed;
	}

	{
		for (pReset = room->area->first_reset ; pReset ; )
		{
			if (is_room_reset(pReset, room))
			{
				delete_reset(room->area, pReset);

				pReset = room->area->first_reset;
			}
			else
			{
				pReset = pReset->next;
			}
		}

		for (vnum = 0 ; vnum < MAX_VNUM ; vnum++)
		{
			if ((prev = room_index[vnum]) == NULL)
			{
				continue;
			}

			for (door = 0 ; door <= 5 ; door++)
			{
				if (prev->exit[door] == NULL)
				{
					continue;
				}

				if (prev->exit[door]->vnum != room->vnum)
				{
					continue;
				}

				log_printf("[%u] Deleting connection to room [%u]", prev->vnum, prev->exit[door]->vnum);

				delete_exit(prev, door);
			}
		}

		if (room->vnum == room->area->low_r_vnum)
		{
			vnum = room->area->low_r_vnum + 1;

			for (room->area->low_r_vnum = MAX_VNUM-1 ; vnum <= room->area->hi_r_vnum ; vnum++)
			{
				if ((prev = get_room_index(vnum)) == NULL)
				{
					continue;
				}
				if (prev->area != room->area)
				{
					continue;
				}
				room->area->low_r_vnum = prev->vnum;
				break;
			}
		}

		if (room->vnum == room->area->hi_r_vnum)
		{
			vnum = room->area->hi_r_vnum - 1;

			for (room->area->hi_r_vnum = 0 ; vnum >= room->area->low_r_vnum ; vnum--)
			{
				if ((prev = get_room_index(vnum)) == NULL)
				{
					continue;
				}
				if (prev->area != room->area)
				{
					continue;
				}
				room->area->hi_r_vnum = prev->vnum;
				break;
			}
		}
	}

	for (door = 0 ; door <= 5 ; door++)
	{
		if (room->exit[door] == NULL)
		{
			continue;
		}
/*
		log_printf("[%u] Deleting connection to room [%u]", room->vnum, room->exit[door]->vnum);
*/
		delete_exit(room, door);
	}

	for (door = 0 ; door < MAX_LAST_LEFT ; door++)
	{
		STRFREE(room->last_left_name[door]);
	}
	STRFREE (room->name);
	STRFREE (room->description);

	if (room->vnum >= ROOM_VNUM_WILDERNESS)
	{
		room_index[room->vnum] = room_index[ROOM_VNUM_WILDERNESS + room->sector_type];
	}
	else
	{
		room_index[room->vnum] = NULL;
	}
	FREEMEM(room );
	--mud->top_room;

	pop_call();
	return TRUE;
}

bool delete_obj( OBJ_INDEX_DATA *obj )
{
	EXTRA_DESCR_DATA	*ed;
	AFFECT_DATA		*af;
	RESET_DATA		*pReset;

	push_call("delete_obj(%p)",obj);

	if (obj->vnum < 100)
	{
		log_printf("Refusing to delete hardcore vnum: %d", obj->vnum);
		dump_stack();
		pop_call();
		return FALSE;
	}

	while (obj->first_instance)
	{
		extract_obj(obj->first_instance);
	}

	for (pReset = obj->area->first_reset ; pReset ; )
	{
		if (is_obj_reset(pReset, obj))
		{
			delete_reset(obj->area, pReset);

			pReset = obj->area->first_reset;
		}
		else
		{
			pReset = pReset->next;
		}
	}

	/* Remove references to object index */

	while ((ed = obj->first_extradesc) != NULL)
	{
		obj->first_extradesc = ed->next;
		STRFREE (ed->keyword);
		STRFREE (ed->description);
		FREEMEM(ed);
		--mud->top_ed;
	}

	while ((af = obj->first_affect) != NULL)
	{
		obj->first_affect = af->next;
		FREEMEM(af);
		--mud->top_affect;
	}

	if (obj->vnum == obj->area->low_o_vnum && obj->vnum == obj->area->hi_o_vnum)
	{
		obj->area->low_o_vnum = MAX_VNUM - 1;
		obj->area->hi_o_vnum  = 0;
	}
	else if (obj->vnum == obj->area->low_o_vnum)
	{
		for (obj->area->low_o_vnum++ ; obj->area->low_o_vnum <= obj->area->hi_o_vnum ; obj->area->low_o_vnum++)
		{
			if (obj_index[obj->area->low_o_vnum] != NULL)
			{
				break;
			}
		}
	}
	else if (obj->vnum == obj->area->hi_o_vnum)
	{
		for (obj->area->hi_o_vnum-- ; obj->area->hi_o_vnum >= obj->area->low_o_vnum ; obj->area->hi_o_vnum--)
		{
			if (obj_index[obj->area->hi_o_vnum] != NULL)
			{
				break;
			}
		}
	}

	STRFREE (obj->name);
	STRFREE (obj->short_descr);
	STRFREE (obj->long_descr);
	STRFREE (obj->description);
	STRFREE (obj->attack_string);

	obj_index[obj->vnum] = NULL;

	FREEMEM(obj);
	--mud->top_obj_index;

	pop_call();
	return TRUE;
}

/*
	Scandum 28-04-2002
*/

bool delete_mob( MOB_INDEX_DATA *mob )
{
	int vnum;
	MOB_INDEX_DATA	*prev;
	RESET_DATA	*pReset;

	push_call("delete_mob(%p)",mob);

	if (mob->vnum >= 9900 && mob->vnum < 10000)
	{
		log_printf("Refusing to delete hardcore vnum: %d", mob->vnum);
		dump_stack();
		pop_call();
		return FALSE;
	}

	while (mob_index[mob->vnum]->first_instance)
	{
		extract_char(mob_index[mob->vnum]->first_instance);
	}

	for (pReset = mob->area->first_reset ; pReset ; )
	{
		if (is_mob_reset(pReset, mob))
		{
			delete_reset(mob->area, pReset);

			pReset = mob->area->first_reset;
		}
		else
		{
			pReset = pReset->next;
		}
	}

	if (mob->pShop)
	{
		delete_shop(mob);
	}

	if (mob->vnum == mob->area->low_m_vnum)
	{
		vnum = mob->area->low_m_vnum+1;

		for (mob->area->low_m_vnum = MAX_VNUM-1 ; vnum <= mob->area->hi_m_vnum ; vnum++)
		{
			if ((prev = get_mob_index(vnum)) == NULL)
			{
				continue;
			}
			if (prev->area != mob->area)
			{
				continue;
			}
			mob->area->low_m_vnum = prev->vnum;
			break;
		}
	}

	if (mob->vnum == mob->area->hi_m_vnum)
	{
		vnum = mob->area->hi_m_vnum-1;

		for (mob->area->hi_m_vnum = 0 ; vnum > mob->area->low_m_vnum ; vnum--)
		{
			if ((prev = get_mob_index(vnum)) == NULL)
			{
				continue;
			}
			if (prev->area != mob->area)
			{
				continue;
			}
			mob->area->hi_m_vnum = prev->vnum;
			break;
		}
	}

	STRFREE (mob->player_name);
	STRFREE (mob->short_descr);
	STRFREE (mob->long_descr);
	STRFREE (mob->description);

	mob_index[mob->vnum] = NULL;

	FREEMEM(mob);
	--mud->top_mob_index;

	pop_call();
	return TRUE;
}

bool delete_help( HELP_DATA *help )
{
	HELP_MENU_DATA *menu;

	push_call("delete_help(%p)",help);

	while ((menu = help->first_menu) != NULL)
	{
		help->first_menu = menu->next;
		FREEMEM(menu);
	}

	STRFREE (help->keyword);
	STRFREE (help->text);
	UNLINK(help, help->area->first_help, help->area->last_help, next, prev);
	FREEMEM(help);
	--mud->top_help;

	pop_call();
	return TRUE;
}


void do_memory( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];

	push_call("do_memory(%p,%p)",ch,argument);

	buf[0] = '\0';

	cat_sprintf(buf, "{078}Players:  {178}%6d    ",	mud->total_plr);
	cat_sprintf(buf, "{078}Descs:    {178}%6d    ",	mud->total_desc);
	cat_sprintf(buf, "{078}Areas:    {178}%6d    ",	mud->top_area);
	cat_sprintf(buf, "{078}Helps:    {178}%6d\n\r",	mud->top_help);

	cat_sprintf(buf, "{078}Rooms:    {178}%6d    ",	mud->top_room);
	cat_sprintf(buf, "{078}MobsIn:   {178}%6d    ",	mud->top_mob_index);
	cat_sprintf(buf, "{078}ObjsIn:   {178}%6d    ",	mud->top_obj_index);
	cat_sprintf(buf, "{078}Resets:   {178}%6d\n\r",	mud->top_reset);

	cat_sprintf(buf, "{078}Exits:    {178}%6d    ",	mud->top_exit);
	cat_sprintf(buf, "{078}Mobs:     {178}%6d    ",	mud->total_mob);
	cat_sprintf(buf, "{078}Objs:     {178}%6d    ",	mud->total_obj);
	cat_sprintf(buf, "{078}Affects:  {178}%6d\n\r",	mud->top_affect);

	cat_sprintf(buf, "{078}Shops:    {178}%6d    ",	mud->top_shop);
	cat_sprintf(buf, "{078}Mprog:    {178}%6d    ",	mud->top_mprog);
	cat_sprintf(buf, "{078}Mtrig:    {178}%6d    ",	mud->top_mtrig);
	cat_sprintf(buf, "{078}Extra:    {178}%6d\n\r",	mud->top_ed);

	send_to_char_color(buf, ch);

	pop_call();
	return;
}

/*
	Create new stuff (OLC) - Scandum 10-07-2002
*/

ROOM_INDEX_DATA *make_room( int vnum )
{
	ROOM_INDEX_DATA *pRoomIndex;
	bool door;

	push_call("make_room(%p)",vnum);

	if (get_area_from_vnum(vnum) == NULL)
	{
		log_printf("make_room: failed to add area data.");
		pop_call();
		return NULL;
	}

	ALLOCMEM( pRoomIndex, ROOM_INDEX_DATA, 1 );

	pRoomIndex->vnum			= vnum;
	pRoomIndex->area			= get_area_from_vnum(vnum);

	if (vnum < pRoomIndex->area->low_r_vnum)
	{
		pRoomIndex->area->low_r_vnum = vnum;
	}

	if (vnum > pRoomIndex->area->hi_r_vnum)
	{
		pRoomIndex->area->hi_r_vnum = vnum;
	}

	pRoomIndex->name        = STRALLOC("Floating in a void");
	pRoomIndex->description = STRDUPE(str_empty);

	for (door = 0 ; door < MAX_LAST_LEFT ; door++)
	{
		pRoomIndex->last_left_name[door] = STRDUPE(str_empty);
	}

	room_index[vnum]		= pRoomIndex;
	mud->top_room++;

	pop_call();
	return pRoomIndex;
}



OBJ_INDEX_DATA *make_object(int vnum)
{
	OBJ_INDEX_DATA	*pObjIndex;

	push_call("make_object(%p)",vnum);

	if (get_area_from_vnum(vnum) == NULL)
	{
		log_printf("Make_object: Failed to add area data to object.");
		pop_call();
		return NULL;
	}

	ALLOCMEM( pObjIndex, OBJ_INDEX_DATA, 1 );

	pObjIndex->vnum		= vnum;
	pObjIndex->area		= get_area_from_vnum(vnum);

	if (vnum < pObjIndex->area->low_o_vnum)
	{
		pObjIndex->area->low_o_vnum = vnum;
	}
	if (vnum > pObjIndex->area->hi_o_vnum)
	{
		pObjIndex->area->hi_o_vnum = vnum;
	}

	obj_index[vnum]		= pObjIndex;
	pObjIndex->name		= STRDUPE(str_empty);
	pObjIndex->short_descr	= STRDUPE(str_empty);
	pObjIndex->long_descr	= STRDUPE(str_empty);
	pObjIndex->description	= STRDUPE(str_empty);
	pObjIndex->attack_string	= STRDUPE(str_empty);
	pObjIndex->item_type	= ITEM_TRASH;
	pObjIndex->extra_flags	= 0;
	pObjIndex->wear_flags	= 0;
	pObjIndex->value[0]		= 0;
	pObjIndex->value[1]		= 0;
	pObjIndex->value[2]		= 0;
	pObjIndex->value[3]		= 0;
	pObjIndex->weight		= 1;
	pObjIndex->cost		= 10;
	pObjIndex->level		= 1;
	pObjIndex->level		= 1;
	pObjIndex->oprogtypes	= 0;

	mud->top_obj_index++;

	pop_call();
	return pObjIndex;
}

HELP_DATA * make_helpfile( char *argument, AREA_DATA *area )
{
	HELP_DATA *help;

	push_call("make_helpfile(%p,%p)",argument,area);

	ALLOCMEM(help, HELP_DATA, 1);

	help->level	= 0;
	help->keyword	= STRALLOC(argument);
	help->text	= STRDUPE(str_empty);
	help->area	= area;

	LINK (help, area->first_help, area->last_help, next, prev );
	mud->top_help++;

	pop_call();
	return help;
}

AREA_DATA * get_area_from_vnum (int vnum)
{
	AREA_DATA * pArea;

	push_call("get_area_from_vnum(%p)",vnum);

	for (pArea = mud->f_area ; pArea ; pArea = pArea->next)
	{
		if (pArea->olc_range_lo <= vnum && pArea->olc_range_hi >= vnum)
		{
			break;
		}
	}
	pop_call();
	return pArea;
}

/*
	Create a new INDEX mobile (OLC)
*/

MOB_INDEX_DATA *make_mobile( int vnum )
{
	MOB_INDEX_DATA *pMobIndex;

	push_call("make_mobile(%p)",vnum);

	if (get_area_from_vnum(vnum) == NULL)
	{
		log_printf("make_mobile: couldn't find area.\n\r");
		pop_call();
		return NULL;
	}

	ALLOCMEM( pMobIndex, MOB_INDEX_DATA, 1 );

	pMobIndex->vnum			= vnum;
	pMobIndex->player_name		= STRALLOC("newly created mobile");
	pMobIndex->short_descr		= STRALLOC("a newly created mobile");
	pMobIndex->long_descr         = STRALLOC("");
	pMobIndex->description        = STRALLOC("");
	pMobIndex->act				= ACT_SENTINEL;
	pMobIndex->level			= 1;
	pMobIndex->hitplus			= 10;
	pMobIndex->damplus			= 2;
	pMobIndex->position			= POS_STANDING;
	pMobIndex->area			= get_area_from_vnum (vnum);

	if (vnum < pMobIndex->area->low_m_vnum)
	{
		pMobIndex->area->low_m_vnum = vnum;
	}
	if (vnum > pMobIndex->area->hi_m_vnum)
	{
		pMobIndex->area->hi_m_vnum = vnum;
	}

	mob_index[vnum]			= pMobIndex;
	mud->top_mob_index++;

	pop_call();
	return pMobIndex;
}


EXIT_DATA *make_exit( ROOM_INDEX_DATA *pRoomIndex, ROOM_INDEX_DATA *to_room, bool door )
{
	EXIT_DATA *pexit;

	push_call("make_exit(%p,%p,%p)",pRoomIndex,to_room,door);

	ALLOCMEM(pexit, EXIT_DATA, 1);

	pexit->flags =  0;
	pexit->key   = -1;

	if (to_room)
	{
		pexit->to_room = to_room->vnum;
		pexit->vnum    = to_room->vnum;
	}
	room_index[pRoomIndex->vnum]->exit[door] = pexit;
	mud->top_exit++;

	pop_call();
	return pexit;
}

/*
	This routine makes sure that files are not cross linked or chopped
	Chaos  - 4/26/99
*/

bool is_valid_save( char *file_name , char *text_crc)
{
	char buf[MAX_INPUT_LENGTH];
	sh_int cnt, cf;
	FILE *fp;

	push_call("is_valid_save(%p,%p)",file_name,text_crc);

	if ((fp = my_fopen(file_name, "r", FALSE)) == NULL)
	{
		pop_call();
		return( FALSE );
	}

	fseek(fp, 0, SEEK_END);

	if (ftell(fp) < 10)
	{
		my_fclose(fp);

		log_printf("Oops, file system full!  %s failed.", file_name);

		pop_call();
		return( FALSE );
	}

	cf  = ' ';
	cnt = 0;

	while (cf != '#' && cnt > -25)
	{
		cnt--;
		fseek(fp, cnt, SEEK_END);
		cf = fgetc( fp );
	}

	if (cnt < -25)
	{
		my_fclose(fp);

		log_printf("Didn't find an #%s on %s", text_crc, file_name);

		pop_call();
		return( FALSE );
	}

	cf = fgetc(fp);

	for (cnt = 0 ; cf != '\n' && cf != EOF ; cnt++)
	{
		buf[cnt] = cf;

		cf = fgetc(fp);
	}
	buf[cnt] = '\0';

	if (!strcmp(buf, text_crc))
	{
		my_fclose( fp );

		pop_call();
		return( TRUE );
	}

	my_fclose( fp );

	log_printf("Cross linked file %s on %s", buf, file_name);

	pop_call();
	return( FALSE );
}
