/***************************************************************************
 * Lola  1.0 by Igor van den Hoven.                                        *
 *                                                                         *
 * Emud  3.0 by Igor van den Hoven.                                        *
 ***************************************************************************/

#include "mud.h"

#define SITEBAN_LIST          "data/siteban.lst"
#define SITEBAN_LIST_TMP      "data/siteban.tmp"
#define NSITEBAN_LIST         "data/nsiteban.lst"
#define NSITEBAN_LIST_TMP     "data/nsiteban.tmp"
#define PLAYER_LIST           "data/player.lst"
#define PLAYER_LIST_TMP       "data/player.tmp"
#define USAGE_LIST            "data/usage.lst"
#define USAGE_LIST_TMP        "data/usage.tmp"
#define VICTORY_LIST          "data/victory.lst"
#define VICTORY_LIST_TMP      "data/victory.tmp"
#define BOUNTY_LIST           "data/bounty.lst"
#define BOUNTY_LIST_TMP       "data/bounty.tmp"
#define HIGHSCORE_LIST        "data/hiscore.lst"
#define HIGHSCORE_LIST_TMP    "data/hiscore.tmp"
#define TIMEINFO_LIST         "data/timeinfo.lst"
#define TIMEINFO_LIST_TMP     "data/timeinfo.lst"
#define NOTE_LIST             "data/note.lst"
#define NOTE_LIST_TMP         "data/note.tmp"

void del_data( CHAR_DATA *ch )
{
	HISCORE_DATA	* pHiscore;
	HISCORE_LIST	* pHilist;
	BOUNTY_DATA	* pBounty;
	int x;

	push_call("del_data(%p)",ch);

	for (x = 0 ; x < MOST_MOST ; x++)
	{
		pHilist = mud->high_scores[x];

		for (pHiscore = pHilist->first ; pHiscore ; pHiscore = pHiscore->next)
		{
			if (pHiscore->vnum == ch->pcdata->pvnum)
			{
				UNLINK(pHiscore, pHilist->first, pHilist->last, next, prev);
				pHiscore->vnum		= -1;
				pHiscore->score	=  0;
				STRFREE(pHiscore->player);
				pHiscore->player	= STRDUPE(str_empty);
				LINK(pHiscore, pHilist->first, pHilist->last, next, prev);

				break;
			}
		}
	}

	for (pBounty = mud->f_bounty ; pBounty ; pBounty = pBounty->next)
	{
		if (!strcasecmp(pBounty->name, ch->name))
		{
			remove_bounty(pBounty);
			break;
		}
	}

	if (ch->pcdata->pvnum && pvnum_index[ch->pcdata->pvnum])
	{
		SET_BIT(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_DELETED);

		save_players();
	}
	pop_call();
	return;
}

void load_hiscores( void )
{
	HISCORE_DATA *hiscore;
	FILE *fpList;
	char letter;
	int x;

	push_call("load_hiscores()");

	log_printf( "Loading %s...", HIGHSCORE_LIST );

	close_reserve();

	if ((fpList = my_fopen(HIGHSCORE_LIST, "r", FALSE)) == NULL)
	{
		perror( HIGHSCORE_LIST );
		exit( 1 );
	}

	for ( ; ; )
	{
		letter = fread_letter( fpList );

		if (letter != 'H')
		{
			if (letter != '$')
			{
				log_printf("***WARNING*** load_hiscores: bad format");
			}
			fread_to_eol(fpList);
			break;
		}
		ALLOCMEM(hiscore, HISCORE_DATA, 1);
		x				= fread_number( fpList );
		hiscore->vnum		= fread_number( fpList );
		hiscore->score		= fread_number( fpList );
		hiscore->player	= fread_string( fpList );
		LINK(hiscore, mud->high_scores[x]->first, mud->high_scores[x]->last, next, prev);
	}

	if (fpList)
	{
		my_fclose( fpList );
	}

	open_reserve();

	pop_call();
	return;
}


void save_hiscores( )
{
	HISCORE_DATA *hiscore;
	FILE *fpout;
	int x, y;

	push_call("save_hiscores()");

	close_reserve();

	if ((fpout = my_fopen(HIGHSCORE_LIST_TMP, "w", FALSE)) == NULL)
	{
		perror(HIGHSCORE_LIST_TMP);
		open_reserve();
		pop_call();
		return;
	}

	for (x = 0 ; x < MOST_MOST ; x++)
	{
		for (y = 0, hiscore = mud->high_scores[x]->first ; hiscore && y < MAX_HISCORE ; hiscore = hiscore->next, y++)
		{
			fprintf( fpout, "H %2d %6d %6d %s~\n", x, hiscore->vnum, hiscore->score, hiscore->player);
		}
	}

	fprintf( fpout, "$\nXXXXXXXXXX\n#%s\n", HIGHSCORE_LIST );

	my_fclose( fpout );

	if (is_valid_save(HIGHSCORE_LIST_TMP, HIGHSCORE_LIST ))
	{
		rename (HIGHSCORE_LIST_TMP, HIGHSCORE_LIST);
	}
	open_reserve();

	pop_call();
	return;
}

void load_players( void )
{
	char letter;
	FILE *fpList;
	PVNUM_DATA *player;
	int vnum;

	push_call("load_players(void)");

	log_printf("Loading %s...", PLAYER_LIST);

	close_reserve();

	if ((fpList = my_fopen(PLAYER_LIST, "r", FALSE)) == NULL)
	{
		perror( PLAYER_LIST );
		exit( 1 );
	}

	for ( ; ; )
	{
		letter = fread_letter( fpList );

		if (letter != 'P')
		{
			if (letter != '$')
			{
				log_printf("***WARNING*** load_players: bad format");
			}
			fread_to_eol(fpList);
			break;
		}
		vnum = fread_number( fpList );

		ALLOCMEM(player, PVNUM_DATA, 1);
		player->date  = fread_number( fpList );
		player->flags = fread_number( fpList );
		player->name  = fread_string( fpList );

		pvnum_index[vnum] = player;
	}

	if (fpList)
	{
		my_fclose( fpList );
	}

	open_reserve();

	pop_call();
	return;
}

void save_players( )
{
	FILE *fpout;
	int vnum;

	push_call("save_players()");

	close_reserve();

	if ((fpout = my_fopen(PLAYER_LIST_TMP, "w", FALSE)) == NULL)
	{
		perror(PLAYER_LIST_TMP);
		open_reserve();
		pop_call();
		return;
	}

	for (vnum = 0 ; vnum < MAX_PVNUM ; vnum++)
	{
		if (pvnum_index[vnum])
		{
			fprintf( fpout, "P %5d %10d %10d %s~\n", vnum, pvnum_index[vnum]->date, pvnum_index[vnum]->flags, pvnum_index[vnum]->name);
		}
	}

	fprintf( fpout, "$\nXXXXXXXXXX\n#%s\n", PLAYER_LIST );

	my_fclose( fpout );

	if (is_valid_save(PLAYER_LIST_TMP, PLAYER_LIST ))
	{
		rename (PLAYER_LIST_TMP, PLAYER_LIST);
	}
	else
	{
		log_printf("Invalid save: %s", PLAYER_LIST);
	}
	open_reserve();

	pop_call();
	return;
}


void load_sites( void )
{
	FILE *fp;
	BAN_DATA *siteban;
	char letter;

	push_call("load_sites()");

	log_printf("Loading %s...", SITEBAN_LIST );

	close_reserve();

	if ((fp = my_fopen(SITEBAN_LIST, "r", FALSE)) == NULL)
	{
		perror(SITEBAN_LIST);
		exit (1);
	}

	for ( ; ; )
	{
		letter = fread_letter(fp);

		if (letter != 'B')
		{
			if (letter != '$')
			{
				log_printf("***WARNING*** load_sites: bad format");
			}
			fread_to_eol(fp);
			break;
		}
		ALLOCMEM(siteban, BAN_DATA, 1);
		siteban->name		= fread_string(fp);
		siteban->banned_by	= fread_string(fp);
		siteban->date		= fread_number(fp);
		LINK(siteban, mud->f_ban, mud->l_ban, next, prev);
	}

	if (fp)
	{
		my_fclose(fp);
	}

	open_reserve();

	pop_call();
	return;
}


void save_sites( )
{
	BAN_DATA *siteban;
	FILE *fpout;

	push_call("save_sites()");

	close_reserve();

	if ((fpout = my_fopen(SITEBAN_LIST_TMP, "w", FALSE)) == NULL)
	{
		perror(SITEBAN_LIST_TMP);
		open_reserve();
		pop_call();
		return;
	}

	for (siteban = mud->f_ban ; siteban ; siteban = siteban->next)
	{
		fprintf(fpout, "B %12s~ %12s~ %d\n",siteban->name, siteban->banned_by, siteban->date);
	}

	fprintf(fpout,"$\nXXXXXXXXXXXXXXXXX\n#%s\n", SITEBAN_LIST);

	if (fpout)
	{
		my_fclose(fpout);
	}

	if (is_valid_save(SITEBAN_LIST_TMP, SITEBAN_LIST))
	{
		rename(SITEBAN_LIST_TMP, SITEBAN_LIST);
	}
	else
	{
		log_printf("Invalid save: %s", SITEBAN_LIST);
	}

	open_reserve();

	pop_call();
	return;
}


void load_nsites( void )
{
	FILE *fp;
	BAN_DATA *siteban;
	char letter;

	push_call("load_sites()");

	log_printf("Loading %s...", NSITEBAN_LIST );

	close_reserve();

	if ((fp = my_fopen(NSITEBAN_LIST, "r", FALSE)) == NULL)
	{
		perror(NSITEBAN_LIST);
		exit (1);
	}

	for ( ; ; )
	{
		letter = fread_letter(fp);

		if (letter != 'B')
		{
			if (letter != '$')
			{
				log_printf("***WARNING*** load_sites: bad format");
			}
			fread_to_eol(fp);
			break;
		}
		ALLOCMEM(siteban, BAN_DATA, 1);
		siteban->name		= fread_string(fp);
		siteban->banned_by	= fread_string(fp);
		siteban->date		= fread_number(fp);
		LINK(siteban, mud->f_nban, mud->l_nban, next, prev);
	}

	if (fp)
	{
		my_fclose(fp);
	}

	open_reserve();

	pop_call();
	return;
}


void save_nsites( )
{
	BAN_DATA *siteban;
	FILE *fpout;

	push_call("save_sites()");

	close_reserve();

	if ((fpout = my_fopen(NSITEBAN_LIST_TMP, "w", FALSE)) == NULL)
	{
		perror(NSITEBAN_LIST_TMP);
		open_reserve();
		pop_call();
		return;
	}

	for (siteban = mud->f_nban ; siteban ; siteban = siteban->next)
	{
		fprintf(fpout, "B %12s~ %12s~ %d\n",siteban->name, siteban->banned_by, siteban->date);
	}

	fprintf(fpout,"$\nXXXXXXXXXXXXXXXXX\n#%s\n", NSITEBAN_LIST);

	if (fpout)
	{
		my_fclose(fpout);
	}

	if (is_valid_save(NSITEBAN_LIST_TMP, NSITEBAN_LIST))
	{
		rename(NSITEBAN_LIST_TMP, NSITEBAN_LIST);
	}
	else
	{
		log_printf("Invalid save: %s", NSITEBAN_LIST);
	}

	open_reserve();

	pop_call();
	return;
}


void load_victors( void )
{
	FILE *fp;
	int i;
	char *ptx;

	push_call("load_victors()");

	log_printf( "Loading %s...", VICTORY_LIST );

	close_reserve();

	if ((fp = my_fopen(VICTORY_LIST, "r", FALSE)) == NULL)
	{
		perror(VICTORY_LIST);
		exit(1);
	}

	for (i = 0 ; i < VICTORY_LIST_SIZE ; i++)
	{
		ptx = fread_string( fp );
		if (ptx[0] != 'Z')
		{
			if (ptx[0] != '$')
			{
				log_printf("***WARNING*** load_victors: bad format");
			}
			break;
		}
		victory_list[i] = STRALLOC(ptx);
	}
	if (fp)
	{
		my_fclose(fp);
	}

	open_reserve();

	pop_call();
	return;
}

void save_victors( void )
{
	FILE *fp;
	int i;

	push_call("save_victors()");

	close_reserve();

	if ((fp = my_fopen(VICTORY_LIST_TMP, "w", FALSE)) == NULL)
	{
		perror(VICTORY_LIST_TMP);
		open_reserve();
		pop_call();
		return;
	}

	for (i = 0 ; i < VICTORY_LIST_SIZE ; i++)
	{
		fprintf (fp, "%s~\n", victory_list[i]);
	}
	fprintf(fp, "$~\nXXXXXXXXXX\n#%s\n", VICTORY_LIST);

	if (fp)
	{
		my_fclose( fp );
	}

	if (is_valid_save(VICTORY_LIST_TMP, VICTORY_LIST))
	{
		rename(VICTORY_LIST_TMP, VICTORY_LIST );
	}
	else
	{
		log_printf("Invalid save: %s", VICTORY_LIST);
	}

	open_reserve();

	pop_call();
	return;
}


void load_usage(void)
{
	FILE *fp;
	int day, hour;

	push_call("load_usage()");

	log_printf( "Loading %s...", USAGE_LIST );

	close_reserve();

	if ((fp = my_fopen(USAGE_LIST, "r", FALSE)) == NULL)
	{
		perror(USAGE_LIST);
		exit(1);
	}

	for (hour = 0 ; hour < 24 ; hour++)
	{
		for (day = 0 ; day < 7 ; day++)
		{
			mud->usage->players[hour][day] = fread_number(fp);
		}
	}

	if (fp)
	{
		my_fclose(fp);
	}

	open_reserve();

	pop_call();
	return;
}

void save_usage( void)
{
	FILE *fp;
	int hour;

	push_call("save_usage()");

	close_reserve();

	if ((fp = my_fopen(USAGE_LIST_TMP, "w", FALSE)) == NULL)
	{
		perror(USAGE_LIST_TMP);
		open_reserve();
		pop_call();
		return;
	}

	for (hour = 0 ; hour < 24 ; hour++)
	{
		fprintf(fp, "%3d %3d %3d %3d %3d %3d %3d\n",
			mud->usage->players[hour][0],
			mud->usage->players[hour][1],
			mud->usage->players[hour][2],
			mud->usage->players[hour][3],
			mud->usage->players[hour][4],
			mud->usage->players[hour][5],
			mud->usage->players[hour][6]);
	}
	fprintf( fp, "\nXXXXXXXXXX\n#%s\n", USAGE_LIST );

	if (fp)
	{
		my_fclose( fp );
	}

	if (is_valid_save(USAGE_LIST_TMP, USAGE_LIST))
	{
		rename(USAGE_LIST_TMP, USAGE_LIST);
	}
	else
	{
		log_printf("Invalid save: %s", USAGE_LIST);
	}

	open_reserve();

	pop_call();
	return;
}

void load_timeinfo( void )
{
	FILE *fp;

	push_call("load_timeinfo()");

	log_printf( "Loading %s...", TIMEINFO_LIST );

	close_reserve();

	if ((fp = my_fopen(TIMEINFO_LIST, "r", FALSE)) == NULL)
	{
		perror(TIMEINFO_LIST);
		exit (1);
	}

	mud->time_info->hour  = fread_number(fp);
	mud->time_info->day   = fread_number(fp);
	mud->time_info->month = fread_number(fp);
	mud->time_info->year  = fread_number(fp);

	if (fp)
	{
		my_fclose(fp);
	}

	open_reserve();

	pop_call();
	return;
}


void save_timeinfo( void)
{
	FILE *fp;

	push_call("save_pvnum()");

	close_reserve();

	if ((fp = my_fopen(TIMEINFO_LIST_TMP, "w", FALSE)) == NULL)
	{
		perror(TIMEINFO_LIST_TMP);
		open_reserve();
		pop_call();
		return;
	}

	fprintf( fp, "%u %u %u %u\nXXXXXXXXXX\n#%s\n",
		mud->time_info->hour,
		mud->time_info->day,
		mud->time_info->month,
		mud->time_info->year,
		TIMEINFO_LIST );

	if (fp)
	{
		my_fclose( fp );
	}

	if (is_valid_save(TIMEINFO_LIST_TMP, TIMEINFO_LIST))
	{
		rename(TIMEINFO_LIST_TMP, TIMEINFO_LIST);
	}
	else
	{
		log_printf("Invalid save: %s", TIMEINFO_LIST);
	}

	open_reserve();

	pop_call();
	return;
}


void load_bounties(void)
{
	FILE *fp;
	BOUNTY_DATA *bounty;
	char letter;

	push_call("load_bounties()");

	log_printf("Loading %s...", BOUNTY_LIST);

	close_reserve();

	if ((fp = my_fopen(BOUNTY_LIST, "r", FALSE)) == NULL)
	{
		perror(BOUNTY_LIST);
		exit (1);
	}

	for ( ; ; )
	{
		letter = fread_letter(fp);

		if (letter != 'B')
		{
			if (letter != '$')
			{
				log_printf("***WARNING*** load_bounties: bad format");
			}
			fread_to_eol(fp);
			break;
		}
		ALLOCMEM(bounty, BOUNTY_DATA, 1);
		bounty->name	= fread_string(fp);
		bounty->amount	= fread_number(fp);
		bounty->expires	= fread_number(fp);
		sort_bounty( bounty );
	}

	if (fp)
	{
		my_fclose(fp);
	}

	open_reserve();

	pop_call();
	return;
}

void save_bounties(void)
{
	FILE *fp;
	BOUNTY_DATA *bounty;

	push_call("save_bounties()");

	close_reserve();

	if ((fp = my_fopen(BOUNTY_LIST_TMP, "w", FALSE)) == NULL)
	{
		perror(BOUNTY_LIST_TMP);
		open_reserve();
		pop_call();
		return;
	}

	for (bounty = mud->f_bounty ; bounty ; bounty = bounty->next)
	{
		fprintf(fp, "B %12s~ %9d %9d\n", bounty->name, bounty->amount, bounty->expires);
	}

	fprintf(fp,"$\nXXXXXXXXXXXXXXXXX\n#%s\n", BOUNTY_LIST);

	if (fp)
	{
		my_fclose(fp);
	}

	if (is_valid_save(BOUNTY_LIST_TMP, BOUNTY_LIST))
	{
		rename(BOUNTY_LIST_TMP, BOUNTY_LIST);
	}
	else
	{
		log_printf("Invalid save: %s", BOUNTY_LIST);
	}

	open_reserve();

	pop_call();
	return;
}


void load_notes(void)
{
	FILE *fp;
	NOTE_DATA *note;
	char *word;

	push_call("load_notes()");

	log_printf("Loading %s...", NOTE_LIST);

	close_reserve();

	if ((fp = my_fopen(NOTE_LIST, "r", FALSE)) == NULL)
	{
		perror(NOTE_LIST);
		exit (1);
	}

	for ( ; ; )
	{
		word = fread_word(fp);

		if (!strcasecmp(word, "Sender"))
		{
			ALLOCMEM(note, NOTE_DATA, 1);

			note->sender    = fread_string(fp); fread_word(fp);
			note->date      = fread_string(fp); fread_word(fp);
			note->time      = fread_number(fp); fread_word(fp);
			note->to_list   = fread_string(fp); fread_word(fp);
			note->subject   = fread_string(fp); fread_word(fp);
			note->topic     = fread_number(fp); fread_word(fp);
			note->text      = fread_string(fp); fread_word(fp);
			note->room_vnum	= fread_number(fp); fread_to_eol(fp);

			LINK(note, mud->f_note, mud->l_note, next, prev);
		}
		else if (!strcasecmp(word, "End"))
		{
			if (fp)
			{
				my_fclose(fp);
			}
			open_reserve();
			break;
		}
		else
		{
			log_printf("***WARNING*** load_notes: bad format '%s'", word);
			fread_to_eol(fp);
			break;
		}
	}
	pop_call();
	return;
}


void save_notes(void)
{
	FILE *fp;
	NOTE_DATA *note;

	push_call("save_bounties()");

	close_reserve();

	if ((fp = my_fopen(NOTE_LIST_TMP, "w", FALSE)) == NULL)
	{
		perror(NOTE_LIST_TMP);
		open_reserve();
		pop_call();
		return;
	}

	for (note = mud->f_note ; note ; note = note->next)
	{
		fprintf(fp, "Sender  %s~\nDate    %s~\nTime  %d\nTo      %s~\nSubject %s~\nTopic   %d\nText\n%s~\nRoom %u\n\n", note->sender, note->date, note->time, note->to_list, note->subject, note->topic, fixer(note->text), note->room_vnum);
	}

	fprintf(fp, "End\n\n$\nXXXXXXXXXXXXXXXXX\n#%s\n", NOTE_LIST);

	if (fp)
	{
		my_fclose(fp);
	}

	if (is_valid_save(NOTE_LIST_TMP, NOTE_LIST))
	{
		rename(NOTE_LIST_TMP, NOTE_LIST);
	}
	else
	{
		log_printf("Invalid save: %s", NOTE_LIST);
	}

	open_reserve();

	pop_call();
	return;
}
