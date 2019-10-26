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
	even_blinded functions to support mobile commands - Scandum 20/03/02
*/

OBJ_DATA *get_obj_here_even_blinded( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;

	push_call("get_obj_here_even_blinded(%p,%p)",ch,argument);

	if ((obj = get_obj_wear_even_blinded(ch, argument)) != NULL)
	{
		pop_call();
		return obj;
	}

	if ((obj = get_obj_carry_even_blinded(ch, argument)) != NULL)
	{
		pop_call();
		return obj;
	}

	if ((obj = get_obj_list_even_blinded(ch, argument, ch->in_room->first_content)) != NULL)
	{
		pop_call();
		return obj;
	}
	pop_call();
	return NULL;
}

OBJ_DATA *get_obj_wear_even_blinded( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int number, count;

	push_call("get_obj_wear_even_blinded(%p,%p)",ch,argument);

	number = number_argument(argument, arg);
	count  = 0;

	for (obj = ch->first_carrying ; obj != NULL ; obj = obj->next_content)
	{
		if (obj->wear_loc != WEAR_NONE)
		{
			if (objref(arg, obj))
			{
				pop_call();
				return obj;
			}
			if (is_name(arg, obj->name) && ++count == number)
			{
				pop_call();
				return obj;
			}
		}
	}
	pop_call();
	return NULL;
}

OBJ_DATA *get_obj_carry_even_blinded( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int number, count;

	push_call("get_obj_carry_even_blinded(%p,%p)",ch,argument);

	number = number_argument( argument, arg );
	count  = 0;

	for (obj = ch->first_carrying ; obj != NULL ; obj = obj->next_content)
	{
		if (obj->wear_loc == WEAR_NONE)
		{
			if (objref(arg, obj))
			{
				pop_call();
				return obj;
			}
			if (is_name(arg, obj->name) && ++count == number)
			{
				pop_call();
				return obj;
			}
		}
	}
	pop_call();
	return NULL;
}

OBJ_DATA *get_obj_list_even_blinded( CHAR_DATA *ch, char *argument, OBJ_DATA *list )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int number, count;

	push_call("get_obj_list_even_blinded(%p,%p,%p)",ch,argument,list);

	number = number_argument( argument, arg );
	count  = 0;

	for (obj = list ; obj != NULL ; obj = obj->next_content)
	{
		if (objref(arg, obj))
		{
			pop_call();
			return obj;
		}
		if (is_name(arg, obj->name) && ++count == number)
		{
			pop_call();
			return obj;
		}
	}
	pop_call();
	return NULL;
}

OBJ_DATA *get_obj_area_even_blinded( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	char arg[MAX_INPUT_LENGTH];
	int number, count, vnum;

	push_call("get_obj_area_even_blinded(%p,%p)",ch,argument);

	number = number_argument(argument, arg);
	count  = 0;

	for (vnum = ch->in_room->area->low_r_vnum ; vnum <= ch->in_room->area->hi_r_vnum ; vnum++)
	{
		if (room_index[vnum] == NULL)
		{
			continue;
		}

		for (obj = room_index[vnum]->first_content ; obj ; obj = obj->next_content)
		{
			if (objref(arg, obj))
			{
				pop_call();
				return obj;
			}
			if (is_name(arg, obj->name) && ++count == number)
			{
				pop_call();
				return obj;
			}
		}
	}
	pop_call();
	return NULL;
}


OBJ_DATA *get_obj_world_even_blinded( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int number, count;

	push_call("get_obj_world_even_blinded(%p,%p)",ch,argument);

	number = number_argument( argument, arg );
	count  = 0;

	if ((obj = get_obj_here_even_blinded(ch, argument)) != NULL)
	{
		pop_call();
		return obj;
	}

	for (obj = mud->f_obj ; obj ; obj = obj->next)
	{
		if (objref(arg, obj))
		{
			pop_call();
			return obj;
		}
		if (is_name(arg, obj->name) && ++count == number)
		{
			pop_call();
			return obj;
		}
	}
	pop_call();
	return NULL;
}

CHAR_DATA *get_char_room_even_blinded( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *rch;
	int number, count;

	push_call("get_char_room_even_blinded(%p,%p)",ch,argument);

	number = number_argument( argument, arg );
	count  = 0;

	if (!strcasecmp(arg, "self"))
	{
		pop_call();
		return ch;
	}

	for (rch = ch->in_room->first_person ; rch != NULL ; rch = rch->next_in_room)
	{
		if (is_name(arg, rch->name))
		{
			if (!IS_NPC(rch) && IS_SET(rch->act, PLR_WIZINVIS))
			{
				continue;
			}
			if (++count == number)
			{
				pop_call();
				return rch;
			}
		}
	}
	pop_call();
	return NULL;
}


CHAR_DATA *get_char_area_even_blinded( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *ach;
	char arg[MAX_INPUT_LENGTH];
	int number, count, vnum;

	push_call("get_char_area_even_blinded(%p,%p)",ch,argument);

	number = number_argument(argument, arg);
	count  = 0;

	for (vnum = ch->in_room->area->low_r_vnum ; vnum <= ch->in_room->area->hi_r_vnum ; vnum++)
	{
		if (room_index[vnum] == NULL)
		{
			continue;
		}

		for (ach = room_index[vnum]->first_person ; ach ; ach = ach->next_in_room)
		{
			if (is_name(arg, ach->name))
			{
				if (!IS_NPC(ach) && IS_SET(ach->act, PLR_WIZINVIS))
				{
					continue;
				}
				if (++count == number)
				{
					pop_call();
					return ach;
				}
			}
		}
	}
	pop_call();
	return NULL;
}


CHAR_DATA *get_char_world_even_blinded( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *wch;
	int number, count;

	push_call("get_char_world(%p,%p)",ch,argument);

	number = number_argument( argument, arg );
	count  = 0;

	if ((wch = get_char_room_even_blinded(ch, argument)) != NULL)
	{
		pop_call();
		return wch;
	}

	if ((wch = get_player_world_even_blinded(ch, argument)) != NULL)
	{
		pop_call();
		return wch;
	}

	for (wch = mud->f_char ; wch ; wch = wch->next)
	{
		if (is_name(arg,wch->name))
		{
			if (!IS_NPC(wch) && IS_SET(wch->act, PLR_WIZINVIS))
			{
				continue;
			}
			if (++count == number)
			{
				pop_call();
				return wch;
			}
		}
	}
	pop_call();
	return NULL;
}

CHAR_DATA *get_player_world_even_blinded( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	PLAYER_GAME *fpl;

	push_call("get_player_world_even_blinded(%p,%p)",ch,argument);

	argument = one_argument( argument, arg );

	for (fpl = mud->f_player ; fpl ; fpl = fpl->next)
	{
		if (is_name(arg, fpl->ch->name))
		{
			if (IS_SET(fpl->ch->act, PLR_WIZINVIS))
			{
				continue;
			}
			pop_call();
			return fpl->ch;
		}
	}
	pop_call();
	return NULL;
}

int find_mp_location( CHAR_DATA *ch, char *arg )
{
	CHAR_DATA *victim;

	push_call("find_mp_location(%p,%p)",ch,arg);

	if (is_number(arg))
	{
		if (get_room_index(atol(arg)))
		{
			pop_call();
			return atol(arg);
		}
		pop_call();
		return -1;
	}

	if ((victim = get_player_world_even_blinded(ch, arg)) != NULL)
	{
		pop_call();
		return victim->in_room->vnum;
	}

	if ((victim = get_char_area_even_blinded(ch, arg)) != NULL)
	{
		pop_call();
		return victim->in_room->vnum;
	}

	pop_call();
	return -1;
}

bool is_room_good_for_teleport( CHAR_DATA *victim, int rvnum )
{
	ROOM_INDEX_DATA *pRoomIndex;

	push_call_format("is_room_good_for_teleport(%p,%d)",victim,rvnum);

	pRoomIndex = get_room_index( rvnum );

	if (pRoomIndex == NULL)
	{
		pop_call();
		return( FALSE );
	}

	if (IS_SET(pRoomIndex->area->flags, AFLAG_NOTELEPORT))
	{
		pop_call();
		return( FALSE );
	}
	if (IS_SET(pRoomIndex->area->flags, AFLAG_NORECALL))
	{
		pop_call();
		return( FALSE );
	}
	if (IS_SET(pRoomIndex->area->flags, AFLAG_NORIP))
	{
		pop_call();
		return( FALSE );
	}

	if (IS_SET(pRoomIndex->room_flags, ROOM_NO_RECALL))
	{
		pop_call();
		return( FALSE );
	}
	if (IS_SET(pRoomIndex->room_flags, ROOM_NO_RIP))
	{
		pop_call();
		return( FALSE );
	}

	if (IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE))
	{
		pop_call();
		return( FALSE );
	}
	if (IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY))
	{
		pop_call();
		return( FALSE );
	}
	if (IS_SET(pRoomIndex->room_flags, ROOM_IS_CASTLE))
	{
		pop_call();
		return( FALSE );
	}
	if (IS_SET(pRoomIndex->room_flags, ROOM_RIP))
	{
		pop_call();
		return( FALSE );
	}

	if (pRoomIndex->area->average_level*3 > victim->level+15)
	{
		pop_call();
		return( FALSE );
	}

	if (pRoomIndex->sector_type == SECT_ETHEREAL)
	{
		pop_call();
		return( FALSE );
	}
	if (pRoomIndex->sector_type == SECT_ASTRAL)
	{
		pop_call();
		return( FALSE );
	}
	if (pRoomIndex->area->low_hard_range != 0 || pRoomIndex->area->hi_hard_range != 0)
	{
		if (victim->level < LEVEL_IMMORTAL)
		{
			if (victim->level < pRoomIndex->area->low_hard_range)
			{
				pop_call();
				return( FALSE );
			}
			if (victim->level > pRoomIndex->area->hi_hard_range)
			{
				pop_call();
				return( FALSE );
			}
		}
	}

	pop_call();
	return( TRUE );
}


void set_exit( int room, int door, int dest)
{
	EXIT_DATA *pexit;

	push_call("set_exit(%p,%p,%p)",room,door,dest);

	if (room <= 0 || dest < -1
	||  room_index[room] == NULL
	||  door < 0 || door > 5
	|| (dest != -1 && room_index[dest] == NULL))
	{
		log_printf("Bad room connect at %d door %d to %d", room, door, dest);
		pop_call();
		return;
	}

	if (room_index[room]->exit[door] == NULL && dest > 0)
	{
		create_exit(room_index[room], door);
	}

	pexit = room_index[room]->exit[door];

	if (dest > 0)
	{
		pexit->to_room	= dest;
	}
	else
	{
		if (pexit)
		{
			pexit->to_room = 0;
		}
	}
	pop_call();
	return;
}

long long translate_bits( char *string )
{
	int cnt, num;
	long long number;
	char c;
	char buf[100];

	push_call("translate_bits(%p)",string);

	cnt    = 0;
	number = 0;

	for (c = string[cnt] ; isdigit(c) || isalpha(c) || c == '_' ; c = string[cnt])
	{
		buf[cnt++] = toupper(c);
	}

	buf[cnt] = '\0';

	if (isupper(buf[0]))
	{
		for (num = mud->bitvector_ref[*buf - 'A'] ; num < MAX_BITVECTOR ; num++)
		{
			if (!strcmp(bitvector_table[num].name, buf))
			{
				number = bitvector_table[num].value;
				break;
			}
		}
		if (num == MAX_BITVECTOR)
		{
			log_printf("translate_bits: bad vector '%s'", buf);
		}
	}
	else
	{
		number = atoll(buf);
	}

	if (c == '|')
	{
		number += translate_bits(&string[cnt + 1]);
	}
	pop_call();
	return number;
}

int is_mp_affected(CHAR_DATA *ch, char *arg )
{
	int sn;
	AFFECT_DATA *paf;

	push_call("is_affected_external(%p,%p)",ch,arg);

	if (ch->first_affect)
	{
		sn = skill_lookup(arg);

		for (paf = ch->first_affect ; paf ; paf = paf->next)
		{
			if (paf->type == sn)
			{
				pop_call();
				return TRUE;
			}
		}
	}
	pop_call();
	return FALSE;
}
