/***************************************************************************
 * Lola  1.0 by Igor van den Hoven.                                        *
 *                                                                         *
 * Emud  3.0 by Igor van den Hoven.                                        *
 ***************************************************************************/

#include <stdarg.h>
#include "mud.h"

sh_int	trivia_timer;
sh_int	morph_game;
bool		tag_game, tag_enabled;
char		*tagged_name;

/*
	Fun for the whole family - Scandum - 24-10-2002
*/

void trivia_echo(char *fmt, ...)
{
	PLAYER_GAME *plg;
	char buf[MAX_STRING_LENGTH], tmp[MAX_STRING_LENGTH], *pt;
	va_list args;

	push_call("trivia_echo(%p)",fmt);

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	if ((pt = strstr(buf, "\n\r")) != NULL)
	{
		*pt = '\0';
	}

	sprintf(tmp, "{078}[{028}T{128}R{178}IV{128}I{028}A{078}] {128}%s", buf);

	for (plg = mud->f_player ; plg ; plg = plg->next)
	{
		if (IS_SET(plg->ch->pcdata->channel, CHANNEL_TRIVIA))
		{
			ch_printf_color(plg->ch, "%s\n\r", ansi_justify(tmp, get_page_width(plg->ch)));
		}
	}

	if (pt)
	{
		trivia_echo((char *) buf + strlen(buf) + 2);
	}
	pop_call();
	return;
}

void morph_echo(char *fmt, ...)
{
	DESCRIPTOR_DATA *d;
	char buf[MAX_STRING_LENGTH], tmp[MAX_STRING_LENGTH];
	va_list args;

	push_call("morph_echo(%p)",fmt);

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	sprintf(tmp, "{078}[{118}M{108}O{118}R{108}P{118}H{078}] {018}%s", buf);

	for (d = mud->f_desc ; d ; d = d->next)
	{
		if (!d->character || d->connected < 0)
		{
			continue;
		}
		if (IS_SET(CH(d)->pcdata->channel, CHANNEL_MORPH))
		{
			ch_printf_color(d->character, "%s\n\r", ansi_justify(tmp, get_page_width(d->character)));
		}
	}
	pop_call();
	return;
}

int load_trivia (bool amount)
{
	char letter;
	int total, skip;
	FILE *fpList;
	TRIVIA_DATA * trivia;

	push_call("load_trivia(void)");

	log_string("Loading Trivia list..");

	close_reserve();

	if ((fpList = my_fopen(TRIVIA_LIST, "r", FALSE)) == NULL)
	{
		perror(TRIVIA_LIST);
		pop_call();
		return -1;
	}

	for (total = skip = 0 ; ; total++)
	{
		letter = fread_letter( fpList );
		if (letter != 'T')
		{
			if (letter != '$')
			{
				log_printf("***WARNING*** load_trivia: bad format");
				if (mud->l_trivia)
				{
					log_printf("Error after: %s", mud->l_trivia->answer);
				}
			}
			break;
		}
		if (amount && number_bits(6))
		{
			skip++;

			do
			{
				if (feof(fpList))
				{
					break;
				}
				letter = getc(fpList);
			}
			while (letter != '~');

			do
			{
				if (feof(fpList))
				{
					break;
				}
				letter = getc(fpList);
			}
			while (letter != '~');
		}
		else
		{
			ALLOCMEM(trivia, TRIVIA_DATA, 1);

			trivia->question	= fread_string( fpList );
			trivia->answer		= fread_string( fpList );

			LINK(trivia, mud->f_trivia, mud->l_trivia, next, prev);
		}
	}

	if (fpList)
	{
		my_fclose( fpList );
	}

	open_reserve();

	log_printf("load_trivia: loaded %d trivias out of a total of %d", amount, total);

	pop_call();
	return total - skip;
}

void update_trivia()
{
	TRIVIA_DATA *trivia;
	PLAYER_GAME *plg;

	push_call("update_trivia()");

	trivia_timer--;

	switch (trivia_timer)
	{
		case 0:
			trivia_timer = 20;
			trivia       = mud->f_trivia;

			if (trivia->next)
			{
				trivia_echo("No correct answer was received.");
				trivia_echo("Proceeding to next question.");
			}
			else
			{
				trivia_echo("No correct answer was received");
				trivia_echo("Trivia Game has ended.");
			}
			UNLINK(trivia, mud->f_trivia, mud->l_trivia, next, prev);
			STRFREE(trivia->question);
			STRFREE(trivia->answer);
			FREEMEM(trivia);
			break;
		case 15:
			trivia_echo("%s", mud->f_trivia->question);

			for (plg = mud->f_player ; plg ; plg = plg->next)
			{
				plg->ch->pcdata->trivia_guesses = 0;
			}
			break;
	}
	pop_call();
	return;
}

void do_trivia( CHAR_DATA *ch, char *argument)
{
	TRIVIA_DATA	*trivia, *trivia_next;
	HISCORE_DATA	*hiscore;
	char arg[MAX_STRING_LENGTH];
	char hs_buf[2][MAX_HISCORE/2][100];
	sh_int scrh, i;

	push_call("do_trivia(%p,%p)",ch,argument);

	if (argument[0] == '\0')
	{
		ch_printf(ch, "Syntax: trivia <answer|talk|hiscore> <string>\n\r");
		if (IS_IMMORTAL(ch))
		{
			ch_printf(ch, "Syntax: trivia <start|abort> <rounds>\n\r");
		}
		pop_call();
		return;
	}

	argument = one_argument(argument, arg);

	if (!str_prefix(arg, "answer"))
	{
		if (mud->f_trivia)
		{
			if (trivia_timer > 15)
			{
				ch_printf(ch, "There is not currently a trivia question you can answer.\n\r");
				pop_call();
				return;
			}
			if (ch->pcdata->trivia_guesses >= 5)
			{
				ch_printf(ch, "You reached your maximum of 5 answers per question.\n\r");
				pop_call();
				return;
			}
			ch->pcdata->trivia_guesses++;

			trivia_echo("{028}%s answers '%s'", get_name(ch), argument);

			if (is_name(argument, mud->f_trivia->answer))
			{
				if (!IS_NPC(ch))
				{
					ch->pcdata->trivia_points++;
				}
				trivia_timer = 20;
				trivia       = mud->f_trivia;

				if (trivia->next)
				{
					trivia_echo("%s has given the correct answer.", get_name(ch));
					trivia_echo("Proceeding to next question.");
				}
				else
				{
					trivia_echo("%s has given the correct answer.", get_name(ch));
					trivia_echo("Trivia Game has ended.");
				}
				UNLINK(trivia, mud->f_trivia, mud->l_trivia, next, prev);
				STRFREE(trivia->question);
				STRFREE(trivia->answer);
				FREEMEM(trivia);
			}
		}
		else
		{
			ch_printf(ch, "There is not currently a Trivia Game running.\n\r");
		}
		pop_call();
		return;
	}

	if (!str_prefix(arg, "talk"))
	{
		if (mud->f_trivia)
		{
			trivia_echo("{178}%s talks '%s'", get_name(ch), argument);
		}
		else
		{
			ch_printf(ch, "There is not currently a Trivia Game running.\n\r");
		}
		pop_call();
		return;
	}

	/*
		Trivia hiscore option - Scandum and Hypnos 14-01-2003
	*/

	if (!str_prefix(arg, "hiscore"))
	{
		scrh = URANGE(1, get_pager_breakpt(CH(ch->desc)) - 2, MAX_HISCORE/2);

		for (arg[0] = '\0', hiscore = mud->high_scores[MOST_TRIVIA]->first, i = 0 ; hiscore && i < scrh * 2 ; hiscore = hiscore->next, i++)
		{
			sprintf(hs_buf[i / scrh][i % scrh], " | {138}%5d {178}%12s {128}%8d  {038}| ", i+1, hiscore->player, hiscore->score);
		}

		cat_sprintf(arg, "{038}       /====================={128} Trivia Hall Of Fame {038}======================\\\n\r");
		cat_sprintf(arg, "{038}       | {158}Rank:        Name:   Score: {038} | {038} | {158}Rank:        Name:   Score: {038} | \n\r");

		for (i = 0 ; i < scrh ; i++)
		{
			cat_sprintf(arg, "{038}      %s%s\n\r", hs_buf[0][i], hs_buf[1][i]);
		}

		ch_printf_color(ch, "%s{038}       \\================================================================/\n\r", arg);

		pop_call();
		return;
	}

	if (IS_GOD(ch) && !str_prefix(arg, "start"))
	{
		int amount, total, pick, count;

		if (mud->f_trivia)
		{
			ch_printf(ch, "There is already a trivia game running.\n\r");
			pop_call();
			return;
		}

		amount = atoi(argument);

		if (amount < 1 || amount > 10)
		{
			ch_printf(ch, "You must enter a value between 1 and 10.\n\r");
			pop_call();
			return;
		}

		total = load_trivia(amount);

		while (total > amount)
		{
			for (trivia = mud->f_trivia ; trivia && total > amount ; trivia = trivia_next)
			{
				trivia_next = trivia->next;

				if (number_bits(2) && total--)
				{
					UNLINK(trivia, mud->f_trivia, mud->l_trivia, next, prev);
					STRFREE(trivia->question);
					STRFREE(trivia->answer);
					FREEMEM(trivia);
				}
			}
		}

		while (--total > 0)
		{
			pick = number_range(1, amount);

			for (count = 1, trivia = mud->f_trivia ; trivia ; trivia = trivia->next)
			{
				if (pick == count++)
				{
					UNLINK(trivia, mud->f_trivia, mud->l_trivia, next, prev);
					LINK(trivia, mud->f_trivia, mud->l_trivia, next, prev);
					break;
				}
			}
		}
		trivia_echo("%s has started an automated Trivia Game!", get_name(ch));

		trivia_timer = 20;

		pop_call();
		return;
	}

	if (IS_GOD(ch) && !str_prefix(arg, "test"))
	{
		if (mud->f_trivia)
		{
			ch_printf(ch, "There is already a trivia game running.\n\r");
			pop_call();
			return;
		}

		load_trivia(0);

		while ((trivia = mud->f_trivia) != NULL)
		{
			UNLINK(trivia, mud->f_trivia, mud->l_trivia, next, prev);
			STRFREE(trivia->question);
			STRFREE(trivia->answer);
			FREEMEM(trivia);
		}

		pop_call();
		return;
	}

	if (IS_IMMORTAL(ch) && !str_prefix(arg, "abort"))
	{
		if (mud->f_trivia)
		{
			while ((trivia = mud->f_trivia) != NULL)
			{
				UNLINK(trivia, mud->f_trivia, mud->l_trivia, next, prev);
				STRFREE(trivia->question);
				STRFREE(trivia->answer);
				FREEMEM(trivia);
			}
			trivia_echo("%s has aborted the Trivia Game.", get_name(ch));
		}
		else
		{
			ch_printf(ch, "There is not currently a Trivia Game running.\n\r");
		}
		pop_call();
		return;
	}

	do_trivia(ch, "");
	pop_call();
	return;
}

void do_morph(CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	CHAR_DATA *player;
	HISCORE_DATA *hiscore;
	char arg[MAX_STRING_LENGTH];
	char hs_buf[2][MAX_HISCORE/2][100];
	sh_int scrh, i;

	push_call("do_morph(%p,%p)",ch,argument);

	argument = one_argument(argument, arg);

	if (arg[0] == '\0')
	{
		if (!IS_IMMORTAL(ch))
		{
			send_to_char("Syntax: morph [talk|hiscore] [string]\n\r", ch);
		}
		else
		{
			send_to_char("Syntax: morph talk <string>\n\r", ch);
			send_to_char("Syntax: morph <open|close|hiscore>\n\r", ch);
			send_to_char("Syntax: morph player <player> <mob>\n\r", ch );
		}
		pop_call();
		return;
	}

	if (!str_prefix(arg, "hiscore"))
	{
		scrh = URANGE(1, get_pager_breakpt(CH(ch->desc)) - 2, MAX_HISCORE/2);

		for (arg[0] = '\0', hiscore = mud->high_scores[MOST_MORPH]->first, i = 0 ; hiscore && i < scrh * 2 ; hiscore = hiscore->next, i++)
		{
			sprintf(hs_buf[i / scrh][i % scrh], " | {138}%5d {178}%12s {128}%8d  {038}| ", i+1, hiscore->player, hiscore->score);
		}

		cat_sprintf(arg, "{038}       /=================={128} Morph Battle Hall Of Fame {038}===================\\\n\r");
		cat_sprintf(arg, "{038}       | {158}Rank:        Name:   Score: {038} | {038} | {158}Rank:        Name:   Score: {038} | \n\r");

		for (i = 0 ; i < scrh ; i++)
		{
			cat_sprintf(arg, "{038}      %s%s\n\r", hs_buf[0][i], hs_buf[1][i]);
		}

		ch_printf_color(ch, "%s{038}       \\================================================================/\n\r", arg);

		pop_call();
		return;
	}

	if (!str_prefix(arg, "talk"))
	{
		if (morph_game)
		{
			morph_echo("{118}%s talks '%s'", get_name(ch), argument);
		}
		else
		{
			ch_printf(ch, "There is not currently a morph game running.\n\r");
		}
		pop_call();
		return;
	}

	if (IS_IMMORTAL(ch) && !str_prefix(arg, "open"))
	{
		if (morph_game)
		{
			ch_printf(ch, "There is already a morph game running.\n\r");
		}
		else
		{
			morph_echo("%s has opened a Morph Game.", get_name(ch));
			morph_game = 1;
		}
		pop_call();
		return;
	}

	if (IS_IMMORTAL(ch) && !str_prefix(arg, "close"))
	{
		morph_echo("%s has closed the Morph Game.", get_name(ch));
		morph_game = 0;
		pop_call();
		return;
	}

	if (IS_IMMORTAL(ch) && !str_prefix(arg, "player"))
	{
		if (morph_game == 0)
		{
			send_to_char("There is not currently a morph game running.\n\r", ch);
			pop_call();
			return;
		}

		argument = one_argument(argument, arg);

		if ((player = get_player_world(ch, arg)) == NULL)
		{
			send_to_char( "No such player.\n\r", ch );
			pop_call();
			return;
		}

		if ((victim = get_char_room(ch, argument)) == NULL)
		{
			send_to_char("That mob isn't here.\n\r", ch);
			pop_call();
			return;
		}

		if (!IS_NPC(victim))
		{
			ch_printf(ch, "%s is a player.\n\r", get_name(victim));
			pop_call();
			return;
		}

		if (ch == player)
		{
			ch_printf(ch, "Use switch instead.\n\r");
			pop_call();
			return;
		}

		if (victim->desc)
		{
			ch_printf(ch, "They are already possessed by a player.\n\r");
			pop_call();
			return;
		}

		if (player->desc)
		{
			if (player->desc->original)
			{
				ch_printf(ch, "%s is already morphed.\n\r", get_name(player));
				pop_call();
				return;
			}
		}
		else
		{
			ch_printf(ch, "%s does not have a descriptor.\n\r", get_name(player));
			pop_call();
			return;
		}

		player->desc->character	= victim;
		player->desc->original	= player;
		victim->desc			= player->desc;
		player->desc			= NULL;
		player->pcdata->switched	= TRUE;

		act("You morph $t into $N.",       ch, player->name, victim, TO_CHAR);
		act("$n has morphed you into $N.", ch, player->name, victim, TO_VICT);
		act("$n has morphed $t into $N.",  ch, player->name, victim, TO_NOTVICT);

		morph_echo("%s has joined the Morph Game.", get_name(victim));
		morph_game++;

		pop_call();
		return;
	}
	do_morph(ch, "");
	pop_call();
	return;
}

void morph_update(CHAR_DATA *ch, CHAR_DATA *victim)
{
	push_call("morph_update(%p,%p)",ch,victim);

	if (morph_game == 0)
	{
		pop_call();
		return;
	}

	if (victim == NULL)
	{
		if (IS_NPC(ch) && ch->desc && ch->pIndexData->vnum == 10)
		{
			morph_echo("%s dies as its controller abandons it.", ch->short_descr);
			morph_game--;
		}
	}
	else if (!IS_NPC(victim) || victim->pIndexData->vnum != 10)
	{
		pop_call();
		return;
	}
	else if (victim == ch && ch->desc)
	{
		morph_echo("{058}F{018}A{158}T{118}AL{158}I{018}T{058}Y {018}%s has slain itself!", ch->short_descr);
		morph_game--;
	}
	else
	{
		if (morph_game < 100)
		{
			morph_echo("{118}{118}F{018}I{118}R{018}S{118}T B{018}L{118}O{018}O{118}D {018}%s has slain %s!", get_name(ch), victim->short_descr);
			morph_game += 100;

			if (IS_NPC(ch) && ch->desc)
			{
				CH(ch->desc)->pcdata->morph_points += 2;
			}
		}
		else
		{
			morph_echo("{118}F{018}A{118}T{018}A{118}L{018}I{118}T{018}Y {018}%s has slain %s!", get_name(ch), victim->short_descr);

			if (IS_NPC(ch) && ch->desc)
			{
				CH(ch->desc)->pcdata->morph_points += 1;
			}
		}
		morph_game--;
	}

	if (morph_game == 101)
	{
		morph_echo("The morph game has ended.");
		morph_game = 0;
	}
	pop_call();
	return;
}

void do_tag_help(CHAR_DATA *ch, char *argument)
{
	push_call_format("do_tag_help(%p, %p)", ch, argument);
	
	if(IS_IMMORTAL(ch))
	{
		ch_printf(ch, "Syntax: tag <player name|stop|enable|disable|status|help>\n\r");
	}
	else
	{
		ch_printf(ch, "Syntax: tag <player name|status|help>\n\r");
	}

	pop_call();
	return;
}


/* Game of TAG by M. J. Bethlehem 
   NOTE: Anyone can start a game of TAG, not just the immortals,
         however only immortals can STOP a TAG game or DISABLE/ENABLE
         the TAG game globally. */ 

void	do_tag(CHAR_DATA *ch, char *argument ) 
{
	char	arg[MAX_STRING_LENGTH];
	CHAR_DATA	*victim, *tch;
	
	push_call_format("do_tag(%p, %p)", ch, argument);
	
	if(argument[0] == '\0')
	{
		do_tag_help(ch, argument);
		pop_call();
		return;
	}

	argument = one_argument(argument, arg);
	
	if(!str_prefix(arg, "help"))
	{
		do_tag_help(ch, argument);
		pop_call();
		return;
	}
	
	if(!str_prefix(arg, "status"))
	{
		if(tag_game == TRUE)
		{
			ch_printf(ch, "Currently, there is a game of TAG in progress.\n\r");

			if(tagged_name != NULL)
			{
				if(!strcmp(tagged_name, ch->name))
				{
					ch_printf(ch, "You are TAGGED at the moment.\n\r");
				}
				else
				{
					ch_printf(ch, "%s is TAGGED at the moment.\n\r", capitalize(tagged_name));
				}
			}
		}
		else
		{
			ch_printf(ch, "Currently, no game of TAG is running.\n\r");
		}
		pop_call();
		return;
	} 

	if(IS_IMMORTAL(ch) && !str_prefix(arg, "stop"))
	{
		ch_printf(ch, "Resetting the game of TAG.\n\r");
		tag_game = FALSE;

		if(tagged_name != NULL)
		{
			tch = get_player_world(ch, tagged_name);
			if(tch != NULL)
			{
				REMOVE_BIT(tch->act, PLR_TAGGED);
				tagged_name = NULL;
			}
		}
		
		pop_call(); 
		return;
	}

	if(IS_IMMORTAL(ch) && !str_prefix(arg, "disable"))
	{
		if(tag_enabled)
		{
			ch_printf(ch, "Resetting and disabling the game of TAG.\n\r");
			tag_game = FALSE;
			tag_enabled = FALSE;
	
			if(tagged_name != NULL)
			{
				tch = get_player_world(ch, tagged_name);
				if(tch != NULL)
				{
					REMOVE_BIT(tch->act, PLR_TAGGED);
					tagged_name = NULL;
				}
			}
		}
		else
		{
			ch_printf(ch, "The TAG game is currently disabled.\n\r");
		}
		
		pop_call(); 
		return;
	}

	if(IS_IMMORTAL(ch) && !str_prefix(arg, "enable"))
	{
		if(!tag_enabled)
		{
			ch_printf(ch, "Enabling the game of tag.\n\r");
			tag_game = FALSE;
			tag_enabled = TRUE;
		}
		else
		{
			ch_printf(ch, "The tag game is already enabled.\n\r");
		}
		pop_call();
		return;
	}
		
	/* Check if the TAG game is enabled. If not, no one can start the game of tag */
	if(!tag_enabled)
	{
		ch_printf(ch, "You cannot do that right now.\n\r");
		pop_call();
		return;
	}
	
	/* if the TAG game is already running and the user is not TAGGED,
	   then bail out, otherwise look if the victim is in the same room
	   as the perpetrator. If so, TAG the victim. */
	if(tag_game == TRUE)
	{
		if(!IS_SET(ch->act, PLR_TAGGED))
		{
			ch_printf(ch, "No way! You're not it!\n\r");
			pop_call();
			return;
		}
		
		if(IS_SET(ch->act, PLR_NOTAG))
		{
			ch_printf(ch, "Since you do not want to play TAG yourself, you may not TAG others either!\n\r");
			pop_call();
			return;
		}
		
		victim = get_char_room(ch, arg);

		if(!victim)
		{
			ch_printf(ch, "You cannot find this person.\n\r");
			pop_call();
			return;
		}
		
		if(ch == victim)
		{
			ch_printf(ch, "Trying to play TAG with yourself is clearly a sign of imminent madness.\n\r");
			pop_call();
			return;
		}
		
		if(IS_NPC(victim) || IS_SET(victim->act, PLR_NOTAG))
		{
			ch_printf(ch, "No way! %s does not want to play TAG with you!\n\r", capitalize(get_name(victim)));
			pop_call();
			return;
		}
	
		REMOVE_BIT(ch->act, PLR_TAGGED);
		SET_BIT(victim->act, PLR_TAGGED);
		ch_printf(ch, "Tagging %s.\n\r", capitalize(get_name(victim)));
		ch_printf(victim, "TAG! You're it!\n\r");
		if(tagged_name != NULL)
		{
			STRFREE(tagged_name);
		}
		tagged_name = STRALLOC(get_name(victim));
		pop_call();
		return;
		
	}
	else if(tag_game == FALSE)
	{
		if(IS_SET(ch->act, PLR_NOTAG))
		{
			ch_printf(ch, "Since you do not want to play TAG yourself, you may not TAG others either!\n\r");
			pop_call();
			return;
		}

		if(!(victim = get_char_room(ch, arg)))
		{
			ch_printf(ch, "You cannot find this person.\n\r");
			pop_call();
			return;
		}
	
		if(ch == victim)
		{
			ch_printf(ch, "Trying to play TAG with yourself is clearly a sign of imminent madness.\n\r");
			pop_call();
			return;
		}
		
		if(IS_NPC(victim) || IS_SET(victim->act, PLR_NOTAG))
		{
			ch_printf(ch, "No way! %s does not want to play TAG with you!\n\r", capitalize(get_name(victim)));
			pop_call();
			return;
		}
		
		ch_printf(ch, "Starting a new game of TAG...\n\r");

		REMOVE_BIT(ch->act, PLR_TAGGED);
		SET_BIT(victim->act, PLR_TAGGED);
		ch_printf(ch, "Tagging %s.\n\r", capitalize(get_name(victim)));
		ch_printf(victim, "TAG! You're it!\n\r");
		tag_game = TRUE;
		if(tagged_name != NULL)
		{
			STRFREE(tagged_name);
		}
		tagged_name = STRALLOC(get_name(victim));

		pop_call();
		return;
	}
	else
	{
		pop_call();
		return;
	}
}

void do_notag_help(CHAR_DATA *ch, char *argument)
{
	push_call_format("do_notag_help(%p, %p)", ch, argument);

	ch_printf(ch, "Syntax: notag <on|off|status|help>\n\r");

	pop_call();
	return;
}

/* This command sets or resets a flag that indicates a player
   does not wish to play tag */
void	do_notag(CHAR_DATA *ch, char *argument )
{
	char	arg[MAX_STRING_LENGTH];
	
	push_call_format("do_notag(%p, %p)", ch, argument);
	
	if(argument[0] == '\0')
	{
		do_notag_help(ch, argument);
		pop_call();
		return;
	}
	
	argument = one_argument(argument, arg);
	
	if(!str_prefix(arg, "help"))
	{
		do_notag_help(ch, argument);
		pop_call();
		return;
	}
	
	if (!str_prefix(arg, "status"))
	{
		if(IS_SET(ch->act, PLR_TAGGED))
		{
			ch_printf(ch, "You are currently TAGGED.\n\r");
			pop_call();
			return;
		}
		
		if(IS_SET(ch->act, PLR_NOTAG))
		{
			ch_printf(ch, "Currently, you are not eligible for TAG.\n\r");
		}
		else
		{
			ch_printf(ch, "Currently, you are eligible for TAG.\n\r");
		}
		pop_call();
		return;
	}
	
	if(IS_SET(ch->act, PLR_TAGGED))
	{
		ch_printf(ch, "You are currently TAGGED, so you cannot change your notag status.\n\r");
		pop_call();
		return;
	}

	if (!str_prefix(arg, "on"))
	{
		SET_BIT(ch->act, PLR_NOTAG);
		ch_printf(ch, "Notag status has been switched on.\n\r");	
		pop_call();
		return;
	}

	if (!str_prefix(arg, "off"))
	{
		REMOVE_BIT(ch->act, PLR_NOTAG);
		ch_printf(ch, "Notag status has been switched off.\n\r");	
		pop_call();
		return;
	}

	pop_call();
	return;
}

