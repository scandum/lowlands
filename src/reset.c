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
	Externals
*/

void 		list_resets	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoom ) );
void			add_reset		args( ( ROOM_INDEX_DATA *room, RESET_DATA *new_reset, int index ) );
RESET_DATA *	find_reset	args( ( ROOM_INDEX_DATA *room, int num ) );
RESET_DATA *	parse_reset	args( ( ROOM_INDEX_DATA *room, char *argument, CHAR_DATA *ch ) );

void do_rredit( CHAR_DATA *ch, char *argument )
{
	ROOM_INDEX_DATA *room;
	RESET_DATA      *reset1, *reset2;

	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];

	char *origarg = argument;
	char colg[10], colw[10], colW[10], colM[10];

	push_call("do_rredit(%p,%p)",ch,argument);

	strcpy(colw, ansi_translate_text(ch, "{078}"));
	strcpy(colg, ansi_translate_text(ch, "{028}"));
	strcpy(colW, ansi_translate_text(ch, "{178}"));
	strcpy(colM, ansi_translate_text(ch, "{158}"));

	if (!ch->desc)
	{
		send_to_char( "You have no descriptor.\n\r", ch );
		pop_call();
		return;
	}

	smash_tilde(argument);

	if (ch->pcdata->editmode == MODE_REPEATCMD)
	{
		room = ch->pcdata->edit_ptr;

		if (argument[0] == '?')
		{
			/*
				Show rredit status
			*/
			ch_printf(ch, "Syntax: <command> <argument>\n\r\n\r");
			ch_printf(ch, "Command being one of:\n\r");
			ch_printf(ch, "  del set ? stat done < >\n\r");
			pop_call();
			return;
		}

		if (!strcasecmp(argument, "done"))
		{
			send_to_char( "RRedit mode off.\n\r", ch );

			stop_olc_editing(ch, room->area);

			pop_call();
			return;
		}

		if (!strcmp(argument, ">"))
		{
			if (get_room_index(room->vnum + 1) && can_olc_modify(ch, room->vnum + 1))
			{
				stop_olc_editing(ch, room->area);
				sprintf(buf, "%d", room->vnum + 1);
				do_rredit(ch, buf);
			}
			else
			{
				ch_printf(ch, "Next index not found.\n\r");
			}
			pop_call();
			return;
		}

		if (!strcmp(argument, "<"))
		{
			if (get_room_index(room->vnum - 1) && can_olc_modify(ch, room->vnum - 1))
			{
				stop_olc_editing(ch, room->area);
				sprintf(buf, "%d", room->vnum - 1);
				do_rredit(ch, buf);
			}
			else
			{
				ch_printf(ch, "Prev index not found.\n\r");
			}
			pop_call();
			return;
		}

		if (!strcasecmp(argument, "stat") || argument[0] == ' ')
		{
			list_resets(ch, room);
			pop_call();
			return;
		}
	}
	else if (argument[0] == '\0')
	{
		if (!can_olc_modify(ch, ch->in_room->vnum))
		{
			send_to_char("This room is not in your allocated range.\n\r", ch);
			pop_call();
			return;
		}
		room = ch->in_room;
	}
	else if ((room = get_room_index(atoi(argument))) == NULL)
	{
		send_to_char("Syntax: edit reset [room vnum]\n\r", ch);
		pop_call();
		return;
	}
	else if (!can_olc_modify(ch, room->vnum))
	{
		send_to_char("That vnum is not in your allocated range.\n\r", ch);
		pop_call();
		return;
	}

	argument = one_argument(argument, arg);

	if (!strcasecmp(arg, "del"))
	{
		if (!is_number(argument))
		{
			ch_printf(ch, "Syntax: del <index>");
		}
		else if ((reset1 = find_reset(room, atoi(argument))) == NULL)
		{
			ch_printf(ch, "Index %s not found.\n\r", argument);
		}
		else
		{
			delete_reset(room->area, reset1);
		}
		pop_call();
		return;
	}

	if (!strcasecmp(arg, "set"))
	{
		argument = one_argument(argument, arg);

		if (is_number(arg))
		{
			if ((reset1 = find_reset(room, atoi(arg))) == NULL)
			{
				switch (argument[0])
				{
					default:
						ch_printf(ch, "You can only place <m o d r> resets on a non existant index.\n\r");
						break;
					case 'm': case 'o': case 'd': case 'r':
						if ((reset1 = parse_reset(room, argument, ch)) != NULL)
						{
							LINK(reset1, room->area->first_reset, room->area->last_reset, next, prev);
						}
						break;
				}
				pop_call();
				return;
			}
			if (reset1->command == 'M')
			{
				if (argument[0] == 'g' || argument[0] == 'e')
				{
					if ((reset2 = parse_reset(room, argument, ch)) != NULL)
					{
						reset2->container = reset1;
						INSERT_RIGHT(reset2, reset1, room->area->last_reset, next, prev);
					}
				}
				else if (argument[0] == 'p')
				{
					ch_printf(ch, "You can only (p)ut objects into <g e o> resets.\n\r");
				}
				else
				{
					if ((reset2 = parse_reset(room, argument, ch)) != NULL)
					{
						INSERT_LEFT(reset2, reset1, room->area->first_reset, next, prev);
					}
				}
				pop_call();
				return;
			}
			if (reset1->command == 'O')
			{
				if (argument[0] == 'p')
				{
					if ((reset2 = parse_reset(room, argument, ch)) != NULL)
					{
						reset2->container = reset1;
						reset2->arg3      = reset1->arg1;
						INSERT_RIGHT(reset2, reset1, room->area->last_reset, next, prev);
					}
				}
				else if (argument[0] == 'g' || argument[0] == 'e')
				{
					ch_printf(ch, "You can only (g)ive or (e)quip m resets.\n\r");
				}
				else
				{
					if ((reset2 = parse_reset(room, argument, ch)) != NULL)
					{
						INSERT_LEFT(reset2, reset1, room->area->first_reset, next, prev);
					}
				}
				pop_call();
				return;
			}
			if (reset1->command == 'G' || reset1->command == 'E')
			{
				if (argument[0] == 'p')
				{
					if ((reset2 = parse_reset(room, argument, ch)) != NULL)
					{
						reset2->container = reset1;
						reset2->arg3      = reset1->arg1;
						INSERT_RIGHT(reset2, reset1, room->area->last_reset, next, prev);
					}
				}
				else if (argument[0] == 'g' || argument[0] == 'e')
				{
					ch_printf(ch, "You can only (g)ive or (e)quip m resets.\n\r");
				}
				else
				{
					ch_printf(ch, "You can only insert (p)ut resets into <g e o> resets.\n\r");
				}
				pop_call();
				return;
			}
			if (argument[0] == 'g' || argument[0] == 'e')
			{
				ch_printf(ch, "You can only (g)ive or (e)quip mobile resets.\n\r");
			}
			else if (argument[0] == 'p')
			{
				ch_printf(ch, "You can only (p)ut objects into <g e o> resets.\n\r");
			}
			else
			{
				if ((reset2 = parse_reset(room, argument, ch)) != NULL)
				{
					INSERT_LEFT(reset2, reset1, room->area->first_reset, next, prev);
				}
			}
			pop_call();
			return;
		}
		ch_printf(ch, "Syntax: set <index> <m g e o p d r> <argument>\n\r");
		pop_call();
		return;
	}

	if (ch->pcdata->editmode != MODE_REPEATCMD)
	{
		ch->pcdata->edit_ptr = room;
		ch->pcdata->edittype = EDIT_TYPE_RESET;
		ch_printf(ch, "Editing Room Reset: %d\n\r", room->vnum);
		ch->pcdata->editmode = MODE_REPEATCMD;
	
		if (ch->pcdata)
		{
			if (ch->pcdata->subprompt)
			{
				STRFREE(ch->pcdata->subprompt);
			}
			sprintf(arg, "Editing Room Reset: %d", room->vnum);
			ch->pcdata->subprompt = STRALLOC( arg );
		}
		pop_call();
		return;
	}

	ch->pcdata->editmode = MODE_RESTRICTED;
	interpret(ch, origarg);
	ch->pcdata->editmode = MODE_REPEATCMD;
	ch->pcdata->last_cmd = do_edit;
	pop_call();
	return;
}


RESET_DATA *find_reset(ROOM_INDEX_DATA *room, int numb)
{
	RESET_DATA *reset;
	int num = 0;

	push_call("find_reset(%p,%p)",room,numb);

	for (reset = room->area->first_reset ; reset ; reset = reset->next)
	{
		if (is_room_reset(reset, room) && ++num == numb)
		{
			pop_call();
			return reset;
		}
	}
	pop_call();
	return NULL;
}


bool is_room_reset(RESET_DATA *reset, ROOM_INDEX_DATA *room)
{
	push_call("is_room_reset(%p,%p)",reset,room);

	switch (reset->command)
	{
		case 'M':
		case 'O':
			if (room->vnum == reset->arg3)
			{
				pop_call();
				return TRUE;
			}
			break;
		case 'P':
			pop_call();
			return is_room_reset(reset->container, room);
		case 'G':
		case 'E':
			if (room->vnum == reset->container->arg3)
			{
				pop_call();
				return TRUE;
			}
			break;
		case 'D':
		case 'R':
			if (room->vnum == reset->arg1)
			{
				pop_call();
				return TRUE;
			}
			break;
	}
	pop_call();
	return FALSE;
}

bool is_obj_reset(RESET_DATA *reset, OBJ_INDEX_DATA *obj)
{
	push_call("is_obj_reset(%p,%p)",reset,obj);

	switch (reset->command)
	{
		case 'O':
		case 'P':
		case 'G':
		case 'E':
			if (obj->vnum == reset->arg1)
			{
				pop_call();
				return TRUE;
			}
			break;
	}
	pop_call();
	return FALSE;
}

bool is_mob_reset(RESET_DATA *reset, MOB_INDEX_DATA *mob)
{
	push_call("is_mob_reset(%p,%p)",reset,mob);

	switch (reset->command)
	{
		case 'M':
			if (mob->vnum == reset->arg1)
			{
				pop_call();
				return TRUE;
			}
			break;
	}
	pop_call();
	return FALSE;
}

bool is_door_reset(RESET_DATA *reset, int door)
{
	push_call("is_door_reset(%p,%p)",reset,door);

	switch (reset->command)
	{
		case 'D':
			if (door == reset->arg2)
			{
				pop_call();
				return TRUE;
			}
			break;
	}
	pop_call();
	return FALSE;
}

/*
	Doing this my way, it works - Scandum
*/

void delete_reset( AREA_DATA *pArea, RESET_DATA *pReset )
{
	RESET_DATA *reset;

	push_call("delete_reset(%p,%p)",pArea,pReset);

	if (pReset->command == 'M')
	{
		for (reset = pReset->next ; reset ; )
		{
			if (reset->command == 'M')
			{
				break;
			}
			if (reset->command == 'G' || reset->command == 'E')
			{
				delete_reset(pArea, reset);
				reset = pReset->next;
			}
			else
			{
				reset = reset->next;
			}
		}
	}
	else if (pReset->command == 'E' || pReset->command == 'G' || pReset->command == 'O')
	{
		for (reset = pReset->next ; reset ; )
		{
			if (reset->command != 'P')
			{
				break;
			}
			delete_reset(pArea, reset);

			reset = pReset->next;
		}
	}
	if (pReset->mob) pReset->mob->reset = NULL;
	if (pReset->obj) pReset->obj->reset = NULL;

	UNLINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
	FREEMEM(pReset);

	pop_call();
	return;
}


/*
	Reset one area.
*/

void reset_area( AREA_DATA *pArea )
{
	ROOM_INDEX_DATA	*pRoomIndex;
	MOB_INDEX_DATA		*pMobIndex;
	OBJ_INDEX_DATA		*pObjIndex;
	RESET_DATA		*pReset;
	CHAR_DATA			*mob;
	EXIT_DATA			*pexit;
	OBJ_DATA			*obj;

	bool d0, d1;

	push_call("reset_area(%p)",pArea);

	if (IS_SET(mud->flags, MUD_EMUD_BOOTDB))
	{
		pArea->average_level	= 0;
		pArea->count			= 0;
		pArea->tmp			= 0;

		for (pReset = pArea->first_reset ; pReset ; pReset = pReset->next)
		{
			switch ( pReset->command )
			{
				case 'M':
					if ((pMobIndex = get_mob_index(pReset->arg1)) == NULL)
					{
						log_printf("reset_area: 'M': bad mobile vnum %d.", pReset->arg1);
						abort();
					}
					if ((pRoomIndex = get_room_index(pReset->arg3)) == NULL)
					{
						log_printf("reset_area: 'M': bad room vnum %d.", pReset->arg3);
						abort();
					}
					if (IS_SET(pRoomIndex->room_flags, ROOM_IS_CASTLE))
					{
						pMobIndex->creator_pvnum = pRoomIndex->creator_pvnum;
					}
					if (IS_SET(pMobIndex->affected_by, AFF_ETHEREAL))
					{
						continue;
					}
					if (!IS_SET(pMobIndex->act , ACT_AGGRESSIVE))
					{
						pArea->average_level += pMobIndex->level/3;
					}
					else
					{
						pArea->average_level += pMobIndex->level;
					}
					pArea->tmp++;
					break;

				case 'O':
					if ((pObjIndex = get_obj_index(pReset->arg1)) == NULL)
					{
						log_printf("reset_area: 'O': bad obj vnum %d.", pReset->arg1);
						abort();
					}
					if ((pRoomIndex = get_room_index(pReset->arg3)) == NULL)
					{
						log_printf("reset_area: 'O': bad room vnum %d.", pReset->arg3);
						abort();
					}
					if (IS_SET(pRoomIndex->room_flags, ROOM_IS_CASTLE))
					{
						pObjIndex->creator_pvnum = pRoomIndex->creator_pvnum;
					}
					break;

				case 'P':
					if (get_obj_index(pReset->arg1) == NULL)
					{
						log_printf( "reset_area: 'P': bad obj vnum %d.", pReset->arg1 );
						abort();
					}
					if (pReset->container == NULL)
					{
						log_printf( "reset_area: 'P': bad container vnum %d.", pReset->arg3 );
						abort();
					}
					break;

				case 'G':
				case 'E':
					if (get_obj_index(pReset->arg1) == NULL)
					{
						log_printf("reset_area: '%c' bad obj vnum %d.", pReset->command, pReset->arg1);
						abort();
					}
					if (pReset->container == NULL)
					{
						log_printf( "reset_area: '%c': bad container vnum %d.", pReset->command, pReset->arg3);
						abort();
					}
					break;

				case 'D':
					if (get_room_index(pReset->arg1) == NULL)
					{
						log_printf("reset_area: 'D': bad vnum %d.", pReset->arg1);
						abort();
					}
					if (pReset->arg2 < 0 || pReset->arg2 > 5)
					{
						log_printf("reset_area: 'D': invalid direction: %d.", pReset->arg2);
						abort();
					}
					if (get_room_index(pReset->arg1)->exit[pReset->arg2] == NULL)
					{
						log_printf("reset_area %s: 'D': invalid connection: %d", pArea->name, pReset->arg2);
						abort();
					}
					break;

				case 'R':
					if (get_room_index(pReset->arg1) == NULL)
					{
						log_printf("Reset_area: 'R': bad vnum %d.", pReset->arg1);
						abort();
					}
					if (pReset->arg2 < 0 || pReset->arg2 > 5)
					{
						log_printf("reset_area: 'R': invalid direction: %d", pReset->arg2);
						abort();
					}
					break;

				default:
					log_printf("reset_area: bad command %c.", pReset->command);
					break;
			}
		}
		if (pArea->tmp != 0)
		{
			pArea->average_level /= pArea->tmp;
		}
		else
		{
			pArea->average_level = 100;
		}
	}

	for (pReset = pArea->first_reset ; pReset ; pReset = pReset->next)
	{
		switch ( pReset->command )
		{
			default:
				break;

			case 'M':
				if (pReset->mob)
				{
					continue;
				}

				pMobIndex = mob_index[pReset->arg1];

				if (pMobIndex->total_mobiles >= pReset->arg2 && number_bits(2))
				{
					continue;
				}

				mob			= create_mobile( pMobIndex );
				mob->reset	= pReset;
				pReset->mob	= mob;

				char_to_room( mob, pReset->arg3 );

				mprog_repop_trigger(mob);

				break;

			case 'O':
				if (pReset->obj)
				{
					continue;
				}

				pObjIndex  =  obj_index[pReset->arg1];

				if (pObjIndex->total_objects >= pReset->arg2 && number_bits(2))
				{
					continue;
				}

				obj			= create_object( pObjIndex, 0);
				obj->reset	= pReset;
				pReset->obj	= obj;

				obj_to_room( obj, pReset->arg3 );
				break;

			case 'P':
				if (pReset->obj)
				{
					continue;
				}

				if (pReset->container->obj == NULL)
				{
					continue;
				}

				pObjIndex = obj_index[pReset->arg1];

				if (pObjIndex->total_objects >= pReset->arg2 && number_bits(2) != 0)
				{
					continue;
				}

				obj			= create_object(pObjIndex, 0);
				obj->reset	= pReset;
				pReset->obj	= obj;

				obj_to_obj(obj, pReset->container->obj);
				break;

			case 'G':
			case 'E':
				if (pReset->obj)
				{
					break;
				}

				if (pReset->container->mob == NULL)
				{
					continue;
				}

				mob = pReset->container->mob;

				pObjIndex = obj_index[pReset->arg1];

				if (pObjIndex->total_objects >= pReset->arg2 && number_bits(2))
				{
					continue;
				}

				obj = create_object(pObjIndex, 0);
				obj->reset	= pReset;
				pReset->obj	= obj;

				obj_to_char(obj, mob);

				if (pReset->command == 'E' && get_eq_char(mob, pReset->arg3) == NULL)
				{
					equip_char(mob, obj, pReset->arg3);
				}
				break;

			case 'D':
				pexit = room_index[pReset->arg1]->exit[pReset->arg2];

				REMOVE_BIT( pexit->flags, EX_UNBARRED);
				REMOVE_BIT( pexit->flags, EX_BASHED);

				switch (pReset->arg3)
				{
					case 0:
						REMOVE_BIT( pexit->flags, EX_CLOSED );
						REMOVE_BIT( pexit->flags, EX_LOCKED );
						break;
					case 1:
						SET_BIT(    pexit->flags, EX_CLOSED );
						REMOVE_BIT( pexit->flags, EX_LOCKED );
						break;
					case 2:
						SET_BIT(    pexit->flags, EX_CLOSED );
						SET_BIT(    pexit->flags, EX_LOCKED );
						break;
				}
				break;

			case 'R':
				pRoomIndex = room_index[pReset->arg1];

				for (d0 = 0 ; d0 <= pReset->arg2 ; d0++)
				{
					d1                   = number_range(d0, pReset->arg2);
					pexit                = pRoomIndex->exit[d0];
					pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
					pRoomIndex->exit[d1] = pexit;
				}
				break;
		}
	}
	pArea->count++;

	if (pArea->low_r_vnum == ROOM_VNUM_SCHOOL)
	{
		pArea->age = 39;
	}
	else
	{
		pArea->age = number_fuzzy(33 - pArea->average_level/3);
	}
	pop_call();
	return;
}

/*
	No checks here, debugging is done elsewhere - Scandum
*/

void list_resets(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
	RESET_DATA 		*reset;
	MOB_INDEX_DATA		*mob;
	int num = 0;
	char buf[MAX_STRING_LENGTH];

	push_call("list_resets(%p,%p)",ch,room);

	mob    = NULL;
	buf[0] = '\0';

	for (reset = room->area->first_reset ; reset ; reset = reset->next)
	{
		if (!is_room_reset(reset, room))
		{
			continue;
		}

		cat_sprintf(buf, "{178}%2d{078}] ", ++num);

		switch (reset->command)
		{
			default:
				cat_sprintf(buf, "{118}*** BAD RESET: {178}%c %d %d %d {138}***\n\r", reset->command, reset->arg1, reset->arg2, reset->arg3);
				break;
			case 'M':
				mob = get_mob_index(reset->arg1);

				cat_sprintf(buf, "{158}M {178}[{078}%5d{178}] {178}[{078}%3d{178}]", reset->arg1, reset->arg2);

				if (get_room_index(reset->arg3-1) && IS_SET(get_room_index(reset->arg3-1)->room_flags, ROOM_PET_SHOP))
				{
					strcat(buf, " {138}PET\n\r");
				}
				else
				{
					strcat(buf, "\n\r");
				}
				break;
			case 'G':
			case 'E':
				cat_sprintf(buf, "{128}%c {178}[{078}%5d{178}] [{078}%3d{178}] ", reset->command, reset->arg1, reset->arg2);
				if (reset->command == 'E')
				{
					cat_sprintf(buf, "{138}%s", capitalize_all(type_string(reset->arg3, wear_locs)));
				}
				else if (mob && mob->pShop)
				{
					strcat(buf, "{138}SHOP");
				}
				strcat(buf, "\n\r");
				break;
			case 'O':
				cat_sprintf(buf, "{128}O {178}[{078}%5d{178}] [{078}%3d{178}]\n\r", reset->arg1, reset->arg2);
				break;
			case 'P':
				cat_sprintf(buf, "{128}P {178}[{078}%5d{178}] [{078}%3d{178}]\n\r", reset->arg1, reset->arg2);
				break;
			case 'D':
				cat_sprintf(buf, "{168}D {178}%-5s ", capitalize_all(dir_name[reset->arg2]));
				cat_sprintf(buf, "{138}%s\n\r", capitalize_all(type_string(reset->arg3, reset_exit_types)));
				break;
			case 'R':
				cat_sprintf(buf, "{168}R {178}%s\n\r", capitalize_all(type_string(reset->arg2, reset_rand_types)));
				break;
		}
	}
	if (strlen(buf) > 0)
	{
		send_to_char_color(buf, ch);
	}
	else
	{
		send_to_char( "This room does not have any resets defined.\n\r", ch);
	}
	pop_call();
	return;
}

/*
	Setup put nesting levels, regardless of whether or not the resets will
	actually reset, or if they're bugged.
*/

RESET_DATA *make_reset( char letter, int arg1, int arg2, int arg3 )
{
	RESET_DATA *reset;

	push_call("make_reset(%p,%p,%p,%p)",letter,arg1,arg2,arg3);

	ALLOCMEM( reset, RESET_DATA, 1 );

	reset->command	= letter;
	reset->arg1		= arg1;
	reset->arg2		= arg2;
	reset->arg3		= arg3;

	mud->top_reset++;

	pop_call();
	return reset;
}


void add_reset( ROOM_INDEX_DATA * room, RESET_DATA *new_reset, int index )
{
	RESET_DATA *reset;

	push_call("add_reset(%p,%p,%p)",room,new_reset,index);

	if ((reset = find_reset(room, index)) == NULL)
	{
		LINK(new_reset, room->area->first_reset, room->area->last_reset, next, prev);
	}
	else
	{
		INSERT_RIGHT(new_reset, reset, room->area->last_reset, next, prev);
	}
	pop_call();
	return;
}


/*
	Parse a reset command string into a reset_data structure
*/

RESET_DATA *parse_reset(ROOM_INDEX_DATA *room, char *argument, CHAR_DATA *ch)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char arg4[MAX_INPUT_LENGTH];
	int  val1, val2, val3;

	push_call("parce_reset(%p,%p,%p)",room,argument,ch);

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );
	argument = one_argument( argument, arg4 );

	val1 = atoi( arg2 );
	val2 = atoi( arg3 );
	val3 = atoi( arg4 );

	if (arg2[0] == '\0')
	{
		switch (arg1[0])
		{
			default:
				ch_printf(ch, "Syntax: set <index> <m g e o p d r> <argument>\n\r");
				break;
			case 'm':
				ch_printf(ch, "Syntax: set <index> m <mob vnum> <max>\n\r");
				break;
			case 'g':
				ch_printf(ch, "Syntax: set <index> g <obj vnum> <max>\n\r");
				break;
			case 'e':
				ch_printf(ch, "Syntax: set <index> e <obj vnum> <max> <wear location>\n\r");
				break;
			case 'o':
				ch_printf(ch, "Syntax: set <index> o <obj vnum> <max>\n\r");
				break;
			case 'p':
				ch_printf(ch, "Syntax: set <index> p <obj vnum> <max>\n\r");
				break;
			case 'd':
				ch_printf(ch, "Syntax: set <index> d <dir> <close type>\n\r");
				break;
			case 'r':
				ch_printf(ch, "Syntax: set <index> r <exits to randomize>\n\r");
				break;
		}
		pop_call();
		return NULL;
	}

	switch (arg1[0])
	{
		default:
			parse_reset(room, "", ch);
			break;
		case 'm':
			if (!get_mob_index(val1))
			{
				ch_printf(ch, "set <index> m <%d> no such mob.\n\r", val1);
				break;
			}
			if (!can_olc_modify(ch, val1))
			{
				ch_printf(ch, "vnum %d is not in your allocated range.\n\r", val1);
				break;
			}
			if (val2 == 0)
			{
				val2 = 100;
			}
			pop_call();
			return make_reset('M', val1, URANGE(1, val2, 999), room->vnum);
		case 'o':
			if (!get_obj_index(val1))
			{
				ch_printf(ch, "o <%d> no such obj.\n\r", val1);
				break;
			}
			if (!can_olc_modify(ch, val1))
			{
				ch_printf(ch, "vnum %d is not in your allocated range.\n\r", val1);
				break;
			}
			if (val2 == 0)
			{
				val2 = 100;
			}
			pop_call();
			return make_reset('O', val1, URANGE(1, val2, 999), room->vnum);
		case 'g':
			if (!get_obj_index(val1))
			{
				ch_printf(ch, "set <index> g <%d> no such obj.\n\r", val1);
				break;
			}
			if (!can_olc_modify(ch, val1))
			{
				ch_printf(ch, "vnum %d is not in your allocated range.\n\r", val1);
				break;
			}
			if (val2 == 0)
			{
				val2 = 100;
			}
			pop_call();
			return make_reset('G', val1, URANGE(1, val2, 999), 0);
		case 'e':
			if (!get_obj_index(val1))
			{
				ch_printf(ch, "set <index> e <%d> no such obj.\n\r", val1);
				break;
			}
			if (!can_olc_modify(ch, val1))
			{
				ch_printf(ch, "vnum %d is not in your allocated range.\n\r", val1);
				break;
			}
			if ((val3 = get_flag(arg4, wear_locs)) == -1)
			{
				ch_printf(ch, "Syntax: set <index> e %d <max> <%s>\n\r", val1, give_flags(wear_locs));
				break;
			}
			pop_call();
			return make_reset('E', val1, URANGE(1, val2, 999), val3);
		case 'p':
			if (!get_obj_index(val1))
			{
				ch_printf(ch, "set <index> p <%d> no such obj.\n\r", val1);
				break;
			}
			if (!can_olc_modify(ch, val1))
			{
				ch_printf(ch, "vnum %d is not in your allocated range.\n\r", val1);
				break;
			}
			if (val2 == 0)
			{
				val2 = 100;
			}
			pop_call();
			return make_reset('P', val1, URANGE(1, val2, 999), 0);
		case 'd':
			if ((val2 = direction_door(arg2)) == -1)
			{
				ch_printf(ch, "set <index> d <%s> is not a valid direction.\n\r", arg2);
				break;
			}
			if (room->exit[val2] == NULL)
			{
				ch_printf(ch, "There is no exit to the %s.\n\r", dir_name[val2]);
				break;
			}
			if ((val3 = get_flag(arg3, reset_exit_types)) == -1)
			{
				ch_printf(ch, "Syntax: set <index> d <dir> <%s>\n\r", give_flags(reset_exit_types));
				break;
			}
			pop_call();
			return make_reset('D', room->vnum, val2, val3);
		case 'r':	
			if ((val2 = get_flag(arg2, reset_rand_types)) == -1)
			{
				ch_printf(ch, "Syntax: set <index> r <%s>\n\r", give_flags(reset_rand_types));
				break;
			}
			pop_call();
			return make_reset('R', room->vnum, val2, 0);
	}
	pop_call();
	return NULL;
}
