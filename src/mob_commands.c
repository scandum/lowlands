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

#include "mud.h"

/*
	 Local functions.
*/

bool			get_obj		args( ( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container, bool fDisplay ) );
bool 		check_social	args( ( CHAR_DATA *ch, char *command, char *argument ) );

/*
	Swaps given room exits
*/

void do_mpseed( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	int seed;

	push_call("do_mpseed(%p,%p)",ch,argument);

	if (!IS_NPC(ch))
	{
		ch_printf(ch, "NPC only, syntax: mpseed <number|player>", argument);

		pop_call();
		return;
	}

	if (is_number(argument))
	{
		seed = atoi(argument);
	}
	else
	{
		victim = get_player_world_even_blinded(ch, argument);

		if (victim == NULL)
		{
			log_build_printf(ch->pIndexData->vnum, "Bad argument <mpseed %s>", argument);

			pop_call();
			return;
		}
		seed = ch->pIndexData->vnum + victim->pcdata->pvnum;
	}

	number_seed(seed);

	pop_call();
	return;
}

void do_mpswap( CHAR_DATA *ch, char *argument )
{
	EXIT_DATA  *pExit;
	RESET_DATA *pReset;

	char arg[MAX_INPUT_LENGTH];
	int dir1, dir2;

	push_call("do_mpswap(%p,%p",ch, argument);

	argument = one_argument(argument, arg);

	dir1 = direction_door(arg);
	dir2 = direction_door(argument);

	if (dir1 == -1 || dir2 == -1 || dir1 == dir2)
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "Bad argument <mpswap %s %s>", arg, argument);
		}
		else
		{
			ch_printf(ch, "Syntax: mpswap <dir1> <dir2>\n\r");
		}
		pop_call();
		return;
	}

	pExit = ch->in_room->exit[dir1];

	ch->in_room->exit[dir1] = ch->in_room->exit[dir2];
	ch->in_room->exit[dir2] = pExit;

	for (pReset = ch->in_room->area->first_reset ; pReset ; pReset = pReset->next)
	{
		if (is_room_reset(pReset, ch->in_room))
		{
			if (is_door_reset(pReset, dir1))
			{
				pReset->arg2 = dir2;
			}
			else if (is_door_reset(pReset, dir2))
			{
				pReset->arg2 = dir1;
			}
		}
	}
	pop_call();
	return;
}


void do_mpmaze( CHAR_DATA *ch, char *argument )
{
	int x, y, z, size;
	int room, start_room, door;

	push_call("do_mpmaze(%p,%p)",ch,argument);

	if (sscanf(argument, "%d %d %d %d", &x, &y, &z, &size) != 4)
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "Bad Syntax <mpmaze %s>", argument);
		}
		else
		{
			send_to_char( "Format: MAZE <X size> <Y size> <Z size> <Total Rooms>\n\r", ch);
		}
		pop_call();
		return;
	}

	if (x < 1 || y < 1 || z < 1 || x * y * z < 4 || x * y * z > 8000 || size < 2 || size > 100)
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "Bad Coordinates <mpmaze %s>", argument);
		}
		else
		{
			ch_printf(ch, "Invalid mpmaze coordinates.\n\r");
		}
		pop_call();
		return;
	}

	for (room = start_room = ch->in_room->vnum ; room < start_room + size ; room++)
	{
		for (door = 0 ; door < 6 ; door++)
		{
			if (room_index[room] && room_index[room]->exit[door] && room_index[room]->exit[door]->vnum <= 0)
			{
				STRFREE(room_index[room]->exit[door]->keyword);
				STRFREE(room_index[room]->exit[door]->description);

				room_index[room]->exit[door]->flags       = 0;
				room_index[room]->exit[door]->key         = 0;
				room_index[room]->exit[door]->keyword     = STRDUPE(str_empty);
				room_index[room]->exit[door]->description = STRDUPE(str_empty);

				set_exit(room, door, -1);
			}
		}
	}

	mazegen(ch, 0, 0, 0, x, y, z, size, 0, number_bits(16), -1);

	if (!IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL)
	{
		send_to_char( "Maze generated.\n\r", ch);
	}
	pop_call();
	return;
}


void mazegen( CHAR_DATA *ch, int cx, int cy, int cz, int x, int y, int z, int size, int count, int seed, int last_door)
{
	int door, room, next_room, cnt;

	push_call("mazegen(%p,%p,%p,%p,%p,%p)",ch,x,y,z,size,count,seed);

	if (count >= size-1)
	{
		pop_call();
		return;
	}

	room = ((count + seed) % size) + ch->in_room->vnum;

	for (cnt = 0 ; cnt < 1000 ; cnt++)
	{
		door = number_door();

		if (door == last_door)
		{
			continue;
		}
		if (room_index[room] && room_index[room]->exit[door] && room_index[room]->exit[door]->vnum > 0)
		{
			continue;
		}
		switch (door)
		{
			case 0: if (cy+1 >= y) continue; break;
			case 1: if (cx+1 >= x) continue; break;
			case 2: if (cy-1 <  0) continue; break;
			case 3: if (cx-1 <  0) continue; break;
			case 4: if (cz+1 >= z) continue; break;
			case 5: if (cz-1 <  0) continue; break;
		}
		break;
	}
	if (cnt == 1000)
	{
		log_printf("Failed to create a maze: room vnum: %d", room);
		pop_call();
		return;
	}
	switch (door)
	{
		case 0: cy++;	break;
		case 1: cx++;	break;
		case 2: cy--;	break;
		case 3: cx--;	break;
		case 4: cz++;	break;
		case 5: cz--;	break;
	}

	count++;
	next_room = ((count + seed) % size) + ch->in_room->vnum;
	last_door = rev_dir[door];

	set_exit(room, door, next_room);
	set_exit(next_room, last_door, room);

	mazegen(ch, cx, cy, cz, x, y, z, size, count, seed, last_door);
	pop_call();
	return;
}

void do_rescale( CHAR_DATA *ch, char *argument )
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *mch;
	int vnum, scale;
	int new_hitplus, new_hitnodice, new_hitsizedice;

	push_call("do_rescale(%p,%p)",ch,argument);

	smash_tilde( argument );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	strcpy( arg3, argument );

	/*
		Added single mob rescale - Scandum 08/02/02
	*/

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
	{
		send_to_char( "Syntax: rescale <mobVnum|mobName> <player> <percentage>\n\r", ch);
		pop_call();
		return;
	}

	if (is_number(arg1))
	{
		vnum = atol(arg1);

		if (get_mob_index(vnum) == NULL)
		{
			if (IS_NPC(ch))
			{
				log_build_printf(ch->pIndexData->vnum, "bad rescale vnum: %d", vnum);
			}
			send_to_char("Vnum out of bounds\n\r", ch);
			pop_call();
			return;
		}

		if ((victim = get_player_world_even_blinded(ch, arg2)) == NULL)
		{
			send_to_char("They aren't playing.\n\r", ch);
			pop_call();
			return;
		}

		scale			= atol(arg3) * victim->level;

		for (mch = mob_index[vnum]->first_instance ; mch ; mch = mch->next_instance)
		{
			mch->level				= UMAX(1, mch->pIndexData->level * scale / 10000);

			mch->npcdata->damnodice		= 1;
			mch->npcdata->damsizedice	= mch->level;
			mch->npcdata->damplus		= 2 + mch->level * 0.005 * mch->level;

			new_hitnodice				= 1;
			new_hitsizedice			= mch->level * mch->level / 4;
			new_hitplus				= 10 + mch->level * mch->level * (0.40 + 0.0015 * mch->level);

			mch->hit 					= new_hitplus + dice(new_hitnodice, new_hitsizedice);
			mch->max_hit				= mch->hit;
		}
	}
	else
	{
		if ((mch = get_char_room(ch, arg1)) == NULL)
		{
			send_to_char("They aren't here.\n\r", ch);
			pop_call();
			return;
		}
		if (!IS_NPC(mch))
		{
			send_to_char("You can only rescale mobiles.\n\r",ch);
			pop_call();
			return;
		}
		if ((victim = get_player_world_even_blinded(ch, arg2)) == NULL)
		{
			send_to_char("They aren't here.\n\r", ch);
			pop_call();
			return;
		}
		if (IS_NPC(victim))
		{
			send_to_char("You can only rescale mobs on players.\n\r",ch);
			pop_call();
			return;
		}
		scale					= atol(arg3) * victim->level;

		mch->level				= UMAX(1, mch->pIndexData->level * scale / 10000);

		mch->npcdata->damnodice		= 1;
		mch->npcdata->damsizedice	= mch->level;
		mch->npcdata->damplus		= 2 + mch->level * 0.005 * mch->level;

		new_hitnodice				= 1;
		new_hitsizedice			= mch->level * mch->level / 4;
		new_hitplus				= 10 + mch->level * mch->level * (0.40 + 0.0015 * mch->level);

		mch->hit 					= new_hitplus + dice(new_hitnodice, new_hitsizedice);
		mch->max_hit				= mch->hit;
	}
	pop_call();
	return;
}

void do_doorset( CHAR_DATA *ch, char *argument )
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	int door;
	EXIT_DATA *pexit;

	push_call("do_doorset(%p,%p)",ch,argument);

	smash_tilde( argument );

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	strcpy( arg3, argument );

	if (arg1[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char( "Syntax: doorset <direction> <field> <argument>\n\r",ch );
		send_to_char( "Field being one of:  \n\r  desc name flag key\n\r",ch );
		pop_call();
		return;
	}

	door = direction_door(arg1);

	if (door < 0)
	{
		door = atol(arg1);
	}
	if (door < 0 || door > 5)
	{
		send_to_char ("Invalid exit direction.\n\r",ch);
		pop_call();
		return;
	}

	pexit = ch->in_room->exit[door];

	if (pexit == NULL)
	{
		send_to_char( "There is no exit in that direction.\n\r", ch);
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "flag"))
	{
		pexit->flags = translate_bits(arg3);
	}
	else if (!strcasecmp(arg2, "name"))
	{
		pexit->keyword = STRALLOC(arg3);
	}
	else if (!strcasecmp(arg2, "desc"))
	{
		pexit->description = STRALLOC(arg3);
	}
	else if (!strcasecmp(arg2, "key"))
	{
		pexit->key = atol(arg3);
	}
	pop_call();
	return;
}

void do_mpdelay( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char arg1[MAX_INPUT_LENGTH];
	sh_int delay;

	push_call("do_mpdelay(%p,%p",ch, argument);

	argument = one_argument_nolower(argument, arg1);

	if (!IS_NPC(ch) || arg1[0] == '\0' || argument[0] == '\0')
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "Bad Syntax <mpdelay %s %s>", arg1, argument);
		}
		else
		{
			send_to_char("NPC ONLY: Syntax: mpdelay <victim> <time> <index> <$X>\n\r", ch);
		}
		pop_call();
		return;
	}

	if ((victim = get_char_room_even_blinded(ch, arg1)) == NULL)
	{
		log_build_printf(ch->pIndexData->vnum, "Target not found <mpdelay %s %s>", arg1, argument);
		pop_call();
		return;
	}

	if (!IS_NPC(victim))
	{
		log_build_printf(ch->pIndexData->vnum, "Target is a player <mpdelay %s %s>", arg1, argument);
		pop_call();
		return;
	}

	argument = one_argument_nolower(argument, arg1);
	delay = atol(arg1);

	if (delay >= 60)
	{
		victim->timer	= delay / 60;
		victim->wait	= 0;
	}
	else
	{
		victim->timer	= 0;
		victim->wait	= delay * 2;
	}

	argument = one_argument_nolower(argument, arg1);
	victim->npcdata->delay_index = atol(arg1);

	strcpy(arg1, argument);

	if (!strcasecmp(arg1, "null"))
	{
		STRFREE(victim->npcdata->remember);
		victim->npcdata->remember = STRDUPE(str_empty);
	}
	else
	{
		STRFREE(victim->npcdata->remember);
		victim->npcdata->remember = STRALLOC(arg1);
	}
	pop_call();
	return;
}


void do_mptrigger( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char arg1[MAX_INPUT_LENGTH];

	push_call("do_mptrigger(%p,%p",ch, argument);

	argument = one_argument_nolower(argument, arg1);

	if (arg1[0] == '\0' || argument[0] == '\0')
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "Bad Syntax <mptrigger %s %s>", arg1, argument);
		}
		else
		{
			send_to_char("Syntax: mptrigger <victim> <wordlist>\n\r", ch);
		}
		pop_call();
		return;
	}

	if ((victim = get_char_room_even_blinded(ch, arg1)) == NULL)
	{
		pop_call();
		return;
	}

	if (MP_VALID_MOB(victim))
	{
		mprog_trigger_trigger(argument, victim, ch);
	}
	pop_call();
	return;
}


void do_mpcalculate( CHAR_DATA *ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char arg4[MAX_INPUT_LENGTH];

	push_call("do_mpcalculate(%p,%p)",ch, argument);

	if (IS_NPC(ch))
	{
		sprintf(arg4, "%lld", mathexp(ch, argument));
		STRFREE(ch->npcdata->remember);
		ch->npcdata->remember = STRALLOC(arg4);
	}
	else
	{
		ch_printf(ch, "%s = %lld\n\r", argument, mathexp(ch, argument));
	}

	pop_call();
	return;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);

	if (*arg1 == '\0' || *arg2 == '\0' || *arg2 == '\0')
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "Bad Syntax <mpcalculate %s %s %s>", arg1, arg2, arg3);
		}
		else
		{
			send_to_char("Syntax: mpcalculate <value> <+ - / * %> <value>\n\r", ch);
		}
		pop_call();
		return;
	}

	if (!is_number(arg1) || !is_number(arg3))
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "Bad Syntax <mpcalculate %s %s %s>", arg1, arg2, arg3);
		}
		else
		{
			send_to_char("You must use numbers to calculate.\n\r", ch);
		}
		pop_call();
		return;
	}

	arg4[0] = '\0';

	switch (arg2[0])
	{
		case '+':
			sprintf(arg4, "%lld", atoll(arg1) + atoll(arg3));
			break;
		case '-':
			sprintf(arg4, "%lld", atoll(arg1) - atoll(arg3));
			break;
		case '*':
			sprintf(arg4, "%lld", atoll(arg1) * atoll(arg3));
			break;
		case '/':
			if (atoll(arg3) == 0)
			{
				if (IS_NPC(ch))
				{
					log_build_printf(ch->pIndexData->vnum, "Division Zero <mpcalculate %s %s %s>", arg1, arg2, arg3);
				}
				else
				{
					send_to_char("You formulate infinity, and store it deep inside you.\n\r", ch);
				}
				pop_call();
				return;
			}
			sprintf(arg4, "%lld", atoll(arg1) / atoll(arg3));
			break;
		case '%':
			sprintf(arg4, "%lld", atoll(arg1) % atoll(arg3));
			break;
		default:
			if (IS_NPC(ch))
			{
				log_build_printf(ch->pIndexData->vnum, "Bad Operator <mpcalculate %s %s %s>", arg1, arg2, arg3);
			}
			else
			{
				send_to_char("Unknown operator.\n\r", ch);
			}
			pop_call();
			return;
	}

	if (IS_NPC(ch))
	{
		STRFREE(ch->npcdata->remember);
		ch->npcdata->remember = STRALLOC(arg4);
	}
	else
	{
		ch_printf(ch, "%s %s %s = %s\n\r", arg1, arg2, arg3, arg4);
	}
	if (argument[0] != '\0')
	{
		sprintf(arg1, "%s %s", arg4, argument);
		pop_call();
		return do_mpcalculate(ch, arg1);
	}
	pop_call();
	return;
}
	
void do_mplog( CHAR_DATA *ch, char *argument )
{
	push_call("do_mplog(%p,%p",ch, argument);

	if (IS_NPC(ch))
	{
		log_build_printf(ch->pIndexData->vnum, "MPLOG %s", argument);
	}
	else
	{
		log_printf("[%s] MPLOG %s", ch->name, argument);
	}
	pop_call();
	return;
}

/*
	Set some area (zone) related stuff
*/

void do_mpzset(CHAR_DATA *ch, char *argument)
{
	AREA_DATA *tarea;
	char arg1[MAX_INPUT_LENGTH];

	push_call("do_mpzset(%p,%p)",ch,argument);

	argument = one_argument_nolower(argument, arg1);

	if (arg1[0] == '\0' || argument[0] == '\0')
	{
		send_to_char("Syntax: mpzset <field> <argument>\n\r", ch);
		send_to_char("  Field being one of:\n\r", ch);
		send_to_char("    resetmsg quest\n\r", ch);
		pop_call();
		return;
	}

	tarea = ch->in_room->area;

	if (!strcasecmp(arg1, "resetmsg"))
	{
		if (strcasecmp(argument, "null"))
		{
			RESTRING(tarea->resetmsg, argument);
		}
		else
		{
			RESTRING(tarea->resetmsg, "");
		}
		pop_call();
		return;
	}

	if (!strcasecmp(arg1, "quest" ) )
	{
		int firstBit,len,value;

		strcpy(arg1, argument);

		if (sscanf(arg1, "%d %d %d", &firstBit, &len, &value) != 3)
		{
			if (IS_NPC(ch))
			{
				log_build_printf(ch->pIndexData->vnum, "bad parameters <mpzset quest %s>", arg1);
			}
			pop_call();
			return;
		}

		if (IS_NPC(ch))
		{
			set_quest_bits(&ch->pIndexData->area->area_quest, firstBit, len, value);
		}
		else
		{
			set_quest_bits(&ch->in_room->area->area_quest, firstBit, len, value);
		}
		pop_call();
		return;
	}
	do_mpzset(ch, "");
	pop_call();
	return;
}

void do_mpdamage( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim, *victim_next;
	int dam_lo, dam_hi, dam_all, dam_tar, dam_pcs;

	push_call("do_mpdamage(%p,%p)",ch,argument);

	if (IS_NPC(ch) && ch->fighting == NULL && ch->hit < ch->max_hit/2)
	{
		pop_call();
		return;
	}

	argument = one_argument_nolower(argument, arg);

	if (arg[0] == '\0')
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "No argument <mpdamage>" );
		}
		else
		{
			send_to_char("Syntax: mpdamage <victim|all|pcs|foe> <dam_lo> <dam_hi> <message>\n\r", ch);
		}
		pop_call();
		return;
	}

	dam_all = !strcasecmp(arg, "all");
	dam_pcs = !strcasecmp(arg, "pcs");
	dam_tar = !strcasecmp(arg, "foe");

	if (dam_all || dam_pcs)
	{
		victim = NULL;
	}
	else if (dam_tar)
	{
		if ((victim = who_fighting(ch)) == NULL)
		{
			pop_call();
			return;
		}
	}
	else if ((victim = get_char_room_even_blinded(ch, arg)) == NULL)
	{
		pop_call();
		return;
	}

	if (victim == ch)
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "victim == ch <mpdamage %s %s>", arg, argument );
		}
		pop_call();
		return;
	}

	argument = one_argument_nolower(argument, arg);
	dam_lo   = atol(arg);
	argument = one_argument_nolower(argument, arg);
	dam_hi   = atol(arg);

	if (argument[0] == '\0')
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "Bad argument <mpdamage: %s %d %d %s>", dam_all ? "all" : victim->name, dam_lo, dam_hi, argument);
		}
		else
		{
			do_mpdamage(ch, "");
		}
		pop_call();
		return;
	}

	strcpy(mpdamstring, argument);

	if (!dam_all && !dam_pcs)
	{
		damage(ch, victim, number_range(dam_lo, dam_hi), gsn_object_rape);
		pop_call();
		return;
	}

	for (victim = ch->in_room->first_person ; victim ; victim = victim_next)
	{
		victim_next = victim->next_in_room;

		if (can_mass_attack(ch, victim))
		{
			if (dam_all || !IS_NPC(victim))
			{
				damage(ch, victim, number_range(dam_lo, dam_hi), gsn_object_rape);
			}
		}
	}
	pop_call();
	return;
}


/*
	set an affect on an object
*/

void do_mpaset( CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char	arg3[MAX_INPUT_LENGTH];
	OBJ_DATA		*obj;
	AFFECT_DATA	*paf;
	int value, type;

	push_call("do_mpaset(%p,%p)",ch,argument);

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	argument = one_argument(argument, arg3);

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "Bad argument <mpaset: %s %s %s>", arg1, arg2, arg3);
		}
		else
		{
			ch_printf(ch, "Syntax: mpaset <object> <%s> <value>\n\r", give_flags(a_types));
		}
		pop_call();
		return;
	}

	if ((obj = get_obj_here_even_blinded(ch, arg1)) == NULL)
	{
		pop_call();
		return;
	}

	if ((type = get_flag(arg2, a_types)) == -1)
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "Bad type <mpaset: %s %s %s>", arg1, arg2, arg3);
		}
		else
		{
			ch_printf(ch, "Syntax: mpaset <object> <%s> <value>\n\r", give_flags(a_types));
		}
		pop_call();
		return;
	}

	value = atol(arg3);

	ALLOCMEM(paf, AFFECT_DATA, 1);
	paf->type       = gsn_object_rape;
	paf->duration   = -1;
	paf->modifier   = value;
	paf->bitvector  = 0;
	paf->location   = type;

	LINK(paf, obj->first_affect, obj->last_affect, next, prev);
	pop_call();
	return;
}

/*
	prints the argument to all the rooms aroud the mobile
*/

void do_mpasound( CHAR_DATA *ch, char *argument )
{
	EXIT_DATA *pexit;
	int door, temp_vnum;

	push_call("do_mpasound(%p,%p)",ch,argument);

	if (argument[0] == '\0')
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "No argument <mpasound>" );
			pop_call();
			return;
		}
	}

	temp_vnum = ch->in_room->vnum;

	for (door = 0 ; door <= 5 ; door++)
	{
		if ((pexit = get_exit(temp_vnum, door)) != NULL)
		{
			ch->in_room = room_index[pexit->to_room];
			act( argument, ch, NULL, NULL, TO_ROOM );
		}
	}
	ch->in_room = room_index[temp_vnum];

	pop_call();
	return;
}

/*
	lets the mobile kill any player or mobile without murder
*/

void do_mpkill( CHAR_DATA *ch, char *argument )
{
	char arg[ MAX_INPUT_LENGTH ];
	CHAR_DATA *victim;

	push_call("do_mpkill(%p,%p)",ch,argument);

	if (IS_NPC(ch) && ch->fighting == NULL && ch->hit < ch->max_hit/2)
	{
		pop_call();
		return;
	}

	one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "No argument <mpkill>" );
			pop_call();
			return;
		}
		pop_call();
		return;
	}

	if ((victim = get_char_room_even_blinded(ch, arg)) == NULL)
	{
		pop_call();
		return;
	}

	if (victim == ch)
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "victim == ch <mpkill %s>", arg );
		}
		pop_call();
		return;
	}

	if (ch->position == POS_FIGHTING)
	{
		pop_call();
		return;
	}

	multi_hit( ch, victim, TYPE_UNDEFINED );

	pop_call();
	return;
}

/*
	destroys an object worn or in inventory using obj name,
	all.obj or all.
*/

void do_mpjunk( CHAR_DATA *ch, char *argument )
{
	char		arg[MAX_INPUT_LENGTH];
	OBJ_DATA	*obj;
	OBJ_DATA	*obj_next;

	push_call("do_mpjunk(%p,%p) <%s, %s>",ch,argument,ch->name,argument);

	one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "No argument <mpjunk>" );
		}
		pop_call();
		return;
	}

	if (strcasecmp(arg, "all") && str_prefix("all.", arg))
	{
		if ((obj = get_obj_wear_even_blinded(ch, arg)) != NULL)
		{
			junk_obj( obj );
		}
		else if ((obj = get_obj_carry_even_blinded(ch, arg)) != NULL)
		{
			junk_obj( obj );
		}
		pop_call();
		return;
	}
	else
	{
		for (obj = ch->first_carrying ; obj != NULL ; obj = obj_next)
		{
			obj_next = obj->next_content;

			if (arg[3] == '\0' || is_name(&arg[4], obj->name))
			{
				junk_obj( obj );
			}
		}
	}
	pop_call();
	return;
}

void do_mpjunk_person( CHAR_DATA *ch, char *argument )
{
	char		arg[MAX_INPUT_LENGTH];
	OBJ_DATA	*obj;
	OBJ_DATA	*obj_next;
	CHAR_DATA	*victim;

	push_call("do_mpjunk_person(%p,%p)",ch,argument);

	argument = one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "No argument <mpjunkperson>" );
		}
		pop_call();
		return;
	}

	if ((victim = get_char_room_even_blinded(ch, arg)) == NULL)
	{
		pop_call();
		return;
	}

	one_argument( argument, arg );

	if (strcasecmp(arg, "all") && str_prefix("all.", arg))
	{
		if ((obj = get_obj_wear_even_blinded(victim, arg)) != NULL)
		{
			junk_obj( obj );
		}
		else if ((obj = get_obj_carry_even_blinded(victim, arg)) != NULL)
		{
			junk_obj( obj );
		}
		pop_call();
		return;
	}
	else
	{
		for (obj = victim->first_carrying ; obj != NULL ; obj = obj_next)
		{
			obj_next = obj->next_content;

			if (arg[3] == '\0' || is_name(&arg[4], obj->name))
			{
				junk_obj( obj );
			}
		}
	}
	pop_call();
	return;
}

/*
	prints the message to everyone in the room other than the mob
	and victim
*/

void do_mpechoaround( CHAR_DATA *ch, char *argument )
{
	char       arg[ MAX_INPUT_LENGTH ];
	CHAR_DATA *victim;

	push_call("do_mpechoaround(%p,%p [%s])",ch,argument,argument);

	argument = one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		pop_call();
		return;
	}

	if ((victim = get_char_room_even_blinded(ch, arg)) != NULL)
	{
		act( argument, victim, NULL, NULL, TO_ROOM );
	}
	pop_call();
	return;
}

/*
	prints the message to only the victim
*/

void do_mpechoat( CHAR_DATA *ch, char *argument )
{
	char       arg[ MAX_INPUT_LENGTH ];
	CHAR_DATA *victim;

	push_call("do_mpechoat(%p,%p)",ch,argument);

	argument = one_argument( argument, arg );

	if (arg[0] == '\0' || argument[0] == '\0')
	{
		pop_call();
		return;
	}

	if ((victim = get_char_room_even_blinded(ch, arg)) != NULL)
	{
		if (victim == ch)
		{
			act(argument, ch, NULL, NULL, TO_CHAR);
		}
		else
		{
			act(argument, ch, NULL, victim, TO_VICT);
		}
	}
	pop_call();
	return;
}

/*
	prints the message to the room at large
*/

void do_mpecho( CHAR_DATA *ch, char *argument )
{
	push_call("do_mpecho(%p,%p)",ch,argument);

	if (argument[0] == '\0')
	{
		pop_call();
		return;
	}

	if (!IS_NPC(ch))
	{
		act( argument, ch, NULL, NULL, TO_CHAR );
	}

	act( argument, ch, NULL, NULL, TO_ROOM );

	pop_call();
	return;
}

/*
	Print the message to every awake player in the area
*/

void do_mpareaecho( CHAR_DATA *ch, char *argument )
{
	PLAYER_GAME	*pch;
	char      	arg[MAX_INPUT_LENGTH];
	int			echo_type;

	push_call("do_mpareaecho(%p,%p)",ch,argument);

	if (ch->in_room->area->nplayer == 0 || argument[0] == '\0')
	{
		pop_call();
		return;
	}

	if (!str_prefix("sleepers", argument))
	{
		echo_type = 1;
	}
	else if (!str_prefix("outdoors", argument))
	{
		echo_type = 2;
	}
	else if (!str_prefix("indoors", argument))
	{
		echo_type = 3;
	}
	else
	{
		echo_type = 0;
	}

	if (echo_type > 0)
	{
		argument = one_argument_nolower(argument, arg);
	}

	for (pch = mud->f_player ; pch ; pch = pch->next)
	{
		if (pch->ch->in_room->area == ch->in_room->area)
		{
			switch (echo_type)
			{
				case 1:
					if (IS_AWAKE(pch->ch))
					{
						continue;
					}
					break;
				case 2:
					if (!IS_AWAKE(pch->ch) || !IS_OUTSIDE(pch->ch) ||  NO_WEATHER_SECT(pch->ch->in_room->sector_type))
					{
						continue;
					}
					break;
				case 3:
					if (!IS_AWAKE(pch->ch) ||  IS_OUTSIDE(pch->ch) || !NO_WEATHER_SECT(pch->ch->in_room->sector_type))
					{
						continue;
					}
					break;
				default:
					if (!IS_AWAKE(pch->ch))
					{
						continue;
					}
					break;
			}
			ch_printf(pch->ch, "%s\n\r", ansi_translate_text(pch->ch, ansi_justify(argument, get_page_width(pch->ch))));
		}
	}
	pop_call();
	return;
}

/*
	load an item or mobile.  All items are loaded into inventory.
*/

void do_mpmload( CHAR_DATA *ch, char *argument )
{
	char            arg[ MAX_INPUT_LENGTH ];
	MOB_INDEX_DATA *pMobIndex;
	CHAR_DATA      *victim;

	push_call("do_mpmload(%p,%p)",ch,argument);

	one_argument( argument, arg );

	if (arg[0] == '\0' || !is_number(arg))
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "Bad argument: <mpmload %s>", arg );
		}
		pop_call();
		return;
	}

	if ((pMobIndex = get_mob_index(atol(arg))) == NULL)
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "Bad vnum: <mpmload %s>", arg );
		}
		pop_call();
		return;
	}

	victim = create_mobile( pMobIndex );
	char_to_room( victim, ch->in_room->vnum );

	pop_call();
	return;
}

void do_mpoload( CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	OBJ_INDEX_DATA *pObjIndex;
	OBJ_DATA       *obj;
	bool			load_to_room = FALSE;
	bool			wear_on_load = FALSE;

	push_call("do_mpoload(%p,%p)",ch,argument);

	argument = one_argument( argument, arg1 );
	one_argument( argument, arg2 );

	if (arg1[0] == '\0')
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "Bad argument: <mpoload %s>", arg1 );
		}
		pop_call();
		return;
	}

	switch (arg2[0])
	{
		case 'r':
			load_to_room = TRUE;
			break;

		case 'w':
			wear_on_load = TRUE;
			break;

		case '\0':
			break;

		default:
			if (IS_NPC(ch))
			{
				log_build_printf(ch->pIndexData->vnum, "Bad argument: <mpoload %s %s>", arg1, arg2 );
			}
			pop_call();
			return;
	}

	if ((pObjIndex = get_obj_index(atol(arg1))) == NULL)
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "Bad vnum: <mpoload %s>", arg1 );
		}
		pop_call();
		return;
	}

	obj = create_object( pObjIndex, 0 );

	if (!CAN_WEAR(obj, ITEM_TAKE) || load_to_room)
	{
		obj->sac_timer = OBJ_SAC_TIME;
		obj_to_room(obj, ch->in_room->vnum);
	}
	else
	{
		obj_to_char(obj, ch);
		if (wear_on_load)
		{
			wear_obj(ch, obj, FALSE, WEAR_NONE, FALSE);
		}
	}

	pop_call();
	return;
}

/*
	purge a mob, obj, entire room, or entire area
*/

void do_mppurge( CHAR_DATA *ch, char *argument )
{
	char       arg[ MAX_INPUT_LENGTH ];
	CHAR_DATA *victim;
	OBJ_DATA  *obj;

	push_call("do_mppurge(%p,%p)",ch,argument);

	one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "No argument: <mppurge>");
		}
		pop_call();
		return;
	}

	if (strcasecmp(arg, "all") && strcasecmp(arg, "area") && str_prefix("all.", arg))
	{
		if ((victim = get_char_room_even_blinded(ch, arg)) == NULL)
		{
			if ((obj = get_obj_list_even_blinded(ch, arg, ch->in_room->first_content)) != NULL)
			{
				if (obj->item_type != ITEM_CORPSE_PC)
				{
					junk_obj( obj );
				}
			}
			pop_call();
			return;
		}

		if (!IS_NPC(victim))
		{
			if (IS_NPC(ch))
			{
				log_build_printf(ch->pIndexData->vnum, "Target is a PC <mppurge %s>", arg );
			}
			pop_call();
			return;
		}

		/*
			Use the junk room - Scandum
		*/

		junk_mob(victim);

		pop_call();
		return;
	}

	if (!strcasecmp(arg, "all") || !str_prefix("all.", arg))
	{
		CHAR_DATA *vnext;
		OBJ_DATA  *obj_next;

		for (victim = ch->in_room->first_person ; victim ; victim = vnext)
		{
			vnext = victim->next_in_room;

			if (IS_NPC(victim) && victim != ch)
			{
				if (arg[3] == '\0' || is_name(&arg[4], victim->name))
				{
					junk_mob(victim);
				}
			}
		}

		for (obj = ch->in_room->first_content ; obj ; obj = obj_next)
		{
			obj_next = obj->next_content;

			if (obj->item_type != ITEM_CORPSE_PC)
			{
				if (arg[3] == '\0' || is_name(&arg[4], obj->name))
				{
					junk_obj( obj );
				}
			}
		}
		pop_call();
		return;
	}
	else
	{
		CHAR_DATA	*vnext;
		OBJ_DATA	*obj_next;
		int		room, room_start;

		/*
			Purge every room in the area, nolonger jams on null rooms - Scandum
		*/

		room_start = ch->in_room->vnum;

		for (room = room_index[room_start]->area->low_r_vnum ; room <= room_index[room_start]->area->hi_r_vnum ; room++)
		{
			if (room_index[room] == NULL)
			{
				continue;
			}
			if (room_index[room_start]->area != room_index[room]->area)
			{
				break;
			}
			char_from_room(ch);
			char_to_room(ch, room);

			for (victim = ch->in_room->first_person ; victim != NULL ; victim = vnext)
			{
				vnext = victim->next_in_room;

				if (IS_NPC(victim) && victim != ch)
				{
					junk_mob(victim);
				}
			}

			for (obj = ch->in_room->first_content ; obj ; obj = obj_next)
			{
				obj_next = obj->next_content;

				if (obj->item_type != ITEM_CORPSE_PC)
				{
					junk_obj( obj );
				}
			}
		}
		char_from_room(ch);
		char_to_room(ch, room_start);
	}
	pop_call();
	return;
}

/*
	sends the mobile to given location
*/

void do_mpgoto( CHAR_DATA *ch, char *argument )
{
	int location;

	push_call("do_mpgoto(%p,%p)",ch,argument);

	if (!IS_NPC(ch) && ch->level < MAX_LEVEL && ch->fighting != NULL)
	{
		pop_call();
		return;
	}

	if (argument[0] == '\0')
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "- No argument <mpgoto>" );
		}
		pop_call();
		return;
	}

	if ((location = find_mp_location(ch, argument)) == -1)
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "Bad location: <mpgoto %s>", argument );
		}
		pop_call();
		return;
	}

	if (is_mounting(ch))
	{
		transference(ch->mounting, location);
	}
	transference(ch, location);

	pop_call();
	return;
}

/*
	execute command at another location.
*/

void do_mpat( CHAR_DATA *ch, char *argument )
{
	char             arg[ MAX_INPUT_LENGTH ];
	int	location, original;
	CHAR_DATA		 *mount;
	OBJ_DATA        *furniture;

	push_call("do_mpat(%p,%p)",ch,argument);

	if (!IS_NPC(ch) && (ch->fighting != NULL || ch->position == POS_FIGHTING))
	{
		pop_call();
		return;
 	}

	argument = one_argument_nolower(argument, arg);

	if (arg[0] == '\0' || argument[0] == '\0')
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "No argument: <mpat>" );
		}
		pop_call();
		return;
	}

	if ((location = find_mp_location(ch, arg)) == -1)
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "Bad location: <mpat %s %s>", arg, argument );
		}
		pop_call();
		return;
	}

	furniture = ch->furniture;
	mount	= ch->mounting;
	original  = ch->in_room->vnum;

	char_from_room( ch );
	char_to_room( ch, location);

	do_mpdo(ch, argument);

	if (ch && ch->name)
	{
		char_from_room(ch);
		char_to_room(ch, original);
		ch->furniture = furniture;
		ch->mounting  = mount;
	}
	pop_call();
	return;
}

/*
	transfer person. [all] and [allgame] are additional parameters
*/

void do_mptransfer( CHAR_DATA *ch, char *argument )
{
	char		arg1[ MAX_INPUT_LENGTH ];
	char		arg2[ MAX_INPUT_LENGTH ];
	int		location;
	CHAR_DATA *victim;

	push_call("do_mptransfer(%p,%p)",ch,argument);

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if (arg1[0] == '\0')
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "bad parameters <mptransfer %s %s>", arg1, arg2);
		}
		pop_call();
		return;
	}

	/* test for numerical name */

	if (*arg1 >= '0' && *arg1 <= '9' && *arg2=='\0' )
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "bad parameters <mptransfer %s %s>", arg1, arg2);
		}
		pop_call();
		return;
	}

	/*
		Thanks to Grodyn for the optional location parameter.
	*/

	if (arg2[0] == '\0')
	{
		location = ch->in_room->vnum;
	}
	else
	{
		if ((location = find_mp_location(ch, arg2)) == -1)
		{
			if (IS_NPC(ch))
			{
				log_build_printf(ch->pIndexData->vnum, "unknown location <mptransfer %s %s>", arg1, arg2);
			}
			pop_call();
			return;
		}
		if (location == ch->in_room->vnum)
		{
			if (IS_NPC(ch))
			{
				log_build_printf(ch->pIndexData->vnum, "already there <mptransfer %s %s>", arg1, arg2);
			}
			pop_call();
			return;
		}
	}

	if (!strcasecmp(arg1, "all" ))
	{
		CHAR_DATA *next_victim;

		if (arg2[0] == '\0')
		{
			if (IS_NPC(ch))
			{
				log_build_printf(ch->pIndexData->vnum, "bad argument <mptransfer %s %s>", arg1, arg2);
			}
			pop_call();
			return;
		}

		for (victim = ch->in_room->first_person ; victim != NULL ; victim = next_victim)
		{
			next_victim = victim->next_in_room;
			if (victim != ch)
			{
				transference( victim, location );
			}
		}
		pop_call();
		return;
	}

	if (!strcasecmp(arg1, "pcs"))
	{
		CHAR_DATA *next_victim;

		if (arg2[0]=='\0')
		{
			if (IS_NPC(ch))
			{
				log_build_printf(ch->pIndexData->vnum, "bad argument <mptransfer %s %s>", arg1, arg2);
			}
			pop_call();
			return;
		}

		for (victim = ch->in_room->first_person ; victim ; victim = next_victim)
		{
			next_victim = victim->next_in_room;

			if (victim != ch && (!IS_NPC(victim) || IS_AFFECTED(victim, AFF_CHARM)))
			{
				transference( victim, location );
			}
		}
		pop_call();
		return;
	}

	if (arg2[0] == '\0')
	{
		if ((victim = get_char_area_even_blinded(ch, arg1)) == NULL)
		{
			if (IS_NPC(ch))
			{
				log_build_printf(ch->pIndexData->vnum, "target not found <mptransfer %s>", arg1);
			}
			pop_call();
			return;
		}
	}
	else
	{
		if ((victim = get_char_room_even_blinded(ch, arg1)) == NULL)
		{
			if (IS_NPC(ch))
			{
				log_build_printf(ch->pIndexData->vnum, "target not found <mptransfer %s %s>", arg1, arg2);
			}
			pop_call();
			return;
		}
	}

	if (victim->in_room == NULL)
	{
		pop_call();
		return;
	}

	if (victim != ch)
	{
		if (is_mounting(victim))
		{
			transference(victim->mounting, location);
		}
		transference(victim, location);
	}
	else
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "target is self <mptransfer %s %s>", arg1, arg2);
		}
	}
	pop_call();
	return;
}

void do_mpdo( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	int value;

	push_call("mpdo(%p,%p)",ch,argument);

	if (!MP_VALID_MOB(ch))
	{
		interpret(ch, argument);
	}
	else
	{
		argument = one_argument_nolower(argument, arg);

		if ((value = find_command(arg, MAX_LEVEL - 1)) > -1)
		{
			(*cmd_table[value].do_fun) (ch, argument);
		}
		else if (!check_social(ch, arg, argument))
		{
			log_build_printf(ch->pIndexData->vnum, "unknown command: %s", arg);
		}
	}
	pop_call();
	return;
}

/*
	force someone to do something.  must be mortal level
	and the all argument only affects those in the room with the mobile
*/

void do_mpforce( CHAR_DATA *ch, char *argument )
{
	char arg[ MAX_INPUT_LENGTH ];

	push_call("do_mpforce(%p,%p)",ch,argument);

	argument = one_argument( argument, arg );

	if (arg[0] == '\0' || argument[0] == '\0')
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "bad syntax <mpforce %s %s>", arg, argument);
		}
		pop_call();
		return;
	}

	if (!strcasecmp(arg, "all"))
	{
		CHAR_DATA *vch;
		CHAR_DATA *vch_next;

		for (vch = ch->in_room->first_person ; vch != NULL ; vch = vch_next)
		{
			vch_next = vch->next_in_room;

			if (!IS_NPC(ch) && !IS_NPC(vch) && vch->level == MAX_LEVEL)
			{
				ch_printf(ch, "%s is a god, you cannot force them to do anything.\n\r",capitalize(vch->name));
				continue;
			}
			if (vch != ch && !IS_AFFECTED(vch, AFF_ETHEREAL))
			{
				if (vch->desc) SET_BIT(CH(vch->desc)->pcdata->interp, INTERP_FORCE);

				do_mpdo(vch, argument);

				if (vch->desc) REMOVE_BIT(CH(vch->desc)->pcdata->interp, INTERP_FORCE);
			}
		}
	}
	else
	{
		CHAR_DATA *victim;

		if ((victim = get_char_room_even_blinded(ch, arg)) == NULL)
		{
			pop_call();
			return;
		}

		if (victim == ch)
		{
			if (IS_NPC(ch))
			{
				log_build_printf(ch->pIndexData->vnum, "mpforce: forcing oneself");
			}
			pop_call();
			return;
		}

		if (!IS_NPC(ch) && !IS_NPC(victim) && victim->level == MAX_LEVEL)
		{
			ch_printf(ch,"%s is a God, and thus cannot be forced.\n\r",capitalize(victim->name));

			pop_call();
			return;
		}

		if (victim->desc) SET_BIT(CH(victim->desc)->pcdata->interp, INTERP_FORCE);

		do_mpdo(victim, argument);

		if (victim->desc) REMOVE_BIT(CH(victim->desc)->pcdata->interp, INTERP_FORCE);
	}
	pop_call();
	return;
}

void do_mpmset( CHAR_DATA *ch, char *argument )
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int value;

	push_call("do_mpmset(%p,%p)",ch,argument);

	smash_tilde( argument );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	strcpy( arg3, argument );

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "bad parameters <mpmset %s %s %s>", arg1, arg2, arg3);
		}
		pop_call();
		return;
	}

	if ((victim = get_char_room_even_blinded(ch, arg1)) == NULL)
	{
		pop_call();
		return;
	}

	value = is_number(arg3) ? atol(arg3) : -1;

	if ( !strcasecmp( arg2, "quest" ) )
	{
		int firstBit,len;

		if (sscanf(arg3,"%d %d %d",&firstBit,&len,&value) != 3)
		{
			if (IS_NPC(ch))
			{
				log_build_printf(ch->pIndexData->vnum, "Bad parameters: <mpmset quest %s>", arg3);
			}
			pop_call();
			return;
		}

		if (IS_NPC(victim))
		{
			set_quest_bits( &victim->npcdata->mob_quest, firstBit, len, value );
		}
		else
		{
			if (IS_NPC(ch))
			{
				set_quest_bits( &victim->pcdata->quest[ch->pIndexData->area->low_r_vnum/100],firstBit,len, value);
			}
			else
			{
				if (ch->level < LEVEL_IMMORTAL)
				{
					if (mud->mudprog_nest == 0)
					{
						bug("%s using quest %s in area [%u]", ch->name, arg3, ch->in_room->area->low_r_vnum);
					}
				}
				set_quest_bits(&victim->pcdata->quest[victim->in_room->area->low_r_vnum/100], firstBit, len, value );
			}
		}
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "questr"))
	{
		int firstBit,len,vnum;

		if (sscanf(arg3,"%d %d %d %d",&vnum, &firstBit,&len,&value)!=4)
		{
			if (IS_NPC(ch))
			{
				log_build_printf(ch->pIndexData->vnum, "Bad parameters: <mpmset questr %s>", arg3);
			}
			pop_call();
			return;
		}
		if (vnum < 0 || vnum >= MAX_VNUM || room_index[vnum]==NULL)
		{
			if (IS_NPC(ch))
			{
				log_build_printf(ch->pIndexData->vnum, "Bad area vnum: <mpmset questr %s>", arg3);
			}
			pop_call();
			return;
		}
		if (IS_NPC(victim))
		{
			set_quest_bits( &victim->npcdata->mob_quest, firstBit, len, value );
		}
		else
		{
			set_quest_bits(&victim->pcdata->quest[room_index[vnum]->area->low_r_vnum/100], firstBit, len, value );
		}
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "randquest" ) )
	{
		int firstBit,len;

		if (sscanf(arg3,"%d %d",&firstBit,&len) !=2)
		{
			if (IS_NPC(ch))
			{
				log_build_printf(ch->pIndexData->vnum, "Bad parameters: <mpmset randquest %s>", arg3);
			}
			pop_call();
			return;
		}

		value = number_bits(len);
		if (IS_NPC(victim))
		{
			set_quest_bits( &victim->npcdata->mob_quest, firstBit, len, value );
		}
		else
		{
			if (IS_NPC(ch))
			{
				set_quest_bits(&victim->pcdata->quest[ch->pIndexData->area->low_r_vnum/100], firstBit, len, value );
			}
			else
			{
				set_quest_bits(&victim->pcdata->quest[victim->in_room->area->low_r_vnum/100], firstBit, len, value );
			}
		}
		pop_call();
		return;
	}

	if (!strcasecmp( arg2, "randquestr"))
	{
		int firstBit,len,vnum;

		if(sscanf(arg3,"%d %d %d",&vnum,&firstBit,&len)!=3)
		{
			if (IS_NPC(ch))
			{
				log_build_printf(ch->pIndexData->vnum, "Bad parameters: <mpmset randquestr %s>", arg3);
			}
			pop_call();
			return;
		}

		if (vnum < 0 || vnum >= MAX_VNUM || room_index[vnum] == NULL)
		{
			if (IS_NPC(ch))
			{
				log_build_printf(ch->pIndexData->vnum, "Bad area vnum: <mpmset randquestr %s>", arg3);
			}
			pop_call();
			return;
		}
		value = number_bits(len);

		if(IS_NPC(victim))
		{
			set_quest_bits( &victim->npcdata->mob_quest, firstBit, len, value );
		}
		else
		{
			set_quest_bits( &victim->pcdata->quest[room_index[vnum]->area->low_r_vnum/100], firstBit, len, value );
		}
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "sex" ) )
	{
		victim->sex = URANGE(0, translate_bits(arg3), 2);

		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "level" ) )
	{
		if ( !IS_NPC(victim) )
		{
			pop_call();
			return;
		}

		if (value < 0 || value > 150 )
		{
			pop_call();
			return;
		}
		victim->level = value;

		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "exp" ) )
	{
		if ( value < 0 )
		{
			pop_call();
			return;
		}
		if (!IS_NPC(victim))
		{
			victim->pcdata->exp = value;
		}
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "gold" ) )
	{
		if ( value < 0 )
		{
			pop_call();
			return;
		}
		victim->gold = value;

		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "hp" ) )
	{
		if ( value < 1 )
		{
			pop_call();
			return;
		}
		victim->max_hit = value;

		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "mana" ) )
	{
		if ( value < 1 )
		{
			pop_call();
			return;
		}
		victim->max_mana = value;

		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "move" ) )
	{
		if ( value < 1 )
		{
			pop_call();
			return;
		}
		victim->max_move = value;

		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "dam"))
	{
		int num, size, plus;
		char char1, char2;

		if (!IS_NPC(victim))
		{
			if (!IS_NPC(ch))
			{
				ch_printf(ch, "Not on players.\n\r");
			}
			else
			{
				log_build_printf(ch->pIndexData->vnum, "Bad target: <mpmset dam %s>", arg3);
			}
			pop_call();
			return;
		}

		value = sscanf(arg3, "%d %c %d %c %d", &num, &char1, &size, &char2, &plus);

		if (char1 != 'd' || char2 != '+' || value != 5)
		{
			if (!IS_NPC(ch))
			{
				ch_printf(ch, "Syntax: dam ?d?+?\n\r");
			}
			else
			{
				log_build_printf(ch->pIndexData->vnum, "Bad syntax: <mpmset dam %s>", arg3);
			}
			pop_call();
			return;
		}

		if (num*size+plus < 1 || num*size+plus > 1000)
		{
			if (!IS_NPC(ch))
			{
				ch_printf(ch, "Dam dice must be between 1 and 1000.\n\r");
			}
			else
			{
				log_build_printf(ch->pIndexData->vnum, "Bad dice: <mpmset dam %s>", arg3);
			}
			pop_call();
			return;
		}

		victim->npcdata->damnodice	= num;
		victim->npcdata->damsizedice	= size;
		victim->npcdata->damplus		= plus;

		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "align" ) )
	{
		if ( value < -1000 || value > 1000 )
		{
			pop_call();
			return;
		}
		victim->alignment = value;

		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "thirst" ) )
	{
		if ( IS_NPC(victim) )
		{
			pop_call();
			return;
		}
		if ( value < 0 || value > 100 )
		{
			pop_call();
			return;
		}
		victim->pcdata->condition[COND_THIRST] = value;

		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "drunk" ) )
	{
		if ( IS_NPC(victim) )
		{
			pop_call();
			return;
		}

		if ( value < 0 || value > 100 )
		{
			pop_call();
			return;
		}
		victim->pcdata->condition[COND_DRUNK] = value;

		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "full" ) )
	{
		if ( IS_NPC(victim) )
		{
			pop_call();
			return;
		}

		if ( value < 0 || value > 100 )
		{
			pop_call();
			return;
		}
		victim->pcdata->condition[COND_FULL] = value;

		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "wait"))
	{
		wait_state(victim, value * PULSE_PER_SECOND);
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "remember"))
	{
		if (!IS_NPC(ch) || !IS_NPC(victim))
		{
			pop_call();
			return;
		}
		STRFREE(ch->npcdata->remember);

		if (!strcasecmp(arg3, "null"))
		{
			victim->npcdata->remember = STRDUPE(str_empty);
		}
		else
		{
			victim->npcdata->remember = STRALLOC( arg3 );
		}
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "name" ) )
	{
		if ( !IS_NPC(victim) )
		{
			pop_call();
			return;
		}
		STRFREE (victim->name );
		if( !strcasecmp( arg3, "null") )
		{
			victim->name = STRALLOC(victim->pIndexData->player_name);
		}
		else
		{
			victim->name = STRALLOC( arg3 );
		}
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "short" ) )
	{
		STRFREE (victim->short_descr );
		if( !strcasecmp( arg3, "null")  )
		{
			if (IS_NPC(victim))
			{
				victim->short_descr = STRALLOC(victim->pIndexData->short_descr);
			}
			else
			{
				victim->short_descr = STRALLOC( "" );
			}
		}
		else
		{
			victim->short_descr = STRALLOC( arg3 );
		}
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "long" ) )
	{
		STRFREE (victim->long_descr );
		if( !strcasecmp( arg3, "null") )
		{
			if( IS_NPC( victim ) )
			{
				victim->long_descr = STRALLOC(victim->pIndexData->long_descr);
			}
			else
			{
				victim->long_descr = STRALLOC( "" );
			}
		}
		else
		{
			victim->long_descr = STRALLOC( arg3 );
		}
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "desc" ) )
	{
		STRFREE (victim->description);
		if( !strcasecmp( arg3, "null") )
		{
			if( IS_NPC(victim))
			{
				victim->description = STRALLOC(victim->pIndexData->description);
			}
			else
			{
				victim->description = STRALLOC( "" );
			}
		}
		else
		{
			victim->description = STRALLOC( arg3 );
		}
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "title" ) )
	{
		if ( IS_NPC(victim) )
		{
			pop_call();
			return;
		}
		set_title( victim, arg3 );

		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "spec" ) )
	{
		if ( !IS_NPC(victim) )
		{
			pop_call();
			return;
		}

		if ( ( victim->pIndexData->spec_fun = spec_lookup( arg3 ) ) == 0 )
		{
			pop_call();
			return;
		}
		pop_call();
		return;
	}

	if (IS_NPC(ch))
	{
		log_build_printf(ch->pIndexData->vnum, "Unknown argument: <mpmset %s %s %s>", arg1, arg2, arg3);
	}
	pop_call();
	return;
}

void do_mposet( CHAR_DATA *ch, char *argument )
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int value;

	push_call("do_mposet(%p,%p)",ch,argument);

	smash_tilde( argument );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	strcpy( arg3, argument );

	if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "bad parameters <mposet %s %s %s>", arg1, arg2, arg3);
		}
		pop_call();
		return;
	}

	if ((obj = get_obj_here_even_blinded(ch, arg1)) == NULL)
	{
		pop_call();
		return;
	}

	value = atol(arg3);

	if ( !strcasecmp( arg2, "value0" ) || !strcasecmp( arg2, "v0" ) )
	{
		obj->value[0] = translate_bits(arg3);
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "value1" ) || !strcasecmp( arg2, "v1" ) )
	{
		obj->value[1] = translate_bits(arg3);
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "value2" ) || !strcasecmp( arg2, "v2" ) )
	{
		obj->value[2] = translate_bits(arg3);
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "value3" ) || !strcasecmp( arg2, "v3" ) )
	{
		obj->value[3] = translate_bits(arg3);
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "extra" ) )
	{
		obj->extra_flags = translate_bits(arg3);
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "setextra" ) )
	{
		obj->extra_flags = obj->extra_flags | translate_bits(arg3);
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "clrextra" ) )
	{
		DEL_BIT(obj->extra_flags, translate_bits(arg3));
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "wear" ) )
	{
		obj->wear_flags = translate_bits(arg3);
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "level" ) )
	{
		obj->level = value;
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "weight" ) )
	{
		obj->weight = value;
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "cost" ) )
	{
		obj->cost = value;
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "timer" ) )
	{
		SET_BIT(obj->extra_flags, ITEM_MODIFIED);
		obj->timer = value;
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "sactimer" ) )
	{
		obj->sac_timer = value;
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "name"))
	{
		STRFREE(obj->name );
		if (!strcasecmp(arg3, "null"))
		{
			obj->name = STRALLOC(obj->pIndexData->name);
		}
		else
		{
			obj->name = STRALLOC( arg3 );
		}
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "short" ) )
	{
		STRFREE (obj->short_descr );
		if( !strcasecmp( arg3, "null") )
		{
			obj->short_descr = STRALLOC(obj->pIndexData->short_descr);
		}
		else
		{
			SET_BIT(obj->extra_flags, ITEM_MODIFIED);
			obj->short_descr = STRALLOC( arg3 );
		}
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "long"))
	{
		if (!strcasecmp(arg3, "null"))
		{
			RESTRING(obj->long_descr, obj->pIndexData->long_descr);
		}
		else
		{
			RESTRING(obj->long_descr, arg3);
		}
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "desc"))
	{
		if (!strcasecmp( arg3, "null"))
		{
			RESTRING(obj->description, obj->pIndexData->description);
		}
		else
		{
			RESTRING(obj->description, arg3);
		}
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "modified"))
	{
		SET_BIT(obj->extra_flags, ITEM_MODIFIED);
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "quest" ) )
	{
		int firstBit,len;

		if (sscanf(arg3,"%d %d %d",&firstBit,&len,&value)!=3)
		{
			if (IS_NPC(ch))
			{
				log_build_printf(ch->pIndexData->vnum, "bad parameters <mposet quest %s>", arg3);
			}
			pop_call();
			return;
		}
		SET_BIT(obj->extra_flags, ITEM_MODIFIED);
		set_quest_bits( &obj->obj_quest, firstBit, len, value);

		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "randquest" ) )
	{
		int firstBit,len;

		if(sscanf(arg3,"%d %d",&firstBit,&len)!=2)
		{
			if( IS_NPC( ch ))
			{
				log_build_printf(ch->pIndexData->vnum, "bad parameters <mposet randquest %s>", arg3);
			}
			pop_call();
			return;
		}
		SET_BIT(obj->extra_flags, ITEM_MODIFIED);
		value = number_bits( len );
		set_quest_bits( &obj->obj_quest, firstBit, len, value);

		pop_call();
		return;
	}

	if (IS_NPC(ch))
	{
		log_build_printf(ch->pIndexData->vnum, "Unknown argument: <mposet %s %s %s>", arg1, arg2, arg3);
	}
	pop_call();
	return;
}

void do_mpmadd( CHAR_DATA *ch, char *argument )
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int value;

	push_call("do_mpmadd(%p,%p)",ch,argument);

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	strcpy( arg3, argument );

	if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "bad parameters <mpmadd %s %s %s>", arg1, arg2, arg3);
		}
		pop_call();
		return;
	}

	if ((victim = get_char_room_even_blinded( ch, arg1 ) ) == NULL)
	{
		pop_call();
		return;
	}

	value = is_number(arg3) ? atol(arg3) : -1;

	if ( !strcasecmp( arg2, "quest" ) )
	{
		int firstBit,len;

		if(sscanf(arg3,"%d %d %d",&firstBit,&len,&value)!=3)
		{
			if (IS_NPC(ch))
			{
				log_build_printf(ch->pIndexData->vnum, "bad parameters <mpadd quest %s>", arg3);
			}
			pop_call();
			return;
		}
		if(IS_NPC(victim))
		{
			value += get_quest_bits( victim->npcdata->mob_quest, firstBit, len);
		}
		else
		{
			if (IS_NPC(ch))
			{
				value += get_quest_bits(victim->pcdata->quest[ch->pIndexData->area->low_r_vnum/100], firstBit, len);
			}
			else
			{
				value += get_quest_bits(victim->pcdata->quest[victim->in_room->area->low_r_vnum/100], firstBit, len);
			}
		}
		if (IS_NPC(victim))
		{
			set_quest_bits( &victim->npcdata->mob_quest, firstBit, len, value );
		}
		else
		{
			if(IS_NPC(ch))
			{
				set_quest_bits(&victim->pcdata->quest[ch->pIndexData->area->low_r_vnum/100], firstBit, len, value );
			}
			else
			{
				set_quest_bits(&victim->pcdata->quest[victim->in_room->area->low_r_vnum/100],firstBit, len, value);
			}
		}
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "questr" ) )
	{
		int firstBit,len,vnum;

		if(sscanf(arg3,"%d %d %d %d",&vnum,&firstBit,&len,&value)!=4)
		{
			if (IS_NPC(ch))
			{
				log_build_printf(ch->pIndexData->vnum, "bad parameters <mpadd questr %s>", arg3);
			}
			pop_call();
			return;
		}
		if (vnum < 0 || vnum >= MAX_VNUM || room_index[vnum] == NULL)
		{
			if (IS_NPC(ch))
			{
				log_build_printf(ch->pIndexData->vnum, "bad area vnum <mpadd questr %s>", arg3);
			}
			pop_call();
			return;
		}
		if (IS_NPC(victim))
		{
			value += get_quest_bits( victim->npcdata->mob_quest, firstBit, len);
		}
		else
		{
			value += get_quest_bits(victim->pcdata->quest[room_index[vnum]->area->low_r_vnum/100], firstBit, len);
		}
		if(IS_NPC(victim))
		{
			set_quest_bits( &victim->npcdata->mob_quest, firstBit, len, value );
		}
		else
		{
			set_quest_bits( &victim->pcdata->quest[room_index[vnum]->area->low_r_vnum/100],	firstBit, len, value );
		}
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "level"))
	{
		if (IS_NPC(victim))
		{
			victim->level = URANGE(1, victim->level + value, 150);
		}
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "exp"))
	{
		if (!IS_NPC(victim))
		{
			victim->pcdata->exp = URANGE(0, victim->pcdata->exp + value, exp_level(victim->class, LEVEL_HERO-1));
		}
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "practice"))
	{
		if (!IS_NPC(victim))
		{
			victim->pcdata->practice = URANGE(0, victim->pcdata->practice + value, 10000);
		}
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "align"))
	{
		victim->alignment = URANGE(-1000, victim->alignment + value, 1000);
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "thirst"))
	{
		if (!IS_NPC(victim))
		{
			victim->pcdata->condition[COND_THIRST] = URANGE(0, victim->pcdata->condition[COND_THIRST] + value, 100);
		}
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "drunk"))
	{
		if (!IS_NPC(victim))
		{
			victim->pcdata->condition[COND_DRUNK] = URANGE(0, victim->pcdata->condition[COND_DRUNK] + value, 100);
		}
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "full"))
	{
		if (!IS_NPC(victim))
		{
			victim->pcdata->condition[COND_FULL] = URANGE(0, victim->pcdata->condition[COND_FULL] + value, 100);
		}
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "gold"))
	{
		victim->gold = URANGE(0, victim->gold + value, 1000000);
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "money"))
	{
		gold_transaction(victim, value);
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "currhp"))
	{
		victim->hit = URANGE(1, victim->hit + value, victim->max_hit);
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "currmana"))
	{
		victim->mana = URANGE(1, victim->mana + value, victim->max_mana);
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "currmove"))
	{
		victim->move = URANGE(1, victim->move + value, victim->max_move);
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "hp"))
	{
		victim->max_hit = URANGE(1, victim->max_hit + value, 32000);
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "mana"))
	{
		victim->max_mana = URANGE(1, victim->max_mana + value, 32000);
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "move"))
	{
		victim->max_move = URANGE(1, victim->max_move + value, 32000);
		pop_call();
		return;
	}

	if (IS_NPC(ch))
	{
		log_build_printf(ch->pIndexData->vnum, "Unknown argument: <mpmadd %s %s %s>", arg1, arg2, arg3);
	}
	pop_call();
	return;
}

void do_mpoadd( CHAR_DATA *ch, char *argument )
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int value;

	push_call("do_mpoadd(%p,%p)",ch,argument);

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	strcpy( arg3, argument );

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "bad parameters <mpoadd %s %s %s>", arg1, arg2, arg3);
		}
		pop_call();
		return;
	}

	if ((obj = get_obj_here_even_blinded(ch, arg1)) == NULL)
	{
		pop_call();
		return;
	}

	/*
		Snarf the value (which need not be numeric).
	*/

 	value = is_number( arg3 ) ? atol( arg3 ) : -1;

	/*
		Set something.
	*/

	if ( !strcasecmp( arg2, "value0" ) || !strcasecmp( arg2, "v0" ) )
	{
		value+=obj->value[0];
		value=(value < 0)?0:(value > 100)?100:value;
		obj->value[0]=value;

		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "value1" ) || !strcasecmp( arg2, "v1" ) )
	{
		value+=obj->value[1];
		value=(value < 0)?0:(value > 100)?100:value;
		obj->value[1] = value;

		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "value2" ) || !strcasecmp( arg2, "v2" ) )
	{
		value+=obj->value[2];
		value=(value < 0)?0:(value > 100)?100:value;
		obj->value[2] = value;

		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "value3" ) || !strcasecmp( arg2, "v3" ) )
	{
		value+=obj->value[3];
		value=(value < 0)?0:(value > 100)?100:value;
		obj->value[3] = value;

		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "level" ) )
	{
		value += obj->level;
		value = URANGE(1, value, LEVEL_IMMORTAL);
		obj->level = value;

		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "weight"))
	{
		value += obj->weight;
		value=(value < 0)?0:(value > 32000)?32000:value;
		obj->weight = value;

		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "cost"))
	{
		value += obj->cost;
		value = URANGE(0, value, 1 << 31);
		obj->cost = value;

		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "timer" ) )
	{
		value += obj->timer;
		value = URANGE(0, value, 32000);

		SET_BIT(obj->extra_flags, ITEM_MODIFIED);
		obj->timer = value;

		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "quest" ) )
	{
		int firstBit,len;

		if(sscanf(arg3,"%d %d %d",&firstBit,&len,&value)!=3)
		{
			if (IS_NPC(ch))
			{
				log_build_printf(ch->pIndexData->vnum, "bad parameters <mpoadd quest %s>", arg3);
			}
			pop_call();
			return;
		}
		SET_BIT(obj->extra_flags, ITEM_MODIFIED);
		value += get_quest_bits( obj->obj_quest, firstBit, len);
		set_quest_bits( &obj->obj_quest, firstBit, len, value);

		pop_call();
		return;
	}
	if (IS_NPC(ch))
	{
		log_build_printf(ch->pIndexData->vnum, "Unknown argument: <mpoadd %s %s %s>", arg1, arg2, arg3);
	}
	pop_call();
	return;
}

/*
 * mpgorand <startroom> <endroom> <offset> <skipsize>
 *
 * say, you have a 4x4 area from room #6400 to #6415 arranged like:
 *
 *      6400 6401 6402 6403
 *      6404 6405 6406 6407
 *      6408 6409 6410 6411
 *      6412 6413 6414 6415
 *
 * then "mpgorand 6400 6415 0 4" would put the mobile in one of the
 * the following rooms: 6400 6404 6408 6412
 *
 * then "mpgorand 6400 6415 3 4" would put the mobile in one of the
 * the following rooms: 6403 6407 6411 6415
 */

void do_mpgorand( CHAR_DATA *ch, char *argument )
{
	int cnt, vnumrange, startroom, endroom, offset, skipsize, fuzzyroom;

	push_call("do_mpgorand(%p,%p)",ch,argument);

	if (sscanf(argument," %d %d %d %d", &startroom, &endroom, &offset, &skipsize) != 4)
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "Bad syntax: <mpgorand %s>", argument);
		}
		else
		{
			ch_printf(ch, "Syntax: mpgorand <firstRoom> <lastRoom> <offset> <skipSize>\n\r");
		}
		pop_call();
		return;
	}

	if (skipsize < 1 || offset < 0 || startroom > endroom)
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "Mpgorand - bad arguments.");
		}
		pop_call();
		return;
	}

	startroom = startroom + offset;
	vnumrange = endroom - startroom;
	fuzzyroom = startroom + number_range(0, vnumrange / skipsize) * skipsize;

	for (cnt = 0 ; cnt <= vnumrange ; cnt += skipsize)
	{
		if (fuzzyroom > endroom)
		{
			fuzzyroom = startroom;
		}

		if (get_room_index(fuzzyroom))
		{
			if (vnumrange > 1000)
			{
				if (is_room_good_for_teleport(ch, fuzzyroom))
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
		fuzzyroom += skipsize;
	}

	if (cnt > vnumrange)
	{
		log_printf("mpgorand: failed after %d loops", vnumrange);
	}
	else
	{
		transference(ch, fuzzyroom);
	}
	pop_call();
	return;
}

void transference( CHAR_DATA *victim, int location )
{
	CHAR_DATA *mount;

	push_call("transference(%p,%p)",victim,location);

	mount = victim->mounting;

	char_from_room( victim );
	char_to_room( victim, location );

	victim->mounting = mount;

	pop_call();
	return;
}

void do_mpquiet( CHAR_DATA *ch, char *arg )
{
	push_call("do_mpquiet(%p,%p)", ch, arg);

	if (!strcasecmp(arg, "on"))
	{
		SET_BIT(mud->flags, MUD_SKIPOUTPUT);
	}

	if (!strcasecmp(arg, "off"))
	{
		DEL_BIT(mud->flags, MUD_SKIPOUTPUT);
	}
	pop_call();
	return;
}
