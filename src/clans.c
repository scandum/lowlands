/***************************************************************************
 * Lola  1.0 by Igor van den Hoven.                                        *
 *                                                                         *
 * Emud  2.2 by Igor van den Hoven, Michiel Lange.                         *
 *                                                                         *
 * MrMud 1.4 by David Bills and Dug Michael.                               *
 ***************************************************************************/

#include "mud.h"

void       fread_clan          args( ( CLAN_DATA *clan, FILE *fp ) );
bool       load_clan           args( ( char *clanname ) );
void       save_clan_list      args( ( void ) );
void       do_castle_entrance  args( ( CHAR_DATA *ch, char *argument ) );

CLAN_DATA *get_clan( char *name )
{
	CLAN_DATA *clan;

	push_call("get_clan(%p)",name);

	for (clan = mud->f_clan ; clan ; clan = clan->next)
	{
		if (!str_prefix(name, clan->name))
		{
			pop_call();
			return clan;
		}
	}
	pop_call();
	return NULL;
}

CLAN_DATA *get_clan_from_vnum( int vnum )
{
	CLAN_DATA *clan;

	push_call("get_clan_from_vnum(%p)",vnum);

	for (clan = mud->f_clan ; clan ; clan = clan->next)
	{
		if (vnum == clan->founder_pvnum)
		{
			pop_call();
			return clan;
		}
	}
	pop_call();
	return NULL;
}

bool is_clan_leader( CHAR_DATA *ch )
{
	push_call("is_clan_leader(%p)",ch);

	if (IS_NPC(ch) || ch->pcdata->clan == NULL)
	{
		pop_call();
		return FALSE;
	}

	if (!strcasecmp(ch->name, ch->pcdata->clan->leader[ch->pcdata->clan_position]))
	{
		pop_call();
		return TRUE;
	}
	pop_call();
	return FALSE;
}

void sort_clans ( CLAN_DATA *clan )
{
	CLAN_DATA *temp_clan;

	push_call("sort_clans(%p)",clan);

	for (temp_clan = mud->f_clan ; temp_clan ; temp_clan = temp_clan->next)
	{
		if (strcmp(clan->name, temp_clan->name) < 0)
		{
			INSERT_LEFT(clan, temp_clan, mud->f_clan, next, prev);
			break;
		}
	}
	if (temp_clan == NULL)
	{
		LINK(clan, mud->f_clan, mud->l_clan, next, prev);
	}
	pop_call();
	return;
}

void save_clan_list( )
{
	CLAN_DATA *clan;
	FILE *fp;
	char templist[50], clanlist[50];

	push_call("save_clan_list()");

	sprintf(clanlist, "%s/clan.lst", CLAN_DIR);
	sprintf(templist, "%s/clan.tmp", CLAN_DIR);

	close_reserve();

	if ((fp = my_fopen(templist, "w", FALSE)) == NULL)
	{
		log_printf("save_clan_list: failed to open %s", templist);
		pop_call();
		return;
	}

	for (clan = mud->f_clan ; clan ; clan = clan->next)
	{
		fprintf(fp, "C %s\n", clan->filename);
	}
	fprintf(fp, "$\nXXXXXXXXXX\n#Clans\n" );

	my_fclose(fp);

	if (is_valid_save(templist, "Clans"))
	{
		rename(templist, clanlist);
	}
	open_reserve();

	pop_call();
	return;
}

void save_all_clans( )
{
	CLAN_DATA *clan;

	push_call("save_all_clans()");

	for (clan = mud->f_clan ; clan ; clan = clan->next)
	{
		save_clan(clan);
	}
	pop_call();
	return;
}

void save_clan( CLAN_DATA *clan )
{
	FILE *fp;
	char filename[50], tempfile[50];
	bool cnt;

	push_call("save_clan(%p)",clan);

	if (clan == NULL)
	{
		log_printf("save_clan: null pointer");
		pop_call();
		return;
	}

	sprintf(filename, "%s/%s",     CLAN_DIR, clan->filename);
	sprintf(tempfile, "%s/%s.tmp", CLAN_DIR, clan->filename);

	close_reserve();

	if ((fp = my_fopen(tempfile, "w", FALSE)) == NULL)
	{
		perror(tempfile);
		open_reserve();
		pop_call();
		return;
	}

	fprintf(fp, "#CLAN\n");

	fprintf(fp, "Name      %s~\n",			clan->name		);
	fprintf(fp, "Motto     %s~\n",			clan->motto		);
	fprintf(fp, "FileName  %s~\n",			clan->filename		);
	fprintf(fp, "Email     %s~\n",			clan->email		);

	for (cnt = 0 ; cnt < 5 ; cnt++)
	{
		fprintf(fp, "Leader%d   %s~\n",		cnt, clan->leader[cnt]);
	}

	for (cnt = 0 ; cnt < MAX_GOD ; cnt++)
	{
		fprintf(fp, "PKill     %d %d\n",	cnt, clan->pkills[cnt]);
	}

	for (cnt = 0 ; cnt < MAX_GOD ; cnt++)
	{
		fprintf(fp, "PDeath    %d %d\n",	cnt, clan->pdeaths[cnt]);
	}

	fprintf(fp, "Flags     %d\n",      clan->flags);
	fprintf(fp, "MKills    %d\n",      clan->mkills);
	fprintf(fp, "MDeaths   %d\n",      clan->mdeaths);
	fprintf(fp, "Members   %d\n",      clan->members);
	fprintf(fp, "Founder   %d\n",      clan->founder_pvnum);
	fprintf(fp, "God       %d\n",      clan->god);
	fprintf(fp, "Tax       %d\n",      clan->tax);
	fprintf(fp, "Coffers   %lld\n",    clan->coffers);
	fprintf(fp, "Home      %d\n",      clan->home);
	fprintf(fp, "Store     %d\n",      clan->store);
	fprintf(fp, "Guards    %d\n",      clan->num_guards);
	fprintf(fp, "Healers   %d\n",      clan->num_healers);
	fprintf(fp, "Trainers  %d\n",      clan->num_trainers );
	fprintf(fp, "Banks     %d\n",      clan->num_banks);
	fprintf(fp, "Morgues   %d\n",      clan->num_morgues);
	fprintf(fp, "Altars    %d\n",      clan->num_altars);
	fprintf(fp, "Backdoors %d\n",      clan->num_backdoors);
	fprintf(fp, "Bamfin    %s~\n",     clan->bamfin);
	fprintf(fp, "Bamfout   %s~\n",     clan->bamfout);
	fprintf(fp, "Bamfself  %s~\n",     clan->bamfself);
	fprintf(fp, "End\n\n");

	save_clan_pit(clan, fp);

	fprintf(fp, "#END\n\n");
	fprintf(fp, "#Clanfile\n");

	my_fclose( fp );

	if (is_valid_save(tempfile, "Clanfile"))
	{
		rename(tempfile, filename);
	}
	open_reserve();

	pop_call();
	return;
}

void fread_clan( CLAN_DATA *clan, FILE *fp )
{
	char *word;
	bool fMatch;
	bool cnt;

	push_call("fread_clan(%p,%p)",clan,fp);

	for ( ; ; )
	{
		word   = feof(fp) ? "End" : fread_word(fp);
		fMatch = FALSE;

		switch (word[0])
		{
			case 'A':
				NKEY("Altars",           clan->num_altars,        fread_number(fp));
				break;

			case 'B':
				NKEY("Backdoors",        clan->num_backdoors,     fread_number(fp));
				NKEY("Banks",            clan->num_banks,		fread_number(fp));
				SKEY("Bamfin",           clan->bamfin,			fread_string(fp));
				SKEY("Bamfout",          clan->bamfout,			fread_string(fp));
				SKEY("Bamfself",         clan->bamfself,		fread_string(fp));
				break;

			case 'C':
				NKEY( "Coffers",         clan->coffers,			fread_number(fp));
				break;

			case 'E':
				SKEY( "Email",           clan->email,			fread_string(fp));
				if (!strcmp(word, "End"))
				{
					pop_call();
					return;
				}
				break;

			case 'F':
				SKEY( "FileName",        clan->filename,		fread_string(fp));
				NKEY( "Flags",           clan->flags,             fread_number(fp));
				NKEY( "Founder",         clan->founder_pvnum,	fread_number(fp));
				break;

			case 'G':
				NKEY( "God",             clan->god,               fread_number(fp));
				NKEY( "Guards",		clan->num_guards,		fread_number(fp));
				break;

			case 'H':
				NKEY( "Home",			clan->home,			fread_number(fp));
				NKEY( "Healers",		clan->num_healers,		fread_number(fp));
				break;

			case 'L':
				SKEY( "Leader0",		clan->leader[0],		fread_string(fp));
				SKEY( "Leader1",		clan->leader[1],		fread_string(fp));
				SKEY( "Leader2",		clan->leader[2],		fread_string(fp));
				SKEY( "Leader3",		clan->leader[3],		fread_string(fp));
				SKEY( "Leader4",		clan->leader[4],		fread_string(fp));
				break;

			case 'M':
				NKEY("MDeaths",		clan->mdeaths,			fread_number(fp));
				NKEY("MKills",			clan->mkills,			fread_number(fp));
				NKEY("Members",		clan->members,			fread_number(fp));
				NKEY("Morgues",		clan->num_morgues,		fread_number(fp));
				SKEY("Motto",			clan->motto,			fread_string(fp));
				break;

			case 'N':
				SKEY( "Name",			clan->name,			fread_string(fp));
				break;

			case 'P':
				AKEY("PDeath",			clan->pdeaths,			fread_number(fp));
				AKEY("PKill",			clan->pkills,			fread_number(fp));
				break;

			case 'S':
				NKEY("Store",			clan->store,			fread_number(fp));
				break;

			case 'T':
				NKEY("Tax",			clan->tax,			fread_number(fp));
				NKEY("Trainers",		clan->num_trainers,		fread_number(fp));
				break;
		}

		if (fMatch == FALSE)
		{
			log_printf("fread_clan: no match: %s %s", word, fread_line(fp));
		}
	}
	pop_call();
	return;
}


bool load_clan( char *clanname )
{
	char filename[256];
	CLAN_DATA *clan;
	FILE *fp;
	char found, loop, letter, *word;

	push_call("load_clan(%p)",clanname);

	log_printf("Loading %s..", clanname);

	ALLOCMEM(clan, CLAN_DATA, 1);

	clan->name		= STRALLOC( "" );
	clan->description	= STRALLOC( "" );
	clan->motto		= STRALLOC( "" );
	clan->leader[0]	= STRALLOC( "" );
	clan->leader[1]	= STRALLOC( "" );
	clan->leader[2]	= STRALLOC( "" );
	clan->leader[3]	= STRALLOC( "" );
	clan->leader[4]	= STRALLOC( "" );
	clan->email		= STRALLOC( "" );
	clan->bamfin		= STRALLOC( "$n appears in the room."			);
	clan->bamfout		= STRALLOC( "$n turns sideways and disappears."	);
	clan->bamfself		= STRALLOC( "You turn sideways and disappear."	);

	found = FALSE;

	sprintf(filename, "%s/%s", CLAN_DIR, clanname);

	if ((fp = my_fopen(filename, "r", FALSE)) != NULL)
	{
		for (found = loop = TRUE ; loop ; )
		{
			letter = fread_letter( fp );

			if (letter != '#')
			{
				log_printf("Load_clan_file: # not found: %s", clanname);
				break;
			}

			word = fread_word(fp);

			switch (word[0])
			{
				case 'C':
					fread_clan( clan, fp );
					break;
				case 'P':
					load_clan_pit(fp);
					break;
				case 'E':
					loop = FALSE;
					break;
				default:
					log_printf("load_clan: bad section: %s", word);
					loop = FALSE;
					break;
			}
		}
		my_fclose( fp );
	}

	sort_clans ( clan );

	pop_call();
	return found;
}

void load_clans( )
{
	FILE *fp;
	char *filename;
	char clanlist[50], letter;

	push_call("load_clans()");

	log_string( "Loading clans..." );

	sprintf(clanlist, "%s/clan.lst", CLAN_DIR);

	close_reserve();

	if ((fp = my_fopen(clanlist, "r", FALSE)) == NULL)
	{
		perror(clanlist);
		exit (1);
	}

	for ( ; ; )
	{
		letter = fread_letter(fp);

		if (letter != 'C')
		{
			if (letter != '$')
			{
				log_printf("***WARNING*** load_clans: bad format");
			}
			break;
		}

		filename = fread_word(fp);

		if (load_clan(filename) == FALSE)
		{
			log_printf("load_clans: failed to load %s", filename);
		}
	}
	my_fclose(fp);

	open_reserve();

	pop_call();
	return;
}

void do_initiate( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CLAN_DATA *clan;

	push_call("do_initiate(%p,%p)",ch,argument);

	if (!is_clan_leader(ch))
	{
		ch_printf(ch, "You must be a clan leader to initiate.\n\r");
		pop_call();
		return;
	}

	clan = ch->pcdata->clan;

	argument = one_argument(argument, arg);

	if (arg[0] == '\0')
	{
		ch_printf(ch, "Initiate whom?\n\r");
		pop_call();
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		ch_printf(ch, "They aren't here.\n\r");
		pop_call();
		return;
	}

	if (IS_NPC(victim))
	{
		ch_printf(ch, "%s cannot be initiated.\n\r", capitalize(get_name(victim)));
		pop_call();
		return;
	}

	if (IS_SET(victim->act, PLR_OUTCAST))
	{
		ch_printf(ch, "%s is an outcast, look elsewhere for new recruiters.\n\r", get_name(victim));
		pop_call();
		return;
	}

	if (victim->pcdata->god != GOD_NEUTRAL && victim->pcdata->god != clan->god)
	{
		ch_printf(ch, "You cannot invite %s followers into the clan.\n\r", god_table[victim->pcdata->god]);
		pop_call();
		return;
	}

	if (IS_IMMORTAL(victim))
	{
		send_to_char( "You cannot initiate such a divine being.\n\r", ch );
		pop_call();
		return;
	}

	if (victim->level < 20)
	{
		ch_printf(ch, "%s is not ready to be initiated yet.\n\r", get_name(victim));
		pop_call();
		return;
	}

	if (clan->coffers < 5000000)
	{
		send_to_char("Your clan is too poor to pay for this initiation.\n\r", ch);
		pop_call();
		return;
	}

	if (victim->pcdata->clan)
	{
		ch_printf(ch, "%s is already a member of a clan.\n\r", get_name(victim));
		pop_call();
		return;
	}

	RESTRING(victim->pcdata->clan_pledge, clan->name);
	ch->pcdata->clan->coffers -= 5000000;

	ch_printf(ch,  "Your clan pays 5,000,000 gold coins on an initiation rite for %s.\n\r", get_name(victim));
	act( "$n initiates $N in the ways of $t.", ch, clan->name, victim, TO_NOTVICT );
	act( "$n initiates you in the ways of $t.\n\rYou must pledge your support to $t to become a full member.", ch, clan->name, victim, TO_VICT );

	pop_call();
	return;
}

void do_nominate( CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CLAN_DATA *clan;
	int position = 0;

	push_call("do_nominate(%p,%p)",ch,argument);

	if (IS_NPC(ch) || ch->pcdata->clan == NULL)
	{
		send_to_char("You are not a member of a clan.\n\r", ch );
		pop_call();
		return;
	}
	clan = ch->pcdata->clan;

	if (strcasecmp(ch->name, clan->leader[0]))
	{
		ch_printf(ch, "Only %s can nominate new leaders.\n\r", clan->leader[0]);
		pop_call();
		return;
	}

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0')
	{
		for (position = 1 ; position <= 4 ; position++)
		{
			ch_printf(ch, " %d: %s\n\r", position, clan->leader[position]);
		}
		pop_call();
		return;
	}

	if (arg2[0] == '\0')
	{
		send_to_char( "Nominate whom?\n\r", ch );
		pop_call();
		return;
	}

	if ((victim = get_player_world(ch, arg1)) == NULL)
	{
		send_to_char( "They aren't here.\n\r", ch);
		pop_call();
		return;
	}

	if (victim == ch)
	{
		ch_printf(ch, "There's no need to nominate yourself.\n\r");
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "number1"))
	{
		position = 1;
	}
	else if (!strcasecmp(arg2, "number2"))
	{
		position = 2;
	}
	else if (!strcasecmp(arg2, "number3"))
	{
		position = 3;
	}
	else if (!strcasecmp(arg2, "number4"))
	{
		position = 4;
	}
	else
	{
		ch_printf(ch, "Nominate %s for what position ?\n\rSyntax: nominate <player> <number1|number2|number3|number4>\n\r", arg1);
		pop_call();
		return;
	}

	if (IS_NPC(victim) || victim->pcdata->clan != clan)
	{
		ch_printf(ch, "%s isn't a member of %s\n\r", get_name(victim), clan->name);
		pop_call();
		return;
	}

	if (victim->pcdata->god != clan->god)
	{
		ch_printf(ch, "Only %s followers can be nominated as leaders.\n\r", god_table[clan->god]);

		pop_call();
		return;
	}

	if (victim->level < 20)
	{
		ch_printf(ch, "%s is too low a level to be a leader of %s\n\r", get_name(victim), clan->name);
		pop_call();
		return;
	}

	if (clan->coffers < 10000000)
	{
		send_to_char( "Your clan is too poor to pay for this nomination.\n\r", ch );
		pop_call();
		return;
	}

	if (victim->pcdata->clan_position)
	{
		RESTRING(clan->leader[victim->pcdata->clan_position], "");
	}

	RESTRING(clan->leader[position], victim->name);

	ch_printf(victim, "You now hold the position of number %d in %s.\n\r", position, clan->name);
	ch_printf(ch,     "%s now holds the position of number %d in %s.\n\r", get_name(victim), position, clan->name);

	clan->coffers -= 10000000;
	victim->pcdata->clan_position = position;

	pop_call();
	return;
}

void do_pledge( CHAR_DATA *ch, char *argument )
{
	CLAN_DATA *clan;

	push_call("do_pledge(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (IS_SET(ch->act, PLR_OUTCAST))
	{
		send_to_char( "You are an outcast and may not pledge support to a new clan.\n\r", ch);
		pop_call();
		return;
	}

	if (ch->level < 20)
	{
		send_to_char( "You are not worthy of joining yet.\n\r", ch );
		pop_call();
		return;
	}

	if ((clan = get_clan(argument)) == NULL)
	{
		send_to_char( "That clan does not exist!\n\r", ch );
		pop_call();
		return;
	}

	if (strcasecmp(clan->name, ch->pcdata->clan_pledge))
	{
		send_to_char( "That clan has not initiated you!\n\r", ch);
		pop_call();
		return;
	}

	if (ch->pcdata->clan)
	{
		send_to_char( "You are already in a clan!\n\r", ch );
		pop_call();
		return;
	}

	if (ch->pcdata->god != GOD_NEUTRAL && ch->pcdata->god != clan->god)
	{
		ch_printf(ch, "%s followers may not join %s clans.", god_table[ch->pcdata->god], god_table[clan->god]);

		pop_call();
		return;
	}

	clan->members++;

	RESTRING(ch->pcdata->clan_name, clan->name);
	RESTRING(ch->pcdata->clan_pledge, "");
	ch->pcdata->clan			= clan;
	ch->pcdata->clan_position	= 0;

	act( "You pledge support to $t.", ch, clan->name, NULL, TO_CHAR );
	act( "$n pledges support to $t.", ch, clan->name, NULL, TO_ROOM );

	pop_call();
	return;
}

void do_renounce( CHAR_DATA *ch, char *argument )
{
	CLAN_DATA *clan;

	push_call("do_renounce(%p,%p)",ch,argument);

	if (IS_NPC(ch) || ch->pcdata->clan == NULL)
	{
		send_to_char( "You aren't a member of a clan.\n\r", ch );
		pop_call();
		return;
	}

	clan = ch->pcdata->clan;

	if (argument[0] == '\0')
	{
		send_to_char("You must use a password with this command.\n\rThis command will cut your ties with your clan.\n\r", ch);
		pop_call();
		return;
	}

	if (encrypt64(argument) != ch->pcdata->password)
	{
		send_to_char( "That was not your password.\n\r", ch);
		pop_call();
		return;
	}

	ch_printf(ch, "You turn your back on %s.\n\r", ch->pcdata->clan_name);

	if (!strcasecmp(ch->name, ch->pcdata->clan->leader[0]))
	{
		destroy_clan(ch->pcdata->clan);
		pop_call();
		return;
	}

	clan->members--;

	if (is_clan_leader(ch))
	{
		RESTRING(ch->pcdata->clan->leader[ch->pcdata->clan_position], "");
	}
	ch->pcdata->clan = NULL;
	RESTRING(ch->pcdata->clan_name, "");

	pop_call();
	return;
}

void do_outcast( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	CLAN_DATA *clan;
	char buf[MAX_STRING_LENGTH];

	push_call("do_outcast(%p,%p)",ch,argument);

	if (!is_clan_leader(ch))
	{
		send_to_char("You must be a clan leader to outcast.\n\r", ch );
		pop_call();
		return;
	}

	clan = ch->pcdata->clan;

	if (*argument == '\0')
	{
		send_to_char( "Outcast whom?\n\r", ch );
		pop_call();
		return;
	}

	if ((victim = get_player_world(ch, argument)) == NULL)
	{
		ch_printf(ch, "%s does not seem to be playing", argument);
		pop_call();
		return;
	}

	if (victim == ch)
	{
		ch_printf(ch, "Use renounce instead.\n\r");
		pop_call();
		return;
	}

	if (victim->pcdata->clan != ch->pcdata->clan)
	{
		ch_printf(ch, "%s does not belong to your clan.\n\r", get_name(victim));
		pop_call();
		return;
	}

	if (is_clan_leader(victim) && strcasecmp(ch->name, clan->leader[0]))
	{
		ch_printf(ch, "Only %s may outcast clan leaders.", clan->leader[0]);
		pop_call();
		return;
	}

	clan->members--;

	if (is_clan_leader(victim))
	{
		RESTRING(clan->leader[victim->pcdata->clan_position], "");
	}
	RESTRING(victim->pcdata->clan_name, "");

	victim->pcdata->clan = NULL;

	char_to_room(victim, ROOM_VNUM_TEMPLE);

	victim->pcdata->recall = victim->pcdata->death_room = ROOM_VNUM_TEMPLE;

	SET_BIT(victim->act, PLR_OUTCAST);
	victim->pcdata->outcast_played = victim->pcdata->played;

	sprintf(buf, "%s has been kicked out of %s!", victim->name, clan->name);
	do_echo(ch, buf);

	pop_call();
	return;
}

void do_clans( CHAR_DATA *ch, char *argument )
{
	CLAN_DATA *clan;
	int count = 0;

	push_call("do_clans(%p,%p)",ch,argument);

	if (argument[0] == '\0')
	{
		send_to_char_color( "{128}\n\rClan          Founder            Pkills         Pdeaths       Members\n\r", ch);
		send_to_char_color( "{118}--------------------------------------------------------------------------------\n\r", ch );
		for (clan = mud->f_clan ; clan ; clan = clan->next)
		{
			ch_printf_color(ch, "{138}%-13s %-13s  %10d      %10d    %10d\n\r",
				clan->name,
				clan->leader[0],
				clan->pkills[0]+clan->pkills[1]+clan->pkills[2]+clan->pkills[3]+clan->pkills[4],
				clan->pdeaths[0]+clan->pdeaths[1]+ clan->pdeaths[2]+clan->pdeaths[3]+clan->pdeaths[4],
				clan->members);
			count++;
		}
		if (!count)
		{
			send_to_char_color( "{118}--------------------------------------------------------------------------------\n\r{138}There are no Clans currently formed.\n\r", ch);
		}
		else
		{
			send_to_char_color( "{118}--------------------------------------------------------------------------------\n\r", ch);
		}
		pop_call();
		return;
	}
	clan = get_clan( argument );
	if (!clan)
	{
		send_to_char_color( "{118}No such clan.\n\r", ch );
		pop_call();
		return;
	}
	ch_printf_color( ch, "{118}%s, '%s'\n\r\n\r", clan->name, clan->motto );
	ch_printf_color( ch, "{078}   Victories:{178} %10d                      {078}         Defeats:{178} %10d\n\r", clan->pkills[CLAN_NEUTRAL_KILL] + clan->pkills[CLAN_CHAOS_KILL] + clan->pkills[CLAN_ORDER_KILL], clan->pkills[CLAN_NEUTRAL_DEATH] + clan->pkills[CLAN_CHAOS_DEATH] + clan->pkills[CLAN_ORDER_DEATH]);
	ch_printf_color( ch, "{078}     Neutral:{178} %10d                      {078}         Neutral:{178} %10d\n\r", clan->pkills[CLAN_NEUTRAL_KILL], clan->pdeaths[CLAN_NEUTRAL_DEATH]);
	ch_printf_color( ch, "{078}       Chaos:{178} %10d                      {078}           Chaos:{178} %10d\n\r", clan->pkills[CLAN_CHAOS_KILL],   clan->pdeaths[CLAN_CHAOS_DEATH]);
	ch_printf_color( ch, "{078}       Order:{178} %10d                      {078}           Order:{178} %10d\n\r", clan->pkills[CLAN_ORDER_KILL],   clan->pdeaths[CLAN_ORDER_DEATH]);

	ch_printf( ch, "\n\r");

	ch_printf_color( ch, "{078}Clan Leaders: {178}%s %s %s %s %s\n\r",
		clan->leader[0],
		clan->leader[1],
		clan->leader[2],
		clan->leader[3],
		clan->leader[4]);
	ch_printf_color( ch, "{078}       Email: {178}%s\n\r", clan->email );

	if (IS_IMMORTAL(ch) || (is_clan_leader(ch) && ch->pcdata->clan == clan))
	{
		ch_printf_color( ch, "{078}      Bamfin: {178}%s\n\r", clan->bamfin );
		ch_printf_color( ch, "{078}     Bamfout: {178}%s\n\r", clan->bamfout );
		ch_printf_color( ch, "{078}    Bamfself: {178}%s\n\r", clan->bamfself );

		ch_printf( ch, "\n\r");

		ch_printf_color( ch, "{078}     Members:{178} %10d{078}     Tax Rate:{178} %10d{078}      Coffers:{178} %10lld\n\r",	clan->members, clan->tax, clan->coffers );
		ch_printf_color( ch, "{078}      Guards:{178} %10d{078}      Healers:{178} %10d{078}   Back Doors:{178} %10d\n\r",		clan->num_guards, clan->num_healers, clan->num_backdoors);
		ch_printf_color( ch, "{078}    Trainers:{178} %10d{078}        Banks:{178} %10d{078}      Morgues:{178} %10d\n\r", 	clan->num_trainers, clan->num_banks, clan->num_morgues);
		ch_printf_color( ch, "{078}      Altars:{178} %10d{078}\n\r", clan->num_altars);
	}

	if (IS_IMMORTAL(ch))
	{
		ch_printf_color( ch, "{078}        Home:{178} %10d{078}        Store:{178} %10d{078} Founder Vnum:{178} %10d\n\r",	clan->home, clan->store, clan->founder_pvnum);
		ch_printf_color( ch, "{078}   Mob Kills:{178} %10d{078}   Mob Deaths:{178} %10d\n\r", clan->mkills, clan->mdeaths);
	}
	pop_call();
	return;
}

void do_rent( CHAR_DATA *ch, char *argument )
{
	CLAN_DATA *clan;
	int rent = RENT_BASIC_CLAN_HALL;

	push_call("do_rent(%p,%p)",ch,argument);

	if (!IS_IMMORTAL(ch) && !is_clan_leader(ch))
	{
		send_to_char("Only clan leaders can see the clan's rent.\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_IMMORTAL(ch))
	{
		clan = ch->pcdata->clan;
	}
	else if (argument[0] == '\0' || (clan = get_clan(argument)) == NULL)
	{
		send_to_char("Syntax: rent <clan>\n\r", ch);
		pop_call();
		return;
	}

	ch_printf_color(ch, "{128}Type            Amount           Cost          Total\n\r");
	ch_printf_color(ch, "{118}--------------------------------------------------------------------------------\n\r");
	ch_printf_color(ch, "{178}%-14s{138} %7d %'14d %'14s\n\r",
		"Basic Hall", 1, RENT_BASIC_CLAN_HALL, tomoney(RENT_BASIC_CLAN_HALL));

	if (clan->num_healers > 0)
	{
		ch_printf_color(ch, "{178}%-14s{138} %7d %14d %14s\n\r",
			"Healers", clan->num_healers,	RENT_PER_HEALER, tomoney(RENT_PER_HEALER * clan->num_healers));
		rent += RENT_PER_HEALER * clan->num_healers;
	}
	if (clan->num_guards > 0)
	{
		ch_printf_color(ch, "{178}%-14s{138} %7d %14d %14s\n\r",
			"Guards", clan->num_guards, RENT_PER_GUARD, tomoney(RENT_PER_GUARD * clan->num_guards));
		rent += RENT_PER_GUARD * clan->num_guards;
	}
	if (clan->num_backdoors > 0)
	{
		ch_printf_color(ch, "{178}%-14s{138} %7d %14d %14s\n\r",
			"Backdoors", clan->num_backdoors, RENT_PER_BACKDOOR, tomoney(RENT_PER_BACKDOOR * clan->num_backdoors));
		rent += clan->num_backdoors * RENT_PER_BACKDOOR;
	}
	if (clan->num_trainers > 0)
	{
		ch_printf_color(ch, "{178}%-14s{138} %7d %14d %14s\n\r",
			"Trainers", clan->num_trainers, RENT_PER_TRAINER, tomoney(RENT_PER_TRAINER * clan->num_trainers));
		rent += RENT_PER_TRAINER * clan->num_trainers;
	}
	if (clan->num_banks > 0)
	{
		ch_printf_color(ch, "{178}%-14s{138} %7d %14d %14s\n\r",
			"Banks", clan->num_banks, RENT_PER_BANK, tomoney(RENT_PER_BANK * clan->num_banks));
		rent += RENT_PER_BANK * clan->num_banks;
	}
	if (clan->num_morgues > 0)
	{
		ch_printf_color(ch, "{178}%-14s{138} %7d %14d %14s\n\r",
			"Morgues", clan->num_morgues, RENT_PER_MORGUE, tomoney(RENT_PER_MORGUE * clan->num_morgues));
		rent += RENT_PER_MORGUE * clan->num_morgues;
	}
	if (clan->num_altars > 0)
	{
		ch_printf_color(ch, "{178}%-14s{138} %7d %14d %14s\n\r",
			"Altars", clan->num_altars, RENT_PER_ALTAR, tomoney(RENT_PER_ALTAR * clan->num_altars));
		rent += RENT_PER_ALTAR * clan->num_altars;
	}
	ch_printf_color(ch, "{118}--------------------------------------------------------------------------------\n\r");
	ch_printf_color(ch, "{128}%-14s{138} %7s %14s %14s\n\r",
			"Total", "", "", tomoney(rent));
	ch_printf_color(ch, "{118}--------------------------------------------------------------------------------\n\r");
	ch_printf_color(ch, "{168}Rent is due on every saturday night.\n\r");
	pop_call();
	return;
}

void do_gohome( CHAR_DATA *ch, char *argument )
{
	ROOM_INDEX_DATA *location;

	push_call("do_gohome(%p,%p)",ch,argument);

	if (IS_NPC(ch) || ch->pcdata->clan == NULL)
	{
		send_to_char("Only clan members can gohome.\n\r", ch );
		pop_call();
		return;
	}

	if (ch->fighting != NULL)
	{
		send_to_char( "You can't gohome while fighting!\n\r", ch);
		pop_call();
		return;
	}

	if (ch->hit < ch->max_hit * 0.75)
	{
		send_to_char( "You are too badly injured to gohome.\n\r", ch);
		pop_call();
		return;
	}

	if ((location = get_room_index(ch->pcdata->clan->home)) == NULL)
	{
		send_to_char( "Your clan doesn't have a home!\n\r", ch );
		pop_call();
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_NO_GOHOME))
	{
		send_to_char( "You cannot do that in this room.\n\r", ch);
		pop_call();
		return;
	}
	else if (IS_SET(ch->in_room->area->flags, AFLAG_NOGOHOME))
	{
		send_to_char( "You cannot do that in this area.\n\r", ch);
		pop_call();
		return;
	}

	if (ch->in_room == location)
	{
		send_to_char( "You are already at home.\n\r", ch);
		pop_call();
		return;
	}

	wait_state(ch, PULSE_PER_SECOND);

	act( ch->pcdata->clan->bamfself, ch, NULL, NULL, TO_CHAR );
	act( ch->pcdata->clan->bamfout,  ch, NULL, NULL, TO_ROOM );

	char_from_room( ch );
	char_to_room( ch, location->vnum );

	act( ch->pcdata->clan->bamfin,      ch, NULL, NULL, TO_ROOM );

	do_look(ch, "auto");

	pop_call();
	return;
}

void do_clanwhere( CHAR_DATA *ch, char *argument )
{
	PLAYER_GAME *fpl;
	char buf[MAX_STRING_LENGTH];

	push_call("do_clanwhere(%p,%p)",ch,argument);

	if (!is_clan_leader(ch))
	{
		send_to_char( "You are not a clan leader.\n\r", ch );
		pop_call();
		return;
	}

	sprintf(buf, "{128}%-12s  %-38s  %-24s\n\r", "Player", "Location", "Area");

	strcat(buf, "{118}--------------------------------------------------------------------------------");

	for (fpl = mud->f_player ; fpl ; fpl = fpl->next)
	{
		if (fpl->ch->pcdata->clan && fpl->ch->pcdata->clan == ch->pcdata->clan)
		{
			cat_snprintf(buf, 19, "\n\r{178}%-12s", fpl->ch->name);
			cat_snprintf(buf, 40, "  %-38s",        fpl->ch->in_room->name);
			cat_snprintf(buf, 26, "  %-24s",        fpl->ch->in_room->area->name);
		}
	}
	strcat(buf, "\n\r{118}--------------------------------------------------------------------------------\n\r");
	send_to_char_color(buf, ch);
	pop_call();
	return;
}

void do_makeclan( CHAR_DATA *ch, char *argument )
{
	char filename[256];
	char buf[MAX_INPUT_LENGTH];
	CLAN_DATA *clan;
	char clan_filenm[MAX_INPUT_LENGTH];
	int i;

	push_call("do_makeclan(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (ch->level < 90)
	{
		send_to_char("You must be level 90 or higher to make a clan.\n\r", ch);
		pop_call();
		return;
	}

	if (which_god(ch) == GOD_NEUTRAL)
	{
		send_to_char("You must be a God follower to make a clan.\n\r", ch);
		pop_call();
		return;
	}

	if (argument[0] == '\0')
	{
		send_to_char( "Usage: makeclan <clan name>\n\r", ch );
		pop_call();
		return;
	}

	if (strlen(argument) > 10 || strlen(argument) < 5)
	{
		send_to_char( "The name of your clan must range from 5 to 10 characters.\n\r", ch);
		pop_call();
		return;
	}

	for (i = 0 ; i < strlen(argument) ; i++)
	{
		if (argument[i] >= 'a' && argument[i] <= 'z')
		{
			continue;
		}
		if (argument[i] >= 'A' && argument[i] <= 'Z')
		{
			continue;
		}
		if (argument[i] == ' ')
		{
			continue;
		}
		send_to_char( "The name of your clan must only contain characters.\n\r", ch);
		pop_call();
		return;
	}

	for (i = 0 ; i < strlen(argument) ; i++)
	{
		argument[i] = LOWER(argument[i]);
	}

	if (ch->pcdata->castle)
	{
		send_to_char("You already have a castle established, contact the Gods.\n\r", ch);
		pop_call();
		return;
	}

	if ((clan = get_clan(argument)) != NULL)
	{
		send_to_char( "That clan already exist!\n\r", ch );
		pop_call();
		return;
	}

	if (ch->pcdata->clan != NULL)
	{
		send_to_char( "You are already in a clan!\n\r", ch );
		pop_call();
		return;
	}

	if (ch->pcdata->account < CLANHALL_CONSTRUCTION)
	{
		ch_printf(ch, "There is not enough money in your bank account to make a clan!\n\rIt costs %d.\n\r", CLANHALL_CONSTRUCTION);
		pop_call();
		return;
	}

	if (get_help(ch, argument) != NULL)
	{
		ch_printf(ch, "A help file with that name already exists.\n\r");
		pop_call();
		return;
	}

	sprintf(buf, "The Castle of %s", argument);

	do_castle_entrance(ch, buf);

	if (ch->pcdata->castle == NULL)
	{
		send_to_char("You failed to create a castle, contact the Gods.\n\r", ch);
		pop_call();
		return;
	}

	make_helpfile(capitalize_all(argument), get_room_index(ROOM_VNUM_CASTLE)->area);

	ch->pcdata->account -= CLANHALL_CONSTRUCTION;

	for (i = 0 ; i < strlen(argument) ; i++)
	{
		argument[i] = LOWER(argument[i]);
	}

	/*
		replace spaces by underscores from arg for clan filename
		20-01-2001 - Manwe
	*/

	strcpy(clan_filenm, argument);

	for (i = 0 ; i < strlen(clan_filenm) ; i++)
	{
		if (clan_filenm[i] == ' ')
		{
			clan_filenm[i] = '_';
		}
	}
	sprintf(filename, "%s.cln", clan_filenm);

	ALLOCMEM(clan, CLAN_DATA, 1);

	argument[0]         = UPPER(argument[0]);

	clan->name          = STRALLOC( argument);
	clan->motto         = STRALLOC( "" );
	clan->description   = STRALLOC( "" );
	clan->leader[0]     = STRALLOC( ch->name );
	clan->leader[1]     = STRALLOC( "" );
	clan->leader[2]     = STRALLOC( "" );
	clan->leader[3]     = STRALLOC( "" );
	clan->leader[4]     = STRALLOC( "" );
	clan->filename      = STRALLOC( filename );
	clan->email         = STRALLOC( "" );
	clan->bamfout       = STRALLOC( "$n turns sideways and disappears.");
	clan->bamfin        = STRALLOC( "$n appears in the room." );
	clan->bamfself      = STRALLOC( "You turn sideways and disappear.");
	clan->home          = ch->pcdata->castle->entrance;
	clan->founder_pvnum = ch->pcdata->pvnum;
	clan->god           = ch->pcdata->god;
	clan->members       = 1;
	ch->pcdata->clan    = clan;

	sort_clans ( clan );

	RESTRING(ch->pcdata->clan_name, clan->name);

	SET_BIT(room_index[clan->home]->room_flags, ROOM_NOTE_BOARD);

	save_clan(clan);
	save_clan_list();

	CHECK_AUTOSAVE(room_index[ROOM_VNUM_CASTLE]->area);

	sprintf(buf, "The clan of %s has been formed!", clan->name);

	do_echo(NULL, buf);

	log_printf("%s created the clan %s (filename: %s)", ch->name, clan->name, clan->filename);

	pop_call();
	return;
}

void destroy_clan(CLAN_DATA *deadclan)
{
	PLAYER_GAME		*fpl;
	ROOM_INDEX_DATA	*room;
	EXIT_DATA			*exit;
	CHAR_DATA			*mob;
	HELP_DATA			*help;
	int vnum, door;

	push_call("destroy_clan(%p)", deadclan);

	log_printf("destroy_clan: deleting %s", deadclan->name);

	save_clan(deadclan);

	for (mob = mud->f_char ; mob ; mob = mob->next)
	{
		if (IS_NPC(mob) && mob->pIndexData->creator_pvnum == deadclan->founder_pvnum)
		{
			if (IS_SET(mob->act, ACT_CLAN_HEALER))
			{
				REMOVE_BIT(mob->act, ACT_CLAN_HEALER);
				mob->pIndexData->act = mob->act;
			}

			if (IS_SET(mob->act, ACT_CLAN_GUARD))
			{
				REMOVE_BIT(mob->act, ACT_CLAN_GUARD);
				mob->pIndexData->act = mob->act;
			}
			if (IS_SET(mob->act, ACT_TRAIN))
			{
				REMOVE_BIT(mob->act, ACT_TRAIN);
				mob->pIndexData->act = mob->act;
			}
			if (IS_SET(mob->act, ACT_PRACTICE))
			{
				REMOVE_BIT(mob->act, ACT_PRACTICE);
				mob->pIndexData->act = mob->act;
			}
		}
	}

	for (help = room_index[ROOM_VNUM_CASTLE]->area->first_help ; help ; help = help->next)
	{
		if (!strcasecmp(help->keyword, deadclan->name))
		{
			delete_help(help);
			break;
		}
	}

	for (vnum = ROOM_VNUM_CASTLE ; vnum < MAX_VNUM ; vnum++)
	{
		if ((room = room_index[vnum]) != NULL
		&&  deadclan->founder_pvnum == room->creator_pvnum
		&&  IS_SET(room->room_flags, ROOM_IS_CASTLE))
		{
			if (IS_SET(room->room_flags, ROOM_BANK))
			{
				REMOVE_BIT(room->room_flags, ROOM_BANK);
			}
			if (IS_SET(room->room_flags, ROOM_MORGUE))
			{
				REMOVE_BIT(room->room_flags, ROOM_MORGUE);
			}
			if (IS_SET(room->room_flags, ROOM_ALTAR))
			{
				REMOVE_BIT(room->room_flags, ROOM_ALTAR);
			}

			for (door = 0 ; door <= 5 ; door++)
			{
				if ((exit = get_exit(room->vnum, door)) != NULL && IS_SET(exit->flags, EX_CLAN_BACKDOOR))
				{
					delete_exit(room, door);
				}
			}
		}
	}

	for (fpl = mud->f_player ; fpl ; fpl = fpl->next)
	{
		if (fpl->ch->pcdata->clan && fpl->ch->pcdata->clan == deadclan)
		{
			fpl->ch->pcdata->clan = NULL;
			RESTRING(fpl->ch->pcdata->clan_name, "");
		}
	}

	if ((room = get_room_index(deadclan->home)) != NULL)
	{
		REMOVE_BIT(room->room_flags, ROOM_NOTE_BOARD);
	}

	if ((room = get_room_index(deadclan->store)) != NULL)
	{
		REMOVE_BIT(room->room_flags, ROOM_CLAN_DONATION);
	}

	UNLINK(deadclan, mud->f_clan, mud->l_clan, next, prev);

	STRFREE(deadclan->name);
	STRFREE(deadclan->motto);
	STRFREE(deadclan->description);
	STRFREE(deadclan->leader[0]);
	STRFREE(deadclan->leader[1]);
	STRFREE(deadclan->leader[2]);
	STRFREE(deadclan->leader[3]);
	STRFREE(deadclan->leader[4]);
	STRFREE(deadclan->filename);
	STRFREE(deadclan->email);
	STRFREE(deadclan->bamfin);
	STRFREE(deadclan->bamfout);
	STRFREE(deadclan->bamfself);

	FREEMEM(deadclan);

	save_clan_list();

	CHECK_AUTOSAVE(room_index[ROOM_VNUM_CASTLE]->area);

	pop_call();
	return;
}

void do_clanset( CHAR_DATA *ch, char *argument )
{
	CLAN_DATA *clan = NULL;
	CHAR_DATA *sch;
	HELP_DATA *pHelp;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	push_call("do_clanset(%p,%p)",ch,argument);

	if (!IS_IMMORTAL(ch) && !is_clan_leader(ch))
	{
		send_to_char("You are not a clan leader.\n\r", ch);
		pop_call();
		return;
	}

	if (ch->desc && ch->pcdata->clan)
	{
		switch( ch->pcdata->editmode )
		{
			case MODE_HELP_EDIT:
				if ((pHelp = ch->pcdata->edit_ptr) == NULL)
				{
					log_printf("do_clanset: null edit_ptr");
					stop_editing( ch );
					pop_call();
					return;
				}
				STRFREE(pHelp->text );
				pHelp->text = copy_buffer(ch);
				stop_editing(ch);
				CHECK_AUTOSAVE(room_index[ROOM_VNUM_CASTLE]->area);
				pop_call();
				return;
		}
	}

	smash_tilde(argument);
	argument = one_argument_nolower(argument, arg1);

	if (IS_IMMORTAL(ch))
	{
		argument = one_argument_nolower(argument, arg2);
	}

	if (arg1[0] == '\0')
	{
		if (IS_IMMORTAL(ch))
		{
			send_to_char( "Syntax: clanset <clan> <field> <argument>\n\r", ch);
			send_to_char( "  Field being one of:\n\r", ch );
			send_to_char( "    leader number1 number2 number3 number4 members home store\n\r", ch);
			send_to_char( "    bamfin bamfout bamfself motto help email founder guards healers\n\r",  ch);
			send_to_char( "    backdoors trainers banks morgues altars coffers\n\r", ch);
		}
		else
		{
			send_to_char( "Syntax: clanset <motto|help|email|tax|storeroom|bamfin|bamfout|bamfself>\n\r", ch);
		}
		pop_call();
		return;
	}

	if (IS_IMMORTAL(ch))
	{
		clan = get_clan(arg1);
		strcpy(arg1, arg2);
	}
	else
	{
		clan = ch->pcdata->clan;
	}

	if (!clan)
	{
		send_to_char( "No such clan.\n\r", ch );
		pop_call();
		return;
	}

	if (!strcasecmp(arg1, "motto"))
	{
		RESTRING(clan->motto, argument);
		ch_printf_color(ch, "Motto set to: {118}'%s'\n\r", clan->motto);
		pop_call();
		return;
	}

	if (!strcasecmp(arg1, "email"))
	{
		RESTRING(clan->email, argument);
		ch_printf_color(ch, "Email set to: %s\n\r", clan->email);
		pop_call();
		return;
	}

	if (!strcasecmp(arg1, "bamfin"))
	{
		int i;

		for (i = 0 ; i < strlen(argument) ; i++)
		{
			if (argument[i] == '$')
			{
				switch (argument[i+1])
				{
					case 'n':
					case 'e':
					case 'm':
					case 's':
						break;
					default:
						ch_printf(ch, "You can only use $n $e $m $s in your bamfin message.\n\r");
						pop_call();
						return;
				}
			}
		}
		RESTRING(clan->bamfin, argument);
		ch_printf_color(ch, "Bamfin set to: %s\n\r", clan->bamfin);
		pop_call();
		return;
	}

	if (!strcasecmp(arg1, "bamfout"))
	{
		int i;

		for (i = 0 ; i < strlen(argument) ; i++)
		{
			if (argument[i] == '$')
			{
				switch (argument[i+1])
				{
					case 'n':
					case 'e':
					case 'm':
					case 's':
						break;
					default:
						ch_printf(ch, "You can only use $n $e $m $s in your bamfout message.\n\r");
						pop_call();
						return;
						break;
				}
			}
		}
		RESTRING(clan->bamfout, argument);
		ch_printf_color(ch, "Bamfout set to: %s\n\r", clan->bamfout);
		save_clan(clan);
		pop_call();
		return;
	}

	if (!strcasecmp(arg1, "bamfself"))
	{
		int i;

		for (i = 0 ; i < strlen(argument) ; i++)
		{
			if (argument[i] == '$')
			{
				ch_printf(ch, "You cannot use $ in your bamfself message.\n\r");
				pop_call();
				return;
			}
		}
		RESTRING(clan->bamfself, argument);
		ch_printf_color(ch, "Bamfself set to: %s\n\r", clan->bamfself);
		pop_call();
		return;
	}

	if (!strcasecmp(arg1, "storeroom"))
	{
		if (!IS_IMMORTAL(ch))
		{
			if (ch->in_room->creator_pvnum != ch->pcdata->pvnum)
			{
				send_to_char("You are NOT in your castle!\n\r", ch);
				pop_call();
				return;
			}
		}
		if (get_room_index(clan->store) != NULL)
		{
			REMOVE_BIT(room_index[clan->store]->room_flags, ROOM_CLAN_DONATION);
		}

		SET_BIT(ch->in_room->room_flags, ROOM_CLAN_DONATION);
		clan->store = ch->in_room->vnum;
		send_to_char( "Storeroom set.\n\r", ch );

		save_clan(clan);
		CHECK_AUTOSAVE(room_index[ROOM_VNUM_CASTLE]->area);

		pop_call();
		return;
	}

	if (!strcasecmp(arg1, "help"))
	{
		if (ch->desc == NULL)
		{
			log_printf("clanset description: no descriptor");
			pop_call();
			return;
		}

		if ((pHelp = get_help(ch, clan->name)) == NULL)
		{
			send_to_char("Your clan doesn't seem to have a help file, contact the immortals.\n\r", ch);
			pop_call();
			return;
		}

		switch( ch->pcdata->editmode )
		{
			default:
				bug( "do_clanset: bad mode");
				pop_call();
				return;

			case MODE_RESTRICTED:
				send_to_char( "You cannot use this command from while editing something else.\n\r",ch);
				pop_call();
				return;

			case MODE_NONE:
				ch->pcdata->edit_ptr = pHelp;
				ch->pcdata->editmode = MODE_HELP_EDIT;
				start_editing(ch, pHelp->text);
				pop_call();
				return;
		}
	}

	if (!strcasecmp(arg1, "tax"))
	{
		if (atoi(argument) < 0 || atoi(argument) > 50)
		{
			ch_printf(ch, "Tax rate must be between 0 and 50%%.\n\r");
			pop_call();
			return;
		}
		clan->tax = atoi( argument );
		ch_printf(ch, "Taxrate set to %d%%.\n\r",clan->tax);

		pop_call();
		return;
	}

	if (!IS_IMMORTAL(ch))
	{
		do_clanset(ch, "");
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "leader")
	||  !strcasecmp(arg2, "number1")
	||  !strcasecmp(arg2, "number2")
	||  !strcasecmp(arg2, "number3")
	||  !strcasecmp(arg2, "number4"))
	{
		int position;

		if ((sch = lookup_char(argument)) == NULL)
		{
			ch_printf(ch, "%s is not logged on right now.\n\r", argument);
			pop_call();
			return;
		}

		if (!strcasecmp(arg2, "leader"))
		{
			position = 0;
		}
		else if (!strcasecmp(arg2, "number1"))
		{
			position = 1;
		}
		else if (!strcasecmp(arg2, "number2"))
		{
			position = 2;
		}
		else if (!strcasecmp(arg2, "number3"))
		{
			position = 3;
		}
		else
		{
			position = 4;
		}

		if (sch->pcdata->clan != NULL)
		{
			sch->pcdata->clan->members--;
		}

		if (is_clan_leader(sch))
		{
			RESTRING(sch->pcdata->clan->leader[sch->pcdata->clan_position], "");
		}
		RESTRING(sch->pcdata->clan_name, clan->name);
		sch->pcdata->clan = clan;
		clan->members++;

		RESTRING(clan->leader[position], sch->name);

		sch->pcdata->clan_position = position;

		ch_printf(ch, "%s now holds leader position %d of %s\n\r", sch->name, position, clan->name);

		save_char_obj( sch, NORMAL_SAVE );
		save_clan(clan);

		pop_call();
		return;
	}

	if (!strcasecmp( arg2, "flags"))
	{
		clan->flags = atoi( argument );
		ch_printf(ch, "Clan flag set to: %d\n\r", clan->flags);
		save_clan(clan);
		pop_call();
		return;
	}

	if (!strcasecmp( arg2, "members"))
	{
		clan->members = atoi( argument );
		ch_printf(ch, "Amount of members set to: %d\n\r", clan->members);
		save_clan(clan);
		pop_call();
		return;
	}

	if (!strcasecmp( arg2, "home"))
	{
		clan->home = atoi( argument );
		ch_printf(ch, "Gohome room vnum set to: %d\n\r", clan->home);
		save_clan(clan);
		pop_call();
		return;
	}

	if (!strcasecmp( arg2, "founder"))
	{
		clan->founder_pvnum = atoi( argument );
		ch_printf(ch, "Founder pvnum set to: %d\n\r", clan->founder_pvnum);
		save_clan(clan);
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "guards"))
	{
		clan->num_guards = atoi(argument);
		ch_printf(ch, "Number of guards set to: %d\n\r", clan->num_guards);
		save_clan(clan);
		pop_call();
		return;
	}

	if (!strcasecmp( arg2, "healers"))
	{
		clan->num_healers = atoi(argument);
		ch_printf(ch, "Number of healers set to: %d\n\r", clan->num_healers);
		save_clan(clan);
		pop_call();
		return;
	}
	if (!strcasecmp(arg2, "backdoors"))
	{
		clan->num_backdoors = atoi(argument);
		ch_printf(ch, "Number of backdoors set to: %d\n\r", clan->num_backdoors);
		save_clan(clan);
		pop_call();
		return;
	}
	if (!strcasecmp(arg2, "trainers"))
	{
		clan->num_trainers = atoi(argument);
		ch_printf(ch, "Number of trainers set to: %d\n\r", clan->num_trainers);
		save_clan(clan);
		pop_call();
		return;
	}
	if (!strcasecmp(arg2, "banks"))
	{
		clan->num_banks = atoi(argument);
		ch_printf(ch, "Number of banks set to: %d\n\r", clan->num_banks);
		save_clan(clan);
		pop_call();
		return;
	}
	if (!strcasecmp(arg2, "morgues"))
	{
		clan->num_morgues = atoi(argument);
		ch_printf(ch, "Number of morgues set to: %d\n\r", clan->num_morgues);
		save_clan(clan);
		pop_call();
		return;
	}
	if (!strcasecmp(arg2, "altars"))
	{
		clan->num_altars = atoi(argument);
		ch_printf(ch, "Number of altars set to: %d\n\r", clan->num_altars);
		save_clan(clan);
		pop_call();
		return;
	}
	if (!strcasecmp(arg2, "coffers"))
	{
		clan->coffers = atoll(argument);
		ch_printf(ch, "Coffers set to: %lld\n\r", clan->coffers);
		save_clan(clan);
		pop_call();
		return;
	}

	do_clanset( ch, "" );
	pop_call();
	return;
}

void do_donate(CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj, *pit;
	ROOM_INDEX_DATA *location;
	CHAR_DATA *rch;
	char buf[MAX_STRING_LENGTH];

	push_call("do_donate(%p,%p)",ch,argument);

	if (IS_NPC(ch) || ch->pcdata->clan == NULL)
	{
		send_to_char("You are not a member of a clan.\n\r", ch);
		pop_call();
		return;
	}

	if (ch->fighting)
	{
		send_to_char( "You can't donate while fighting!\n\r", ch);
		pop_call();
		return;
	}

	if ((location = get_room_index(ch->pcdata->clan->store)) == NULL)
	{
		send_to_char("Your clan does not have a storeroom to recieve donated items.\n\r", ch);
		pop_call();
		return;
	}

	if ((obj = get_obj_carry(ch, argument)) == NULL)
	{
		send_to_char("What would you like to donate ?\n\r", ch);
		pop_call();
		return;
	}

	if (IS_OBJ_STAT(obj, ITEM_NODROP))
	{
		send_to_char( "You can't seem to let go of it.\n\r", ch );
		pop_call();
		return;
	}

	for (pit = location->first_content ; pit ; pit = pit->next_content)
	{
		if (pit->pIndexData->creator_pvnum == ch->pcdata->clan->founder_pvnum
		&&  pit->item_type == ITEM_CONTAINER)
		{
			if (count_total_objects(pit) > 150)
			{
				ch_printf(ch, "Your clan's donation pit is full.\n\r");
			}
			else
			{
				obj_from_char(obj);
				obj_to_obj(obj, pit);
	
				ch_printf(ch, "You chant the words to one of %s's rituals and watch %s slowly disappear...\n\r", ch->pcdata->clan->name, obj->short_descr);
	
				sprintf(buf, "%s slowly materializes from thin air, tumbling into %s.", obj->short_descr, pit->short_descr);
		
				for (rch = location->first_person ; rch ; rch = rch->next_in_room)
				{
					if (rch != ch && rch->position > POS_SLEEPING)
					{
						ch_printf(rch, "%s\n\r", capitalize(justify(buf, get_page_width(rch))));
					}
				}
			}
			pop_call();
			return;
		}
	}
	if (location->content_count >= MAX_OBJECTS_IN_ROOM)
	{
		ch_printf(ch, "Your clan's donation room is full.\n\r");
	}
	else
	{
		obj_from_char( obj );
		obj_to_room(obj, location->vnum);

		ch_printf(ch, "You chant the words to one of %s's rituals and watch the %s slowly disappear...\n\r", ch->pcdata->clan->name, obj->short_descr);

		sprintf(buf, "Before your very eyes, %s slowly appears.", obj->short_descr);

		for (rch = location->first_person ; rch ; rch = rch->next_in_room)
		{
			if (rch != ch && rch->position > POS_SLEEPING)
			{
				ch_printf(rch, "%s\n\r", justify(buf, get_page_width(rch)));
			}
		}
	}
	pop_call();
	return;
}

/*
	ranges between 200 and 1000M - Scandum
*/

long long max_coffer( CLAN_DATA *clan )
{
	push_call("max_coffer(%p)",clan);
	pop_call();
	return (URANGE(8, clan->members, 400)*25000000LL);
}

void do_coffer( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *target;
	char choice[MAX_STRING_LENGTH], amt[MAX_INPUT_LENGTH];
	char arg[MAX_STRING_LENGTH];
	long long amount, tar_max;

	push_call("do_coffer(%p,%p)",ch,argument);

	if (IS_NPC(ch) || ch->pcdata->clan == NULL)
	{
		send_to_char("You are not part of a clan.\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_SET(ch->in_room->room_flags, ROOM_BANK) && ch->in_room->vnum != ch->pcdata->clan->home)
	{
		send_to_char("You must be in a bank or the home of your clan to access the coffers.\n\r", ch);
		pop_call();
		return;
	}

	argument = one_argument(argument, choice);
	argument = one_argument(argument, amt);
	amount   = URANGE(0, atoi(amt), ch->level*1000000);

	switch (*choice)
	{
		case 'd':
			if (amount > ch->gold)
			{
				send_to_char("You don't have that much gold.\n\r", ch);
				pop_call();
				return;
			}
			else
			{
				if (ch->pcdata->clan->coffers >= max_coffer(ch->pcdata->clan))
				{
					ch_printf(ch, "The clan coffers are full!\n\r" );
					pop_call();
					return;
				}
				if (ch->pcdata->clan->coffers + amount > max_coffer(ch->pcdata->clan))
				{
					amount = max_coffer(ch->pcdata->clan) - ch->pcdata->clan->coffers;
				}
				ch->gold -= amount;
				ch->pcdata->clan->coffers += amount;
				save_char_obj( ch, NORMAL_SAVE );
				save_clan (ch->pcdata->clan);
				ch_printf(ch, "You deposit %lld gold coins into your clan's coffers.\n\r", amount);
				pop_call();
				return;
			}
			break;

		case 'w':
			if (!is_clan_leader(ch))
			{
				send_to_char("Only clan leaders can withdraw from the clan coffers.\n\r", ch);
				pop_call();
				return;
			}

			if (ch->gold > ch->level*1000000)
			{
				ch->gold = ch->level*1000000;
			}
			if (amount + ch->gold > ch->level*1000000)
			{
				amount = ch->level*1000000 - ch->gold;
			}
			if (amount > ch->pcdata->clan->coffers)
			{
				amount = ch->pcdata->clan->coffers;
			}
			ch->gold += amount;
			ch->pcdata->clan->coffers -= amount;
			save_char_obj( ch, NORMAL_SAVE );
			save_clan (ch->pcdata->clan);
			ch_printf(ch, "You withdraw %lld gold coins from the clan coffers.\n\r", amount);
			break;

		case 'b':
			if (!is_clan_leader(ch))
			{
				send_to_char("Only clan leaders can check the coffers.\n\r", ch);
				pop_call();
				return;
			}
			else
			{
				ch_printf(ch, "The coffers currently hold %lld gold coins.\n\r", ch->pcdata->clan->coffers);
			}
			break;

		case 't':
			if (!is_clan_leader(ch))
			{
				send_to_char("Only clan leaders can organize transfers from the coffers.\n\r", ch);
				pop_call();
				return;
			}
			argument = one_argument(argument, arg);
			if (*arg == '\0')
			{
				send_to_char("You must specify a target.\n\r", ch);
				pop_call();
				return;
			}
			if ((target = get_char_room(ch, arg)) == NULL)
			{
				send_to_char("They must be in the bank with you.\n\r", ch);
				pop_call();
				return;
			}
			if (IS_NPC(target))
			{
				send_to_char("Only players have accounts, silly.\n\r", ch);
				pop_call();
				return;
			}
			if (ch->pcdata->clan->coffers < amount)
			{
				amount = ch->pcdata->clan->coffers;
			}
			tar_max = (target->level + 5 + target->pcdata->reincarnation * 5) * 2000000;

			if (target->pcdata->account + amount > tar_max)
			{
				amount = tar_max - target->pcdata->account;
			}
			ch->pcdata->clan->coffers -= amount;
			target->pcdata->account   += amount;
			save_clan(ch->pcdata->clan);
			save_char_obj( ch, NORMAL_SAVE );
			save_char_obj( target, NORMAL_SAVE );
			ch_printf(ch, "You transfer %lld coins into %s's account.\n\rThe coffers now hold %lld gold coins.\n\r", amount, target->name, ch->pcdata->clan->coffers);
			ch_printf(target, "%s transfers %lld coins into your account.\n\rYour account balance is now %d gold coins.\n\r", ch->name, amount, target->pcdata->account);
			break;

		default:
			if (!is_clan_leader(ch))
			{
				send_to_char("Usage: coffer deposit <amount>\n\r", ch);
			}
			else
			{
				send_to_char("Usage: coffer <deposit|withdraw|balance|transfer> [amount] [target]\n\r", ch);
			}
			break;
	}
	pop_call();
	return;
}

void send_clan_message(CLAN_DATA *clan, char *message)
{
	NOTE_DATA *pnote;
	char buf[MAX_INPUT_LENGTH];

	push_call("send_clan_message(%p,%p)",clan,message);

	strcpy(buf, "Immortal");
	cat_sprintf(buf, " %s", clan->leader[0]);
	cat_sprintf(buf, " %s", clan->leader[1]);
	cat_sprintf(buf, " %s", clan->leader[2]);
	cat_sprintf(buf, " %s", clan->leader[3]);
	cat_sprintf(buf, " %s", clan->leader[4]);

	ALLOCMEM( pnote, NOTE_DATA, 1);

	pnote->next				= NULL;
	pnote->sender				= STRALLOC("Lola Messenger");
	pnote->date				= STRALLOC(get_time_string(mud->current_time));
	pnote->time				= mud->current_time;
	pnote->to_list				= STRALLOC(buf);
	pnote->subject				= STRALLOC("Important!");
	pnote->room_vnum			= clan->home;
	pnote->text				= STRALLOC(message);
	pnote->topic				= MAX_TOPIC -1;

	LINK(pnote, mud->f_note, mud->l_note, next, prev);

	pop_call();
	return;
}

void do_heal(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *mob;
	char arg[MAX_INPUT_LENGTH];
	int sn;
	char *words;

	push_call("(do_heal%p,%p)",ch,argument);

	/*
		check for healer
	*/

	for ( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
	{
		if ( IS_NPC(mob) && IS_SET(mob->act, ACT_CLAN_HEALER) )
		{
			break;
		}
	}

	if ( mob == NULL )
	{
		send_to_char( "You can't do that here.\n\r", ch );
		pop_call();
		return;
	}

	if (!is_own_clanhall(ch))
	{
		send_to_char("You can't do that here.\n\r",ch);
		pop_call();
		return;
	}

	one_argument(argument,arg);
	if (arg[0] == '\0')
	{
		ch_printf(ch, "%s%s says 'I offer the following spells:'\n\r", get_color_string(ch, COLOR_SPEACH, VT102_DIM), get_name(mob));
		send_to_char("  blind:   cure blindness\n\r",		ch);
		send_to_char("  poison:  cure poison\n\r",			ch);
		send_to_char("  curse:   remove curse\n\r",			ch);
		send_to_char("  dispel:  dispel magic\n\r",			ch);
		ch_printf(ch, "%s%s says 'Type: heal <type> to be healed.'\n\r", get_color_string(ch, COLOR_SPEACH, VT102_DIM), get_name(mob));
		pop_call();
		return;
	}

	if (!str_prefix(arg,"blindness"))
	{
		sn    = skill_lookup("cure blindness");
		words = "judicandus noselacri";
	}
	else if (!str_prefix(arg,"poison"))
	{
		sn    = skill_lookup("cure poison");
		words = "judicandus sausabru";
	}
	else if (!str_prefix(arg,"uncurse") || !str_prefix(arg,"curse"))
	{
		sn    = skill_lookup("remove curse");
		words = "candussido judifgz";
	}
	else if (!str_prefix(arg,"dispel"))
	{
		sn    = skill_lookup("dispel magic");
		words = "eugszer waouq";
	}
	else
	{
		ch_printf(ch, "%s%s says 'Type 'heal' for a list of spells.'\n\r", get_color_string(ch, COLOR_SPEACH, VT102_DIM), capitalize(mob->short_descr));
		pop_call();
		return;
	}

	wait_state(ch, PULSE_PER_SECOND);

	act("$n utters the words '$T'.",mob,NULL,words,TO_ROOM);

	if (sn == -1)
	{
		pop_call();
		return;
	}

	(*skill_table[sn].spell_fun) (sn, mob->level, mob, ch, TAR_CHAR_DEFENSIVE);

	pop_call();
	return;
}

bool is_own_clanhall(CHAR_DATA *ch)
{
	push_call("is_own_clanhall(%p)",ch);

	if (IS_NPC(ch))
	{
		pop_call();
		return TRUE;
	}

	if (IS_IMMORTAL(ch))
	{
		pop_call();
		return TRUE;
	}

	if (ch->pcdata->clan == NULL)
	{
		pop_call();
		return FALSE;
	}

	if (ch->pcdata->clan->founder_pvnum == ch->in_room->creator_pvnum)
	{
		pop_call();
		return TRUE;
	}
	else
	{
		pop_call();
		return FALSE;
	}
}

bool is_in_clanhall(CHAR_DATA *ch)
{
	CLAN_DATA *ctmp;

	push_call("is_in_clanhall(%p)",ch);

	if (!IS_SET(ch->in_room->room_flags, ROOM_IS_CASTLE))
	{
		pop_call();
		return FALSE;
	}

	for (ctmp = mud->f_clan ; ctmp ; ctmp = ctmp->next)
	{
		if (ctmp->founder_pvnum == ch->in_room->creator_pvnum)
		{
			pop_call();
			return TRUE;
		}
	}
	pop_call();
	return FALSE;
}

void do_forceren( CHAR_DATA *ch, char *argument )
{
	send_to_char( "If you want to forcerent, spell it out.\n\r", ch );
}

void do_forcerent( CHAR_DATA *ch, char *argument )
{
	CLAN_DATA			*clan, *next_clan;
	CHAR_DATA			*mob;
	EXIT_DATA			*exit;
	ROOM_INDEX_DATA	*room;
	char buf[MAX_INPUT_LENGTH];
	int vnum, door;
	long long rent;

	push_call("do_forcerent(%p,%p)",ch,argument);

	CHECK_AUTOSAVE(room_index[ROOM_VNUM_CASTLE]->area);

	for (clan = mud->f_clan ; clan ; clan = next_clan)
	{
		next_clan = clan->next;
		rent      = RENT_BASIC_CLAN_HALL;

		log_printf("Calculating rent for clan %s...", clan->name);

		rent += RENT_PER_GUARD		* clan->num_guards;
		rent += RENT_PER_HEALER		* clan->num_healers;
		rent += RENT_PER_TRAINER		* clan->num_trainers;
		rent += RENT_PER_BANK		* clan->num_banks;
		rent += RENT_PER_MORGUE		* clan->num_morgues;
		rent += RENT_PER_ALTAR		* clan->num_altars;
		rent += RENT_PER_BACKDOOR	* clan->num_backdoors;

		if (clan->coffers >= rent)
		{
			clan->coffers -= rent;
			sprintf(buf, "{128}You paid %lld in rent. Coffers now at %lld.\n\r", rent, clan->coffers);
			send_clan_message(clan, buf);
			log_printf("Clan %s paid %lld in rent, coffers at %lld", clan->name, rent, clan->coffers);
			continue;
		}
		else
		{
			buf[0] = '\0';
		}

		if (clan->num_healers > 0)
		{
			for (mob = mud->f_char ; mob ; mob = mob->next)
			{
				if (IS_NPC(mob) && mob->pIndexData->creator_pvnum == clan->founder_pvnum)
				{
					if (IS_SET(mob->act, ACT_CLAN_HEALER))
					{
						REMOVE_BIT(mob->act, ACT_CLAN_HEALER);
						mob->pIndexData->act = mob->act;
					}
				}
			}
			clan->coffers += clan->num_healers * CLANHALL_FLAG / 2;
			clan->num_healers = 0;
			strcat(buf, "{128}Your clan's healers were all fired to cover costs.\n\r");
			log_printf("Clan %s had their Healers fired, coffers at %lld", clan->name, clan->coffers);
		}
		if (clan->coffers >= rent)
		{
			clan->coffers -= rent;
			cat_sprintf(buf, "{128}You paid %lld in rent. Coffers now at %lld.\n\r", rent, clan->coffers);
			send_clan_message(clan, buf);
			log_printf("Clan %s paid %lld in rent, coffers at %lld", clan->name, rent, clan->coffers);
			continue;
		}
		if (clan->num_guards > 0)
		{
			for (mob = mud->f_char ; mob ; mob = mob->next)
			{
				if (IS_NPC(mob) && mob->pIndexData->creator_pvnum == clan->founder_pvnum)
				{
					if (IS_SET(mob->act, ACT_CLAN_GUARD))
					{
						REMOVE_BIT(mob->act, ACT_CLAN_GUARD);
						mob->pIndexData->act = mob->act;
					}
				}
			}
			clan->coffers += clan->num_guards*CLANHALL_FLAG / 2;
			clan->num_guards = 0;
			strcat(buf, "{128}Your clan's guards were all fired to cover costs.\n\r");
			log_printf("Clan %s had their Guards fired, coffers at %lld", clan->name, clan->coffers);
		}
		if (clan->coffers >= rent)
		{
			clan->coffers -= rent;
			cat_sprintf(buf, "{128}You paid %lld in rent. Coffers now at %lld.\n\r", rent, clan->coffers);
			send_clan_message(clan, buf);
			log_printf("Clan %s paid %lld in rent, coffers at %lld", clan->name, rent, clan->coffers);
			continue;
		}
		if (clan->num_trainers > 0)
		{
			for (mob = mud->f_char ; mob ; mob = mob->next)
			{
				if (IS_NPC(mob) && mob->pIndexData->creator_pvnum == clan->founder_pvnum)
				{
					if (IS_SET(mob->act, ACT_TRAIN))
					{
						REMOVE_BIT(mob->act, ACT_TRAIN);
						mob->pIndexData->act = mob->act;
					}
					if (IS_SET(mob->act, ACT_PRACTICE))
					{
						REMOVE_BIT(mob->act, ACT_PRACTICE);
						mob->pIndexData->act = mob->act;
					}
				}
			}
			clan->coffers += clan->num_trainers*CLANHALL_FLAG / 2;
			clan->num_trainers = 0;
			strcat(buf, "{128}Your clan's trainers were all fired to cover costs.\n\r");
			log_printf("Clan %s had their Trainers fired, coffers at %lld", clan->name, clan->coffers);
		}
		if (clan->coffers >= rent)
		{
			clan->coffers -= rent;
			cat_sprintf(buf, "{128}You paid %lld in rent. Coffers now at %lld.\n\r", rent, clan->coffers);
			send_clan_message(clan, buf);
			log_printf("Clan %s paid %lld in rent, coffers at %lld", clan->name, rent, clan->coffers);
			continue;
		}
		if (clan->num_banks > 0)
		{
			for (vnum = ROOM_VNUM_CASTLE ; vnum < MAX_VNUM ; vnum++)
			{
				if ((room = room_index[vnum]) != NULL
				&&  clan->founder_pvnum == room->creator_pvnum
				&&  IS_SET(room->room_flags, ROOM_IS_CASTLE))
				{
					if (IS_SET(room->room_flags, ROOM_BANK))
					{
						REMOVE_BIT(room->room_flags, ROOM_BANK);
					}
				}
			}
			clan->coffers += clan->num_banks*CLANHALL_FLAG / 2;
			clan->num_banks = 0;
			strcat(buf, "{128}Your clan's banks were all destroyed to cover costs.\n\r");
			log_printf("Clan %s had their Banks destroyed, coffers at %lld", clan->name, clan->coffers);
		}
		if (clan->coffers >= rent)
		{
			clan->coffers -= rent;
			cat_sprintf(buf, "{128}You paid %lld in rent. Coffers now at %lld.\n\r", rent, clan->coffers);
			send_clan_message(clan, buf);
			log_printf("Clan %s paid %lld in rent, coffers at %lld", clan->name, rent, clan->coffers);
			continue;
		}
		if (clan->num_morgues > 0)
		{
			for (vnum = ROOM_VNUM_CASTLE ; vnum < MAX_VNUM ; vnum++)
			{
				if ((room = room_index[vnum]) != NULL
				&&  clan->founder_pvnum == room->creator_pvnum
				&&  IS_SET(room->room_flags, ROOM_IS_CASTLE))
				{
					if (IS_SET(room->room_flags, ROOM_MORGUE))
					{
						REMOVE_BIT(room->room_flags, ROOM_MORGUE);
					}
				}
			}
			clan->coffers += clan->num_morgues*CLANHALL_FLAG / 2;
			clan->num_morgues = 0;
			strcat(buf, "{128}Your clan's morgues were all destroyed to cover costs.\n\r");
			log_printf("Clan %s had their Morgues destroyed, coffers at %lld", clan->name, clan->coffers);
		}
		if (clan->coffers >= rent)
		{
			clan->coffers -= rent;
			cat_sprintf(buf, "{128}You paid %lld in rent. Coffers now at %lld.\n\r", rent, clan->coffers);
			send_clan_message(clan, buf);
			log_printf("Clan %s paid %lld in rent, coffers at %lld", clan->name, rent, clan->coffers);
			continue;
		}
		if (clan->num_altars > 0)
		{
			for (vnum = ROOM_VNUM_CASTLE ; vnum < MAX_VNUM ; vnum++)
			{
				if ((room = room_index[vnum]) != NULL
				&&  clan->founder_pvnum == room->creator_pvnum
				&&  IS_SET(room->room_flags, ROOM_IS_CASTLE))
				{
					if (IS_SET(room->room_flags, ROOM_ALTAR))
					{
						REMOVE_BIT(room->room_flags, ROOM_ALTAR);
					}
				}
			}
			clan->coffers += clan->num_altars*CLANHALL_FLAG / 2;
			clan->num_altars = 0;
			strcat(buf, "{128}Your clan's altars were all destroyed to cover costs.\n\r");
			log_printf("Clan %s had their Morgues destroyed, coffers at %lld", clan->name, clan->coffers);
		}
		if (clan->coffers >= rent)
		{
			clan->coffers -= rent;
			cat_sprintf(buf, "{128}You paid %lld in rent. Coffers now at %lld.\n\r", rent, clan->coffers);
			send_clan_message(clan, buf);
			log_printf("Clan %s paid %lld in rent, coffers at %lld", clan->name, rent, clan->coffers);
			continue;
		}
		if (clan->num_backdoors > 0)
		{
			for (vnum = ROOM_VNUM_CASTLE ; vnum < MAX_VNUM ; vnum++)
			{
				if ((room = room_index[vnum]) != NULL
				&&  clan->founder_pvnum == room->creator_pvnum
				&&  IS_SET(room->room_flags, ROOM_IS_CASTLE))
				{
					for (door = 0 ; door <= 5 ; door++)
					{
						if ((exit = get_exit(room->vnum, door)) && IS_SET(exit->flags, EX_CLAN_BACKDOOR))
						{
							delete_exit(room, door);

							clan->coffers += COST_OF_CLANDOOR * 500;
							clan->num_backdoors--;
							cat_sprintf(buf, "{128}One backdoor was destroyed to cover costs.\n\r");
							log_printf("Clan %s had one Backdoor destroyed, coffers at %lld", clan->name, clan->coffers);
						}
						if (clan->coffers >= rent)
						{
							break;
						}
					}
				}
				if (clan->coffers >= rent)
				{
					break;
				}
			}
		}
		if (clan->coffers >= rent)
		{
			clan->coffers -= rent;
			cat_sprintf(buf, "{128}You paid %lld in rent. Coffers now at %lld.\n\r", rent, clan->coffers);
			send_clan_message(clan, buf);
			log_printf("Clan %s paid %lld in rent, coffers at %lld", clan->name, rent, clan->coffers);
			continue;
		}
		sprintf(buf, "Clan %s has been disbanded due to lack of adequate funding!\n\r", clan->name);
		do_echo(NULL, buf);
		save_clan(clan);
		destroy_clan(clan);
	}
	pop_call();
	return;
}

void do_clan_message( CHAR_DATA *ch, char *argument )
{
	CLAN_DATA *clan = NULL;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	argument = one_argument_nolower(argument, arg1);

	push_call("do_send_clan_message(%p,%p)",ch,argument);

	if (arg1[0] == '\0')
	{
		ch_printf(ch, "Syntax: clanmessage <clan> <message>\n\r");
		pop_call();
		return;
	}

	if ((clan = get_clan(arg1)) == NULL)
	{
		ch_printf(ch, "%s is not a valid clan!\n\r", arg1);
		pop_call();
		return;
	}

	if (argument[0] == '\0')
	{
		ch_printf(ch, "You should enter a message for %s.\n\r", arg1);
		pop_call();
		return;
	}

	sprintf(arg2, "%s\n\r", argument);

	send_clan_message(clan, arg2);

	pop_call();
	return;
}
