/***************************************************************************
 * Lola  1.0 by Igor van den Hoven.                                        *
 *                                                                         *
 * Emud  2.2 by Igor van den Hoven, Michiel Lange.                         *
 *                                                                         *
 * MrMud 1.4 by David Bills and Dug Michael.                               *
 ***************************************************************************/

#include "mud.h"

bool clanfounder;

bool get_bitvector_value(char *name, int *number, char *allowed)
{
	int cnt;

	push_call("get_bitvector_value(%p,%p,%p)",name,number,allowed);

	if (allowed == NULL && is_number(name))
	{
		*number = atol(name);

		pop_call();
		return TRUE;
	}
	if (is_name_short(allowed, name))
	{
		for (cnt = 0 ; cnt < MAX_BITVECTOR ; cnt++)
		{
			if (is_name(name, bitvector_table[cnt].name))
			{
				*number = bitvector_table[cnt].value;

				pop_call();
				return TRUE;
			}
		}
	}
	*number = -1;

	pop_call();
	return FALSE;
}

void list_bitvectors(CHAR_DATA *ch,char *prefix)
{
	int cnt;
	char buf[81],bigBuf[MAX_STRING_LENGTH];

	push_call("list_bitvectors(%p,%p)",ch,prefix);

	bigBuf[0] = '\0';

	for (cnt = 0 ; cnt < MAX_BITVECTOR ; cnt++)
	{
		if (is_name_short(prefix, bitvector_table[cnt].name))
		{
			sprintf(buf,"   %s\n\r",bitvector_table[cnt].name);
			strcat(bigBuf,buf);
		}
	}

	if (bigBuf[0] == '\0')
	{
		sprintf(buf,"There are no bitvectors starting with '%s'.\n\r",prefix);
		send_to_char(buf,ch);
	}
	else
	{
		send_to_char(bigBuf,ch);
	}

	pop_call();
	return;
}

bool do_castle_mstat( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
 	char tmp[MAX_STRING_LENGTH];
 	char buf1[MAX_STRING_LENGTH];
  	char buf2[MAX_STRING_LENGTH];
 	char buf3[MAX_STRING_LENGTH];
	char bld[10], dim[10];
	CHAR_DATA *victim;
	OBJ_DATA *wield;

	push_call("do_castle_mstat(%p,%p)",ch,argument);

	one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		send_to_char( "Mstat whom?\n\r", ch );
		pop_call();
		return FALSE;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char( "They aren't here.\n\r", ch );
		pop_call();
		return FALSE;
	}

	if (!IS_NPC(victim))
	{
		send_to_char( "Don't be silly.\n\r", ch );
		pop_call();
		return FALSE;
	}

	if (victim->pIndexData->creator_pvnum != ch->pcdata->pvnum)
	{
		send_to_char( "You can't stat them!\n\r", ch );
		pop_call();
		return FALSE;
	}

	strcpy(dim, get_color_string(ch, COLOR_PROMPT,  VT102_DIM ));
	strcpy(bld, get_color_string(ch, COLOR_PROMPT,  VT102_BOLD));

	tmp[0] = '\0';

	ch_printf(ch, "%s        Name: %s%s\n\r"  , dim, bld, victim->name);
	ch_printf(ch, "%s       Short: %s%s\n\r"  , dim, bld, victim->short_descr);
	ch_printf(ch, "%s        Long: %s%s\n\r"  , dim, bld, victim->long_descr);

	sprintf(buf1, "%s        Vnum: %s%11u "   , dim, bld, victim->pIndexData->vnum);
	sprintf(buf2, "%s       Level: %s%11d "   , dim, bld, victim->level);
	sprintf(buf3, "%s        Room: %s%11u\n\r", dim, bld, victim->in_room->vnum);

	ch_printf(ch, "%s%s%s", buf1, buf2, buf3);

	sprintf(buf1, "%s   Alignment: %s%11d "   , dim, bld, victim->alignment);
	sprintf(buf2, "%s Armor Class: %s%11d "   , dim, bld, 0 - (mob_armor(victim) + victim->level*4 - 100));
	sprintf(buf3, "%s        Gold: %s%11d\n\r", dim, bld, victim->gold);

	ch_printf(ch, "%s%s%s", buf1, buf2, buf3);

	sprintf(buf1, "%s     Hitroll: %s%11d "   , dim, bld, GET_HITROLL(victim));
	sprintf(tmp,  "%dd%d+%d", victim->npcdata->damnodice, victim->npcdata->damsizedice, GET_DAMROLL(victim) + victim->npcdata->damplus);
	sprintf(buf2, "%s     Damroll: %s%11s "   , dim, bld, tmp);
	if ((wield = get_eq_char(victim, WEAR_WIELD)) != NULL && wield->item_type == ITEM_WEAPON)
	{
		sprintf(tmp, "%dd%d", wield->value[1], wield->value[2]);
	}
	else
	{
		sprintf(tmp, "none");
	}
	sprintf(buf3, "%s      Weapon: %s%11s\n\r", dim, bld, tmp);

	ch_printf(ch, "%s%s%s", buf1, buf2, buf3);

	sprintf(tmp, "%d/%d", victim->hit, victim->max_hit);
	sprintf(buf1, "%s          Hp: %s%11s "   , dim, bld, tmp);
	sprintf(tmp, "%d/%d", victim->mana, victim->max_mana);
	sprintf(buf2, "%s        Mana: %s%11s "   , dim, bld, tmp);
	sprintf(tmp, "%d/%d", victim->move, victim->max_move);
	sprintf(buf3, "%s        Move: %s%11s\n\r", dim, bld, tmp);

	ch_printf(ch, "%s%s%s", buf1, buf2, buf3);

	sprintf(buf1, "%s        Race: %s%11s "   , dim, bld, race_table[victim->race].race_name);
	sprintf(buf2, "%s         Sex: %s%11s "   , dim, bld, victim->sex <= 0 ? "Neutral" : victim->sex == 1 ? "Male" : "Female");
	sprintf(buf3, "%s    Position: %s%11s\n\r", dim, bld, victim->position == 8 ? "Standing" : victim->position == 7 ? "fighting" : victim->position == 6 ? "sitting" : victim->position == 5 ? "resting" : victim->position == 4 ? "sleeping" : "dying");

	ch_printf(ch, "%s%s%s", buf1, buf2, buf3);

	sprintf(buf1, "%sCarry Number: %s%11d "   , dim, bld, victim->carry_number);
	sprintf(buf2, "%sCarry Weight: %s%11d "   , dim, bld, victim->carry_weight);
	sprintf(buf3, "%sSaving Spell: %s%11d\n\r", dim, bld, GET_SAVING_THROW(victim));

	ch_printf(ch, "%s%s%s", buf1, buf2, buf3);

	ch_printf(ch, "%sAction Flags: %s%s\n\r", dim, bld, flag_string(victim->act, act_flags));
	ch_printf(ch, "%sAffect Flags: %s%s\n\r", dim, bld, affect_bit_name(victim->affected_by));

	if (IS_SET(victim->pIndexData->act, ACT_BODY))
	{
		int cnt;
		char buf[MAX_STRING_LENGTH];

		strcpy(buf, "");

		for (cnt = 0 ; cnt < MAX_BODY ; cnt++)
		{
			if (IS_SET(victim->pIndexData->body_parts, 1<<cnt))
			{
				strcat( buf, body_table[cnt].name);
				strcat( buf, " ");
			}
		}
		ch_printf(ch, "%s  Body Parts: %s%s\n\r", dim, bld, buf);

		strcpy(buf, "");

		for (cnt = 0; cnt < MAX_BODY ; cnt++)
		{
			if (IS_SET(victim->pIndexData->attack_parts, 1<<cnt))
			{
				strcat( buf, body_table[cnt].name);
				strcat( buf, " ");
			}
		}
		ch_printf(ch, "%sAttack Parts: %s%s\n\r", dim, bld, buf);
	}

	pop_call();
	return TRUE;
}

bool do_castle_ostat( CHAR_DATA *ch, char *argument )
{
	char arg [MAX_INPUT_LENGTH];
	char bld [10], dim [10];
	OBJ_DATA    *obj;

	push_call("do_castle_ostat(%p,%p)",ch,argument);

	one_argument(argument, arg);

	if (arg[0] == '\0')
	{
		send_to_char( "Syntax: castle ostat <name>\n\r", ch );
		pop_call();
		return FALSE;
	}

	if ((obj = get_obj_here(ch, arg)) == NULL)
	{
		send_to_char( "You do not see that here.\n\r", ch );
		pop_call();
		return FALSE;
	}

	if (obj->pIndexData->creator_pvnum != ch->pcdata->pvnum)
	{
		send_to_char("You can't ostat that.\n\r", ch );
		pop_call();
		return FALSE;
	}

	strcpy(dim, get_color_string(ch, COLOR_PROMPT,  VT102_DIM ));
	strcpy(bld, get_color_string(ch, COLOR_PROMPT,  VT102_BOLD));

	ch_printf(ch, "%s    Name:%s %s\n\r", dim, bld, obj->name);
	ch_printf(ch, "%s   Short:%s %s\n\r", dim, bld, obj->short_descr);
	ch_printf(ch, "%s    Long:%s %s\n\r", dim, bld, obj->long_descr);
	ch_printf(ch, "%s    Type:%s %s\n\r", dim, bld, item_type_name(obj));
	ch_printf(ch, "%sItemflag:%s %s\n\r", dim, bld, flag_string(obj->extra_flags, o_flags));

	switch (obj->item_type)
	{
		case ITEM_FOUNTAIN:
			ch_printf(ch, "%s  Liquid:%s %s\n\r", dim, bld, liq_table[(obj->value[2])].liq_name);
			break;
		case ITEM_CONTAINER:
			ch_printf(ch, "%s   Carry:%s %d\n\r", dim, bld, obj->value[0]);
			ch_printf(ch, "%s     Lid:%s %s\n\r", dim, bld, flag_string(obj->value[1], cont_flags));
			ch_printf(ch, "%s     Key:%s %s\n\r", dim, bld, obj->value[2] < 1 ? "none" : get_obj_index(obj->value[2]) ? get_obj_index(obj->value[2])->short_descr : "unknown");
			break;
		case ITEM_FURNITURE:
			ch_printf(ch, "%s    Size:%s %d\n\r", dim, bld, obj->value[0]);
			ch_printf(ch, "%s   Poses:%s %s\n\r", dim, bld, flag_string(obj->value[2], f_flags));
			ch_printf(ch, "%s   Regen:%s %d\n\r", dim, bld, obj->value[3]);
			break;
	}

	pop_call();
	return TRUE;
}

void do_castle_rstat( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA        *obj;
	CHAR_DATA       *rch;

	char buf1[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];
	char bld [10], dim [10];
	int door;

	push_call("do_castle_rstat(%p,%p)",ch,argument);

	one_argument(argument, argument);

	strcpy(dim, get_color_string(ch, COLOR_PROMPT,  VT102_DIM ));
	strcpy(bld, get_color_string(ch, COLOR_PROMPT,  VT102_BOLD));

	ch_printf(ch, "%s       Name:%s %s\n\r", dim, bld, ch->in_room->name);
	ch_printf(ch, "%s       Area:%s %s\n\r", dim, bld, ch->in_room->area->name);
	ch_printf(ch, "%s       Vnum:%s %u\n\r", dim, bld, ch->in_room->vnum);
	ch_printf(ch, "%s     Sector:%s %s\n\r", dim, bld, broken_bits(ch->in_room->sector_type, "SECT_", TRUE));

	ch_printf(ch, "%s Room flags: %s%s\n\r", dim, bld, flag_string(ch->in_room->room_flags, r_flags));

	if (ch->in_room->first_extradesc != NULL )
	{
		EXTRA_DESCR_DATA *ed;

		sprintf(buf1, "%sExtra Descs:%s ",      dim, bld);
		sprintf(buf2, "%s,%s ",                 dim, bld);
		for (ed = ch->in_room->first_extradesc ; ed ; ed = ed->next)
		{
			strcat(buf1, ed->keyword);
			if (ed->next)
			{
				strcat(buf1, buf2);
			}
		}
		ch_printf(ch, "%s\n\r", buf1);
	}

	sprintf( buf1, "%s Characters:%s ",     dim, bld);
	sprintf( buf2, "%s,%s ",                dim, bld);
	for (rch = ch->in_room->first_person ; rch ; rch = rch->next_in_room)
	{
		if (can_see(ch, rch))
		{
			if (str_suffix(" ", buf1))
			{
				strcat(buf1, buf2);
			}
			one_argument_nolower(rch->name, buf3);
			strcat(buf1, buf3);
		}
	}
	ch_printf(ch, "%s\n\r", buf1);

	if (ch->in_room->first_content)
	{
		sprintf(buf1, "%s    Objects:%s ",      dim, bld);
		sprintf(buf2, "%s,%s ",                 dim, bld);
		for (obj = ch->in_room->first_content ; obj ; obj = obj->next_content)
		{
			if (can_see_obj(ch, obj))
			{
				if (strlen(buf1) > 33)
				{
					strcat(buf1, buf2);
				}
				one_argument_nolower(obj->name, buf3);
				strcat(buf1, buf3);
			}
		}
		ch_printf(ch, "%s\n\r", buf1);
	}

	for (door = 0 ; door <= 5 ; door++)
	{
		EXIT_DATA *pexit;

		if ((pexit = ch->in_room->exit[door]) != NULL)
		{
			sprintf(buf1,"%s      %5s:%s %-10s", dim, capitalize(dir_name[door]), bld, room_index[pexit->to_room] ? str_resize(room_index[pexit->to_room]->name, argument, -10) : "null");
			sprintf(buf2,"%s Key:%s %-10s",      dim, bld, pexit->key < 1 ? "none" : get_obj_index(pexit->key) ? str_resize(get_obj_index(pexit->key)->short_descr, argument, -10) : "unknown");
			sprintf(buf3,"%s Flags:%s %s",       dim, bld, flag_string(pexit->flags, exit_flags));

			ch_printf(ch, "%s%s%s\n\r", buf1, buf2, buf3);
		}
	}

	pop_call();
	return;
}


void do_castle_entrance( CHAR_DATA *ch, char *argument )
{
	EXIT_DATA       *pexit;
	ROOM_INDEX_DATA *pRoomIndex;
	int vnum, door;

	push_call("do_castle_entrance(%p,%p)",ch,argument);

	for (vnum = ROOM_VNUM_CASTLE ; room_index[vnum] != NULL && vnum < MAX_VNUM ; vnum++);
	
	if (vnum >= MAX_VNUM)
	{
		send_to_char("You cannot create a castle entrance. Contact the Gods.\n\r",ch);

		pop_call();
		return;
	}

	ALLOCMEM(ch->pcdata->castle, CASTLE_DATA, 1);

	ch->pcdata->castle->door_room    = -1;
	ch->pcdata->castle->door_dir     = -1;

	pRoomIndex                = create_room(vnum);

	pRoomIndex->name          = STRALLOC(argument);
	pRoomIndex->area          = room_index[ROOM_VNUM_CASTLE]->area;
	pRoomIndex->vnum          = vnum;
	pRoomIndex->creator_pvnum = ch->pcdata->pvnum;
	pRoomIndex->description   = STRALLOC( "This room is bare of all ornamentation and appears to be relatively new." );
	pRoomIndex->room_flags    = ROOM_IS_CASTLE|ROOM_SAFE|ROOM_IS_ENTRANCE;
	pRoomIndex->sector_type   = SECT_INSIDE;

	for (door = 0 ; door < MAX_LAST_LEFT ; door++)
	{
		pRoomIndex->last_left_name[door] = STRDUPE(str_empty);
	}

	create_exit(pRoomIndex, DIR_DOWN);

	pexit = pRoomIndex->exit[DIR_DOWN];

	RESTRING(pexit->description, "You see a wrinkle in time and space leading to Chakkor.");
	pexit->vnum    = ROOM_VNUM_TEMPLE;
	pexit->to_room = ROOM_VNUM_TEMPLE;

	if (vnum > pRoomIndex->area->hi_r_vnum)
	{
		pRoomIndex->area->hi_r_vnum = vnum;
	}

	if (vnum < pRoomIndex->area->low_r_vnum)
	{
		pRoomIndex->area->low_r_vnum = vnum;
	}

	ch->pcdata->castle->num_rooms += 1;
	ch->pcdata->castle->entrance   = vnum;

	CHECK_AUTOSAVE(room_index[ROOM_VNUM_CASTLE]->area);

	pop_call();
	return;
}

void do_castle_connect( CHAR_DATA *ch, char *argument )
{
	CASTLE_DATA *castle;
	EXIT_DATA *pexit, *castleexit;
	char buf[81], dim[10], bld[10];
	int cost, argn; 

	push_call("do_castle_connect(%p,%p)",ch,argument);

	strcpy(dim, get_color_string(ch, COLOR_PROMPT,  VT102_DIM ));
	strcpy(bld, get_color_string(ch, COLOR_PROMPT,  VT102_BOLD));
	castle = ch->pcdata->castle;

	if (!strcasecmp(argument, "clear"))
	{
		castle->door_room = -1;
		castle->door_dir  = -1;
		send_to_char("Clearing first connection.\n\r", ch);
		pop_call();
		return;
	}

	/*
		Check for backdoors
		Only allow if they already defined first connect - Scandum
	*/

	if (ch->in_room->creator_pvnum != ch->pcdata->pvnum)
	{
		if (castle->door_room == -1)
		{
			send_to_char("You must have defined the first connect to create a backdoor.\n\r", ch);
			pop_call();
			return;
		}

		if (castle->has_backdoor && !clanfounder)
		{
			send_to_char("You can only have one back door.\n\r",ch);
			pop_call();
			return;
		}

		if (clanfounder)
		{
			cost = COST_OF_CLANDOOR * 1000;
			if (ch->pcdata->clan->coffers < cost)
			{
				ch_printf(ch, "It will cost you %s gold coins to build a clan-wide back door.\n\r", tomoney(cost));
				pop_call();
				return;
			}
		}
		else
		{
			cost = bargain(ch, COST_OF_BACKDOOR) * 1000;

			if (ch->gold < cost)
			{
				ch_printf(ch, "It will cost you %s gold coins to build a back door.\n\r", tomoney(cost));
				pop_call();
				return;
			}
		}
	}
	else
	{
		cost = bargain(ch, COST_OF_DOOR) * 1000;
		if (ch->gold < cost)
		{
			ch_printf(ch, "It will cost you %s gold coins to build a connection.\n\r", tomoney(cost));
			pop_call();
			return;
		}
	}

	if (!get_bitvector_value(argument, &argn, "DIR_") && castle->door_room == -1)
	{
		int door;
		char exit_buf[MAX_STRING_LENGTH];

		for (door = 0, exit_buf[0] = '\0' ; door <= 5 ; door++)
		{
			if (ch->in_room->exit[door] == NULL)
			{
				sprintf(buf, "%s%s%s|", bld, broken_bits(door, "DIR_", TRUE), dim);
				strcat(exit_buf, buf);
			}
		}

		if (exit_buf[0] == '\0')
		{
			send_to_char( "There are no available exits here to create a castle connection.\n\r", ch);
		}
		else
		{
			exit_buf[strlen(exit_buf)-1] = '\0';
			ch_printf(ch, "%sSyntax:%s castle connect %s<%s>\n\r", dim, bld, dim, exit_buf);
		}
		pop_call();
		return;
	}

	/*
		default direction is opposite of first one
	*/

	if (castle->door_room != -1 && argn == -1)
	{
		argn = rev_dir[castle->door_dir];
	}

	if (ch->in_room->exit[argn] != NULL)
	{
		send_to_char("You cannot create a connection in that direction.\n\r",ch);
		pop_call();
		return;
	}

	if (!IS_SET(ch->in_room->room_flags, ROOM_IS_CASTLE))
	{
		switch(ch->in_room->sector_type)
		{
			case SECT_FIELD:
			case SECT_FOREST:
			case SECT_HILLS:
			case SECT_MOUNTAIN:
			case SECT_DESERT:
				if (IS_SET(ch->in_room->room_flags, ROOM_NO_CASTLE)
				||  IS_SET(ch->in_room->area->flags, AFLAG_NOCASTLE))
				{
					send_to_char("You cannot build a back door in this room.\n\r",ch);
					pop_call();
					return;
				}
				break;
			default:
				send_to_char("You cannot build a back door in this room.\n\r",ch);
				pop_call();
				return;
				break;
		}
	}

	if (castle->door_room == -1)
	{
		castle->door_room = ch->in_room->vnum;
		castle->door_dir  = argn;
		send_to_char("Now you have to go define the other side of the connection.\n\r",ch);
		pop_call();
		return;
	}

	if (room_index[castle->door_room] == NULL
	||  room_index[castle->door_room]->exit[castle->door_dir] != NULL
	||  room_index[castle->door_room]->creator_pvnum != ch->pcdata->pvnum)
	{
		send_to_char("Your first connection became corrupted.  Clearing first connection.\n\r",ch);
		castle->door_room = -1;
		castle->door_dir  = -1;
		pop_call();
		return;
	}

	if (ch->in_room->creator_pvnum != ch->pcdata->pvnum)
	{
		if (IS_SET(room_index[castle->door_room]->room_flags, ROOM_IS_ENTRANCE))
		{
			send_to_char("You cannot build a back door from your castle entrance.\n\r", ch);
			pop_call();
			return;
		}
		if (IS_SET(ch->in_room->room_flags, ROOM_IS_ENTRANCE))
		{
			send_to_char("You cannot build a back door from an entrance.\n\r", ch);
			pop_call();
			return;
		}
	}

	/*
		other rooms exit points to room
	*/

	create_exit(room_index[castle->door_room], castle->door_dir);
	pexit = room_index[castle->door_room]->exit[castle->door_dir];

	pexit->vnum		= ch->in_room->vnum;
	pexit->to_room		= ch->in_room->vnum;

	castleexit = pexit;

	/*
		room exit points to other room
	*/

	if (room_index[ch->in_room->vnum]->creator_pvnum == ch->pcdata->pvnum)
	{
		create_exit(ch->in_room, argn);
		pexit = ch->in_room->exit[argn];
	
		pexit->vnum			= castle->door_room;
		pexit->to_room			= castle->door_room;
		castleexit			= pexit;
	}

	act("Builders come, take a break, and leave.",ch,NULL,NULL,TO_ROOM);

	/*
		modify the appropriate character and castle data
	*/

	CHECK_AUTOSAVE(room_index[ROOM_VNUM_CASTLE]->area);
	castle->door_room	 = -1;
	castle->door_dir	 = -1;

	if (ch->in_room->creator_pvnum != ch->pcdata->pvnum)
	{
		if (!clanfounder)
		{
			SET_BIT(castleexit->flags, EX_BACKDOOR);
			castle->has_backdoor = TRUE;
			ch->gold			-= cost;
			castle->cost		+= cost/1000;
			send_to_char("The back door has been made.\n\r",ch);
		}
		else
		{
			SET_BIT(castleexit->flags, EX_CLAN_BACKDOOR);
			ch->pcdata->clan->num_backdoors++;
			ch->pcdata->clan->coffers -= cost;
			ch_printf(ch, "You just made a clan-wide backdoor for %s.\n\r", ch->pcdata->clan->name);
		}
	}
	else
	{
		ch->gold			-= cost;
		castle->cost		+= cost/1000;
		send_to_char("The door has been made.\n\r",ch);
	}
	pop_call();
	return;
}

void do_castle_rcreate( CHAR_DATA *ch, char *argument )
{
	CASTLE_DATA *castle;
	EXIT_DATA *pexit;
	ROOM_INDEX_DATA *pRoomIndex;
	char buf[81], dim[10], bld[10];
	int vnum, door, cost, argn;

	push_call("do_castle_rcreate(%p,%p)",ch,argument);

	strcpy(dim, get_color_string(ch, COLOR_PROMPT,  VT102_DIM ));
	strcpy(bld, get_color_string(ch, COLOR_PROMPT,  VT102_BOLD));
	castle = ch->pcdata->castle;

	if (!get_bitvector_value(argument, &argn, "DIR_"))
	{
		char exit_buf[MAX_STRING_LENGTH];

		for (door = 0, exit_buf[0] = '\0' ; door <= 5 ; door++)
		{
			if (ch->in_room->exit[door] == NULL)
			{
				sprintf(buf, "%s%s%s|", bld, broken_bits(door, "DIR_", TRUE), dim);
				strcat(exit_buf, buf);
			}
		}
		if (exit_buf[0] == '\0')
		{
			send_to_char( "There are no available exits here to create a room.\n\r", ch);
		}
		else
		{
			exit_buf[strlen(exit_buf)-1] = '\0';
			ch_printf(ch, "%sSyntax:%s castle rcreate %s<%s>\n\r", dim, bld, dim, exit_buf);
		}
		pop_call();
		return;
	}

	if (ch->in_room->exit[argn] != NULL)
	{
		send_to_char("You cannot create a room in that direction.\n\r",ch);
		pop_call();
		return;
	}

	cost = (bargain(ch, COST_OF_CREATE)) * 1000;

	if (ch->gold < cost)
	{
		ch_printf(ch, "It will cost you %s gold coins to build a room.\n\r", tomoney(cost));
		pop_call();
		return;
	}

	if (castle != NULL && castle->num_rooms >= 20)
	{
		send_to_char("You cannot make any more rooms.\n\r",ch);
		pop_call();
		return;
	}

	/* find an empty vnum */

	for (vnum = ROOM_VNUM_CASTLE ; room_index[vnum] != NULL && vnum < MAX_VNUM ; vnum++);

	if (vnum >= MAX_VNUM)
	{
		send_to_char("I'm sorry, you can't create a room, tell the Implementors.\n\r",ch);
		pop_call();
		return;
	}

	ch_printf(ch, "A contractor comes and takes %s gold coins.\n\r", tomoney(cost));

	sprintf(buf, "The Castle of %s", ch->name);

	pRoomIndex = create_room(vnum);

	pRoomIndex->name			= STRALLOC(buf);
	pRoomIndex->area			= room_index[ROOM_VNUM_CASTLE]->area;
	pRoomIndex->vnum			= vnum;
	pRoomIndex->creator_pvnum	= ch->pcdata->pvnum;
	pRoomIndex->description		= STRALLOC( "This room is bare of all ornamentation and appears to be relatively new." );
	pRoomIndex->room_flags		= ROOM_IS_CASTLE;
	pRoomIndex->sector_type		= SECT_INSIDE;

	for (door = 0 ; door < MAX_LAST_LEFT ; door++)
	{
		pRoomIndex->last_left_name[door] = STRDUPE(str_empty);
	}

	create_exit(pRoomIndex, rev_dir[argn]);
	pexit = pRoomIndex->exit[rev_dir[argn]];

	RESTRING(pexit->description, buf);
	pexit->vnum				= ch->in_room->vnum;
	pexit->to_room				= ch->in_room->vnum;

	create_exit(ch->in_room, argn);
	pexit = ch->in_room->exit[argn];
	RESTRING(pexit->description, buf);
	pexit->vnum				= pRoomIndex->vnum;
	pexit->to_room				= pRoomIndex->vnum;

	if (vnum > pRoomIndex->area->hi_r_vnum)
	{
		pRoomIndex->area->hi_r_vnum = vnum;
	}

	if (vnum < pRoomIndex->area->low_r_vnum)
	{
		pRoomIndex->area->low_r_vnum = vnum;
	}

	act("Builders come, work, take a break, and leave.",ch,NULL,NULL,TO_ROOM);
	ch_printf(ch, "Builders come and build a room %swards.\n\r", dir_name[argn]);

	ch->gold			-= cost;
	castle->cost		+= cost / 1000;
	castle->num_rooms	+= 1;

	CHECK_AUTOSAVE(room_index[ROOM_VNUM_CASTLE]->area);

	pop_call();
	return;
}

void do_castle_mcreate( CHAR_DATA *ch )
{
	AREA_DATA		*pArea;
	MOB_INDEX_DATA	*pMobIndex;
	CHAR_DATA		*mob;
	RESET_DATA	*pReset;
	int vnum, cost;

	push_call("do_castle_mcreate(%p)",ch);

	cost   = bargain(ch, COST_OF_CREATE) * 1000;

	if (ch->gold < cost)
	{
		ch_printf(ch, "It will cost you %s gold coins to create a mobile.\n\r", tomoney(cost));
		pop_call();
		return;
	}

	if (ch->pcdata->castle->num_mobiles >= 20)
	{
		send_to_char("You cannot create more than 20 mobiles.\n\r", ch);
		pop_call();
		return;
	}

	for (vnum = ROOM_VNUM_CASTLE ; get_mob_index(vnum) != NULL && vnum < MAX_VNUM ; vnum++);

	if (vnum >= MAX_VNUM)
	{
		send_to_char("You can't create a mobile, tell the Immortals.\n\r",ch);
		pop_call();
		return;
	}

	ALLOCMEM( pMobIndex, MOB_INDEX_DATA, 1);
	pMobIndex->creator_pvnum        = ch->pcdata->pvnum;
	pMobIndex->vnum                 = vnum;
	pMobIndex->area                 = ch->in_room->area;
	pMobIndex->player_name          = STRALLOC("golem wafer");
	pMobIndex->short_descr          = STRALLOC("a wafer golem");
	pMobIndex->long_descr           = STRALLOC("A wafer golem stands here, looking useless.");
	pMobIndex->description          = STRALLOC("The wafer golem looks *very* dumb.");
	pMobIndex->act                  = ACT_SENTINEL;
	pMobIndex->affected_by          = 0;
	pMobIndex->pShop                = NULL;
	pMobIndex->alignment            = ch->alignment;
	pMobIndex->level                = 0;
	pMobIndex->body_parts           = 0;
	pMobIndex->attack_parts         = 0;
	pMobIndex->hitnodice            = 1;
	pMobIndex->hitsizedice          = 1;
	pMobIndex->hitplus              = 0;
	pMobIndex->damnodice            = 1;
	pMobIndex->damsizedice          = 1;
	pMobIndex->damplus              = 0;
	pMobIndex->gold                 = 0;
	pMobIndex->race                 = ch->race;
	pMobIndex->position             = POS_STANDING;
	pMobIndex->sex                  = SEX_NEUTRAL;
	pMobIndex->first_prog           = NULL;
	pMobIndex->last_prog            = NULL;

	mob_index[vnum]                 = pMobIndex;

	if (vnum > pMobIndex->area->hi_m_vnum)
	{
		pMobIndex->area->hi_m_vnum = vnum;
	}
	if (vnum < pMobIndex->area->low_m_vnum)
	{
		pMobIndex->area->low_m_vnum = vnum;
	}
	mud->top_mob_index++;

	/* make the mobile reset in this room */

	pReset = make_reset('M', vnum, 100, ch->in_room->vnum);
	pArea			= room_index[ROOM_VNUM_CASTLE]->area;
	LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);

	mob			= create_mobile( pMobIndex );
	mob->reset	= pReset;
	pReset->mob	= mob;
	char_to_room( mob, ch->in_room->vnum );

	act("A being takes form in the center of the room.",ch,NULL,NULL,TO_ROOM);
	act("A being takes form in the center of the room.",ch,NULL,NULL,TO_CHAR);

	ch->gold						-= cost;
	ch->pcdata->castle->cost			+= cost/1000;
	ch->pcdata->castle->num_mobiles	+= 1;

	CHECK_AUTOSAVE(room_index[ROOM_VNUM_CASTLE]->area);

	pop_call();
	return;
}

void do_castle_screate( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA		*victim;
	int cost;

	push_call("do_castle_screate(%p)",ch);

	cost   = bargain(ch, COST_OF_CREATE) * 1000;

	if (argument[0] == '\0')
	{
		send_to_char("Syntax: castle screate <mob>\n\r", ch);
		pop_call();
		return;
	}

	if (ch->gold < cost)
	{
		ch_printf(ch, "It will cost you %s gold coins to create a shop.\n\r", tomoney(cost));
		pop_call();
		return;
	}

	if ((victim = get_char_room(ch, argument)) == NULL)
	{
		send_to_char( "They aren't here.\n\r", ch );
		pop_call();
		return;
	}

	if (!IS_NPC(victim) || victim->pIndexData->creator_pvnum != ch->pcdata->pvnum)
	{
		ch_printf(ch, "You cannot turn %s into a shopkeeper.\n\r", get_name(victim));
		pop_call();
		return;
	}

	if (victim->pIndexData->pShop != NULL)
	{
		ch_printf(ch, "%s is already a shopkeeper.\n\r", capitalize(get_name(victim)));
		pop_call();
		return;
	}

	create_shop(victim->pIndexData);

	act("Builders come and build a shop for $N.", ch, NULL, victim, TO_ROOM);
	act("Builders come and build a shop for $N.", ch, NULL, victim, TO_CHAR);

	ch->gold						-= cost;
	ch->pcdata->castle->cost			+= cost/1000;

	CHECK_AUTOSAVE(room_index[ROOM_VNUM_CASTLE]->area);

	pop_call();
	return;
}

void do_castle_ocreate( CHAR_DATA *ch )
{
	AREA_DATA		*pArea;
	OBJ_INDEX_DATA	*pObjIndex;
	OBJ_DATA		*obj;
	RESET_DATA	*pReset;
	int vnum, cost;

	push_call("do_castle_ocreate(%p)",ch);

	cost   = bargain(ch, COST_OF_CREATE) * 1000;

	if (ch->gold < cost)
	{
		ch_printf(ch, "It will cost you %s gold coins to create an object.\n\r", tomoney(cost));
		pop_call();
		return;
	}

	if (ch->pcdata->castle->num_objects >= 20)
	{
		send_to_char("You cannot create more than 20 objects.\n\r", ch);
		pop_call();
		return;
	}

	for (vnum = ROOM_VNUM_CASTLE ; get_obj_index(vnum) != NULL && vnum < MAX_VNUM ; vnum++);

	if (vnum >= MAX_VNUM)
	{
		send_to_char("You can't create an object, tell the Immortals.\n\r",ch);
		pop_call();
		return;
	}

	ALLOCMEM( pObjIndex, OBJ_INDEX_DATA, 1);

	pObjIndex->creator_pvnum		= ch->pcdata->pvnum;
	pObjIndex->vnum			= vnum;
	pObjIndex->area			= ch->in_room->area;

	pObjIndex->name			= STRALLOC("pile dust");
	pObjIndex->short_descr		= STRALLOC("a pile of dust");
	pObjIndex->long_descr		= STRALLOC("A pile of dust lies here.");
	pObjIndex->description		= STRALLOC("The pile of dust looks very dusty.");
	pObjIndex->attack_string		= STRALLOC("");

	pObjIndex->item_type		= ITEM_TRASH;
	pObjIndex->extra_flags		= 0;
	pObjIndex->wear_flags		= 0;
	pObjIndex->value[0]			= 0;
	pObjIndex->value[1]			= 0;
	pObjIndex->value[2]			= 0;
	pObjIndex->value[3]			= 0;
	pObjIndex->weight			= 100;
	pObjIndex->cost			= 100;
	pObjIndex->level			= 100;

	obj_index[vnum]			= pObjIndex;
	mud->top_obj_index++;

	if (vnum > pObjIndex->area->hi_o_vnum)
	{
		pObjIndex->area->hi_o_vnum = vnum;
	}
	if (vnum < pObjIndex->area->low_o_vnum)
	{
		pObjIndex->area->low_o_vnum = vnum;
	}

	/* make the object reset in this room */

	pReset = make_reset('O', vnum, 100, ch->in_room->vnum);
	pArea			= room_index[ROOM_VNUM_CASTLE]->area;
	LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);

	obj			= create_object(pObjIndex, 0);
	obj->reset	= pReset;
	pReset->obj	= obj;
	obj_to_room( obj, ch->in_room->vnum);

	act("A pile of dust heaps up in the center of the room.", ch, NULL, NULL, TO_ROOM);
	act("A pile of dust heaps up in the center of the room.", ch, NULL, NULL, TO_CHAR);

	ch->gold						-= cost;
	ch->pcdata->castle->cost			+= cost/1000;
	ch->pcdata->castle->num_objects	+= 1;

	CHECK_AUTOSAVE(room_index[ROOM_VNUM_CASTLE]->area);

	pop_call();
	return;
}

void do_castle_mdelete( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	push_call("do_castle_mdelete(%p,%p)",ch,argument);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax: castle mdelete <mob>\n\r", ch);
		pop_call();
		return;
	}

	if ((victim = get_char_room(ch, argument)) == NULL)
	{
		send_to_char( "They aren't here.\n\r", ch );
		pop_call();
		return;
	}

	if (victim->pIndexData->creator_pvnum != ch->pcdata->pvnum)
	{
		act("You cannot delete $N.", ch, NULL, victim, TO_CHAR);
		pop_call();
		return;
	}

	if (clanfounder)
	{
		if (IS_SET(victim->act, ACT_CLAN_GUARD))
		{
			ch->pcdata->clan->num_guards--;
		}
		if (IS_SET(victim->act, ACT_CLAN_HEALER))
		{
			ch->pcdata->clan->num_healers--;
		}
		if (IS_SET(victim->act, ACT_TRAIN))
		{
			ch->pcdata->clan->num_trainers--;
		}
		if (IS_SET(victim->act, ACT_PRACTICE))
		{
			ch->pcdata->clan->num_trainers--;
		}
	}

	act("Demolishers come and butcher $N.", ch, NULL, victim, TO_ROOM);
	act("Demolishers come and butcher $N.", ch, NULL, victim, TO_CHAR);

	delete_mob(victim->pIndexData);

	CHECK_AUTOSAVE(room_index[ROOM_VNUM_CASTLE]->area);

	ch->pcdata->castle->num_mobiles	-= 1;
	pop_call();
	return;
}

void do_castle_odelete( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;

	push_call("do_castle_odelete(%p,%p)",ch,argument);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax: castle odelete <obj>\n\r", ch);
		pop_call();
		return;
	}

	if ((obj = get_obj_list(ch, argument, ch->in_room->first_content)) == NULL)
	{
		send_to_char( "You can't find it.\n\r", ch );
		pop_call();
		return;
	}

	if (obj->pIndexData->creator_pvnum != ch->pcdata->pvnum)
	{
		act("You can't delete $p.", ch, obj, NULL, TO_CHAR);
		pop_call();
		return;
	}

	act("Demolishers come and pulverize $p.", ch, obj, NULL, TO_ROOM);
	act("Demolishers come and pulverize $p.", ch, obj, NULL, TO_CHAR);

	delete_obj(obj->pIndexData);

	CHECK_AUTOSAVE(room_index[ROOM_VNUM_CASTLE]->area);

	ch->pcdata->castle->num_objects	-= 1;

	pop_call();
	return;
}

void do_castle_sdelete( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	push_call("do_castle_sdelete(%p,%p)",ch,argument);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax: castle sdelete <mob>\n\r", ch);
		pop_call();
		return;
	}

	if ((victim = get_char_room(ch, argument)) == NULL)
	{
		send_to_char( "They aren't here.\n\r", ch );
		pop_call();
		return;
	}

	if (!IS_NPC(victim) || victim->pIndexData->pShop == NULL)
	{
		ch_printf(ch, "%s is not a shopkeeper.\n\r", capitalize(get_name(victim)));
		pop_call();
		return;
	}

	if (victim->pIndexData->creator_pvnum != ch->pcdata->pvnum)
	{
		ch_printf(ch, "You cannot delete %s's shop.\n\r", get_name(victim));
		pop_call();
		return;
	}

	act("Demolishers come and dismantle $N's shop.", ch, NULL, victim, TO_ROOM);
	act("Demolishers come and dismantle $N's shop.", ch, NULL, victim, TO_CHAR);

	delete_shop(victim->pIndexData);

	CHECK_AUTOSAVE(room_index[ROOM_VNUM_CASTLE]->area);

	pop_call();
	return;
}

void do_castle_rset( CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char bld[10], dim[10], grn[10];
	int argn, cost;

	push_call("do_castle_rset(%p,%p)",ch,argument);

	strcpy(dim, get_color_string(ch, COLOR_PROMPT,  VT102_DIM ));
	strcpy(bld, get_color_string(ch, COLOR_PROMPT,  VT102_BOLD));
	strcpy(grn, ansi_translate_text(ch, "{128}"));

	if (strlen(argument) > 160)
	{
		argument[160] = '\0';
	}
	smash_tilde( argument );

	argument = one_argument( argument, arg1 );

	if (arg1[0] == '\0')
	{
		send_to_char( "Syntax: castle rset <name|desc|flag|sector|door|extra> <argument>\n\r", ch );
		pop_call();
		return;
	}

	cost = bargain(ch, COST_OF_SET) * 1000;

	if (ch->gold < cost)
	{
		ch_printf(ch, "It will cost you %s gold coins to do a rset.\n\r", tomoney(cost));
		pop_call();
		return;
	}

	if (!strcasecmp(arg1, "name"))
	{
		STRFREE (ch->in_room->name );
		ch->in_room->name = STRALLOC( argument );
	}
	else if (!strcasecmp(arg1, "desc"))
	{
		if (ch->pcdata->editmode == MODE_NONE)
		{
			ch->gold     -= cost;
			ch->pcdata->editmode  = MODE_ROOM_DESC;
			ch->pcdata->edit_ptr  = ch->in_room;
			start_editing( ch, ch->in_room->description );
		}
		else
		{
			bug("do_castle_rset: bad mode");
		}
		pop_call();
		return;
	}
	else if (!strcasecmp(arg1, "flag"))
	{
		get_bitvector_value(argument, &argn, "ROOM_");

		if (argn == -1	|| (!IS_SET(CASTLE_R_FLAG_LIST, argn) && (!clanfounder || !IS_SET(CLAN_R_FLAG_LIST, argn))))
		{
			int bit;
			char flag_buf[MAX_STRING_LENGTH];

			ch_printf(ch, "%sSyntax:%s castle rset flag %s<%sroom_flag%s>\n\r\n\r", dim, bld, dim, bld, dim);

			for (bit = 0, flag_buf[0] = '\0' ; bit < 32 ; bit++)
			{
				if (IS_SET(CASTLE_R_FLAG_LIST, 1 << bit))
				{
					cat_sprintf(flag_buf, "%s%-20s", bld, broken_bits(1 << bit, "ROOM_", TRUE));
				}
				if (IS_SET(CLAN_R_FLAG_LIST, 1 << bit) && clanfounder)
				{
					cat_sprintf(flag_buf, "%s%-20s", grn, broken_bits(1 << bit, "ROOM_", TRUE));
				}
				if (strlen(flag_buf) == 120)
				{
					ch_printf(ch, "%s\n\r", flag_buf);
					flag_buf[0] = '\0';
				}
			}
			if (strlen(flag_buf) > 0)
			{
				ch_printf(ch, "%s\n\r", flag_buf);
			}
			pop_call();
			return;
		}
		if (clanfounder && IS_SET(CLAN_R_FLAG_LIST, argn))
		{
			if (ch->pcdata->clan->coffers < CLANHALL_FLAG)
			{
				ch_printf(ch, "It will cost you %s gold coins to set a clan flag.\n\r", tomoney(CLANHALL_FLAG));
				pop_call();
				return;
			}
			ch->pcdata->clan->coffers -= CLANHALL_FLAG;
			switch (argn)
			{
				case ROOM_ALTAR:   ch->pcdata->clan->num_altars  += IS_SET(ch->in_room->room_flags, ROOM_ALTAR)   ? -1 : 1; break;
				case ROOM_BANK:    ch->pcdata->clan->num_banks   += IS_SET(ch->in_room->room_flags, ROOM_BANK)    ? -1 : 1; break;
				case ROOM_MORGUE:  ch->pcdata->clan->num_morgues += IS_SET(ch->in_room->room_flags, ROOM_MORGUE)  ? -1 : 1; break;
			}
		}
		TOGGLE_BIT(ch->in_room->room_flags, argn);
	}
	else if (!strcasecmp(arg1, "sector"))
	{
		if (!get_bitvector_value(argument, &argn, "SECT_"))
		{
			int cnt;
			char sect_buf[MAX_STRING_LENGTH];

			ch_printf(ch, "%sSyntax:%s castle rset sector %s<%ssector_type%s>\n\r\n\r", dim, bld, dim, bld, dim);

			for (cnt = 0, sect_buf[0] = '\0' ; cnt < SECT_MAX ; cnt++)
			{
				cat_sprintf(sect_buf, "%s%-20s", bld, broken_bits(cnt, "SECT_", TRUE));

				if (strlen(sect_buf) == 120)
				{
					ch_printf(ch, "%s\n\r", sect_buf);
					sect_buf[0] = '\0';
				}
			}
			if (strlen(sect_buf) > 0)
			{
				ch_printf(ch, "%s\n\r", sect_buf);
			}
			pop_call();
			return;
		}
		ch->in_room->sector_type = argn;
	}

	else if (!strcasecmp(arg1, "extra"))
	{
		EXTRA_DESCR_DATA *ed;
		int cnt;

		argument = one_argument( argument, arg1 );

		if (!strcasecmp(arg1, "desc"))
		{
			argument = one_argument( argument, arg1 );
			strcpy(arg2, arg1);
		}
		else if (!strcasecmp(arg1, "del"))
		{
			argument = one_argument( argument, arg1 );

			for (ed = ch->in_room->first_extradesc ; ed ; ed = ed->next)
			{
				if (is_name(arg1, ed->keyword))
				{
					ch_printf(ch, "Deleted the following extras: '%s'\n\r", ed->keyword);
					UNLINK( ed, ch->in_room->first_extradesc, ch->in_room->last_extradesc, next, prev);
					STRFREE (ed->description);
					STRFREE (ed->keyword);
					FREEMEM(ed);

					CHECK_AUTOSAVE(room_index[ROOM_VNUM_CASTLE]->area);

					pop_call();
					return;
				}
			}
			ch_printf(ch, "Couldn't find an extra named '%s' to delete.\n\r", arg1);
			pop_call();
			return;
		}
		else
		{
			send_to_char("Syntax: castle rset extra <del|desc> 'keywords'\n\r",ch);
			pop_call();
			return;
		}

		for (cnt = 1, ed = ch->in_room->first_extradesc ; ed ; ed = ed->next, cnt++)
		{
			if (is_name(arg1, ed->keyword))
			{
				break;
			}
			if (cnt >= 10)
			{
				ch_printf(ch, "You cannot create more than ten extra descriptions per room.\n\r");
				pop_call();
				return;
			}
		}

		if (ed == NULL)
		{
			ALLOCMEM(ed, EXTRA_DESCR_DATA, 1);
			ed->description = STRALLOC("");
			ed->keyword     = STRALLOC(arg2);
			LINK( ed, ch->in_room->first_extradesc, ch->in_room->last_extradesc, next, prev);
		}
		if (ch->pcdata->editmode == MODE_NONE)
		{
			ch->gold     -= cost;
			ch->pcdata->editmode  = MODE_ROOM_EXTRA;
			ch->pcdata->edit_ptr  = ed;
			start_editing( ch, ed->description);
		}
		else
		{
			bug( "do_castle_rset: bad mode");
		}
		pop_call();
		return;
	}
	else
	{
		do_castle_rset( ch, "" );
		pop_call();
		return;
	}

	act("The room appears to flicker for an instant.",ch,NULL,NULL,TO_ROOM);
	act("The contractor comes, takes your money, looks around, hammers on a post, then\n\rheads back the way he came.",ch,NULL,NULL,TO_CHAR);

	ch->gold				-= cost;
	ch->pcdata->castle->cost	+= cost/1000;

	CHECK_AUTOSAVE(room_index[ROOM_VNUM_CASTLE]->area);

	pop_call();
	return;
}

void do_castle_mset( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA   *victim;
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	char buf2 [MAX_INPUT_LENGTH];
	char *buf1 = '\0';

	char bld[10], dim[10], grn[10];
	int argn, cost;

	push_call("do_castle_mset(%p,%p)",ch,argument);

	strcpy(dim, get_color_string(ch, COLOR_PROMPT,  VT102_DIM ));
	strcpy(bld, get_color_string(ch, COLOR_PROMPT,  VT102_BOLD));
	strcpy(grn, ansi_translate_text(ch, "{128}"));

	if (strlen(argument) > 160)
	{
		argument[160] = '\0';
	}

	smash_tilde( argument );

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	strcpy( arg3, argument );

	if (arg1[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char("Syntax: castle mset <mob> <sex|race|act|affect|body|attack> <bitvector>\n\r", ch);
		send_to_char("Syntax: castle mset <mob> <level|alignment> <value>\n\r", ch);
		send_to_char("Syntax: castle mset <mob> <name|short|long|desc> <string>\n\r", ch);
		pop_call();
		return;
	}

	cost = bargain(ch, COST_OF_SET) * 1000;

	if (ch->gold < cost)
	{
		ch_printf(ch,"It will cost you %s gold coins to do an mset.\n\r", tomoney(cost));
		pop_call();
		return;
	}

	if ((victim = get_char_room(ch, arg1)) == NULL)
	{
		send_to_char( "They aren't here.\n\r", ch );
		pop_call();
		return;
	}

	if (!IS_NPC(victim) || victim->pIndexData->creator_pvnum != ch->pcdata->pvnum)
	{
		ch_printf(ch, "You cannot mset them.\n\r");
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "sex"))
	{
		if (!get_bitvector_value(argument, &argn, "SEX_"))
		{
			int sex;
			char sex_buf[MAX_STRING_LENGTH];

			for (sex = 0, sex_buf[0] = '\0' ; sex <= 2 ; sex++)
			{
				cat_sprintf(sex_buf, "%s%s%s|", bld, broken_bits(sex, "SEX_", TRUE), dim);
			}
			sex_buf[strlen(sex_buf)-1] = '\0';
			ch_printf(ch, "%sSyntax:%s castle mset %s<%smob%s>%s sex %s<%s>\n\r", dim, bld, dim, bld, dim, bld, dim, sex_buf);
			pop_call();
			return;
		}
		victim->sex = argn;
	}
	else if (!strcasecmp(arg2, "level"))
	{
		int value = is_number(arg3) ? atol(arg3) : -1;

		if (value < 1 || value > ch->level)
		{
			ch_printf(ch, "Level range is between 1 and %d\n\r", ch->level);
			pop_call();
			return;
		}
		victim->pIndexData->level	= victim->level				= value;
		victim->pIndexData->damplus	= victim->npcdata->damplus		= value;
		victim->pIndexData->hitplus	= victim->hit = victim->max_hit	= value * value;
	}
	else if (!strcasecmp(arg2, "alignment"))
	{
		int value = atol(arg3);

		if (value < -1000 || value > 1000)
		{
			ch_printf(ch, "Alignment range is between -1000 and 1000.\n\r");
			pop_call();
			return;
		}
		victim->pIndexData->alignment = victim->alignment = value;
	}
	else if (!strcasecmp(arg2, "race"))
	{
		if (!get_bitvector_value(argument, &argn, "RACE_"))
		{
			int cnt;
			char race_buf[MAX_STRING_LENGTH];

			ch_printf(ch, "%sSyntax:%s castle mset %s<%smob%s>%s race %s<%ssector_type%s>\n\r\n\r", dim, bld, dim, bld, dim, bld, dim, bld, dim);

			for (cnt = 0, race_buf[0] = '\0' ; cnt < MAX_RACE ; cnt++)
			{
				cat_sprintf(race_buf, "%s%-20s", bld, broken_bits(cnt, "RACE_", TRUE));

				if (strlen(race_buf) == 120)
				{
					ch_printf(ch, "%s\n\r", race_buf);
					race_buf[0] = '\0';
				}
			}
			if (strlen(race_buf) > 0)
			{
				ch_printf(ch, "%s\n\r", race_buf);
			}
			pop_call();
			return;
		}
		SET_BIT(victim->act, ACT_RACE);
		victim->race			= argn;
		victim->pIndexData->race	= victim->race;
		victim->pIndexData->act	= victim->act;
	}
	else if (!strcasecmp(arg2, "act"))
	{
		get_bitvector_value(argument, &argn, "ACT_");

		if (argn == -1	|| (!IS_SET(CASTLE_M_ACT_LIST, argn) && (!clanfounder || !IS_SET(CLAN_M_ACT_LIST, argn))))
		{
			int bit;
			char act_buf[MAX_STRING_LENGTH];

			ch_printf(ch, "%sSyntax:%s castle mset %s<%smob%s>%s act %s<%sact_flag%s>\n\r\n\r", dim, bld, dim, bld, dim, bld, dim, bld, dim);

			for (bit = 0, act_buf[0] = '\0' ; bit < 32 ; bit++)
			{
				if (IS_SET(CASTLE_M_ACT_LIST, 1 << bit))
				{
					cat_sprintf(act_buf, "%s%-20s", bld, broken_bits(1 << bit, "ACT_", TRUE));
				}
				if (IS_SET(CLAN_M_ACT_LIST, 1 << bit) && clanfounder)
				{
					cat_sprintf(act_buf, "%s%-20s", grn, broken_bits(1 << bit, "ACT_", TRUE));
				}
				if (strlen(act_buf) == 120)
				{
					ch_printf(ch, "%s\n\r", act_buf);
					act_buf[0] = '\0';
				}
			}
			if (strlen(act_buf) > 0)
			{
				ch_printf(ch, "%s\n\r", act_buf);
			}
			pop_call();
			return;
		}
		if (clanfounder && IS_SET(CLAN_M_ACT_LIST, argn))
		{
			if (ch->pcdata->clan->coffers < CLANHALL_FLAG)
			{
				ch_printf(ch, "It will cost you %s gold coins to set a clan act.\n\r", tomoney(CLANHALL_FLAG));
				pop_call();
				return;
			}
			ch->pcdata->clan->coffers -= CLANHALL_FLAG;
			switch (argn)
			{
				case ACT_CLAN_GUARD:
					ch->pcdata->clan->num_guards   += IS_SET(victim->act, ACT_CLAN_GUARD)  ? -1 : 1;
					break;
				case ACT_CLAN_HEALER:
					ch->pcdata->clan->num_healers  += IS_SET(victim->act, ACT_CLAN_HEALER) ? -1 : 1;
					break;
				case ACT_TRAIN:
					ch->pcdata->clan->num_trainers += IS_SET(victim->act, ACT_TRAIN)       ? -1 : 1;
					break;
				case ACT_PRACTICE:
					ch->pcdata->clan->num_trainers += IS_SET(victim->act, ACT_PRACTICE)    ? -1 : 1;
					break;
			}
		}
		TOGGLE_BIT(victim->act, argn);
		victim->pIndexData->act = victim->act;
	}
	else if (!strcasecmp(arg2, "affect"))
	{
		get_bitvector_value(argument, &argn, "AFF_");

		if (argn == -1	|| !IS_SET(CASTLE_M_AFF_LIST, argn))
		{
			int bit;
			char aff_buf[MAX_STRING_LENGTH];

			ch_printf(ch, "%sSyntax:%s castle mset %s<%smob%s>%s affect %s<%saffect_flag%s>\n\r\n\r", dim, bld, dim, bld, dim, bld, dim, bld, dim);

			for (bit = 0, aff_buf[0] = '\0' ; bit < 32 ; bit++)
			{
				if (IS_SET(CASTLE_M_AFF_LIST, 1 << bit))
				{
					cat_sprintf(aff_buf, "%s%-20s", bld, broken_bits(1 << bit, "AFF_", TRUE));
				}
				if (strlen(aff_buf) == 120)
				{
					ch_printf(ch, "%s\n\r", aff_buf);
					aff_buf[0] = '\0';
				}
			}
			if (strlen(aff_buf) > 0)
			{
				ch_printf(ch, "%s\n\r", aff_buf);
			}
			pop_call();
			return;
		}
		TOGGLE_BIT(victim->affected_by, argn);
		victim->pIndexData->affected_by = victim->affected_by;
	}
	else if (!strcasecmp(arg2, "body"))
	{
		get_bitvector_value(argument, &argn, "BODY_");

		if (argn == -1	|| !IS_SET(CASTLE_M_BODY_LIST, argn))
		{
			int bit;
			char body_buf[MAX_STRING_LENGTH];

			ch_printf(ch, "%sSyntax:%s castle mset %s<%smob%s>%s body %s<%sbody_part%s>\n\r\n\r", dim, bld, dim, bld, dim, bld, dim, bld, dim);

			for (bit = 0, body_buf[0] = '\0' ; bit < 32 ; bit++)
			{
				if (IS_SET(CASTLE_M_BODY_LIST, 1 << bit))
				{
					cat_sprintf(body_buf, "%s%-20s", bld, broken_bits(1 << bit, "BODY_", TRUE));
				}
				if (strlen(body_buf) == 120)
				{
					ch_printf(ch, "%s\n\r", body_buf);
					body_buf[0] = '\0';
				}
			}
			if (strlen(body_buf) > 0)
			{
				ch_printf(ch, "%s\n\r", body_buf);
			}
			pop_call();
			return;
		}
		TOGGLE_BIT(victim->pIndexData->body_parts, argn);
	}
	else if (!strcasecmp(arg2, "attack"))
	{
		get_bitvector_value(argument, &argn, "BODY_");

		if (argn == -1	|| !IS_SET(CASTLE_M_BODY_LIST, argn))
		{
			int bit;
			char body_buf[MAX_STRING_LENGTH];

			ch_printf(ch, "%sSyntax:%s castle mset %s<%smob%s>%s attack %s<%sattack_part%s>\n\r\n\r", dim, bld, dim, bld, dim, bld, dim, bld, dim);

			for (bit = 0, body_buf[0] = '\0' ; bit < 32 ; bit++)
			{
				if (IS_SET(CASTLE_M_BODY_LIST, 1 << bit))
				{
					cat_sprintf(body_buf, "%s%-20s", bld, broken_bits(1 << bit, "BODY_", TRUE));
				}
				if (strlen(body_buf) == 120)
				{
					ch_printf(ch, "%s\n\r", body_buf);
					body_buf[0] = '\0';
				}
			}
			if (strlen(body_buf) > 0)
			{
				ch_printf(ch, "%s\n\r", body_buf);
			}
			pop_call();
			return;
		}
		TOGGLE_BIT(victim->pIndexData->attack_parts, argn);
	}
	else if (!strcasecmp(arg2, "name"))
	{
		int cnt;

		for (cnt = 0 ; cnt < strlen(arg3) ; cnt++)
		{
			if (!isalpha(arg3[cnt]) && !isspace(arg3[cnt]))
			{
				ch_printf(ch, "You may only use alpha characters.\n\r");
				pop_call();
				return;
			}
		}
		for (buf1 = one_argument(arg3, buf2) ; buf2[0] != '\0' ; buf1 = one_argument(buf1, buf2))
		{
			if (char_exists(buf2))
			{
				ch_printf(ch, "There is already a player with that name.\n\r");
				pop_call();
				return;
			}
		}
		STRFREE (victim->name);
		STRFREE (victim->pIndexData->player_name);
		victim->pIndexData->player_name	= STRALLOC(arg3);
		victim->name					= STRALLOC(arg3);
	}
	else if (!strcasecmp(arg2, "short"))
	{
		STRFREE (victim->short_descr );
		STRFREE (victim->pIndexData->short_descr);
		victim->pIndexData->short_descr	= STRALLOC(arg3);
		victim->short_descr				= STRALLOC(arg3);
	}
	else if (!strcasecmp(arg2, "long"))
	{
		STRFREE (victim->long_descr );
		STRFREE (victim->pIndexData->long_descr );
		victim->pIndexData->long_descr	= STRALLOC(arg3);
		victim->long_descr				= STRALLOC(arg3);
	}
	else if (!strcasecmp(arg2, "desc"))
	{
		if (ch->pcdata->editmode == MODE_NONE)
		{
			ch->gold		-= cost;
			ch->pcdata->editmode	= MODE_MOB_DESC;
			ch->pcdata->edit_ptr	= victim;
			start_editing( ch, victim->pIndexData->description);
		}
		else
		{
			bug( "do_castle_mset: bad mode");
		}
		pop_call();
		return;
	}
	else
	{
		do_castle_mset(ch, "");
		pop_call();
		return;
	}

	act("Your eyes go fuzzy for a moment.",ch,NULL,NULL,TO_ROOM);
	act("The contractor comes, takes your money, looks around, pinches the mobile, then\n\rheads back the way he came.",ch,NULL,NULL,TO_CHAR);

	ch->gold				-= cost;
	ch->pcdata->castle->cost	+= cost/1000;

	CHECK_AUTOSAVE(room_index[ROOM_VNUM_CASTLE]->area);

	pop_call();
	return;
}

void do_castle_oset( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA    *obj;
	OBJ_DATA    *key;
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];

	char bld[10], dim[10], grn[10];
	int argn, cost;

	push_call("do_castle_oset(%p,%p)",ch,argument);

	strcpy(dim, get_color_string(ch, COLOR_PROMPT,  VT102_DIM ));
	strcpy(bld, get_color_string(ch, COLOR_PROMPT,  VT102_BOLD));
	strcpy(grn, ansi_translate_text(ch, "{128}"));

	if (strlen(argument) > 160)
	{
		argument[160] = '\0';
	}

	smash_tilde( argument );

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	strcpy( arg3, argument );

	if (arg1[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char("Syntax: castle oset <obj> <flag|type> <bitvector>\n\r", ch);
		send_to_char("Syntax: castle oset <obj> <name|short|long|desc> <string>\n\r\n\r", ch);
		send_to_char("Syntax: castle oset <fountain>  <liquid> <argument>\n\r", ch);
		send_to_char("Syntax: castle oset <container> <carry|lid|key|max> <argument>\n\r", ch);
		send_to_char("Syntax: castle oset <furniture> <size|pose|regen> <argument>\n\r", ch);
		pop_call();
		return;
	}

	cost = bargain(ch, COST_OF_SET) * 1000;

	if (ch->gold < cost)
	{
		ch_printf(ch,"It will cost you %s gold coins to do an oset.\n\r", tomoney(cost));
		pop_call();
		return;
	}

	if ((obj = get_obj_list(ch, arg1, ch->in_room->first_content)) == NULL)
	{
		send_to_char( "You can't find it.\n\r", ch );
		pop_call();
		return;
	}

	if (obj->pIndexData->creator_pvnum != ch->pcdata->pvnum)
	{
		ch_printf(ch, "You cannot oset that.\n\r");
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "flag"))
	{
		get_bitvector_value(argument, &argn, "ITEM_FLAG_");

		if (argn == -1	|| !IS_SET(CASTLE_O_FLAG_LIST, argn))
		{
			int bit;
			char flag_buf[MAX_STRING_LENGTH];

			ch_printf(ch, "%sSyntax:%s castle oset %s<%sobj%s>%s flag %s<%sobj_flag%s>\n\r\n\r", dim, bld, dim, bld, dim, bld, dim, bld, dim);

			for (bit = 0, flag_buf[0] = '\0' ; bit < 32 ; bit++)
			{
				if (IS_SET(CASTLE_O_FLAG_LIST, 1 << bit))
				{
					cat_sprintf(flag_buf, "%s%-20s", bld, broken_bits(1 << bit, "ITEM_FLAG_", TRUE));
				}
				if (strlen(flag_buf) == 120)
				{
					ch_printf(ch, "%s\n\r", flag_buf);
					flag_buf[0] = '\0';
				}
			}
			if (strlen(flag_buf) > 0)
			{
				ch_printf(ch, "%s\n\r", flag_buf);
			}
			pop_call();
			return;
		}
		TOGGLE_BIT(obj->extra_flags, argn);
		obj->pIndexData->extra_flags = obj->extra_flags;
	}
	else if (!strcasecmp(arg2, "type"))
	{
		get_bitvector_value(argument, &argn, "ITEM_TYPE_");

		if (argn == -1	|| (argn != ITEM_FURNITURE && argn != ITEM_CONTAINER && argn != ITEM_FOUNTAIN))
		{
			int bit;
			char flag_buf[MAX_STRING_LENGTH];

			ch_printf(ch, "%sSyntax:%s castle oset %s<%sobj%s>%s type %s<%sobj_flag%s>\n\r\n\r", dim, bld, dim, bld, dim, bld, dim, bld, dim);

			for (bit = 0, flag_buf[0] = '\0' ; bit <= ITEM_FOUNTAIN ; bit++)
			{
				if (bit == ITEM_FURNITURE || bit == ITEM_CONTAINER || bit == ITEM_FOUNTAIN)
				{
					cat_sprintf(flag_buf, "%s%-20s", bld, broken_bits(bit, "ITEM_TYPE_", TRUE));
				}
				if (strlen(flag_buf) == 120)
				{
					ch_printf(ch, "%s\n\r", flag_buf);
					flag_buf[0] = '\0';
				}
			}
			if (strlen(flag_buf) > 0)
			{
				ch_printf(ch, "%s\n\r", flag_buf);
			}
			pop_call();
			return;
		}
		obj->item_type = argn;
		obj->pIndexData->item_type = obj->item_type;
		obj->value[0] = 0;
		obj->value[1] = 0;
		obj->value[2] = 0;
		obj->value[3] = 0;
	}
	else if (!strcasecmp(arg2, "liquid"))
	{
		if (obj->item_type != ITEM_FOUNTAIN)
		{
			act("$p is not a fountain.", ch, obj, NULL, TO_CHAR);
			pop_call();
			return;
		}
		if (!get_bitvector_value(argument, &argn, "LIQ_"))
		{
			int cnt;
			char liq_buf[MAX_STRING_LENGTH];

			ch_printf(ch, "%sSyntax:%s castle oset %s<%sobj%s>%s liquid %s<%ssector_type%s>\n\r\n\r", dim, bld, dim, bld, dim, bld, dim, bld, dim);

			for (cnt = 0, liq_buf[0] = '\0' ; cnt < LIQ_MAX ; cnt++)
			{
				cat_sprintf(liq_buf, "%s%-20s", bld, broken_bits(cnt, "LIQ_", TRUE));

				if (strlen(liq_buf) == 120)
				{
					ch_printf(ch, "%s\n\r", liq_buf);
					liq_buf[0] = '\0';
				}
			}
			if (strlen(liq_buf) > 0)
			{
				ch_printf(ch, "%s\n\r", liq_buf);
			}
			pop_call();
			return;
		}
		obj->value[2] = argn;
		obj->pIndexData->value[2] = obj->value[2];
	}
	else if (!strcasecmp(arg2, "lid"))
	{
		if (obj->item_type != ITEM_CONTAINER)
		{
			act("$p is not a container.", ch, obj, NULL, TO_CHAR);
			pop_call();
			return;
		}
		get_bitvector_value(argument, &argn, "CONT_");

		if (argn == -1	|| !IS_SET(CASTLE_O_LID_LIST, argn))
		{
			int bit;
			char lid_buf[MAX_STRING_LENGTH];

			ch_printf(ch, "%sSyntax:%s castle oset %s<%sobj%s>%s lid %s<%slid_flag%s>\n\r\n\r", dim, bld, dim, bld, dim, bld, dim, bld, dim);

			for (bit = 0, lid_buf[0] = '\0' ; bit < 32 ; bit++)
			{
				if (IS_SET(CASTLE_O_LID_LIST, 1 << bit))
				{
					cat_sprintf(lid_buf, "%s%-20s", bld, broken_bits(1 << bit, "CONT_", TRUE));
				}
				if (strlen(lid_buf) == 120)
				{
					ch_printf(ch, "%s\n\r", lid_buf);
					lid_buf[0] = '\0';
				}
			}
			if (strlen(lid_buf) > 0)
			{
				ch_printf(ch, "%s\n\r", lid_buf);
			}
			pop_call();
			return;
		}
		TOGGLE_BIT(obj->value[1], argn);
		obj->pIndexData->value[1] = obj->value[1];
	}
	else if (!strcasecmp(arg2, "carry"))
	{
		int value = is_number(arg3) ? atol(arg3) : -1;

		if (obj->item_type != ITEM_CONTAINER)
		{
			act("$p is not a container.", ch, obj, NULL, TO_CHAR);
			pop_call();
			return;
		}

		if (value < 1 || value > 1500)
		{
			ch_printf(ch, "A container's carry weight range is between 1 and 1500.\n\r");
			pop_call();
			return;
		}
		obj->value[0] = value;
		obj->pIndexData->value[0] = value;
	}
	else if (!strcasecmp(arg2, "max"))
	{
		int value = is_number(arg3) ? atol(arg3) : -1;

		if (obj->item_type != ITEM_CONTAINER)
		{
			act("$p is not a container.", ch, obj, NULL, TO_CHAR);
			pop_call();
			return;
		}

		if (value < 1 || value > 150)
		{
			ch_printf(ch, "A container's max item range is between 1 and 150.\n\r");
			pop_call();
			return;
		}
		obj->value[3] = obj->pIndexData->value[3] = value;
	}
	else if (!strcasecmp(arg2, "key"))
	{
		if (obj->item_type != ITEM_CONTAINER)
		{
			act("$p is not a container.", ch, obj, NULL, TO_CHAR);
			pop_call();
			return;
		}

		if ((key = get_obj_carry(ch, argument)) == NULL)
		{
			send_to_char("You are not carrying that.\n\r", ch);
			pop_call();
			return;
		}
		else
		{
			obj->pIndexData->value[2] = obj->value[2] = key->pIndexData->vnum;
		}
	}
	else if (!strcasecmp(arg2, "pose"))
	{
		if (obj->item_type != ITEM_FURNITURE)
		{
			act("$p is not a piece of furniture.", ch, obj, NULL, TO_CHAR);
			pop_call();
			return;
		}
		get_bitvector_value(argument, &argn, "FURNITURE_");

		if (argn == -1	|| !IS_SET(CASTLE_O_FURNIT_LIST, argn))
		{
			int bit;
			char pos_buf[MAX_STRING_LENGTH];

			ch_printf(ch, "%sSyntax:%s castle oset %s<%sobj%s>%s position %s<%slid_flag%s>\n\r\n\r", dim, bld, dim, bld, dim, bld, dim, bld, dim);

			for (bit = 0, pos_buf[0] = '\0' ; bit < 32 ; bit++)
			{
				if (IS_SET(CASTLE_O_FURNIT_LIST, 1 << bit))
				{
					cat_sprintf(pos_buf, "%s%-20s", bld, broken_bits(1 << bit, "FURNITURE_", TRUE));
				}
				if (strlen(pos_buf) == 120)
				{
					ch_printf(ch, "%s\n\r", pos_buf);
					pos_buf[0] = '\0';
				}
			}
			if (strlen(pos_buf) > 0)
			{
				ch_printf(ch, "%s\n\r", pos_buf);
			}
			pop_call();
			return;
		}
		TOGGLE_BIT(obj->value[2], argn);
		obj->pIndexData->value[2] = obj->value[2];
	}
	else if (!strcasecmp(arg2, "size"))
	{
		int value = is_number(arg3) ? atol(arg3) : -1;

		if (obj->item_type != ITEM_FURNITURE)
		{
			act("$p is not a piece of furniture.", ch, obj, NULL, TO_CHAR);
			pop_call();
			return;
		}

		if (value < 1 || value > 100)
		{
			ch_printf(ch, "A piece of furniture's size range is between 1 and 100.\n\r");
			pop_call();
			return;
		}
		obj->value[0] = value;
		obj->pIndexData->value[0] = value;
	}
	else if (!strcasecmp(arg2, "regen"))
	{
		int value = is_number(arg3) ? atol(arg3) : -1;

		if (obj->item_type != ITEM_FURNITURE)
		{
			act("$p is not a piece of furniture.", ch, obj, NULL, TO_CHAR);
			pop_call();
			return;
		}

		if (value < 1 || value > 200)
		{
			ch_printf(ch, "A piece of furniture's regen range is between 1 and 200.\n\r");
			pop_call();
			return;
		}
		obj->value[3] = value;
		obj->pIndexData->value[3] = value;
	}
	else if (!strcasecmp(arg2, "name"))
	{
		int cnt;

		for (cnt = 0 ; cnt < strlen(arg3) ; cnt++)
		{
			if (!isalpha(arg3[cnt]) && !isspace(arg3[cnt]))
			{
				ch_printf(ch, "You may only use alpha characters.\n\r");
				pop_call();
				return;
			}
		}
		STRFREE (obj->name);
		STRFREE (obj->pIndexData->name);
		obj->pIndexData->name	= STRALLOC(arg3);
		obj->name				= STRALLOC(arg3);
	}
	else if (!strcasecmp(arg2, "short"))
	{
		STRFREE (obj->short_descr );
		STRFREE (obj->pIndexData->short_descr);
		obj->pIndexData->short_descr	= STRALLOC(arg3);
		obj->short_descr			= STRALLOC(arg3);
	}
	else if (!strcasecmp(arg2, "long"))
	{
		STRFREE (obj->long_descr);
		STRFREE (obj->pIndexData->long_descr);
		obj->pIndexData->long_descr	= STRALLOC(arg3);
		obj->long_descr			= STRALLOC(arg3);
	}
	else if (!strcasecmp(arg2, "desc"))
	{
		if (ch->pcdata->editmode == MODE_NONE)
		{
			ch->gold		-= cost;
			ch->pcdata->editmode	= MODE_OBJ_DESC;
			ch->pcdata->edit_ptr	= obj;
			start_editing(ch, obj->pIndexData->description);
		}
		else
		{
			bug( "do_castle_oset: bad mode");
		}
		pop_call();
		return;
	}
	else
	{
		do_castle_oset(ch, "");
		pop_call();
		return;
	}

	act("Your eyes go fuzzy for a moment.",ch,NULL,NULL,TO_ROOM);
	act("The contractor comes, takes your money, looks around, pinches the mobile, then\n\rheads back the way he came.",ch,NULL,NULL,TO_CHAR);

	ch->gold				-= cost;
	ch->pcdata->castle->cost	+= cost/1000;

	CHECK_AUTOSAVE(room_index[ROOM_VNUM_CASTLE]->area);

	pop_call();
	return;
}

void do_castle_dset( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA    *key;
	EXIT_DATA   *pexit;
	char arg1[MAX_INPUT_LENGTH];
	char buf [MAX_INPUT_LENGTH];
	char bld[10], dim[10];
	int argn, cost, door_dir;

	push_call("do_castle_dset(%p,%p)",ch,argument);

	strcpy(dim, get_color_string(ch, COLOR_PROMPT,  VT102_DIM ));
	strcpy(bld, get_color_string(ch, COLOR_PROMPT,  VT102_BOLD));

	if (strlen(argument) > 200)
	{
		argument[200] = '\0';
	}
	smash_tilde( argument );

	argument = one_argument( argument, arg1 );

	if (arg1[0] == '\0')
	{
		send_to_char( "Syntax: castle dset <dir> <name|desc|flag|key|del> <argument>\n\r", ch );
		pop_call();
		return;
	}

	cost = bargain(ch, COST_OF_SET) * 1000;

	if (ch->gold < cost)
	{
		ch_printf(ch, "It will cost you %s gold coins to do a rset.\n\r", tomoney(cost));
		pop_call();
		return;
	}

	if (!get_bitvector_value(arg1, &argn, "DIR_"))
	{
		int door;
		char exit_buf[MAX_STRING_LENGTH];

		for (door = 0, exit_buf[0] = '\0' ; door <= 5 ; door++)
		{
			sprintf(buf, "%s%s%s|", bld, broken_bits(door, "DIR_", TRUE), dim);
			strcat(exit_buf, buf);
		}
		exit_buf[strlen(exit_buf)-1] = '\0';
		ch_printf(ch, "%sSyntax:%s castle dset %s<%s>\n\r", dim, bld, dim, exit_buf);

		pop_call();
		return;
	}
	door_dir = argn;

	if ((pexit = ch->in_room->exit[door_dir]) == NULL)
	{
		create_exit(ch->in_room, door_dir);
		pexit = ch->in_room->exit[door_dir];
		pexit->vnum                 = ch->in_room->vnum;
	}

	argument = one_argument( argument, arg1 );

	if (!strcasecmp(arg1, "name"))
	{
		STRFREE (pexit->keyword );
		if (argument[0] == '\'' || argument[0]=='"')
		{
			argument = one_argument_nolower(argument, arg1);
		}
		pexit->keyword = STRALLOC( argument );

		if (room_index[pexit->to_room] != NULL && room_index[pexit->to_room]->creator_pvnum != ch->pcdata->pvnum && IS_SET(ch->in_room->room_flags, ROOM_IS_ENTRANCE))
		{
			EXIT_DATA *pexit2 = room_index[pexit->to_room]->exit[rev_dir[door_dir]];

			if (pexit2 != NULL)
			{
				if (pexit2->keyword != NULL)
				{
					STRFREE (pexit2->keyword);
				}
				pexit2->keyword = STRALLOC( pexit->keyword );
			}
		}
	}
	else if (!strcasecmp(arg1, "del"))
	{
		if (room_index[pexit->to_room] != NULL
		&& room_index[pexit->to_room]->creator_pvnum != ch->pcdata->pvnum
		&& IS_SET(ch->in_room->room_flags, ROOM_IS_ENTRANCE)
		&& !IS_SET(pexit->flags, EX_BACKDOOR)
		&& !IS_SET(pexit->flags, EX_CLAN_BACKDOOR))
		{
			send_to_char("You wouldn't want to delete your entrance.\n\r",ch);
			pop_call();
			return;
		}

		if (IS_SET(pexit->flags, EX_BACKDOOR))
		{
			ch->pcdata->castle->has_backdoor = FALSE;
		}
		if (IS_SET(pexit->flags, EX_CLAN_BACKDOOR) && clanfounder)
		{
			ch->pcdata->clan->num_backdoors--;
		}
		delete_exit(ch->in_room, door_dir);
		cost = 0;
	}
	else if (!strcasecmp(arg1, "desc"))
	{
		STRFREE (pexit->description);
		pexit->description = STRALLOC( justify(argument, 80) );

		if (room_index[pexit->to_room] && room_index[pexit->to_room]->creator_pvnum != ch->pcdata->pvnum && IS_SET(ch->in_room->room_flags, ROOM_IS_ENTRANCE))
		{
			EXIT_DATA *pexit2;
			pexit2 = room_index[pexit->to_room]->exit[rev_dir[door_dir]];
			if (pexit2 != NULL)
			{
				if (pexit2->description != NULL)
				{
					STRFREE (pexit2->description);
				}
				pexit2->description = STRALLOC( pexit->description );
			}
		}
	}
	else if (!strcasecmp(arg1, "flag"))
	{
		get_bitvector_value(argument, &argn, "EX_");

		if (argn == -1	|| !IS_SET(CASTLE_D_FLAG_LIST, argn))
		{
			int bit;
			char flag_buf[MAX_STRING_LENGTH];

			ch_printf(ch, "%sSyntax:%s castle dset %s flag %s<%sdoor_flag%s>\n\r\n\r", dim, bld, broken_bits(door_dir, "DIR_", TRUE), dim, bld, dim);

			for (bit = 0, flag_buf[0] = '\0' ; bit < 32 ; bit++)
			{
				if (IS_SET(CASTLE_D_FLAG_LIST, 1 << bit))
				{
					cat_sprintf(flag_buf, "%s%-20s", bld, broken_bits(1 << bit, "EX_", TRUE));
				}
				if (strlen(flag_buf) == 120)
				{
					ch_printf(ch, "%s\n\r", flag_buf);
					flag_buf[0] = '\0';
				}
			}
			if (strlen(flag_buf) > 0)
			{
				ch_printf(ch, "%s\n\r", flag_buf);
			}
			pop_call();
			return;
		}
		TOGGLE_BIT(pexit->flags, argn);

		if (room_index[pexit->to_room] && room_index[pexit->to_room]->creator_pvnum != ch->pcdata->pvnum && IS_SET(ch->in_room->room_flags, ROOM_IS_ENTRANCE))
		{
			EXIT_DATA *pexit2 = room_index[pexit->to_room]->exit[rev_dir[door_dir]];

			if (pexit2)
			{
				pexit2->flags = pexit->flags;
			}
		}
	}
	else if (!strcasecmp(arg1, "key"))
	{
		if ((key = get_obj_carry(ch, argument)) == NULL)
		{
			send_to_char("You are not carrying that.\n\r", ch);
			pop_call();
			return;
		}
		else
		{
			pexit->key = key->pIndexData->vnum;
		}

		if (room_index[pexit->to_room] && room_index[pexit->to_room]->creator_pvnum != ch->pcdata->pvnum && IS_SET(ch->in_room->room_flags, ROOM_IS_ENTRANCE))
		{
			EXIT_DATA *pexit2 = room_index[pexit->to_room]->exit[rev_dir[door_dir]];

			if (pexit2)
			{
				pexit2->key = pexit->key;
			}
		}
	}
	else
	{
		do_castle_dset(ch, "");
		pop_call();
		return;
	}
	act("The surrounding exits appear to shimmer for an instant.", ch,NULL,NULL,TO_ROOM);
	act("The contractor comes, takes your money, looks around, hammers on a post, then\n\rheads back the way he came.",ch,NULL,NULL,TO_CHAR);

	ch->gold				-= cost;
	ch->pcdata->castle->cost	+= cost/1000;

	CHECK_AUTOSAVE(room_index[ROOM_VNUM_CASTLE]->area);

	pop_call();
	return;
}

void do_castle( CHAR_DATA *ch, char *argument )
{
	CASTLE_DATA      *castle;
	ROOM_INDEX_DATA  *location;
	EXTRA_DESCR_DATA *ed;
	CHAR_DATA        *victim;
	OBJ_DATA         *obj;

	char arg[MAX_INPUT_LENGTH];
	char dim[10], bld[10];

	push_call("do_castle(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	strcpy(dim, get_color_string(ch, COLOR_PROMPT,  VT102_DIM ));
	strcpy(bld, get_color_string(ch, COLOR_PROMPT,  VT102_BOLD));

	switch (ch->pcdata->editmode)
	{
		default: break;

		case MODE_RESTRICTED:
			send_to_char( "You cannot use this command from while editing something else.\n\r",ch);
			pop_call();
			return;

		case MODE_ROOM_DESC:
			if (!ch->pcdata->edit_ptr)
			{
				send_to_char("Editor Error: Report to the Immortals.\n\r", ch);
				bug( "do_castle_rset: sub_room_desc: NULL ch->pcdata->edit_ptr", 0);
				ch->pcdata->editmode = MODE_NONE;
			}
			else
			{
				location = ch->pcdata->edit_ptr;
				STRFREE(location->description);
				location->description = copy_buffer(ch);
				stop_editing( ch );
			}

			CHECK_AUTOSAVE(room_index[ROOM_VNUM_CASTLE]->area);

			pop_call();
			return;

		case MODE_ROOM_EXTRA:
			if (!ch->pcdata->edit_ptr)
			{
				send_to_char("Editor Error: Report to the Immortals.\n\r", ch);
				bug( "do_castle_rset: sub_room_extra: NULL ch->pcdata->edit_ptr", 0 );
				ch->pcdata->editmode = MODE_NONE;
			}
			else
			{
				ed = ch->pcdata->edit_ptr;
				STRFREE( ed->description );
				ed->description = copy_buffer( ch );
				stop_editing( ch );
			}

			CHECK_AUTOSAVE(room_index[ROOM_VNUM_CASTLE]->area);

			pop_call();
			return;

		case MODE_MOB_DESC:
			if (!ch->pcdata->edit_ptr)
			{
				send_to_char("Fatal error: report to Chaste.\n\r", ch);
				bug( "do_castle: sub_mob_desc: NULL ch->pcdata->edit_ptr", 0);
				ch->pcdata->editmode = MODE_NONE;
			}
			else
			{
				victim = ch->pcdata->edit_ptr;
				STRFREE (victim->description );
				STRFREE (victim->pIndexData->description );
				victim->description             = copy_buffer(ch);
				victim->pIndexData->description = copy_buffer(ch);
				stop_editing( ch );
			}

			CHECK_AUTOSAVE(room_index[ROOM_VNUM_CASTLE]->area);

			pop_call();
			return;

		case MODE_OBJ_DESC:
			if (!ch->pcdata->edit_ptr)
			{
				send_to_char("Fatal error: report to Demise.\n\r", ch);
				bug( "do_castle: sub_obj_desc: NULL ch->pcdata->edit_ptr", 0);
				ch->pcdata->editmode = MODE_NONE;
			}
			else
			{
				obj = ch->pcdata->edit_ptr;
				STRFREE (obj->description);
				STRFREE (obj->pIndexData->description);
				obj->description             = copy_buffer(ch);
				obj->pIndexData->description = copy_buffer(ch);
				stop_editing( ch );
			}

			CHECK_AUTOSAVE(room_index[ROOM_VNUM_CASTLE]->area);

			pop_call();
			return;
	}

	if (ch->pcdata->clan && !strcasecmp(ch->name, ch->pcdata->clan->leader[0]))
	{
		clanfounder = TRUE;
	}
	else
	{
		clanfounder = FALSE;
	}

	if (argument[0] == '\0')
	{
		if ((castle = ch->pcdata->castle) != NULL)
		{
			if (room_index[castle->entrance] == NULL)
			{
				ch_printf(ch, "Your castle entrance no longer exists. Contact the immortals.\n\r");
			}
			else
			{
				ch_printf(ch, "%s  Castle Cost:%s %s,000\n\r",	dim, bld, tomoney(castle->cost));
				ch_printf(ch, "%s        Rooms:%s %d\n\r",	dim, bld, castle->num_rooms);
				ch_printf(ch, "%s    Creatures:%s %d\n\r",	dim, bld, castle->num_mobiles);
				ch_printf(ch, "%s      Objects:%s %d\n\r",	dim, bld, castle->num_objects);
				ch_printf(ch, "%s   Back Doors:%s %d\n\r",   dim, bld, castle->has_backdoor);
				if (room_index[castle->door_room])
				{
					ch_printf(ch, "%s Connect Room:%s %s\n\r",	dim, bld, room_index[castle->door_room]->name);
					ch_printf(ch, "%s  Connect Dir:%s %s\n\r",	dim, bld, broken_bits(castle->door_dir, "DIR_", TRUE));
				}
			}
		}
		else
		{
			switch(ch->in_room->sector_type)
			{
				case SECT_FIELD:
				case SECT_FOREST:
				case SECT_HILLS:
				case SECT_MOUNTAIN:
				case SECT_DESERT:
					if (!IS_SET(ch->in_room->room_flags, ROOM_NO_CASTLE)
					&&  !IS_SET(ch->in_room->area->flags, AFLAG_NOCASTLE))
					{
						send_to_char("CASTLE: You could start a castle here.\n\r",ch);
					}
					else
					{
						send_to_char("CASTLE: You can't start a castle here.\n\r",ch);
					}
					break;
				default:
					send_to_char("CASTLE: You can't start a castle here.\n\r",ch);
					break;
			}
		}
		pop_call();
		return;
	}

	argument = one_argument( argument, arg );

	smash_tilde( argument );

	castle = ch->pcdata->castle;

	clanfounder = (ch->pcdata->clan && !strcasecmp(ch->name, ch->pcdata->clan->leader[0]));

	if ((castle = ch->pcdata->castle) == NULL)
	{
		send_to_char("You have no castle established yet.\n\r",ch);
		pop_call();
		return;
	}

	if (!strcasecmp(arg, "connect"))
	{
		do_castle_connect(ch, argument);
		pop_call();
		return;
	}

	if (ch->in_room->creator_pvnum != ch->pcdata->pvnum)
	{
		send_to_char("You are NOT in your castle!\n\r",ch);
		pop_call();
		return;
	}

	if (!strcasecmp(arg, "rcreate"))
	{
		do_castle_rcreate(ch, argument);
	}
	else if (!strcasecmp(arg, "rstat"))
	{
		do_castle_rstat(ch, argument);
	}
	else if (!strcasecmp(arg, "mstat"))
	{
		do_castle_mstat(ch, argument);
	}
	else if (!strcasecmp(arg, "ostat"))
	{
		do_castle_ostat(ch, argument);
	}
	else if (!strcasecmp(arg, "mcreate"))
	{
		do_castle_mcreate(ch);
	}
	else if (!strcasecmp(arg, "ocreate"))
	{
		do_castle_ocreate(ch);
	}
	else if (!strcasecmp(arg, "screate"))
	{
		do_castle_screate(ch, argument);
	}
	else if (!strcasecmp(arg, "mdelete"))
	{
		do_castle_mdelete(ch, argument);
	}
	else if (!strcasecmp(arg, "rdelete"))
	{
		ch_printf(ch, "Deletion of rooms has not been implemented yet.\n\r");
	}
	else if (!strcasecmp(arg, "odelete"))
	{
		do_castle_odelete(ch, argument);
	}
	else if (!strcasecmp(arg, "sdelete"))
	{
		do_castle_sdelete(ch, argument);
	}
	else if (!strcasecmp(arg, "rset"))
	{
		do_castle_rset(ch, argument);
	}
	else if (!strcasecmp(arg, "mset"))
	{
		do_castle_mset(ch, argument);
	}
	else if (!strcasecmp(arg, "oset"))
	{
		do_castle_oset(ch, argument);
	}
	else if (!strcasecmp(arg, "dset"))
	{
		do_castle_dset(ch, argument);
	}
	else
	{
		send_to_char("Use what castle option? (see 'help castles')\n\r", ch);
		/*
			<connect|rcreate|mcreate|screate|rset|mset|dset|mstat|rstat>
		*/
	}
	pop_call();
	return;
}

void del_castle( CHAR_DATA *victim)
{
	ROOM_INDEX_DATA *pRoomIndex;
	MOB_INDEX_DATA  *pMobIndex;
	OBJ_INDEX_DATA  *pObjIndex;
	int vnum;

	push_call("del_castle(%p)",victim);

	if (victim->pcdata->castle == NULL)
	{
		pop_call();
		return;
	}

	if (victim->pcdata->pvnum == 0)
	{
		log_printf("del_castle: corrupted pvnum found");
		pop_call();
		return;
	}

	log_printf("del_castle: %s", victim->name);

	for (vnum = ROOM_VNUM_CASTLE ; vnum < MAX_VNUM ; vnum++)
	{
		if ((pMobIndex = mob_index[vnum]) != NULL
		&& pMobIndex->area->low_r_vnum == ROOM_VNUM_CASTLE
		&& victim->pcdata->pvnum == pMobIndex->creator_pvnum)
		{
			delete_mob(pMobIndex);
		}

		if ((pObjIndex = obj_index[vnum]) != NULL
		&& pObjIndex->area->low_r_vnum == ROOM_VNUM_CASTLE
		&& victim->pcdata->pvnum == pObjIndex->creator_pvnum)
		{
			delete_obj(pObjIndex);
		}

		if ((pRoomIndex = room_index[vnum]) != NULL
		&& pRoomIndex->area->low_r_vnum == ROOM_VNUM_CASTLE
		&& victim->pcdata->pvnum == pRoomIndex->creator_pvnum)
		{
			delete_room(pRoomIndex);
		}
	}

	FREEMEM(victim->pcdata->castle);
	victim->pcdata->castle				= NULL;

	CHECK_AUTOSAVE(room_index[ROOM_VNUM_CASTLE]->area);

	pop_call();
	return;
}
