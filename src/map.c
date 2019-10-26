/***************************************************************************
 * Lola  1.0 by Igor van den Hoven.                                        *
 *                                                                         *
 * Emud  3.0 by Igor van den Hoven.                                        *
 ***************************************************************************/

#include "mud.h"


char *	draw_room		args ( ( CHAR_DATA *ch, int vnum ) );
void		draw_map		args ( ( CHAR_DATA *ch ) );

int					area_map_x = 0;
int					area_map_y = 0;
int					area_map[301][101];
sh_int					valid_exit;
short                                   search_stamp;

sh_int		area_map_color[]	 = {0,1,0,1,1,0,0,0,0,1,1,0,0,0,1,0,0,1,0,1,0,1,
							   7,5,3,2,7,6,4,4,4,6,3,1,2,3,4,6,6,0,2,3,7,0};

char           *area_map_palet_unicode[] = { "⭑", "╹", "╺", "┗", "╻", "┃", "┏", "┣", "╸", "┛", "━", "┻", "┓", "┫", "┳", "╋" };
sh_int		area_map_palet0[]	 = {  42,  35,  35,  43,  35, 124,  43,  43,  35,  43,  45,  43,  43,  43,  43,  43 };
sh_int		area_map_palet1[]	 = { 126, 247, 247, 109, 247, 120, 108, 116, 247, 106, 113, 118, 107, 117, 119, 110 };

typedef struct map_data		MAP_DATA;
typedef struct map_room		MAP_ROOM;

struct __attribute__((packed)) map_data
{
	char		id[4];
	short int	version;
	short int length;
	short int tileset;
	short int width;
	short int height;
	short int spare;
};

struct __attribute__((packed)) map_room
{
	short int	sector:6;
	short int exit:6;
	short int spare:4;
};

int valid_map_exit(CHAR_DATA *mob, int vnum, sh_int door)
{
	int dest;

	push_call("valid_map_exit(%p,%d,%d",mob,vnum,door);

	if (vnum < 0 || vnum >= MAX_VNUM)
	{
		log_printf("valid_map_exit(%p, %d, %d) vnum %d is invalid.", mob, vnum, door, vnum);
		pop_call();
		return -1;
	}

	if (room_index[vnum] == NULL)
	{
		log_printf("valid_map_exit(%p, %d, %d) room_index[%d] == NULL", mob, vnum, door, vnum);
		pop_call();
		return -1;
	}

	if (room_index[vnum]->exit[door] == NULL)
	{
		pop_call();
		return -1;
	}

	dest = room_index[vnum]->exit[door]->to_room;

	if (dest < 0 || dest >= MAX_VNUM)
	{
		log_printf("valid_map_exit(%p, %d, %d) dest %d is invalid.", mob, vnum, door, dest);

		pop_call();
		return -1;
	}

	if (room_index[dest] == NULL)
	{
		pop_call();
		return -1;
	}

	if (IS_SET(room_index[dest]->room_flags, ROOM_IS_ENTRANCE))
	{
		pop_call();
		return -1;
	}

	if (IS_SET(room_index[vnum]->exit[door]->flags, EX_RIP))
	{
		pop_call();
		return -1;
	}
	pop_call();
	return dest;
}

void build_area_map(int vnum, CHAR_DATA *mob, sh_int x, sh_int y)
{
	sh_int door, xx = 0, yy = 0;
	int dest;

	room_index[vnum]->stamp = search_stamp;

	area_map[x][y] = vnum;

	if (IS_SET(room_index[vnum]->room_flags, ROOM_MAZE))
	{
		return;
	}

	for (door = 0 ; door < 4 ; door++)
	{
		dest = valid_map_exit(mob, vnum, door);

		if (dest != -1)
		{
			switch (door)
			{
				case 0: xx = x;   yy = y+1; break;
				case 1: xx = x+1; yy = y;   break;
				case 2: xx = x;   yy = y-1; break;
				case 3: xx = x-1; yy = y;   break;
			}

			if (xx < 0 || xx > area_map_x || yy < 0 || yy > area_map_y)
			{
				continue;
			}

			if (area_map[xx][yy])
			{
				continue;
			}

			if (room_index[dest]->stamp == search_stamp)
			{
				continue;
			}

			build_area_map(dest, mob, xx, yy);
		}
	}
}


void do_wizmap( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH * 2];
	int x, y, size;

	push_call("do_wizmap1(%p,%p)",ch,argument);

	if (!can_olc_modify(ch, ch->in_room->vnum))
	{
		send_to_char("This room is not in your allocated range.\n\r", ch);
		pop_call();
		return;
	}

	search_stamp++;

	memset(area_map, 0, sizeof(area_map));
	memset(buf, 0, MAX_STRING_LENGTH);

	if (!str_prefix("draw", argument))
	{
		TOG_BIT(ch->pcdata->vt100_flags, VT100_DRAW);
		pop_call();
		return;
	}

	if (!str_prefix("goto ", argument))
	{
		if (sscanf(argument, "%s %d %d", buf, &x, &y) != 3)
		{
			send_to_char("Syntax: wizmap goto <x> <y>\n\r", ch);
			pop_call();
			return;
		}
		area_map_y = 90;
		area_map_x = 200;

		build_area_map(ch->in_room->vnum, ch, area_map_x/2, area_map_y/2);

		if (area_map[URANGE(0, area_map_x / 2 + x, area_map_x)][URANGE(0, area_map_y / 2 + y, area_map_y)])
		{
			sprintf(buf, "%d", area_map[area_map_x / 2 + x][area_map_y / 2 + y]);
			do_goto(ch, buf);
		}
		else
		{
			send_to_char("No room found at given cooridinate.\n\r", ch);
		}
		pop_call();
		return;
	}

	size = *argument ? atol(argument) : 10;

	area_map_y = get_pager_breakpt(ch) -1;
	area_map_x = get_page_width(ch) -1;

	area_map_y = URANGE(10, size * 1, area_map_y);
	area_map_x = URANGE(40, size * 4, area_map_x);

	pop_call();
	push_call("do_wizmap2(%p,%p)",ch,argument);

	build_area_map(ch->in_room->vnum, ch, area_map_x/2, area_map_y/2);

	pop_call();
	push_call("do_wizmap3(%p,%p)",ch,argument);

	for (y = area_map_y ; y >= 0 ; y--)
	{
		buf[0] = 0;

		for (x = 0 ; x <= area_map_x ; x++)
		{
			strcat(buf, draw_room(ch, area_map[x][y]));
		}
		strcat(buf, "\n\r");

		ch_printf(ch, "%s", ansi_compress(ch, buf, COLOR_TEXT, VT102_DIM));
	}
	pop_call();
	return;
}


char *draw_room(CHAR_DATA *ch, int vnum)
{
	static char buf[11];
	sh_int door, exits;

	push_call("draw_room(%p,%p)",ch,vnum);

	if (vnum == 0)
	{
		pop_call();
		return " ";
	}

	if (IS_SET(room_index[vnum]->room_flags, ROOM_MAZE))
	{
		sprintf(buf, "{118}M");

		pop_call();
		return buf;
	}

	if (vnum == ch->in_room->vnum)
	{
		sprintf(buf, "{118}X");

		pop_call();
		return buf;
	}

	sprintf(buf, "{%d%d8}", area_map_color[room_index[vnum]->sector_type], area_map_color[SECT_MAX + room_index[vnum]->sector_type]);

	for (exits = door = 0 ; door < 4 ; door++)
	{
		if (valid_map_exit(ch, room_index[vnum]->vnum, door) != -1)
		{
			SET_BIT(exits, 1 << door);
		}
	}

	if (HAS_BIT(ch->pcdata->vt100_flags, VT100_DRAW))
	{
		cat_sprintf(buf, "%s", area_map_palet_unicode[exits]);
	}
	else
	{
		cat_sprintf(buf, "%c", area_map_palet0[exits]);
	}
	pop_call();
	return buf;
}

/*
	Scandum - 28-05-2003
*/

void draw_map(CHAR_DATA *ch)
{
	FILE	*fp;
	MAP_DATA	*map;
	MAP_ROOM	*room;
	char		buf[100];
	int		x, y, door, exits;

	push_call("draw_map(%p)",ch);

	ALLOCMEM(map,  MAP_DATA, 1);
	ALLOCMEM(room, MAP_ROOM, 1);

	memcpy(map->id, "MAP$", 4);
	map->version	= 1 * 256;
	map->length	= sizeof(MAP_DATA);
	map->tileset	= 0;
	map->width	= area_map_x + 1;
	map->height	= area_map_y + 1;
	map->spare	= 0;

	sprintf(buf, "maps/%04d.map", ch->in_room->area->low_r_vnum / 100);

	if ((fp = my_fopen(buf, "w", FALSE)) == NULL)
	{
		ch_printf(ch, "Failed to open map file.\n\r");
		pop_call();
		return;
	}

	fwrite(map, sizeof(MAP_DATA), 1, fp);

	for (y = area_map_y ; y >= 0 ; y--)
	{
		for (x = 0 ; x <= area_map_x ; x++)
		{
			if (area_map[x][y])
			{
				for (exits = door = 0 ; door < 6 ; door++)
				{
					if (valid_map_exit(ch, area_map[x][y], door) != -1)
					{
						SET_BIT(exits, 1 << door);
					}
				}
				room->sector	= room_index[area_map[x][y]]->sector_type;
				room->exit	= exits;
			}
			else
			{
				room->sector	= 63; /* reserved for NULL rooms */
				room->exit	= 0;
			}
			fwrite(room, sizeof(MAP_ROOM), 1, fp);
		}
	}
	my_fclose(fp);

	FREEMEM(map);
	FREEMEM(room);

	pop_call();
	return;
}
