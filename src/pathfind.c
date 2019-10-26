/***************************************************************************
 * Lola  1.0 by Igor van den Hoven.                                        *
 *                                                                         *
 * Emud  3.0 by Igor van den Hoven.                                        *
 ***************************************************************************/

#include "mud.h"

bool			world_index[2000];

ROOM_INDEX_DATA	*	pfind_dest;
CHAR_DATA			*	pfind_char;
CHAR_DATA 		*	pfind_mob;
AREA_DATA			*	pfind_area;
char				*	pfind_name;

bool					pfind_cnt;
sh_int				valid_exit;
sh_int				pfind_dir;

bool valid_pfind_exit( EXIT_DATA *pexit )
{
	if (room_index[pexit->to_room] == NULL)
	{
		return FALSE;
	}

	if (room_index[pexit->to_room]->area != pfind_area)
	{
		return FALSE;
	}

	return TRUE;
}


/*
	Fast short distance node weighted algorithm, can be used to backtrack the
	entire shortest path - Scandum
*/

void findpath_dest(ROOM_INDEX_DATA *room, bool cnt)
{
	bool door;

	world_index[room->vnum - room->area->low_r_vnum] = cnt;

	if (room == pfind_dest)
	{
		pfind_cnt  = cnt;
		valid_exit = pfind_dir;
	}
	else if (cnt < pfind_cnt)
	{
		cnt++;

		for (door = 0 ; door < 6 ; door++)
		{
			if (room->exit[door] && valid_pfind_exit(room->exit[door]))
			{
				if (world_index[room->exit[door]->to_room - room->area->low_r_vnum] > cnt)
				{
					findpath_dest(room_index[room->exit[door]->to_room], cnt);
				}
			}
		}
	}
}

void findpath_name(ROOM_INDEX_DATA *room, bool cnt)
{
	CHAR_DATA *rch;
	bool door;

	world_index[room->vnum - room->area->low_r_vnum] = cnt;

	if (pfind_cnt > cnt)
	{
		for (rch = room->first_person ; rch ; rch = rch->next_in_room)
		{
			if (is_name_short(pfind_name, rch->name) && can_see(pfind_char, rch) && can_hear(pfind_char, rch))
			{
				pfind_cnt = cnt;
				pfind_mob = rch;
				return;
			}
		}

		cnt++;

		for (door = 0 ; door < 6 ; door++)
		{
			if (room->exit[door] && valid_pfind_exit(room->exit[door]))
			{
				if (world_index[room->exit[door]->to_room - room->area->low_r_vnum] > cnt)
				{
					findpath_name(room_index[room->exit[door]->to_room], cnt);
				}
			}
		}
	}
}

int findpath_search_victim( CHAR_DATA *ch, CHAR_DATA *victim, int range )
{
	int area_size;

	push_call("findpath_search_victim(%p,%p,%p)",ch,victim,range);

	if (ch->in_room == victim->in_room)
	{
		pop_call();
		return -2;
	}

	if (ch->in_room->area != victim->in_room->area)
	{
		pop_call();
		return -1;
	}

	if ((area_size = ch->in_room->area->hi_r_vnum - ch->in_room->area->low_r_vnum) >= 2000)
	{
		log_build_printf(ch->in_room->vnum, "findpath_search_victim: Area too big");
		pop_call();
		return -1;
	}

	pfind_area = ch->in_room->area;
	pfind_mob  = victim;
	pfind_dest = victim->in_room;
	pfind_cnt  = range;
	valid_exit = -1;

	memset(world_index, 128, area_size);
	world_index[ch->in_room->vnum - ch->in_room->area->low_r_vnum] = 0;

	for (pfind_dir = 0 ; pfind_dir < 6 ; pfind_dir++)
	{
		if (ch->in_room->exit[pfind_dir] && valid_pfind_exit(ch->in_room->exit[pfind_dir]))
		{
			findpath_dest(room_index[ch->in_room->exit[pfind_dir]->to_room], 1);
		}
	}
	pop_call();
	return valid_exit;
}



CHAR_DATA *findpath_search_name( CHAR_DATA *ch, char *name, int range )
{
	CHAR_DATA *victim;
	int area_size;

	push_call("findpath_search_name(%p,%p,%p)",ch,name,range);

	if ((victim = get_char_room(ch, name)) != NULL)
	{
		pop_call();
		return victim;
	}

	if ((area_size = ch->in_room->area->hi_r_vnum - ch->in_room->area->low_r_vnum) >= 2000)
	{
		log_build_printf(ch->in_room->vnum, "findpath_search_name: Area too big");
		pop_call();
		return NULL;
	}

	pfind_area = ch->in_room->area;
	pfind_name = name;
	pfind_mob  = NULL;
	pfind_char = ch;
	pfind_cnt  = range;

	memset(world_index, 128, area_size);
	world_index[ch->in_room->vnum - ch->in_room->area->low_r_vnum] = 0;

	for (pfind_dir = 0 ; pfind_dir < 6 ; pfind_dir++)
	{
		if (ch->in_room->exit[pfind_dir] && valid_pfind_exit(ch->in_room->exit[pfind_dir]))
		{
			findpath_name(room_index[ch->in_room->exit[pfind_dir]->to_room], 1);
		}
	}
	pop_call();
	return pfind_mob;
}
