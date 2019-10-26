
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
int	scan_door		args( ( CHAR_DATA *ch, char *arg ) );
OBJ_DATA *find_key	args( ( CHAR_DATA *ch, int key ) );

/*
	Interesting little ACT replacement.
	Solves problem of seeing invis players leave.  - Chaos  5/22/96
*/

void show_who_can_see( CHAR_DATA *ch, char *txt )
{
	CHAR_DATA *fch;

	push_call("show_who_can_see(%p,%p)",ch,txt);

	for (fch = ch->in_room->first_person ; fch ; fch = fch->next_in_room)
	{
		if (fch->desc == NULL || fch == ch || !IS_AWAKE(fch))
		{
			continue;
		}
		if (!can_hear(fch, ch))
		{
			if (!can_see(fch, ch))
			{
				continue;
			}
			if (IS_NPC(fch) || number_range(1, 100 + get_curr_dex(ch)) > learned(fch, gsn_perception))
			{
				continue;
			}
			check_improve(fch, gsn_perception);
		}
		if (is_same_group(fch, ch) && !IS_NPC(fch) && IS_SET(fch->pcdata->spam, 2048))
		{
			continue;
		}
		ch_printf(fch, "%s%s", capitalize(PERS(ch, fch)), txt);
	}
	pop_call();
	return;
}

bool move_char( CHAR_DATA *ch, int door, bool forreal )
{
	CHAR_DATA *fch, *fch_next, *fch_last, *sch, *mount;
	ROOM_INDEX_DATA *in_room, *to_room;
	int in_room_vnum, to_room_vnum;
	EXIT_DATA *pexit;
	char buf[MAX_STRING_LENGTH];
	int sdoor, move, cnt;
	CLAN_DATA *pClan;
	bool drunk, found;

	push_call("move_char(%p,%p)",ch,door);

	if (door < 0 || door > 5)
	{
		bug( "do_move: bad door %d.", door );
		pop_call();
		return FALSE;
	}

	move         = 1;
	drunk        = is_drunk(ch);
	found        = FALSE;
	in_room_vnum = ch->in_room->vnum;
	in_room      = ch->in_room;
	mount        = is_mounting(ch) ? ch->mounting : ch;

	if (forreal && IS_SET(in_room->room_flags, ROOM_ICE) && !CAN_FLY(mount))
	{
		if (number_percent() > 80 && number_percent() > get_curr_dex(mount))
		{
			switch (number_bits(1))
			{
				case 0:
					act("You try to move but your feet slip on the icy surface.",	ch, NULL, NULL, TO_CHAR);
					act("$n tries to move but $s feet slip on the icy surface.",	ch, NULL, NULL, TO_ROOM);
					break;
				case 1:
					act("Your feet slip as you try to move and you crash to the ice.",	ch, NULL, NULL, TO_CHAR);
					act("$n's feet slip as $e tries to move and $e crashes to the ice.",	ch, NULL, NULL, TO_ROOM);
					break;
			}
			mount->position = POS_SITTING;

			if (!MP_VALID_MOB(mount))
			{
				wait_state(mount, PULSE_PER_SECOND);
			}
			else
			{
				mount->distracted += 10;
			}
			pop_call();
			return FALSE;
		}
	}

	pexit = get_exit(in_room->vnum, door);

	if (pexit == NULL || !can_use_exit(ch, pexit))
	{
		if (forreal)
		{
			if (drunk)
			{
				send_to_char( "You hit a wall in your drunken state.\n\r", ch );
			}
			else
			{
				send_to_char( "Alas, you cannot go that way.\n\r", ch );
			}
		}
		pop_call();
		return FALSE;
	}

	to_room_vnum = pexit->to_room;
	to_room = room_index[to_room_vnum];

	if (IS_SET(to_room->room_flags, ROOM_BLOCK))   /* Chaos 11/16/93 */
	{
		if (IS_AFFECTED(mount, AFF_CLEAR))
		{
			if (forreal)
			{
				to_room->room_flags -= ROOM_BLOCK;
				send_to_char("You managed to clear out a path.\n\r", ch);
				check_improve(ch, gsn_clear_path);
				wait_state(mount, skill_table[gsn_clear_path].beats);

				if (!del_room_timer(to_room->vnum, ROOM_BLOCK))
				{
					set_room_timer(to_room->vnum, ROOM_TIMER_UNBLOCK, multi_skill_level(mount, gsn_clear_path));
				}
			}
		}
		else if (!IS_NPC(mount) && number_percent() > learned(mount, gsn_climb))
		{
			send_to_char("You climb and wring your way through the dense plantlife.\n\r", ch);
			check_improve(ch, gsn_climb);
			wait_state(mount, skill_table[gsn_climb].beats);
		}
		else
		{
			if (forreal)
			{
				send_to_char( "The plantlife in that room is too dense to pass.\n\r", ch);
			}
			pop_call();
			return FALSE;
		}
	}

	if (!IS_IMMORTAL(ch) && !IS_NPC(ch) && ch->in_room->area != to_room->area )
	{
		if (ch->level < to_room->area->low_hard_range)
		{
			if (forreal)
			{
				switch( to_room->area->low_hard_range - ch->level )
				{
					case 1:
						send_to_char( "A voice in your mind says, 'You are nearly ready to go that way...'\n\r", ch );
						break;
					case 2:
					case 3:
					case 4:
						send_to_char( "A voice in your mind says, 'Soon you shall be ready to travel down this path... soon.'\n\r", ch );
						break;
					case 5:
						send_to_char( "A voice in your mind says, 'You are not ready to go down that path... yet.'.\n\r", ch);
						break;
					default:
						send_to_char( "A voice in your mind says, 'You are not ready to go down that path.'.\n\r", ch);
				}
			}
			pop_call();
			return FALSE;
		}
		else if ( ch->level > to_room->area->hi_hard_range )
		{
			if (forreal)
			{
				send_to_char( "A voice in your head says, 'There is nothing more for you down that path.'\n\r", ch );
			}
			pop_call();
			return FALSE;
		}
	}

	if (IS_SET(pexit->flags, EX_CLOSED)
	&& !IS_AFFECTED(ch, AFF_PASS_DOOR)
	&& !rspec_req(ch,RSPEC_PASS_DOOR))
	{
		if (IS_SET(pexit->flags, EX_HIDDEN))
		{
			if (forreal)
			{
				if (drunk)
				{
					send_to_char( "You hit a wall in your drunken state.\n\r", ch );
				}
				else
				{
					send_to_char( "Alas, you cannot go that way.\n\r", ch );
				}
			}
		}
		else
		{
			if (forreal)
			{
				act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
			}
		}
		pop_call();
		return FALSE;
	}

	if (IS_SET(pexit->flags, EX_CLOSED) && IS_SET(pexit->flags, EX_PASSPROOF))
	{
		if (forreal)
		{
			act( "The $d remains firm.", ch, NULL, pexit->keyword, TO_CHAR );
		}
		pop_call();
		return FALSE;
	}

	if (IS_AFFECTED(ch, AFF_CHARM)
	&& (!IS_AFFECTED(ch, AFF_HUNT) || !IS_NPC(ch))
	&& ch->master != NULL
	&& in_room == ch->master->in_room )
	{
		if (forreal)
		{
			send_to_char( "What?  And leave your beloved master?\n\r", ch );
		}
		pop_call();
		return FALSE;
	}

	if (!IS_NPC(ch) && ch->level < LEVEL_IMMORTAL && room_is_private(to_room))
	{
		if (forreal)
		{
			send_to_char( "That room is private right now.\n\r", ch );
		}
		pop_call();
		return FALSE;
	}

	if (in_room->sector_type == SECT_AIR
	||  to_room->sector_type == SECT_AIR)
	{
		if (vnum_in_group(ch, MOB_VNUM_AIR_ELEMENTAL))
		{
			send_to_char( "The air elemental bears you aloft.\n\r", ch);
		}
		else	if (!is_mounting(ch) && !CAN_FLY(ch))
		{
			if (forreal)
			{
				send_to_char( "You can't fly.\n\r", ch );
			}
			pop_call();
			return FALSE;
		}
		else if (is_mounting(ch) && !CAN_FLY(ch->mounting))
		{
			if (forreal)
			{
				send_to_char("Your mount can't fly.\n\r", ch);
			}
			pop_call();
			return FALSE;
		}
	}

	if (in_room->sector_type == SECT_OCEAN
	||  to_room->sector_type == SECT_OCEAN)
	{
		if (!is_mounting(ch) && !CAN_FLY(ch) && !CAN_SWIM(ch))
		{
			if (in_room->sector_type != SECT_UNDER_WATER)
			{
				OBJ_DATA *boat;
				for (boat = ch->first_carrying ; boat ; boat = boat->next_content)
				{
					if (boat->item_type == ITEM_BOAT)
					{
						found = TRUE;
						break;
					}
				}
				if (!boat)
				{
					if (forreal)
					{
						send_to_char( "You need a boat to go there.\n\r", ch );
					}
					pop_call();
					return FALSE;
				}
			}
		}

		if (is_mounting(ch) && !CAN_FLY(ch->mounting) && !CAN_SWIM(ch->mounting))
		{
			if (in_room->sector_type != SECT_UNDER_WATER)
			{
				if (forreal)
				{
					send_to_char("Your mount can't swim.\n\r", ch);
				}
				pop_call();
				return FALSE;
			}
		}
	}

	if (!IS_NPC(ch))
	{
		if (to_room->sector_type == SECT_ASTRAL)
		{
			if (!CAN_ASTRAL_WALK(ch))
			{
				if (forreal)
				{
					send_to_char( "You cannot enter the astral plane.\n\r", ch );
				}
				pop_call();
				return FALSE;
			}
		}

		if (in_room->sector_type == SECT_ETHEREAL)
		{
			if (!CAN_ETHEREAL_WALK(ch))
			{
				if (forreal)
				{
					send_to_char( "Your movement is futile.\n\r", ch );
				}
				pop_call();
				return FALSE;
			}
		}

		if (to_room->sector_type == SECT_ETHEREAL)
		{
			if (!CAN_ETHEREAL_WALK(ch))
			{
				if (forreal)
				{
					send_to_char( "You may not leave this world.\n\r", ch );
				}
				pop_call();
				return FALSE;
			}
		}

		if (IS_SET(to_room->room_flags, ROOM_IS_CASTLE) && ch->level < LEVEL_IMMORTAL)
		{
			for (pClan = mud->f_clan ; pClan ; pClan = pClan->next)
			{
				if (pClan->home == to_room->vnum)
				{
					if (!ch->pcdata->clan || ch->pcdata->clan->founder_pvnum != to_room->creator_pvnum)
					{
						if (forreal)
						{
							send_to_char( "You are not in the correct clan to enter that room.\n\r", ch );
						}
						pop_call();
						return FALSE;
					}
				}
			}
		}

		/*
			8 bits bite the dust - Scandum 24-04-2002
		*/

		if (IS_SET(to_room->room_flags, ROOM_ALLOW_WAR)
		||  IS_SET(to_room->room_flags, ROOM_ALLOW_GLA)
		||  IS_SET(to_room->room_flags, ROOM_ALLOW_MAR)
		||  IS_SET(to_room->room_flags, ROOM_ALLOW_NIN)
		||  IS_SET(to_room->room_flags, ROOM_ALLOW_DRU)
		||  IS_SET(to_room->room_flags, ROOM_ALLOW_SOR)
		||  IS_SET(to_room->room_flags, ROOM_ALLOW_SHA)
		||  IS_SET(to_room->room_flags, ROOM_ALLOW_WLC))
		{
			int belong;

			for (cnt = 0 ; cnt < MAX_CLASS ; cnt++)
			{
				if (ch->pcdata->mclass[cnt] > 0)
				{
					switch (cnt)
					{
						case CLASS_RANGER:
							SET_BIT(belong, ROOM_ALLOW_WAR);
							break;
						case CLASS_GLADIATOR:
							SET_BIT(belong, ROOM_ALLOW_GLA);
							break;
						case CLASS_MARAUDER:
							SET_BIT(belong, ROOM_ALLOW_MAR);
							break;
						case CLASS_NINJA:
							SET_BIT(belong, ROOM_ALLOW_NIN);
							break;
						case CLASS_ELEMENTALIST:
							SET_BIT(belong, ROOM_ALLOW_DRU);
							break;
						case CLASS_ILLUSIONIST:
							SET_BIT(belong, ROOM_ALLOW_SOR);
							break;
						case CLASS_MONK:
							SET_BIT(belong, ROOM_ALLOW_SHA);
							break;
						case CLASS_NECROMANCER:
							SET_BIT(belong, ROOM_ALLOW_WLC);
							break;
					}
				}
			}

			if (!HAS_BIT(to_room->room_flags, belong))
			{
				if (forreal)
				{
					send_to_char("You don't belong in there!\n\r",ch);
				}
				pop_call();
				return FALSE;
			}
		}
	}

	if (mount != ch)
	{
		switch (mount->position)
		{
			case POS_DEAD:
				if (forreal)
				{
					send_to_char("Your mount is dead!\n\r", ch);
				}
				pop_call();
				return FALSE;
			case POS_MORTAL:
			case POS_INCAP:
				if (forreal)
				{
					send_to_char("Your mount is hurt far too badly to move.\n\r", ch);
				}
				pop_call();
				return FALSE;
			case POS_STUNNED:
				if (forreal)
				{
					send_to_char("Your mount is too stunned to move.\n\r", ch);
				}
				pop_call();
				return FALSE;
			case POS_SLEEPING:
				if (forreal)
				{
					send_to_char("Your mount is sleeping.\n\r", ch);
				}
				pop_call();
				return FALSE;
			case POS_RESTING:
				if (forreal)
				{
					send_to_char("Your mount is resting.\n\r", ch);
				}
				pop_call();
				return FALSE;
			case POS_SITTING:
				if (forreal)
				{
					send_to_char("Your mount is sitting down.\n\r", ch);
				}
				pop_call();
				return FALSE;
		}
	}

	if (!IS_NPC(ch) || ch->desc)
	{
		int oldspeed;

		if (!CAN_FLY(mount))
		{
			move = (sector_table[in_room->sector_type].movement + sector_table[to_room->sector_type].movement) / 2;
	
			if (move >= 4 && learned(ch, gsn_endurance))
			{
				move -= 2;
			}
		}
		else
		{
			move = 1;
		}

		switch (mount->speed)
		{
			case 0: move = 1;          break;
			case 1: move = move * 1;   break;
			case 2: move = move * 1.5; break;
			case 3: move = move * 2;   break;
			case 4: move = move * 3;   break;
			case 5: move = move * 4;   break;
		}

		/*
			Add race dependancy
		*/

		move += move * race_table[mount->race].move_rate;

		/*
			Add Weather factors
		*/

		if (IS_OUTSIDE(ch) && NO_WEATHER_SECT(in_room->sector_type))
		{
			if (in_room->area->weather_info->sky > 0)
			{
				move += in_room->area->weather_info->wind_speed / 2;

				if (CAN_FLY(mount))
				{
					move *= 2;
				}
			}
		}

		if (IS_AFFECTED(mount, AFF_CLEAR) && mount->wait > 0)
		{
			move *= 2;
		}

		/*
			Make movement based on carry weight
		*/

		move = move * (1 + 2 * ch->carry_weight / UMAX(1, can_carry_w(ch)));

		move = UMAX(1, move/3);

		if (!is_mounting(ch) && ch->move < move)
		{
			if (forreal)
			{
				send_to_char( "You are too exhausted.\n\r", ch );
			}
			pop_call();
			return FALSE;
		}

		if (is_mounting(ch) && ch->mounting->move < move)
		{
			if (forreal)
			{
				send_to_char( "Your mount is too exhaused.\n\r", ch);
			}
			pop_call();
			return FALSE;
		}

		if (forreal == FALSE)
		{
			pop_call();
			return TRUE;
		}

		oldspeed = mount->speed;

		if (mount->speed < 3 && vnum_in_group(ch, MOB_VNUM_EARTH_ELEMENTAL))
		{
			mount->speed = 3;
		}

		if (ch->in_room->sector_type == SECT_UNDER_WATER)
		{
			if (!CAN_SWIM(mount))
			{
				wait_state(ch, (16 - mount->speed * 2) * 2);
			}
			else
			{
				wait_state(ch, (16 - mount->speed * 2) * 1);
			}
		}
		else if (ch->in_room->sector_type == SECT_LAKE && !CAN_FLY(mount))
		{
			if (!CAN_SWIM(mount))
			{
				wait_state(ch, (14 - mount->speed * 2) * 2);
			}
			else
			{
				wait_state(ch, (14 - mount->speed * 2) * 1);
			}
		}
		else if (ch->in_room->sector_type == SECT_OCEAN && !CAN_FLY(mount))
		{
			if (!CAN_SWIM(mount))
			{
				wait_state(ch, (16 - mount->speed * 2) * 2);
			}
			else
			{
				wait_state(ch, (16 - mount->speed * 2) * 1);
			}
		}
		else
		{
			wait_state(ch, 11 - mount->speed * 2);
		}
		mount->speed = oldspeed;

		if (vnum_in_group(ch, MOB_VNUM_EARTH_ELEMENTAL))
		{
			move = UMAX(1, move/2);
			send_to_char( "The earth elemental carries you.\n\r", ch);
		}
		mount->move -= move;
	}

	if (forreal == FALSE)
	{
		pop_call();
		return TRUE;
	}

	if (IS_AFFECTED(ch, AFF_HIDE))
	{
		REMOVE_BIT(ch->affected_by, AFF_HIDE);
	}

	if (!IS_NPC(ch) && strlen(ch->pcdata->ambush) > 0)
	{
		send_to_char("You leave your ambush.\n\r", ch);
		STRFREE(ch->pcdata->ambush);
		ch->pcdata->ambush = STRALLOC("");
	}

	if (!CAN_FLY(mount)
	&& ch->in_room->sector_type != SECT_UNDER_WATER
	&& ch->in_room->sector_type != SECT_LAKE
	&& ch->in_room->sector_type != SECT_RIVER
	&& ch->in_room->sector_type != SECT_OCEAN)
	{
		if (is_mounting(ch))
		{
			sprintf(buf, " rides %s", dir_name[door]);
		}
		else if (!drunk && IS_NPC(ch) && !IS_SET(ch->act, ACT_RACE))
		{
			sprintf(buf, " leaves %s", dir_name[door]);
		}
		else if (drunk)
		{
			sprintf(buf, " stumbles satiated %swards", dir_name[door]);
		}
		else
		{
			switch (ch->speed)
			{
				case 0: sprintf(buf, " walks %s",	dir_name[door]); break;
				case 1: sprintf(buf, " leaves %s",	dir_name[door]); break;
				case 2: sprintf(buf, " jogs %s",	dir_name[door]); break;
				case 3: sprintf(buf, " runs %s",	dir_name[door]); break;
				case 4: sprintf(buf, " zips %s",	dir_name[door]); break;
				case 5: sprintf(buf, " blazes %s",	dir_name[door]); break;
			}
		}
	}
	else if (CAN_FLY(mount) && ch->in_room->sector_type != SECT_UNDER_WATER)
	{
		if (drunk)
		{
			sprintf(buf, " flies zigzagging %swards", dir_name[door]);
		}
		else
		{
			sprintf(buf, " flies %s", dir_name[door]);
		}
	}
	else
	{
		if (found && ch->in_room->sector_type != SECT_UNDER_WATER)
		{
			if (drunk)
			{
				sprintf(buf, " sails zigzagging %swards", dir_name[door]);
			}
			else
			{
				sprintf(buf," sails %s", dir_name[door]);
			}
		}
		else
		{
			if (drunk)
			{
				sprintf(buf," swims zigzagging %swards", dir_name[door]);
			}
			else
			{
				sprintf(buf," swims %s", dir_name[door]);
			}
		}
	}
	if (is_mounting(ch))
	{
		cat_sprintf(buf, " on %s.\n\r", get_name(ch->mounting));
	}
	else
	{
		cat_sprintf(buf, ".\n\r");
	}
	show_who_can_see(ch, buf);


	/*  Chaos - code for last_left - track skill   11/14/93  */

	switch (in_room->sector_type)
	{
		case SECT_LAKE:
		case SECT_RIVER:
		case SECT_OCEAN:
		case SECT_LAVA:
		case SECT_AIR:
		case SECT_ASTRAL:
		case SECT_ETHEREAL:
			goto end_of_track;
	}

	if (IS_NPC(ch))
	{
		if (IS_AFFECTED(ch, AFF_ETHEREAL))
		{
			goto end_of_track;
		}
	}
	else
	{
		if (IS_AFFECTED(ch, AFF_SNEAK) && number_percent() < learned(ch, gsn_pass_without_trace))
		{
			goto end_of_track;
		}
	}

	STRFREE(in_room->last_left_name[0]);

	for (cnt = 0 ; cnt < MAX_LAST_LEFT-1 ; cnt++)
	{
		in_room->last_left_name[cnt] = in_room->last_left_name[cnt + 1];
		in_room->last_left_bits[cnt] = in_room->last_left_bits[cnt + 1];
		in_room->last_left_time[cnt] = in_room->last_left_time[cnt + 1];
	}

	in_room->last_left_name[MAX_LAST_LEFT-1] = IS_NPC(ch) ? STRDUPE( ch->short_descr ) : STRDUPE( ch->name );

	in_room->last_left_time[MAX_LAST_LEFT-1] = mud->current_time;

	in_room->last_left_bits[MAX_LAST_LEFT-1] = 1 << door;

	if (100 * ch->hit / ch->max_hit < 20 && !IS_UNDEAD(ch))
	{
		SET_BIT(in_room->last_left_bits[MAX_LAST_LEFT-1], TRACK_BLOOD);
	}
	else if (CAN_FLY(ch))
	{
		SET_BIT(in_room->last_left_bits[MAX_LAST_LEFT-1], TRACK_FLY);
	}

	end_of_track:

	if (!IS_NPC(ch))
	{
		mprog_exit_trigger(ch, door);

		if (ch->in_room != in_room)
		{
			pop_call();
			return FALSE;
		}
	}

	char_from_room( ch );

	char_to_room(ch, to_room_vnum);

	in_room = room_index[in_room_vnum];
	to_room = room_index[to_room_vnum];

	if (mount == ch && drunk)
	{
		sprintf(buf," stumbles satiated into the room.\n\r");
	}
	else
	{
		switch (mount->speed)
		{
			case 0: sprintf(buf," arrives at a leisurely pace.\n\r");	break;
			case 1: sprintf(buf," has arrived.\n\r");				break;
			case 2: sprintf(buf," arrives briskly.\n\r");			break;
			case 3: sprintf(buf," arrives suddenly.\n\r");			break;
			case 4: sprintf(buf," arrives quickly.\n\r");			break;
			case 5: sprintf(buf," arrives in a blaze.\n\r");			break;
		}
	}
	show_who_can_see( ch, buf );

	do_look( ch, "auto" );

	if (ch->in_room->sector_type != SECT_ASTRAL && !IS_NPC(ch) && ch->in_room->vnum > 5)
	{
		ch->pcdata->last_real_room = ch->in_room->vnum;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_ALTAR) && IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_char( "You feel very safe here.\n\r", ch);
	}

	if (!IS_NPC(ch) && IS_SET(ch->in_room->room_flags, ROOM_BANK) && IS_SET(ch->in_room->room_flags, ROOM_IS_CASTLE))
	{
		do_bank(ch, "balance");
	}

	if (!IS_IMMORTAL(ch) && !IS_NPC(ch) && in_room->area != to_room->area)
	{
		if (ch->level < to_room->area->low_soft_range)
		{
			send_to_char("You feel uncomfortable being in this strange land...\n\r", ch);
		}
		else if (ch->level > to_room->area->hi_soft_range)
		{
			send_to_char("You feel there is not much to gain visiting this place...\n\r", ch);
		}
	}

	if (mount == ch)
	{
		if (IS_AFFECTED(ch, AFF_SNEAK))
		{
			check_improve(ch, gsn_sneak);
			check_improve(ch, gsn_pass_without_trace);
		}
		if (IS_AFFECTED(ch, AFF_STEALTH))
		{
			check_improve(ch, gsn_stealth);
			check_improve(ch, gsn_greater_stealth);
		}
		check_improve(ch, gsn_endurance);
	}

	if (!IS_NPC(ch))
	{
		mprog_greet_trigger(ch);
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_ICE) && !CAN_FLY(mount))
	{
		if (number_percent() > 80 && number_percent() > get_curr_dex(mount))
		{
			switch (number_bits(1))
			{
				case 0:
					act("You crash to the ground as you slip on an unexpected slab of ice.",	ch, NULL, NULL, TO_CHAR);
					act("$n crashes to the ground as $e sips on an unexpected slab of ice.",	ch, NULL, NULL, TO_ROOM);
					break;
				case 1:
					act("Your feet fly out from under you as you slip on the ice.",			ch, NULL, NULL, TO_CHAR);
					act("$n's feet fly out from under $m as $e slips on the ice.",			ch, NULL, NULL, TO_ROOM);
					break;
			}
			mount->position = POS_SITTING;

			if (!MP_VALID_MOB(mount))
			{
				wait_state(mount, PULSE_PER_SECOND);
			}
			else
			{
				mount->distracted += 10;
			}
			pop_call();
			return FALSE;
		}
	}
	for (fch = ch->in_room->first_person ; fch ; fch = fch->next_in_room)
	{
		if (!IS_NPC(fch) && fch != ch && can_see(fch, ch) && strlen(fch->pcdata->ambush) > 0)
		{
			if (!strcasecmp(ch->name, fch->pcdata->ambush) || !strcasecmp(ch->short_descr, fch->pcdata->ambush))
			{
				send_to_char("You've found your prey!\n\r", fch);
				check_improve(fch, gsn_ambush);

				if (!IS_NPC(fch))
				{
					do_assassinate(fch, fch->pcdata->ambush);
				}
				else
				{
					do_kill(fch, ch->name);
				}
				STRFREE(fch->pcdata->ambush);
				fch->pcdata->ambush = STRALLOC("");
			}
		}
	}

	if (mount != ch)
	{
		ch->mounting = mount;
	}

	if (ch->in_room == in_room)
	{
		pop_call();
		return FALSE;
	}
	else if (mount != ch)
	{
		char_from_room(mount);
		char_to_room(mount, ch->in_room->vnum);
	}

	/*
		Take followers along
	*/

	fch_last = in_room->last_person;

	for (fch = in_room->first_person ; fch ; fch = fch_next)
	{
		fch_next = fch->next_in_room;

		if (fch->in_room != in_room)
		{
			fch_next = in_room->first_person;
			continue;
		}

		if (fch->master == ch && fch->position == POS_STANDING && fch->in_room != ch->in_room)
		{
			if (mount->speed <= get_max_speed(fch))
			{
				fch->speed = mount->speed;
			}
			else
			{
				if (fch->leader != ch)
				{
					act("You move too fast for $N to follow.",  ch, NULL, fch, TO_CHAR);
					act("$n moves too fast for you to follow.", ch, NULL, fch, TO_VICT);
					continue;
				}
				act( "$N can't move that fast, so you slow down.", ch, NULL, fch, TO_CHAR);
				mount->speed  = get_max_speed( fch );
				fch->speed = mount->speed;

				for (sch = ch->master ; sch ; sch = sch->master)
				{
					if (sch->speed > mount->speed && sch->in_room == ch->in_room)
					{
						sch->speed = UMIN(mount->speed, get_max_speed(sch));
					}
					else
					{
						break;
					}
				}
			}

			act( "You follow $N.", fch, NULL, ch, TO_CHAR );
			move_char( fch, door, TRUE );

			if (IS_NPC(fch) && IS_SET(fch->act, ACT_ELEMENTAL))
			{
				switch (fch->pIndexData->vnum)
				{
					case MOB_VNUM_FIRE_ELEMENTAL:
						if (to_room->sector_type != SECT_LAVA )
						{
							send_to_char( "The fire elemental melts into the magma.\n\r", ch);
							raw_kill(fch);
						}
						break;
					case MOB_VNUM_WATER_ELEMENTAL:
						if (to_room->sector_type != SECT_LAKE
						&&  to_room->sector_type != SECT_RIVER
						&&  to_room->sector_type != SECT_OCEAN
						&&  to_room->sector_type != SECT_UNDER_WATER )
						{
							send_to_char( "The water elemental flows away in a strong undercurrent.\n\r", ch);
							raw_kill(fch);
						}
						break;
					case MOB_VNUM_AIR_ELEMENTAL:
						if (to_room->sector_type != SECT_AIR )
						{
							send_to_char( "The air elemental disappears in a strong gust of wind.\n\r", ch);
							raw_kill(fch);
						}
						break;
					case MOB_VNUM_EARTH_ELEMENTAL:
						if (to_room->sector_type != SECT_FOREST
						&&  to_room->sector_type != SECT_DESERT
						&&  to_room->sector_type != SECT_MOUNTAIN
						&&  to_room->sector_type != SECT_HILLS
						&&  to_room->sector_type != SECT_UNDER_GROUND
						&&  to_room->sector_type != SECT_DEEP_EARTH )
						{
							send_to_char( "The earth elemental allows its form to dissolve into the ground.\n\r", ch);
							raw_kill(fch);
						}
						break;
				}
			}
		}
		if (fch == fch_last)
		{
			break;
		}
	}

	/* make shadowers shadow player -Dug */

	if (!IS_NPC(ch))
	{
		if ((fch = ch->pcdata->shadowed_by) != NULL)
		{
			if (fch->position == POS_STANDING && (fch->in_room != ch->in_room))
			{
				for (sdoor = 0 ; sdoor < 6 ; sdoor++)
				{
					if ((pexit = get_exit(fch->in_room->vnum, sdoor)) != NULL && room_index[pexit->to_room] == in_room)
					{
						CHAR_DATA *tch;

						ch_printf(fch, "%s leaves %s.\n\r", dir_name[door], get_name(ch));

						if (mount->speed <= get_max_speed(fch))
						{
							fch->speed = ch->speed;
							ch_printf(fch, "You follow at a discrete distance.\n\r");
							tch = fch->pcdata->shadowing;
							fch->pcdata->shadowing = NULL;
							move_char(fch, sdoor, TRUE);
							fch->pcdata->shadowing = tch;
							check_improve(fch, gsn_shadow);
						}
						else
						{
							send_to_char( "They are moving too fast for you.\n\r", fch);
						}
						break;
					}
				}
				if (sdoor == 6)
				{
					stop_shadow(fch);
				}
			}
			else
			{
				stop_shadow(fch);
			}
		}

		if (ch->pcdata->shadowing)
		{
			stop_shadow(ch);
		}

		if (IS_SET(ch->act, PLR_WIZTIME))
		{
			ch_printf(ch, "(%d move used)\n\r", move);
		}
	}
	/*
		Made Clan guards more friendly - Scandum 25-04-2002

		Pondering a more simplistic approach by make aggressive
		castle mobs clan guard
	*/

	if (IS_SET(ch->in_room->room_flags, ROOM_IS_CASTLE) && (!IS_NPC(ch) || ch->desc != NULL))
	{
		for (fch = ch->in_room->first_person ; fch != NULL ; fch = fch_next)
		{
			fch_next = fch->next_in_room;
			if (IS_NPC(fch) && IS_SET(fch->act, ACT_CLAN_GUARD))
			{
				if (IS_NPC(ch))
				{
					act( "$n slaughters you in cold blood!", fch, NULL, ch, TO_VICT);
					act( "$N slaughters $n in cold blood!",  ch, NULL, fch, TO_ROOM);
					raw_kill(ch);
					pop_call();
					return FALSE;
				}
				else
				{
					pClan = get_clan_from_vnum(fch->pIndexData->creator_pvnum);

					if (!ch->pcdata->clan || ch->pcdata->clan != pClan)
					{
						ch_printf(ch, "%s%s tells you 'You are not a member of %s!'\n\r", get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(fch->short_descr), pClan->name);
						char_from_room(ch);
						char_to_room(ch, ROOM_VNUM_TEMPLE);
						pop_call();
						return FALSE;
					}
					else
					{
						act( "$n straightens $s back and salutes you.", fch, NULL, ch, TO_VICT );
						act( "$n straightens $s back and salutes $N." , fch, NULL, ch, TO_NOTVICT );
					}
				}
			}
		}
	}
	mprog_entry_trigger(ch);

	pop_call();
	return FALSE;
}

bool can_move_char( CHAR_DATA *ch, int door )
{
	return move_char(ch, door, FALSE);
}

void do_speed( CHAR_DATA *ch, char *argument )
{
	push_call("do_speed(%p,%p)",ch,argument);

	if (argument[0]=='\0')
	{
		send_to_char( "Usage:  speed <walk,normal,jog,run,haste>\n\r" ,ch);
		send_to_char( "This allows changing of rate of movement.  Rates effect the amount of MV lost.\n\rwalk   - 1 mv lost per move\n\rnormal - normal mv lost\n\rjog    - 1.5 times normal mv lost\n\rrun    - 2 times mv lost\n\r", ch);
		switch(ch->speed)
		{
			case 0: send_to_char("Currently: walking\n\r",	ch); break;
			case 1: send_to_char("Currently: normal\n\r",	ch); break;
			case 2: send_to_char("Currently: jogging\n\r",	ch); break;
			case 3: send_to_char("Currently: running\n\r",	ch); break;
			case 4: send_to_char("Currently: haste\n\r",		ch); break;
			case 5: send_to_char("Currently: blazing\n\r",	ch); break;
		}
		pop_call();
		return;
	}

	if (argument[0] == 'w')
	{
		ch->speed = 0;
	}
	else	if (argument[0] == 'n')
	{
		ch->speed = 1;
	}
	else if (argument[0] == 'j')
	{
		ch->speed = 2;
	}
	else	if (argument[0] == 'r')
	{
		ch->speed = 3;
	}
	else if (argument[0] == 'h')
	{
		ch->speed = 4;
	}
	else
	{
		ch->speed = 5;
	}
	if (ch->speed > get_max_speed(ch))
	{
		ch->speed = get_max_speed( ch );
		send_to_char( "You cannot move that fast.\n\r", ch);
	}

	switch (ch->speed)
	{
		case 0: send_to_char( "Speed set to Walk.\n\r",		ch); break;
		case 1: send_to_char( "Speed set to Normal.\n\r",		ch); break;
		case 2: send_to_char( "Speed set to Jog.\n\r",		ch); break;
		case 3: send_to_char( "Speed set to Run.\n\r",		ch); break;
		case 4: send_to_char( "Speed set to Haste.\n\r",		ch); break;
		case 5: send_to_char( "Speed set to Blazing.\n\r",	ch); break;
	}
	pop_call();
	return;
}
void do_north( CHAR_DATA *ch, char *argument )
{
	push_call("do_north(%p,%p)",ch,argument);

	move_char( ch, DIR_NORTH, TRUE );
	pop_call();
	return;
}

void do_east( CHAR_DATA *ch, char *argument )
{
	push_call("do_east(%p,%p)",ch,argument);

	move_char( ch, DIR_EAST, TRUE );
	pop_call();
	return;
}

void do_south( CHAR_DATA *ch, char *argument )
{

	push_call("do_south(%p,%p)",ch,argument);

	move_char( ch, DIR_SOUTH, TRUE );
	pop_call();
	return;
}

void do_west( CHAR_DATA *ch, char *argument )
{

	push_call("do_west(%p,%p)",ch,argument);

	move_char( ch, DIR_WEST, TRUE );
	pop_call();
	return;
}

void do_up( CHAR_DATA *ch, char *argument )
{
	push_call("do_up(%p,%p)",ch,argument);

	move_char( ch, DIR_UP, TRUE );
	pop_call();
	return;
}

void do_down( CHAR_DATA *ch, char *argument )
{
	push_call("do_down(%p,%p)",ch,argument);

	move_char( ch, DIR_DOWN, TRUE );
	pop_call();
	return;
}

int scan_door( CHAR_DATA *ch, char *arg )
{
	char door_name[MAX_INPUT_LENGTH];
	int count, number, door;
	EXIT_DATA *pexit;

	push_call("scan_door(%p,%p)",ch,arg);

	number = number_argument(arg, door_name);
	count  = 0;

	for (door = 0 ; door <= 5 ; door++)
	{
		if ((pexit = get_exit(ch->in_room->vnum, door)) == NULL)
		{
			continue;
		}
		if (!IS_SET(pexit->flags, EX_ISDOOR))
		{
			continue;
		}
		if (IS_SET(pexit->flags, EX_UNBARRED))
		{
			continue;
		}
		if (!pexit->keyword || !is_name(door_name, pexit->keyword))
		{
			continue;
		}
		if (!can_use_exit(ch, pexit))
		{
			continue;
		}
		if (IS_SET(pexit->flags, EX_HIDDEN) && !can_see_hidden(ch))
		{
			continue;
		}
		if (++count != number)
		{
			continue;
		}
		pop_call();
		return door;
	}
	pop_call();
	return( -1 );
}

int find_door( CHAR_DATA *ch, char *arg )
{
	EXIT_DATA *pexit;
	int door = -1;

	push_call("find_door(%p,%p)",ch,arg);

	if ((door = scan_door(ch, arg)) >= 0)
	{
		pop_call();
		return door;
	}

	if ((door = direction_door(arg)) == -1)
	{
		pop_call();
		return -1;
	}

	if ((pexit = get_exit(ch->in_room->vnum, door)) == NULL)
	{
		pop_call();
		return -1;
	}

	if (!IS_SET(pexit->flags, EX_ISDOOR))
	{
		pop_call();
		return -1;
	}

	if (IS_SET(pexit->flags, EX_UNBARRED))
	{
		pop_call();
		return -1;
	}

	if (!can_use_exit(ch, pexit))
	{
		pop_call();
		return -1;
	}

	if (IS_SET(pexit->flags, EX_HIDDEN) && !can_see_hidden(ch))
	{
		pop_call();
		return -1;
	}
	pop_call();
	return door;
}


void do_open( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int door;

	push_call("do_open(%p,%p)",ch,argument);

	one_argument( argument, arg );

	if (!check_blind(ch))
	{
		pop_call();
		return;
	}

	if (arg[0] == '\0')
	{
		send_to_char( "Open what?\n\r", ch );
		pop_call();
		return;
	}

	if ((door = find_door(ch, arg)) >= 0)
	{
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit;
		EXIT_DATA *pexit_rev;

		pexit = ch->in_room->exit[door];

		if (!IS_SET(pexit->flags, EX_CLOSED))
		{
			send_to_char( "It's already open.\n\r", ch);
			pop_call();
			return;
		}

		/*
			Auto unlock - Scandum 02-07-2002
		*/

		if (IS_SET(pexit->flags, EX_LOCKED))
		{
			if (pexit->key < 1)
			{
				send_to_char("It's locked.\n\r", ch);
				pop_call();
				return;
			}
			if (find_key(ch, pexit->key) == NULL)
			{
				send_to_char( "It's locked.\n\r", ch);
				pop_call();
				return;
			}
			REMOVE_BIT(pexit->flags, EX_LOCKED);
			REMOVE_BIT(pexit->flags, EX_CLOSED);
			act( "You unlock and open the $d.",  ch, NULL, pexit->keyword, TO_CHAR);
			act( "$n unlocks and opens the $d.", ch, NULL, pexit->keyword, TO_ROOM);

			if ((to_room = room_index[pexit->to_room]) != NULL
			&&  (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
			&&  room_index[pexit_rev->to_room] == ch->in_room)
			{
				REMOVE_BIT(pexit_rev->flags, EX_LOCKED);
				REMOVE_BIT(pexit_rev->flags, EX_CLOSED);

				ch->in_room = to_room;

				act( "The $d opens.", ch, NULL, pexit_rev->keyword, TO_ROOM);

				ch->in_room = room_index[pexit_rev->to_room];
			}
			pop_call();
			return;
		}

		REMOVE_BIT(pexit->flags, EX_CLOSED);
		act( "$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM );
		act( "You open the $d.", ch, NULL, pexit->keyword, TO_CHAR );

		if ((to_room = room_index[pexit->to_room]) != NULL
		&&  (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
		&&  room_index[pexit_rev->to_room] == ch->in_room )
		{
			REMOVE_BIT(pexit_rev->flags, EX_CLOSED);

			ch->in_room = to_room;

			act( "The $d opens.", ch, NULL, pexit_rev->keyword, TO_ROOM);

			ch->in_room = room_index[pexit_rev->to_room];
		}
		pop_call();
		return;
	}

	if ((obj = get_obj_here(ch, arg)) != NULL)
	{
		if (obj->item_type != ITEM_CONTAINER)
		{
			send_to_char( "That's not a container.\n\r", ch );
			pop_call();
			return;
		}
		if (!IS_SET(obj->value[1], CONT_CLOSED))
		{
			send_to_char( "It's already open.\n\r", ch );
			pop_call();
			return;
		}
		if (!IS_SET(obj->value[1], CONT_CLOSEABLE))
		{
			send_to_char( "You can't do that.\n\r", ch );
			pop_call();
			return;
		}
		if (IS_SET(obj->value[1], CONT_LOCKED))
		{
			if (obj->value[2] < 1)
			{
				send_to_char( "It's locked.\n\r", ch );
				pop_call();
				return;
			}
			if (find_key(ch, obj->value[2]) == NULL)
			{
				send_to_char( "It's locked.\n\r", ch );
				pop_call();
				return;
			}
			REMOVE_BIT(obj->value[1], CONT_LOCKED);
			REMOVE_BIT(obj->value[1], CONT_CLOSED);
			act( "You unlock and open $p.",  ch, obj, NULL, TO_CHAR );
			act( "$n unlocks and opens $p.", ch, obj, NULL, TO_ROOM );
			pop_call();
			return;
		}

		REMOVE_BIT(obj->value[1], CONT_CLOSED);
		act( "You open $p.", ch, obj, NULL, TO_CHAR );
		act( "$n opens $p.", ch, obj, NULL, TO_ROOM );
		pop_call();
		return;
	}
	else
	{
		act( "You see no $T here.", ch, NULL, arg, TO_CHAR );
	}
	pop_call();
	return;
}

void do_close( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int door;

	push_call("do_close(%p,%p)",ch,argument);

	one_argument( argument, arg );

	if (!check_blind(ch))
	{
		pop_call();
		return;
	}

	if (arg[0] == '\0')
	{
		send_to_char( "Close what?\n\r", ch );
		pop_call();
		return;
	}

	if ((door = find_door(ch, arg)) >= 0)
	{
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit;
		EXIT_DATA *pexit_rev;

		pexit = ch->in_room->exit[door];
		if (IS_SET(pexit->flags, EX_CLOSED))
		{
			send_to_char( "It's already closed.\n\r",    ch );
			pop_call();
			return;
		}
		if (IS_SET(pexit->flags, EX_BASHED))
		{
			send_to_char( "It's been bashed off its hinges!\n\r",    ch );
			pop_call();
			return;
		}
		SET_BIT(pexit->flags, EX_CLOSED);
		act( "$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM );
		act( "You close the $d.", ch, NULL, pexit->keyword, TO_CHAR );

		if ((to_room = room_index[pexit->to_room]) != NULL
		&&  (pexit_rev = to_room->exit[rev_dir[door]]) != 0
		&&   room_index[pexit_rev->to_room] == ch->in_room)
		{
			CHAR_DATA *rch;

			SET_BIT( pexit_rev->flags, EX_CLOSED );
			for (rch = to_room->first_person ; rch ; rch = rch->next_in_room)
			{
				act( "The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR );
			}
		}
		pop_call();
		return;
	}

	if ((obj = get_obj_here(ch, arg)) != NULL)
	{
		if (obj->item_type != ITEM_CONTAINER)
		{
			send_to_char( "That's not a container.\n\r", ch );
			pop_call();
			return;
		}
		if (IS_SET(obj->value[1], CONT_CLOSED))
		{
			send_to_char( "It's already closed.\n\r",    ch );
			pop_call();
			return;
		}
		if (!IS_SET(obj->value[1], CONT_CLOSEABLE))
		{
			send_to_char( "You can't do that.\n\r",      ch );
			pop_call();
			return;
		}
		SET_BIT(obj->value[1], CONT_CLOSED);
		act( "You close $p.", ch, obj, NULL, TO_CHAR );
		act( "$n closes $p.", ch, obj, NULL, TO_ROOM );
		pop_call();
		return;
	}
	else
	{
		act( "You see no $T here.", ch, NULL, arg, TO_CHAR );
	}
	pop_call();
	return;
}

OBJ_DATA *find_key( CHAR_DATA *ch, int key )
{
	OBJ_DATA *obj;

	push_call("find_key(%p,%p)",ch,key);

	for (obj = ch->first_carrying ; obj ; obj = obj->next_content)
	{
		if ((obj->pIndexData->vnum == key) || ((obj->item_type == ITEM_KEY) && (key == obj->value[0])))
		{
			pop_call();
			return obj;
		}
	}
	pop_call();
	return NULL;
}

void do_lock( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int door;

	push_call("do_lock(%p,%p)",ch,argument);

	if (!check_blind(ch))
	{
		pop_call();
		return;
	}

	one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		send_to_char( "Lock what?\n\r", ch );
		pop_call();
		return;
	}

	if ((door = find_door(ch, arg)) >= 0)
	{
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit;
		EXIT_DATA *pexit_rev;

		pexit = ch->in_room->exit[door];

		if (!IS_SET(pexit->flags, EX_CLOSED))
		{
			if (IS_SET(pexit->flags, EX_BASHED))
			{
				send_to_char("It's been bashed off its hinges!\n\r", ch);
				pop_call();
				return;
			}
		}
		if (IS_SET(pexit->flags, EX_LOCKED))
		{
			send_to_char( "It's already locked.\n\r", ch);
			pop_call();
			return;
		}

		if (pexit->key < 0)
		{
			send_to_char( "It can't be locked.\n\r",     ch );
			pop_call();
			return;
		}

		if (find_key(ch, pexit->key) == NULL)
		{
			if (IS_NPC(ch) || learned(ch, gsn_lock_lock) == 0)
			{
				send_to_char( "You lack the key.\n\r", ch);
				pop_call();
				return;
			}
			else if (number_percent() > learned(ch, gsn_lock_lock))
			{
				send_to_char( "You failed.\n\r", ch);
				pop_call();
				return;
			}
			else
			{
				check_improve(ch, gsn_lock_lock);
			}
		}
		if (!IS_SET(pexit->flags, EX_CLOSED))
		{
			SET_BIT(pexit->flags, EX_CLOSED);
			SET_BIT(pexit->flags, EX_LOCKED);
			act( "$n closes and locks the $d.", ch, NULL, pexit->keyword, TO_ROOM );
			act( "You close and lock the $d.", ch, NULL, pexit->keyword, TO_CHAR );

			if ((to_room = room_index[pexit->to_room]) != NULL
			&&  (pexit_rev = to_room->exit[rev_dir[door]]) != 0
			&&   room_index[pexit_rev->to_room] == ch->in_room)
			{
				CHAR_DATA *rch;

				SET_BIT(pexit_rev->flags, EX_CLOSED);
				SET_BIT(pexit_rev->flags, EX_LOCKED);
				for (rch = to_room->first_person ; rch ; rch = rch->next_in_room)
				{
					act( "The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR );
				}
			}
			pop_call();
			return;
		}
		SET_BIT(pexit->flags, EX_LOCKED);
		act( "You lock the $d.", ch, NULL, pexit->keyword, TO_CHAR);
		act( "$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM);

		if ((to_room = room_index[pexit->to_room]) != NULL
		&&  (pexit_rev = to_room->exit[rev_dir[door]]) != 0
		&&  room_index[pexit_rev->to_room] == ch->in_room)
		{
			SET_BIT( pexit_rev->flags, EX_LOCKED );
		}
		pop_call();
		return;
	}

	if ((obj = get_obj_here(ch, arg)) != NULL)
	{
		if (obj->item_type != ITEM_CONTAINER)
		{
			send_to_char( "That's not a container.\n\r", ch );
			pop_call();
			return;
		}
		if (IS_SET(obj->value[1], CONT_LOCKED))
		{
			send_to_char( "It's already locked.\n\r",    ch );
			pop_call();
			return;
		}
		if (obj->value[2] < 1)
		{
			send_to_char( "It can't be locked.\n\r", ch);
			pop_call();
			return;
		}

		if (find_key(ch, obj->value[2]) == NULL)
		{
			if (IS_NPC(ch) || (ch->pcdata->learned[gsn_lock_lock] == 0))
			{
				send_to_char( "You lack the key.\n\r", ch);
				pop_call();
				return;
			}
			else if (number_percent() > learned(ch, gsn_lock_lock))
			{
				send_to_char( "You failed.\n\r", ch );
				pop_call();
				return;
			}
			else
			{
				check_improve(ch, gsn_lock_lock);
			}
		}
		if (!IS_SET(obj->value[1], CONT_CLOSED))
		{
			SET_BIT(obj->value[1], CONT_CLOSED);
			SET_BIT(obj->value[1], CONT_LOCKED);
			act( "You close and lock $p.",  ch, obj, NULL, TO_CHAR );
			act( "$n closes and locks $p.", ch, obj, NULL, TO_ROOM );
			pop_call();
			return;
		}
		SET_BIT(obj->value[1], CONT_LOCKED);
		act( "You lock $p.", ch, obj, NULL, TO_CHAR );
		act( "$n locks $p.", ch, obj, NULL, TO_ROOM );
		pop_call();
		return;
	}
	else
	{
		act( "You see no $T here.", ch, NULL, arg, TO_CHAR );
	}
	pop_call();
	return;
}

void do_unlock( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj, *key;
	int door;

	push_call("do_unlock(%p,%p)",ch,argument);

	obj=NULL;
	key=NULL;

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Unlock what?\n\r", ch );
		pop_call();
		return;
	}

	if ((door = find_door(ch, arg)) >= 0)
	{
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit;
		EXIT_DATA *pexit_rev;

		pexit = ch->in_room->exit[door];

		if (!IS_SET(pexit->flags, EX_CLOSED))
		{
			act("The $d is not closed.", ch, NULL, pexit->keyword, TO_CHAR);
			pop_call();
			return;
		}
		if (pexit->key < 1)
		{
			act("The $d cannot be unlocked.", ch, NULL, pexit->keyword, TO_CHAR);
			pop_call();
			return;
		}
		if ((key = find_key(ch, pexit->key)) == NULL)
		{
			act("You lack the key to unlock the $d.", ch, NULL, pexit->keyword, TO_CHAR);
			pop_call();
			return;
		}
		if (!IS_SET(pexit->flags, EX_LOCKED))
		{
			act("The $d is already unlocked.", ch, NULL, pexit->keyword, TO_CHAR);
			pop_call();
			return;
		}
		REMOVE_BIT(pexit->flags, EX_LOCKED);

		act( "You unlock the $d with $p.", ch, key, pexit->keyword, TO_CHAR);
		act( "$n unlocks the $d with $p.", ch, key, pexit->keyword, TO_ROOM);

		if ((to_room = room_index[pexit->to_room]) != NULL
		&& (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
		&& room_index[pexit_rev->to_room] == ch->in_room )
		{
			REMOVE_BIT(pexit_rev->flags, EX_LOCKED);
		}
		pop_call();
		return;
	}

	if ((obj = get_obj_here(ch, arg)) != NULL)
	{
		if (obj->item_type != ITEM_CONTAINER)
		{
			act("$p is not a container.", ch, obj, NULL, TO_CHAR);
			pop_call();
			return;
		}
		if (!IS_SET(obj->value[1], CONT_CLOSED))
		{
			act("$p is not closed.", ch, obj, NULL, TO_CHAR);
			pop_call();
			return;
		}
		if (obj->value[2] < 0)
		{
			act("$p cannot be unlocked.", ch, obj, NULL, TO_CHAR);
			pop_call();
			return;
		}
		if ((key = find_key(ch, obj->value[2])) == NULL)
		{
			act("You lack the key to unlock $p.", ch, obj, NULL, TO_CHAR);
			pop_call();
			return;
		}
		if (!IS_SET(obj->value[1], CONT_LOCKED))
		{
			act("$p is already unlocked.", ch, obj, NULL, TO_CHAR);
			pop_call();
			return;
		}

		REMOVE_BIT(obj->value[1], CONT_LOCKED);

		act( "You unlock $p with $P.", ch, obj, key, TO_CHAR );
		act( "$n unlocks $p with $P.", ch, obj, key, TO_ROOM );

		pop_call();
		return;
	}
	else
	{
		act( "You see no $T here.", ch, NULL, arg, TO_CHAR );
	}
	pop_call();
	return;
}

void do_pick( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *gch;
	OBJ_DATA *obj, *lockpick;
	int door, pick_lvl;

	push_call("do_pick(%p,%p)",ch,argument);

	if (!check_blind(ch))
	{
		pop_call();
		return;
	}
/*
	if (!IS_NPC(ch) && multi(ch, gsn_pick_lock) == -1)
	{
		act("You pick your nose.", ch, NULL, NULL, TO_CHAR);
		act("$n picks $s nose.",   ch, NULL, NULL, TO_ROOM);
		pop_call();
		return;
	}
*/
	one_argument(argument, arg);

	if (arg[0] == '\0')
	{
		send_to_char( "Pick what?\n\r", ch );
		pop_call();
		return;
	}

	if ((lockpick = get_eq_char(ch, WEAR_HOLD)) == NULL || lockpick->item_type != ITEM_LOCKPICK)
	{
		send_to_char( "You need to hold a lockpick to pick locks.\n\r", ch);
		pop_call();
		return;
	}

	wait_state(ch, skill_table[gsn_pick_lock].beats);

	/*
		look for guards
	*/

	for (gch = ch->in_room->first_person ; gch ; gch = gch->next_in_room)
	{
		if (IS_NPC(gch) && IS_AWAKE(gch) && IS_IMMORTAL(gch)
		&& multi_skill_level(ch, gsn_pick_lock) < gch->level
		&& can_see(ch, gch))
		{
			act( "$N is standing too close to the lock.", ch, NULL, gch, TO_CHAR );
			pop_call();
			return;
		}
	}

	if ((door = find_door(ch, arg)) >= 0)
	{
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit;
		EXIT_DATA *pexit_rev;

		pexit = ch->in_room->exit[door];

		if (!IS_SET(pexit->flags, EX_CLOSED))
		{
			act("The $d is not closed.", ch, NULL, pexit->keyword, TO_CHAR);
			pop_call();
			return;
		}

		if (!IS_SET(pexit->flags, EX_LOCKED))
		{
			act("The $d is already unlocked.", ch, NULL, pexit->keyword, TO_CHAR);
			pop_call();
			return;
		}

		if (IS_SET(pexit->flags, EX_PICKPROOF))
		{
			act("The $d's lock cannot be picked.", ch, NULL, pexit->keyword, TO_CHAR);
			pop_call();
			return;
		}

		if (IS_SET(pexit->flags, EX_SMALL_LOCK) && !IS_SET(lockpick->value[2], PICK_SMALL_LOCK))
		{
			act("$p is too big to fit the $d's lock.", ch, lockpick, pexit->keyword, TO_CHAR);
			pop_call();
			return;
		}

		if (IS_SET(pexit->flags, EX_BIG_LOCK) && !IS_SET(lockpick->value[2], PICK_BIG_LOCK))
		{
			act("$p is too small to fit the $d's lock.", ch, lockpick, pexit->keyword, TO_CHAR);
			pop_call();
			return;
		}

		if (IS_SET(pexit->flags, EX_TRAPPED_LOCK))
		{
			pick_lvl  = multi_skill_level(ch, gsn_greater_pick);
			pick_lvl += IS_SET(lockpick->value[2], PICK_TRAPPED_LOCK) ? lockpick->level : 0;
			pick_lvl += get_curr_dex(ch);
			pick_lvl += get_curr_int(ch);

			if (IS_SET(pexit->flags, EX_MAGICAL_LOCK) && !IS_SET(lockpick->value[2], PICK_MAGICAL_LOCK))
			{
				pick_lvl = UMAX(1, pick_lvl - ch->in_room->area->average_level);
			}

			if (IS_SET(pexit->flags, EX_MECHANICAL_LOCK) && !IS_SET(lockpick->value[2], PICK_MECHANICAL_LOCK))
			{
				pick_lvl = UMAX(1, pick_lvl - ch->in_room->area->average_level);
			}

			if (IS_SET(pexit->flags, EX_HARD_PICK))
			{
				pick_lvl = UMAX(1, pick_lvl - ch->in_room->area->average_level);
			}

			if (IS_SET(pexit->flags, EX_EASY_PICK))
			{
				pick_lvl = UMAX(1, pick_lvl + ch->in_room->area->average_level);
			}

			if (number_range(1, pick_lvl) < ch->in_room->area->average_level * 3)
			{
				switch (number_bits(2))
				{
					case 0:
						act("The $d snarls and bites you viciously.", ch, NULL, pexit->keyword, TO_CHAR);
						act("The $d snarls and bites $n viciously.",   ch, NULL, pexit->keyword, TO_ROOM);
						break;
					case 1:
						act("The $d shakes and zaps you with a bolt of lightning.", ch, NULL, pexit->keyword, TO_CHAR);
						act("The $d shakes and zaps $n with a bolt of lightning.",  ch, NULL, pexit->keyword, TO_ROOM);
						break;
					case 2:
						act("A tiny creature slithers out of the $d's lock and explodes in your face.", ch, NULL, pexit->keyword, TO_CHAR);
						act("A tiny creature slithers out of the $d's lock and explodes in $n's face.", ch, NULL, pexit->keyword, TO_ROOM);
						break;
					case 3:
						act("Your $p morphs into a snake, its fangs sinking into your flesh.", ch, lockpick, NULL, TO_CHAR);
						act("$n's $p morphs into a snake, its fangs sinking into $s flesh.",   ch, lockpick, NULL, TO_ROOM);
						break;
				}
				damage(ch, ch, dice(ch->in_room->area->average_level, 10), TYPE_NOFIGHT);
				pop_call();
				return;
			}
		}

		if (lockpick->value[1] == 0)
		{
			act("$p has grown dull from overuse and is useless.", ch, lockpick, NULL, TO_CHAR);
			pop_call();
			return;
		}

		if (IS_SET(pexit->flags, EX_EASY_PICK))
		{
			pick_lvl = 50;
		}
		else if (IS_SET(pexit->flags, EX_HARD_PICK))
		{
			pick_lvl = 150;
		}
		else
		{
			pick_lvl = 100;
		}

		pick_lvl -= lockpick->value[0];

		if (number_range(1, pick_lvl) > learned(ch, gsn_pick_lock))
		{
			lockpick->value[1]--;

			act("You attempt to pick the $d's lock but fail.",  ch, NULL, pexit->keyword, TO_CHAR);
			act("$n attempts to pick the $d's lock but fails.", ch, NULL, pexit->keyword, TO_ROOM);

			pop_call();
			return;
		}

		if (IS_SET(pexit->flags, EX_MAGICAL_LOCK) && !IS_SET(lockpick->value[2], PICK_MAGICAL_LOCK))
		{
			act("Your lockpick is useless against the $d's magical lock.", ch, NULL, pexit->keyword, TO_CHAR);
			pop_call();
			return;
		}

		if (IS_SET(pexit->flags, EX_MECHANICAL_LOCK) && !IS_SET(lockpick->value[2], PICK_MECHANICAL_LOCK))
		{
			act("Your lockpick is useless against the $d's mechanical lock.", ch, NULL, pexit->keyword, TO_CHAR);
			pop_call();
			return;
		}

		if (IS_SET(pexit->flags, EX_MAGICAL_LOCK) || IS_SET(pexit->flags, EX_MECHANICAL_LOCK))
		{
			if (learned(ch, gsn_greater_pick) == 0)
			{
				act("You're not skilled enough to pick the $d's lock.", ch, NULL, pexit->keyword, TO_CHAR);
				pop_call();
				return;
			}

			pick_lvl = ch->in_room->area->average_level;

			if (IS_SET(pexit->flags, EX_EASY_PICK))
			{
				pick_lvl *= 3;
			}
			else if (IS_SET(pexit->flags, EX_HARD_PICK))
			{
				pick_lvl *= 9;
			}
			else
			{
				pick_lvl *= 6;
			}

			pick_lvl = UMAX(1, pick_lvl - lockpick->value[0]);

			if (number_range(ch->in_room->area->average_level, pick_lvl) > multi_skill_level(ch, gsn_greater_pick))
			{
				lockpick->value[1]--;

				act("You attempt to pick the $d's lock with $p but fail.",  ch, lockpick, pexit->keyword, TO_CHAR);
				act("$n attempts to pick the $d's lock with $p but fails.", ch, lockpick, pexit->keyword, TO_ROOM);

				pop_call();
				return;
			}
			check_improve(ch, gsn_greater_pick);
		}

		REMOVE_BIT(pexit->flags, EX_LOCKED);

		act("You pick the $d's lock with $p.", ch, lockpick, pexit->keyword, TO_CHAR);
		act("$n picks the $d's lock with $p.", ch, lockpick, pexit->keyword, TO_ROOM);

		/*
			pick the other side
		*/

		if ((to_room = room_index[pexit->to_room]) != NULL
		&&  (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
		&&   room_index[pexit_rev->to_room] == ch->in_room)
		{
			REMOVE_BIT( pexit_rev->flags, EX_LOCKED );
		}
		check_improve(ch, gsn_pick_lock);

		pop_call();
		return;
	}

	if ((obj = get_obj_here(ch, arg)) != NULL)
	{
		/*
			'pick object'
		*/

		if (obj->item_type != ITEM_CONTAINER)
		{
			act("$p is not a container.", ch, obj, NULL, TO_CHAR);
			pop_call();
			return;
		}

		if (!IS_SET(obj->value[1], CONT_CLOSED))
		{
			act("$p is not closed.", ch, obj, NULL, TO_CHAR);
			pop_call();
			return;
		}

		if (!IS_SET(obj->value[1], CONT_LOCKED))
		{
			act("$p is already unlocked.", ch, obj, NULL, TO_CHAR);
			pop_call();
			return;
		}

		if (IS_SET(obj->value[1], CONT_PICKPROOF))
		{
			act("$p's lock cannot be picked.", ch, obj, NULL, TO_CHAR);
			pop_call();
			return;
		}

		if (IS_SET(obj->value[1], CONT_SMALL_LOCK) && !IS_SET(lockpick->value[2], PICK_SMALL_LOCK))
		{
			act("$p is too big to fit $P's lock.", ch, lockpick, obj, TO_CHAR);
			pop_call();
			return;
		}

		if (IS_SET(obj->value[1], CONT_BIG_LOCK) && !IS_SET(lockpick->value[2], PICK_BIG_LOCK))
		{
			act("$p is too small to fit $P's lock.", ch, lockpick, obj, TO_CHAR);
			pop_call();
			return;
		}

		if (IS_SET(obj->value[1], CONT_TRAPPED_LOCK))
		{
			pick_lvl  = multi_skill_level(ch, gsn_greater_pick);
			pick_lvl += IS_SET(lockpick->value[2], PICK_TRAPPED_LOCK) ? lockpick->level : 0;
			pick_lvl += get_curr_dex(ch);

			if (IS_SET(obj->value[1], CONT_MAGICAL_LOCK) && !IS_SET(lockpick->value[2], PICK_MAGICAL_LOCK))
			{
				pick_lvl = UMAX(1, pick_lvl - ch->in_room->area->average_level);
			}

			if (IS_SET(obj->value[1], CONT_MECHANICAL_LOCK) && !IS_SET(lockpick->value[2], PICK_MECHANICAL_LOCK))
			{
				pick_lvl = UMAX(1, pick_lvl - ch->in_room->area->average_level);
			}

			if (IS_SET(obj->value[1], CONT_HARD_PICK))
			{
				pick_lvl = UMAX(1, pick_lvl - ch->in_room->area->average_level);
			}

			if (IS_SET(obj->value[1], CONT_EASY_PICK))
			{
				pick_lvl = UMAX(1, pick_lvl + ch->in_room->area->average_level);
			}

			if (number_range(1, pick_lvl) < ch->in_room->area->average_level * 3)
			{
				switch (number_bits(2))
				{
					case 0:
						act("$P snarls and bites you viciously.", ch, NULL, obj, TO_CHAR);
						act("$P snarls and bites $n viciously.",   ch, NULL, obj, TO_ROOM);
						break;
					case 1:
						act("$P shakes and zaps you with a bolt of lightning.", ch, NULL, obj, TO_CHAR);
						act("$P shakes and zaps $n with a bolt of lightning.",  ch, NULL, obj, TO_ROOM);
						break;
					case 2:
						act("A tiny creature slithers out of $P's lock and explodes in your face.", ch, NULL, obj, TO_CHAR);
						act("A tiny creature slithers out of $P's lock and explodes in $n's face.", ch, NULL, obj, TO_ROOM);
						break;
					case 3:
						act("Your $p melts into a snake, its fangs sinking into your flesh.", ch, lockpick, NULL, TO_CHAR);
						act("$n's $p melts into a snake, its fangs sinking into $s flesh.",   ch, lockpick, NULL, TO_ROOM);
						break;
				}
				damage(ch, ch, dice(ch->in_room->area->average_level, 10), TYPE_NOFIGHT);
				pop_call();
				return;
			}
		}

		if (lockpick->value[1] == 0)
		{
			act("$p has grown dull from overuse.", ch, lockpick, NULL, TO_CHAR);
			pop_call();
			return;
		}

		if (IS_SET(obj->value[1], CONT_EASY_PICK))
		{
			pick_lvl = 50;
		}
		else if (IS_SET(obj->value[1], CONT_HARD_PICK))
		{
			pick_lvl = 150;
		}
		else
		{
			pick_lvl = 100;
		}

		if (number_range(1, pick_lvl) > learned(ch, gsn_pick_lock) + lockpick->value[0])
		{
			lockpick->value[1]--;

			act("You attempt to pick $p's lock but fail.",  ch, obj, NULL, TO_CHAR);
			act("$n attempts to pick $p's lock but fails.", ch, obj, NULL, TO_ROOM);

			pop_call();
			return;
		}

		if (IS_SET(obj->value[1], CONT_MAGICAL_LOCK) && !IS_SET(lockpick->value[2], PICK_MAGICAL_LOCK))
		{
			act("$P is useless against $p's magical lock.", ch, obj, lockpick, TO_CHAR);
			pop_call();
			return;
		}

		if (IS_SET(obj->value[1], CONT_MECHANICAL_LOCK) && !IS_SET(lockpick->value[2], PICK_MECHANICAL_LOCK))
		{
			act("$P is useless against $p's mechanical lock.", ch, obj, lockpick, TO_CHAR);
			pop_call();
			return;
		}

		if (IS_SET(obj->value[1], CONT_MAGICAL_LOCK) || IS_SET(obj->value[1], CONT_MECHANICAL_LOCK))
		{
			if (learned(ch, gsn_greater_pick) == 0)
			{
				act("You're not skilled enough to pick $p's lock.", ch, obj, NULL, TO_CHAR);
				pop_call();
				return;
			}

			pick_lvl = ch->in_room->area->average_level;

			if (IS_SET(obj->value[1], CONT_EASY_PICK))
			{
				pick_lvl *= 3;
			}
			else if (IS_SET(obj->value[1], CONT_HARD_PICK))
			{
				pick_lvl *= 9;
			}
			else
			{
				pick_lvl *= 6;
			}

			pick_lvl = UMAX(1, pick_lvl - lockpick->value[0]);

			if (number_range(ch->in_room->area->average_level, pick_lvl) > multi_skill_level(ch, gsn_greater_pick))
			{
				lockpick->value[1]--;

				act("You attempt to pick $p's lock but fail.",  ch, obj, NULL, TO_CHAR);
				act("$n attempts to pick $p's lock but fails.", ch, obj, NULL, TO_ROOM);

				pop_call();
				return;
			}
			check_improve(ch, gsn_greater_pick);
		}

		REMOVE_BIT(obj->value[1], CONT_LOCKED);

		act("You pick $p's lock with $P.", ch, obj, lockpick, TO_CHAR);
		act("$n picks $p's lock with $P.", ch, obj, lockpick, TO_ROOM);

		check_improve(ch, gsn_pick_lock);

		pop_call();
		return;
	}
	else
	{
		act( "You see no $T here.", ch, NULL, arg, TO_CHAR );
	}
	pop_call();
	return;
}

void do_stand( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj = NULL;

	push_call("do_stand(%p,%p)",ch,argument);

	if (ch->position == POS_FIGHTING)
	{
		send_to_char("You are fighting!\n\r", ch);
		pop_call();
		return;
	}

	if (IS_AFFECTED(ch, AFF_SLEEP))
	{
		send_to_char( "You can't wake up!\n\r", ch );
		pop_call();
		return;
	}

	if (argument[0] != '\0')
	{
		if ((obj = get_obj_list(ch, argument, ch->in_room->first_content)) == NULL)
		{
			send_to_char("You do not see that here.\n\r", ch);
			pop_call();
			return;
		}
	}

	if (IS_AFFECTED(ch, AFF2_CAMPING))
	{
		send_to_char( "You break camp.\n\r", ch );
		act( "$n breaks camp.", ch, NULL, NULL, TO_ROOM );
		REMOVE_BIT(ch->affected2_by, 0-AFF2_CAMPING);
	}

	if (obj) 
	{
		if (obj->item_type != ITEM_FURNITURE
		|| (!IS_SET(obj->value[2], STAND_ON)
		&&  !IS_SET(obj->value[2], STAND_IN)))
		{
			send_to_char("You can't stand on that.\n\r", ch);
			pop_call();
			return;
		}

		if (ch->furniture != obj && count_users(obj) >= obj->value[0]) 
		{
			act("There's no more room on $p.", ch, obj, NULL, TO_CHAR);
			pop_call();
			return;
		}
	}

	switch (ch->position)
	{
		case POS_SLEEPING:
			ch->position = POS_STANDING;
			if (obj == NULL)
			{
				act("You wake and stand up.",  ch, NULL, NULL, TO_CHAR);
				act("$n wakes and stands up.", ch, NULL, NULL, TO_ROOM);
			}
			else if (IS_SET(obj->value[2], STAND_ON))
			{
				act("You wake and jump onto $p.",  ch, obj, NULL, TO_CHAR);
				act("$n wakes and jumps onto $p.", ch, obj, NULL, TO_ROOM);
			}
			else
			{
				act("You wake and step into $p.",  ch, obj, NULL, TO_CHAR);
				act("$n wakes and steps into $p.", ch, obj, NULL, TO_ROOM);
			}
			break;

		case POS_RESTING:
		case POS_SITTING:
		case POS_STANDING:
			if ((obj == NULL || obj == ch->furniture) && ch->position == POS_STANDING)
			{
				send_to_char("You are already standing.\n\r", ch);
				break;
			}
			if (ch->fighting)
			{
				ch->position = POS_FIGHTING;
			}
			else
			{
				ch->position = POS_STANDING;
			}
			if (obj == NULL)
			{
				act("You stand up.",  ch, NULL, NULL, TO_CHAR);
				act("$n stands up.", ch, NULL, NULL, TO_ROOM);
			}
			else if (IS_SET(obj->value[2], STAND_ON))
			{
				act("You jump onto $p.", ch, obj, NULL, TO_CHAR);
				act("$n jumps onto $p.", ch, obj, NULL, TO_ROOM);
			}
			else
			{
				act("You step into $p.", ch, obj, NULL, TO_CHAR);
				act("$n steps into $p.", ch, obj, NULL, TO_ROOM);
			}
			break;
	}
	ch->furniture = obj;
	pop_call();
	return;
}

void do_sit (CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj = NULL;

	push_call("do_sit(%p,%p)",ch,argument);

	if (ch->position == POS_FIGHTING)
	{
		send_to_char("You are fighting!\n\r", ch);
		pop_call();
		return;
	}

	if (IS_AFFECTED(ch, AFF_SLEEP))
	{
		send_to_char( "You can't wake up!\n\r", ch );
		pop_call();
		return;
	}

	if (is_mounting(ch))
	{
		send_to_char("You should dismount first.\n\r", ch);
		pop_call();
		return;
	}

	if (argument[0] != '\0')
	{
		if ((obj = get_obj_list(ch, argument, ch->in_room->first_content)) == NULL)
		{
			send_to_char("You do not see that here.\n\r", ch);
			pop_call();
			return;
		}

		if (obj->item_type != ITEM_FURNITURE
		|| (!IS_SET(obj->value[2], SIT_ON)
		&&  !IS_SET(obj->value[2], SIT_IN)))
		{
			send_to_char("You can't sit on that.\n\r", ch);
			pop_call();
			return;
		}

		if (ch->furniture != obj && count_users(obj) >= obj->value[0]) 
		{
			act("There's no more room on $p.", ch, obj, NULL, TO_CHAR);
			pop_call();
			return;
		}
	}

	if (IS_AFFECTED(ch, AFF2_CAMPING))
	{
		send_to_char( "You break camp.\n\r", ch );
		act( "$n breaks camp.", ch, NULL, NULL, TO_ROOM );
		REMOVE_BIT(ch->affected2_by, 0-AFF2_CAMPING);
	}

	switch (ch->position)
	{
		case POS_SLEEPING:
			ch->position = POS_SITTING;
			if (obj == NULL)
			{
				act("You wake and sit up.",  ch, NULL, NULL, TO_CHAR);
				act("$n wakes and sits up.", ch, NULL, NULL, TO_ROOM);
			}
			else if (IS_SET(obj->value[2], SIT_ON))
			{
				act("You wake and sit down on $p.",  ch, obj, NULL, TO_CHAR);
				act("$n wakes and sits down on $p.", ch, obj, NULL, TO_ROOM);
			}
			else
			{
				act("You wake and sit down in $p.",  ch, obj, NULL, TO_CHAR);
				act("$n wakes and sits down in $p.", ch, obj, NULL, TO_ROOM);
			}
			break;

		case POS_RESTING:
		case POS_STANDING:
		case POS_SITTING:
			if ((obj == NULL || obj == ch->furniture) && ch->position == POS_SITTING)
			{
				send_to_char("You are already sitting down.\n\r", ch);
				break;
			}
			ch->position = POS_SITTING;
			if (obj == NULL)
			{
				act("You sit down on the ground.", ch, NULL, NULL, TO_CHAR);
				act("$n sits down on the ground.", ch, NULL, NULL, TO_ROOM);
			}
			else if (IS_SET(obj->value[2], SIT_ON))
			{
				act("You sit down on $p.", ch, obj, NULL, TO_CHAR);
				act("$n sits down on $p.", ch, obj, NULL, TO_ROOM);
			}
			else
			{
				act("You sit down in $p.", ch, obj, NULL, TO_CHAR);
				act("$n sits down in $p.", ch, obj, NULL, TO_ROOM);
			}
			break;
	}
	ch->furniture = obj;
	pop_call();
	return;
}

void do_rest (CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj = NULL;

	push_call("do_rest(%p,%p)",ch,argument);

	if (ch->position == POS_FIGHTING)
	{
		send_to_char("You are fighting!\n\r", ch);
		pop_call();
		return;
	}

	if (IS_AFFECTED(ch, AFF_SLEEP))
	{
		send_to_char( "You can't wake up!\n\r", ch );
		pop_call();
		return;
	}

	if (is_mounting(ch))
	{
		send_to_char("You should dismount first.\n\r", ch);
		pop_call();
		return;
	}

	if (argument[0] != '\0')
	{
		if ((obj = get_obj_list(ch, argument, ch->in_room->first_content)) == NULL)
		{
			send_to_char("You do not see that here.\n\r", ch);
			pop_call();
			return;
		}

		if (obj->item_type != ITEM_FURNITURE
		|| (!IS_SET(obj->value[2], REST_ON)
		&&  !IS_SET(obj->value[2], REST_IN)))
		{
			send_to_char("You can't rest on that.\n\r", ch);
			pop_call();
			return;
		}

		if (ch->furniture != obj && count_users(obj) >= obj->value[0]) 
		{
			act("There's no more room on $p.", ch, obj, NULL, TO_CHAR);
			pop_call();
			return;
		}
	}

	switch (ch->position)
	{
		case POS_SLEEPING:
			ch->position = POS_RESTING;
			if (obj == NULL)
			{
				act("You wake and start resting.",  ch, NULL, NULL, TO_CHAR);
				act("$n wakes and starts resting.", ch, NULL, NULL, TO_ROOM);
			}
			else if (IS_SET(obj->value[2], REST_ON))
			{
				act("You wake and rest on $p.",  ch, obj, NULL, TO_CHAR);
				act("$n wakes and rests on $p.", ch, obj, NULL, TO_ROOM);
			}
			else
			{
				act("You wake and rest in $p.",  ch, obj, NULL, TO_CHAR);
				act("$n wakes and rests in $p.", ch, obj, NULL, TO_ROOM);
			}
			if (in_camp(ch))
			{
				send_to_char("You camp and begin recuperating.\n\r", ch);
				act("$n joins the camp.\n\r",ch,NULL,NULL,TO_ROOM);
			}
			break;

		case POS_SITTING:
		case POS_STANDING:
		case POS_RESTING:
			if ((obj == NULL || obj == ch->furniture) && ch->position == POS_RESTING)
			{
				send_to_char("You are already resting.\n\r", ch);
				break;
			}
			ch->position = POS_RESTING;
			if (obj == NULL)
			{
				act("You sit down and rest.",  ch, NULL, NULL, TO_CHAR);
				act("$n sits down and rests.", ch, NULL, NULL, TO_ROOM);
			}
			else if (IS_SET(obj->value[2], REST_ON))
			{
				act("You rest on $p.", ch, obj, NULL, TO_CHAR);
				act("$n rests on $p.", ch, obj, NULL, TO_ROOM);
			}
			else
			{
				act("You rest in $p.", ch, obj, NULL, TO_CHAR);
				act("$n rests in $p.", ch, obj, NULL, TO_ROOM);
			}
			if (in_camp(ch))
			{
				send_to_char("You camp and begin recuperating.\n\r", ch);
				act("$n joins the camp.\n\r",ch,NULL,NULL,TO_ROOM);
			}
			break;
	}
	ch->furniture = obj;
	pop_call();
	return;
}

void do_sleep( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj = NULL;

	push_call("do_sleep(%p,%p)",ch,argument);

	if (ch->position == POS_FIGHTING)
	{
		send_to_char("You are fighting!\n\r", ch);
		pop_call();
		return;
	}

	if (is_mounting(ch))
	{
		send_to_char("You should dismount first.\n\r", ch);
		pop_call();
		return;
	}

	if (argument[0] != '\0')
	{
		if ((obj = get_obj_list(ch, argument, ch->in_room->first_content)) == NULL)
		{
			send_to_char("You do not see that here.\n\r", ch);
			pop_call();
			return;
		}

		if (obj->item_type != ITEM_FURNITURE
		|| (!IS_SET(obj->value[2], SLEEP_ON)
		&&  !IS_SET(obj->value[2], SLEEP_IN)))
		{
			send_to_char("You can't sleep on that.\n\r", ch);
			pop_call();
			return;
		}

		if (ch->furniture != obj && count_users(obj) >= obj->value[0]) 
		{
			act("There's no more room on $p.", ch, obj, NULL, TO_CHAR);
			pop_call();
			return;
		}
	}

	switch (ch->position)
	{
		case POS_SLEEPING:
			if (obj == NULL || obj == ch->furniture)
			{
				send_to_char("You are already sleeping.\n\r", ch);
				break;
			}
			ch->position = POS_RESTING;
			if (IS_SET(obj->value[2], SLEEP_ON))
			{
				act("You wake up and go to sleep on $p.",   ch, obj, NULL, TO_CHAR);
				act("$n wakes up and goes to sleep on $p.", ch, obj, NULL, TO_ROOM);
			}
			else
			{
				act("You wake up and go to sleep in $p.",   ch, obj, NULL, TO_CHAR);
				act("$n wakes up and goes to sleep in $p.", ch, obj, NULL, TO_ROOM);
			}
			ch->position = POS_SLEEPING;
			break;

		case POS_SITTING:
		case POS_STANDING:
		case POS_RESTING:
			if (obj == NULL)
			{
				act("You go to sleep.",  ch, NULL, NULL, TO_CHAR);
				act("$n goes to sleep.", ch, NULL, NULL, TO_ROOM);
			}
			else if (IS_SET(obj->value[2], SLEEP_ON))
			{
				act("You go to sleep on $p.",  ch, obj, NULL, TO_CHAR);
				act("$n goes to sleep on $p.", ch, obj, NULL, TO_ROOM);
			}
			else
			{
				act("You go to sleep in $p.",  ch, obj, NULL, TO_CHAR);
				act("$n goes to sleep in $p.", ch, obj, NULL, TO_ROOM);
			}
			if (ch->position != POS_RESTING && in_camp(ch))
			{
				send_to_char("You camp and begin recuperating.\n\r", ch);
				act("$n joins the camp.\n\r", ch, NULL, NULL, TO_ROOM);
			}
			ch->position = POS_SLEEPING;
			break;
	}
	ch->furniture = obj;
	pop_call();
	return;
}

void do_wake( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	push_call("do_wake(%p,%p)",ch,argument);

	one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		do_stand( ch, argument );
		pop_call();
		return;
	}

	if (!IS_AWAKE(ch))
	{
		send_to_char("You are asleep yourself!\n\r", ch);
		pop_call();
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char( "They aren't here.\n\r", ch);
		pop_call();
		return;
	}

	if (IS_AWAKE(victim))
	{
		act( "$N is already awake.", ch, NULL, victim, TO_CHAR );
		pop_call();
		return;
	}

	if (IS_AFFECTED(victim, AFF_SLEEP))
	{
		act("You can't wake $M!", ch, NULL, victim, TO_CHAR );
		pop_call();
		return;
	}
	act("You wake $M.", ch, NULL, victim, TO_CHAR );
	victim->position = POS_STANDING;
	act("$n wakes you.", ch, NULL, victim, TO_VICT );

	if (victim->furniture)
	{
		victim->furniture = NULL;
	}

	if (IS_AFFECTED(victim, AFF2_CAMPING))
	{
		send_to_char( "You break camp.\n\r", victim);
		act("$n breaks camp.", victim, NULL, NULL, TO_ROOM );
		REMOVE_BIT(victim->affected2_by, 0-AFF2_CAMPING);
	}
	pop_call();
	return;
}

void do_sneak( CHAR_DATA *ch, char *argument )
{
	AFFECT_DATA af;

	push_call("do_sneak(%p,%p)",ch,argument);

	if (number_percent() < learned(ch, gsn_sneak))
	{
		affect_strip( ch, gsn_sneak );

		af.type      = gsn_sneak;
		af.duration  = ch->level;
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		af.bittype   = AFFECT_TO_CHAR;
		af.bitvector = AFF_SNEAK;
		affect_to_char( ch, &af );

		if (learned(ch, gsn_pass_without_trace) > 0)
		{
			send_to_char("The wind would leave more trace of it's passage than you.\n\r",ch);
		}
		else
		{
			send_to_char("You start moving silently.\n\r",ch);
		}
	}
	else
	{
		send_to_char( "You attempt to move silently.\n\r", ch );
	}
	pop_call();
	return;
}

void do_hide( CHAR_DATA *ch, char *argument )
{
	push_call("do_hide(%p,%p)",ch,argument);

	if (number_percent() < learned(ch, gsn_hide))
	{
		send_to_char("You skillfully hide yourself.\n\r",ch);
		SET_BIT(ch->affected_by, AFF_HIDE);
		check_improve(ch, gsn_hide);
	}
	else
	{
		send_to_char( "You attempt to hide.\n\r", ch );
	}
	pop_call();
	return;
}

/*
	Contributed by Alander.
*/

/* The STOP command.  Added selectives.  - Chaos 6/22/97  */

void do_visible( CHAR_DATA *ch, char *argument )
{
	bool all, pick;

	push_call("do_visible(%p,%p)",ch,argument);

	all  = FALSE;
	pick = FALSE;

	if (!strcasecmp(argument, "all"))
	{
		all = TRUE;
	}

	if (!strcasecmp(argument, "invis") || all)
	{
		pick = TRUE;

		if (IS_AFFECTED(ch, AFF_INVISIBLE) || IS_AFFECTED(ch, AFF_IMP_INVISIBLE))
		{
			send_to_char( "You become visible.\n\r", ch );
		}
		else if (!all)
		{
			send_to_char( "You are already visible.\n\r", ch );
		}
		affect_strip ( ch, gsn_invis			);
		affect_strip ( ch, gsn_improved_invis	);
		REMOVE_BIT   ( ch->affected_by, AFF_INVISIBLE);
		REMOVE_BIT   ( ch->affected_by, AFF_IMP_INVISIBLE);
	}

	if (!strcasecmp(argument, "sneak") || all)
	{
		pick = TRUE;

		if (IS_AFFECTED(ch, AFF_SNEAK))
		{
			send_to_char( "You stop sneaking.\n\r", ch );
		}
		else if (!all)
		{
			send_to_char( "You are not sneaking.\n\r", ch );
		}
		affect_strip ( ch, gsn_sneak			);
		REMOVE_BIT   ( ch->affected_by, AFF_SNEAK);
	}

	if (!strcasecmp(argument, "flash") || all)
	{
		pick = TRUE;
		if (IS_AFFECTED(ch, AFF2_HAS_FLASH))
		{
			send_to_char( "You throw away the flash powder.\n\r", ch );
		}
		else if (!all)
		{
			send_to_char( "You don't have any flash powder.\n\r", ch );
		}
		REMOVE_BIT(ch->affected2_by, 0-AFF2_HAS_FLASH );
	}

	if (!strcasecmp(argument, "stealth") || all)
	{
		pick = TRUE;
		if (IS_AFFECTED(ch, AFF_STEALTH))
		{
			if (learned(ch, gsn_greater_stealth) > 0)
			{
				send_to_char("You reveal yourself to mortal eyes.\n\r", ch);
			}
			else
			{
				send_to_char( "You stop being stealthy.\n\r", ch );
			}
		}
		else if (!all)
		{
			send_to_char( "You are not stealthy.\n\r", ch );
		}
		affect_strip ( ch, gsn_stealth			);
		REMOVE_BIT   ( ch->affected_by, AFF_STEALTH	);
	}

	if (!strcasecmp(argument, "clear") || all)
	{
		pick = TRUE;
		if (IS_AFFECTED(ch, AFF_CLEAR))
		{
			send_to_char( "You stop clearing a path.\n\r", ch );
		}
		else if (!all)
		{
			send_to_char( "You are not clearing a path.\n\r", ch );
		}
		affect_strip ( ch, gsn_clear_path         );
		REMOVE_BIT   ( ch->affected_by, AFF_CLEAR );
	}

	if (!strcasecmp(argument, "hunt") || all)
	{
		pick = TRUE;
		if (IS_AFFECTED(ch, AFF_HUNT))
		{
			send_to_char( "You stop hunting.\n\r", ch );
		}
		else if (!all)
		{
			send_to_char( "You are not hunting.\n\r", ch );
		}
		affect_strip ( ch, gsn_hunt              );
		REMOVE_BIT   ( ch->affected_by, AFF_HUNT );
	}

	if (!strcasecmp(argument, "hide") || all)
	{
		pick = TRUE;
		if (IS_AFFECTED(ch, AFF_HIDE))
		{
			send_to_char( "You stop hiding.\n\r", ch );
		}
		else if (!all)
		{
			send_to_char( "You are not hiding.\n\r", ch );
		}
		REMOVE_BIT   ( ch->affected_by, AFF_HIDE );
	}

	if (!strcasecmp(argument, "disguise") || all)
	{
		pick = TRUE;
		if (!IS_NPC(ch) && ch->long_descr != NULL && ch->long_descr[0] != '\0')
		{
			STRFREE (ch->long_descr );
			ch->long_descr = STRALLOC("");
			send_to_char( "You remove your disguise.\n\r", ch );
		}
		else if (IS_NPC(ch) && ch->long_descr != ch->pIndexData->long_descr)
		{
			STRFREE (ch->long_descr );
			ch->long_descr = STRALLOC(ch->pIndexData->long_descr);
		}
		else if (!all)
		{
			send_to_char( "You are not disguised.\n\r", ch );
		}
	}

	if (!pick && all)
	{
		send_to_char( "You are not doing anything.\n\r", ch);
	}
	if (!all && !pick)
	{
		send_to_char( "Syntax: stop <all invis sneak stealth hunt hide clear disguise flash>\n\r", ch);
	}
	pop_call();
	return;
}

void do_recall( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *old_room;
	CHAR_DATA *rch;
	CHAR_DATA *victim;
	OBJ_DATA *scroll;

	bool pkill;

	push_call("do_recall(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (ch->in_room->sector_type == SECT_ETHEREAL)
	{
		send_to_char( "You don't know the path back to the real universe.\n\r", ch);
		pop_call();
		return;
	}

	if (ch->in_room->sector_type == SECT_ASTRAL)
	{
		send_to_char( "You are too far out of your mind to recall anything.\n\r", ch);
		pop_call();
		return;
	}

	if (ch->pcdata->recall == ROOM_VNUM_SCHOOL)
	{
		if (ch->in_room->area->low_r_vnum != ROOM_VNUM_SCHOOL)
		{
			ch->pcdata->recall = ROOM_VNUM_TEMPLE;
		}
	}

	if (room_index[ch->pcdata->recall] == NULL)
	{
		ch->pcdata->recall = ROOM_VNUM_TEMPLE;
	}

	/* Chaos 10/8/93 */

	old_room = ch->in_room;

	if (argument && tolower(*argument) == 's')
	{
		if (IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL) || HAS_BIT(ch->in_room->room_flags, ROOM_NO_SAVE))
		{
			send_to_char( "You cannot do that in this room.\n\r", ch);
			pop_call();
			return;
		}
		if (IS_SET(ch->in_room->area->flags, AFLAG_NORECALL))
		{
			send_to_char( "You cannot do that in this area.\n\r", ch);
			pop_call();
			return;
		}
		ch->pcdata->recall = ch->in_room->vnum;
		send_to_char( "Your recall room has been set.\n\r", ch);
		pop_call();
		return;
	}

	if (argument && tolower(*argument) == 'r')
	{
		ch->pcdata->recall = ch->pcdata->death_room;
		send_to_char( "Your recall room has been reset.\n\r", ch);
		pop_call();
		return;
	}

	if (ch->in_room->vnum == ch->pcdata->recall)
	{
		send_to_char("You are already in your recall room.\n\r", ch);
		pop_call();
		return;
	}

	act( "$n prays for transportation!", ch, NULL, NULL, TO_ROOM );
	wait_state(ch, PULSE_PER_SECOND);

	if (argument)
	{
		if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
		{
			send_to_char( "You cannot do that here.\n\r", ch);
			pop_call();
			return;
		}

		for (scroll = ch->first_carrying ; scroll ; scroll = scroll->next_content)
		{
			if (scroll->item_type == ITEM_SCROLL)
			{
				if (scroll->value[1] == gsn_word_of_recall
				||  scroll->value[2] == gsn_word_of_recall
				||  scroll->value[3] == gsn_word_of_recall)
				{
					break;
				}
			}
		}
		if (!scroll)
		{
			send_to_char( "You do not have that scroll.\n\r", ch );
			pop_call();
			return;
		}
		junk_obj(scroll);
	}

	pkill = (who_fighting(ch) && !IS_NPC(ch->fighting->who));

	if (IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
	||  IS_AFFECTED(ch, AFF_CURSE)
	||  IS_SET(ch->in_room->area->flags, AFLAG_NORECALL)
	||  number_percent() <= ch->level / 20
	|| (ch->hit > ch->max_hit / 2 && pkill))
	{
		ch_printf(ch, "%s\n\r", god_table[which_god(ch)].recall_msg);

		pop_call();
		return;
	}

	if ((victim = who_fighting(ch)) != NULL)
	{
		int lose = 99 + ch->level * ch->level * 10;
		gain_exp(ch, 0 - lose);

		ch_printf( ch, "You recall from combat!  You lose %d experience points.\n\r", lose );

		if (pkill && which_god(ch) != which_god(victim))
		{
			sprintf(buf, "%s", get_name(ch));
			do_battle(god_table[which_god(victim)].pk_recall_msg, buf, get_name(victim));
		}
	}

	act("$n disappears.", ch, NULL, NULL, TO_ROOM);

	stop_fighting(ch, TRUE);

	char_to_room(ch, ch->pcdata->recall);

	act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );

	do_look(ch, "auto");

	/*
		Pets recall with char    Chaos 12/6/93
	*/

	for (rch = old_room->first_person ; rch ; rch = rch->next_in_room)
	{
	 	if (IS_AFFECTED(rch, AFF_CHARM) && rch->master == ch)
	 	{
	 		act("$n prays for transportation!", rch, NULL, NULL, TO_ROOM);
	 		act("$n disappears.", rch, NULL, NULL, TO_ROOM);
	 		char_to_room(rch, ch->pcdata->recall);
	 		act("$n appears in the room.", rch, NULL, NULL, TO_ROOM);
		}
	}
	pop_call();
	return;
}

void do_train( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_INPUT_LENGTH];
	char	arg[MAX_INPUT_LENGTH];
	CHAR_DATA *mob;
	int hp_gain = 0;
	int mana_gain = 0;
	int move_gain = 0;
	sh_int *pAbility;
	char *pOutput;
	int cost, pt, count;

	push_call("do_train(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}
	cost     = 5;
	count    = 1;
	pAbility = NULL;
	pOutput  = NULL;

	/*
		Check for trainer.
	*/

	for (mob = ch->in_room->first_person ; mob ; mob = mob->next_in_room)
	{
		if (IS_NPC(mob) && IS_SET(mob->act, ACT_TRAIN))
		{
			break;
		}
	}

	if (mob == NULL)
	{
		send_to_char( "You can't do that here.\n\r", ch );
		pop_call();
		return;
	}

	one_argument(argument, arg);

	if (is_number(arg))
	{
		count = atol(arg);
		argument = one_argument(argument, arg);
	}

	count = URANGE(1, count, 1000);

	if (argument[0] == '\0')
	{
		ch_printf(ch, "You have %d practice sessions.\n\r", ch->pcdata->practice);
		argument = "foo";
	}

	pt = 18 + (ch->level/15) + ch->pcdata->reincarnation;

	if (!strcasecmp(argument, "str"))
	{
		if (class_table[ch->class].attr_prime == APPLY_STR)
		{
			cost -= class_table[ch->class].prime_mod;
		}
		else if (class_table[ch->class].attr_second == APPLY_STR)
		{
			cost -= class_table[ch->class].sec_mod;
		}
		pAbility = &ch->pcdata->perm_str;
		pOutput  = "strength";
	}
	else if (!strcasecmp(argument, "int"))
	{
		if (class_table[ch->class].attr_prime == APPLY_INT)
		{
			cost -= class_table[ch->class].prime_mod;
		}
		else if (class_table[ch->class].attr_second == APPLY_INT)
		{
			cost -= class_table[ch->class].sec_mod;
		}
		pAbility = &ch->pcdata->perm_int;
		pOutput  = "intelligence";
	}
	else if (!strcasecmp(argument, "wis"))
	{
		if (class_table[ch->class].attr_prime == APPLY_WIS)
		{
			cost -= class_table[ch->class].prime_mod;
		}
		else if (class_table[ch->class].attr_second == APPLY_WIS)
		{
			cost -= class_table[ch->class].sec_mod;
		}
		pAbility = &ch->pcdata->perm_wis;
		pOutput  = "wisdom";
	}
	else if (!strcasecmp(argument, "dex"))
	{
		if (class_table[ch->class].attr_prime == APPLY_DEX)
		{
			cost -= class_table[ch->class].prime_mod;
		}
		else if (class_table[ch->class].attr_second == APPLY_DEX)
		{
			cost -= class_table[ch->class].sec_mod;
		}
		pAbility = &ch->pcdata->perm_dex;
		pOutput  = "dexterity";
	}
	else if ( !strcasecmp( argument, "con" ) )
	{
		if (class_table[ch->class].attr_prime == APPLY_CON)
		{
			cost -= class_table[ch->class].prime_mod;
		}
		else if (class_table[ch->class].attr_second == APPLY_CON)
		{
			cost -= class_table[ch->class].sec_mod;
		}
		pAbility = &ch->pcdata->perm_con;
		pOutput  = "constitution";
	}
	else if (!strcasecmp(argument, "hp"))
	{
		cost     = count;
		hp_gain  = 2;
		pOutput  = "number of hit points";
	}
	else if (!strcasecmp(argument, "mana"))
	{
		cost      = count;
		mana_gain = 3;
		pOutput   = "amount of mana";

	}
	else if (!strcasecmp( argument, "move"))
	{
		cost      = 1;
		move_gain = 6;
		pOutput   = "amount of move";
	}
	else
	{
		strcpy(buf, "You can train: hp");

		cat_sprintf(buf, "%s", cflag(ch, CFLAG_MANA) ? " mana" : "");
		cat_sprintf(buf, "%s", " move");
		cat_sprintf(buf, "%s", ch->pcdata->perm_str < pt ? " str" : "");
		cat_sprintf(buf, "%s", ch->pcdata->perm_int < pt ? " int" : "");
		cat_sprintf(buf, "%s", ch->pcdata->perm_wis < pt ? " wis" : "");
		cat_sprintf(buf, "%s", ch->pcdata->perm_dex < pt ? " dex" : "");
		cat_sprintf(buf, "%s", ch->pcdata->perm_con < pt ? " con" : "");

		ch_printf(ch, "%s.\n\r", buf);
		pop_call();
		return;
	}

	if (!strcasecmp(argument, "hp"))
	{
		if (cost > ch->pcdata->practice)
		{
			if (count == 1)
			{
				ch_printf(ch, "You need %d practice session to train your hp.\n\r", cost);
			}
			else
			{
				ch_printf(ch, "You need %d practice sessions to train your hp %d times.\n\r", cost, count);
			}
			pop_call();
			return;
		}
		ch->pcdata->practice -= cost;
		ch->max_hit          += hp_gain * count;
		ch->hit              += hp_gain * count;
		ch->pcdata->actual_max_hit   += hp_gain * count;
		act( "Your $T increases!", ch, NULL, pOutput, TO_CHAR );
		act( "$n's $T increases!", ch, NULL, pOutput, TO_ROOM );
		pop_call();
		return;
	}

	if (!strcasecmp(argument, "move"))
	{
		if (cost > ch->pcdata->practice)
		{
			if (count == 1)
			{
				ch_printf(ch, "You need %d practice session to train your move.\n\r", cost);
			}
			else
			{
				ch_printf(ch, "You need %d practice sessions to train your move %d times\n\r", cost, count);
			}
			pop_call();
			return;
		}
		ch->pcdata->practice -= cost;
		ch->max_move         += move_gain * count;
		ch->move             += move_gain * count;
		ch->pcdata->actual_max_move += move_gain * count;
		act( "Your $T increases!", ch, NULL, pOutput, TO_CHAR );
		act( "$n's $T increases!", ch, NULL, pOutput, TO_ROOM );
		pop_call();
		return;
	}

	if (!strcasecmp(argument, "mana"))
	{
		if (!cflag(ch, CFLAG_MANA))
		{
			send_to_char("You need to be a spell caster to train your mana.\n\r", ch);

			pop_call();
			return;
		}

		if (cost > ch->pcdata->practice)
		{
			if (count == 1)
			{
				ch_printf(ch, "You need %d practice session to train your mana.\n\r", cost);
			}
			else
			{
				ch_printf(ch, "You need %d practice sessions to train your mana %d times.\n\r", cost, count);
			}
			pop_call();
			return;
		}
		ch->pcdata->practice -= cost;
		ch->max_mana         += mana_gain * count;
		ch->mana             += mana_gain * count;
		ch->pcdata->actual_max_mana += mana_gain * count;
		act( "Your $T increases!", ch, NULL, pOutput, TO_CHAR );
		act( "$n's $T increases!", ch, NULL, pOutput, TO_ROOM );
		pop_call();
		return;
	}

	if (*pAbility >= pt)
	{
		act( "Your $T is already at maximum.", ch, NULL, pOutput, TO_CHAR );
		pop_call();
		return;
	}

	if (*pAbility + count > pt)
	{
		act( "You cannot train your $T beyond the maximum.", ch, NULL, pOutput, TO_CHAR);
		pop_call();
		return;
	}

	cost *= count;

	if (cost > ch->pcdata->practice)
	{
		if (count == 1)
		{
			ch_printf(ch, "You need %d practice sessions to train your %s.\n\r", cost, pOutput);
		}
		else
		{
			ch_printf(ch, "You need %d practice sessions to train your %s %d times.\n\r", cost, pOutput, count);
		}
		pop_call();
		return;
	}

	ch->pcdata->practice -= cost;
	*pAbility            += count;
	act( "Your $T increases!", ch, NULL, pOutput, TO_CHAR );
	act( "$n's $T increases!", ch, NULL, pOutput, TO_ROOM );
	pop_call();
	return;
}

void do_stealth( CHAR_DATA *ch, char *argument )
{
	AFFECT_DATA af;

	push_call("do_stealth(%p,%p)",ch,argument);

	if (number_percent() < learned(ch, gsn_stealth))
	{
		affect_strip( ch, gsn_stealth );

		af.type      = gsn_stealth;
		af.duration  = multi_skill_level(ch, gsn_stealth);
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		af.bittype   = AFFECT_TO_CHAR;
		af.bitvector = AFF_STEALTH;
		affect_to_char( ch, &af );

		if (learned(ch, gsn_greater_stealth) > 0)
		{
			send_to_char("You vanish from the prying eyes of mortality.\n\r",ch);
		}
		else
		{
			send_to_char("You start moving stealthily.\n\r", ch);
		}
	}
	else
	{
		send_to_char( "You attempt to move silently and remain hidden.\n\r", ch );
	}
	pop_call();
	return;
}

void do_clear_path( CHAR_DATA *ch, char *argument )
{
	AFFECT_DATA af;

	push_call("do_clear_path(%p,%p)",ch,argument);

	affect_strip( ch, gsn_clear_path );

	if (learned(ch, gsn_clear_path) == 0)
	{
		send_to_char("You are not skilled enough to clear paths.\n\r", ch);
		pop_call();
		return;
	}

	send_to_char("You are now clearing a path.\n\r", ch);

	ch->speed		= get_max_speed(ch);
	af.type		= gsn_clear_path;
	af.duration	= 24;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bittype     = AFFECT_TO_CHAR;
	af.bitvector	= AFF_CLEAR;
	affect_to_char( ch, &af);

	pop_call();
	return;
}

void do_hunt( CHAR_DATA *ch, char *argument )
{
	AFFECT_DATA af;

	push_call("do_hunt(%p,%p)",ch,argument);

	affect_strip( ch, gsn_hunt);

	if (learned(ch, gsn_hunt) == 0)
	{
		send_to_char("You are not skilled enough to hunt.\n\r", ch);
		pop_call();
		return;
	}

	send_to_char ("You start to hunt.\n\r", ch);
	af.type      = gsn_hunt;
	af.duration  = 24;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF_HUNT;
	affect_to_char( ch, &af);

	pop_call();
	return;
}

int can_see_track(CHAR_DATA *ch, int index)
{
	if (ch->in_room->last_left_bits[index] == 0)
	{
		return -1;
	}

	if (ch->in_room->last_left_time[index] < mud->current_time - 1200)
	{
		ch->in_room->last_left_bits[index] = 0;

		return -1;
	}

	if (!IS_SET(ch->in_room->last_left_bits[index], TRACK_BLOOD) && number_percent() > learned(ch, gsn_track))
	{
		return -1;
	}

	if (IS_SET(ch->in_room->last_left_bits[index], TRACK_FLY))
	{
		if (number_percent() > learned(ch, gsn_greater_track))
		{
			return -1;
		}
	}

	switch (HAS_BIT(ch->in_room->last_left_bits[index], TRACK_NORTH|TRACK_SOUTH|TRACK_EAST|TRACK_WEST|TRACK_UP|TRACK_DOWN))
	{
		case TRACK_NORTH:
			return DIR_NORTH;

		case TRACK_SOUTH:
			return DIR_SOUTH;
			
		case TRACK_EAST:
			return DIR_EAST;
			
		case TRACK_WEST:
			return DIR_WEST;
			
		case TRACK_UP:
			return DIR_UP;

		case TRACK_DOWN:
			return DIR_DOWN;
	}
	return -1;
}

void do_track( CHAR_DATA *ch, char *argument)
{
	int cnt;
	int TRACK_DIR;

	push_call("do_track(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (IS_AFFECTED(ch, AFF_BLIND))
	{
		send_to_char("You can't see a thing!\n\r",ch);
		pop_call();
		return;
	}

	if (multi(ch, gsn_track) == -1)
	{
		send_to_char( "You lack the aptitude to track.\n\r", ch);
		pop_call();
		return;
	}

	if (argument[0] == '\0')
	{
		if (*ch->pcdata->tracking)
		{
			RESTRING(ch->pcdata->tracking, "");
			send_to_char("You stop tracking.\n\r", ch);
			pop_call();
			return;
		}

		send_to_char("You locate the following tracks:\n\r", ch);

		for (cnt = 0 ; cnt < MAX_LAST_LEFT ; cnt++)
		{
			TRACK_DIR = can_see_track(ch, cnt);

			if (TRACK_DIR != -1)
			{
				ch_printf(ch, "%s leads %s.\n\r", capitalize(ch->in_room->last_left_name[cnt]), dir_name[TRACK_DIR]);
			}
		}
		check_improve(ch, gsn_track);
	}
	else
	{
		CHAR_DATA *rch;

		if (multi(ch, gsn_greater_track) == -1)
		{
			send_to_char("You lack the aptitude to track targets.\n\r", ch);
			pop_call();
			return;
		}

		if (ch->position != POS_STANDING)
		{
			RESTRING(ch->pcdata->tracking, "");
			send_to_char("You stop tracking.\n\r", ch);
			pop_call();
			return;
		}

		if (*ch->pcdata->tracking)
		{
			ch->pcdata->was_in_room = ch->in_room->vnum;
		}

		for (rch = ch->in_room->first_person ; rch ; rch = rch->next_in_room)
		{
			if (!can_see(ch, rch))
			{
				continue;
			}

			if (!strcasecmp(rch->name, argument) || !strcasecmp(rch->short_descr, argument))
			{
				act( "You're already in the same room as $N.", ch, NULL, rch, TO_CHAR);
				pop_call();
				return;
			}
		}

		for (cnt = MAX_LAST_LEFT - 1 ; cnt >= 0 ; cnt--)
		{
			TRACK_DIR = can_see_track(ch, cnt);

			if (TRACK_DIR == -1)
			{
				continue;
			}

			if (!strcasecmp(ch->in_room->last_left_name[cnt], argument))
			{
				ch_printf(ch, "You find the tracks of %s and quickly follow them.\n\r", ch->in_room->last_left_name[cnt]);
				RESTRING(ch->pcdata->tracking, ch->in_room->last_left_name[cnt]);
				break;
			}
		}

		if (cnt >= 0)
		{
			if (ch->in_room->exit[TRACK_DIR]->to_room == ch->pcdata->was_in_room)
			{
				send_to_char ("Your prey has doubled back!\n\r", ch);
				RESTRING(ch->pcdata->tracking, "");

				pop_call();
				return;
			}

			if (!can_move_char(ch, TRACK_DIR))
			{
				RESTRING(ch->pcdata->tracking, "");
			}
			else
			{
				check_improve(ch, gsn_greater_track);
			}
			move_char(ch, TRACK_DIR, TRUE);
		}
		else
		{
			if (*ch->pcdata->tracking )
			{
				ch_printf(ch, "%s's tracks end here.\n\r", capitalize(ch->pcdata->tracking));
				RESTRING(ch->pcdata->tracking, "");
			}
			else
			{
				ch_printf(ch, "You cannot find any tracks made by %s.\n\r", argument);
			}

		}
	}
	pop_call();
	return;
}

int get_max_speed( CHAR_DATA *ch )
{
	int spd;

	push_call("get_max_speed(%p)",ch);

	spd = race_table[ch->race].max_speed;

	if (IS_AFFECTED(ch, AFF_HASTE))
	{
		spd++;
	}

	if (is_affected(ch, gsn_slow))
	{
		spd = UMAX(0, spd-2);
	}

	if (is_affected(ch, gsn_cripple))
	{
		spd = UMAX(0, spd-2);
	}

	if (IS_AFFECTED(ch, AFF_CLEAR))
	{
		spd = UMAX(0, spd-2);
	}

	if (!IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL)
	{
		spd = 5;
	}

	pop_call();
	return( spd );
}

void do_travel( CHAR_DATA *ch, char *argument )
{
	push_call("do_travel(%p,%p)",ch,argument);

	if( IS_NPC( ch ) )
	{
		pop_call();
		return;
	}

	if( ch->position != POS_STANDING )
		send_to_char( "You can't travel right now.\n\r", ch );

	if( ch->pcdata->travel == -1 && *argument == '\0' )
	{
		send_to_char( "You must pick a direction to travel.\n\r", ch );
		pop_call();
		return;
	}

	ch->pcdata->travel = -1;

	if( *argument=='N' || *argument=='n' )
	{
		send_to_char( "You travel north.\n\r", ch );
		ch->pcdata->travel = 0;
	}

	if( *argument=='S' || *argument=='s' )
	{
		send_to_char( "You travel south.\n\r", ch );
		ch->pcdata->travel = 2;
	}

	if( *argument=='E' || *argument=='e' )
	{
		send_to_char( "You travel east.\n\r", ch );
		ch->pcdata->travel = 1;
	}

	if( *argument=='w' || *argument=='w' )
	{
		send_to_char( "You travel west.\n\r", ch );
		ch->pcdata->travel = 3;
	}

	if( *argument=='U' || *argument=='u' )
	{
		send_to_char( "You travel up.\n\r", ch );
		ch->pcdata->travel = 4;
	}

	if( *argument=='D' || *argument=='d' )
	{
		send_to_char( "You travel down.\n\r", ch );
		ch->pcdata->travel = 5;
	}

	if( ch->pcdata->travel == -1 )
	{
		ch->pcdata->travel_from = NULL;
		send_to_char( "You stop traveling.\n\r", ch );
	}
	else
	{
		ch->pcdata->travel_from = ch->in_room;
		move_char( ch, ch->pcdata->travel, TRUE );
	}
	pop_call();
	return;
}

void do_bashdoor( CHAR_DATA *ch, char *argument )
{
	EXIT_DATA *pexit;
	int        door;
	char       arg[MAX_INPUT_LENGTH];

	push_call("do_bashdoor(%p,%p)",ch,argument);

	if (!IS_NPC(ch) && multi(ch, gsn_door_bash) == -1)
	{
		send_to_char( "You slam yourself against the door, but nothing happens.\n\r", ch );
		send_to_char( "Maybe it's not a bad idea to learn how to do this first.\n\r", ch );
		pop_call();
		return;
	}

	one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		send_to_char( "Bash what door?\n\r", ch );
		pop_call();
		return;
	}

	if ((door = find_door(ch, arg)) >= 0)
	{
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA       *pexit_rev;
		int              chance;

		pexit = ch->in_room->exit[door];

		if (IS_SET(pexit->flags, EX_BASHED))
		{
			send_to_char( "It's already been bashed off its hinges.\n\r", ch );
			pop_call();
			return;
		}

		if (!IS_SET(pexit->flags, EX_CLOSED))
		{
			send_to_char( "Calm down.  It is already open.\n\r", ch );
			pop_call();
			return;
		}

		wait_state(ch, skill_table[gsn_door_bash].beats);

		chance = (get_curr_str(ch) + learned(ch, gsn_door_bash)) / 2;

		if (ch->fighting)
		{
			chance /= 2;
		}

		if (!IS_SET(pexit->flags, EX_BASHPROOF) &&  number_percent() < chance)
		{
			REMOVE_BIT(pexit->flags, EX_CLOSED);
			REMOVE_BIT(pexit->flags, EX_LOCKED);
			SET_BIT( pexit->flags, EX_BASHED );

			act("WHAAAAM!!!  You bash open the $d.", ch, NULL, pexit->keyword, TO_CHAR );
			act("WHAAAAM!!!  $n bashes open the $d.",  ch, NULL, pexit->keyword, TO_ROOM );

			if ((to_room = room_index[pexit->to_room]) != NULL
			&&  (pexit_rev = room_index[pexit->to_room]->exit[rev_dir[direction_door(arg)]]) != NULL
			&&  room_index[pexit_rev->to_room] == ch->in_room )
			{
				CHAR_DATA *rch;

				REMOVE_BIT(pexit_rev->flags, EX_CLOSED);
				REMOVE_BIT(pexit_rev->flags, EX_LOCKED);
				SET_BIT( pexit_rev->flags, EX_BASHED );

				for (rch = to_room->first_person ; rch ; rch = rch->next_in_room)
				{
					act("The $d crashes open.", rch, NULL, pexit_rev->keyword, TO_CHAR);
				}
			}
			damage(ch, ch, number_range(1, ch->max_hit / 40), gsn_door_bash);
			check_improve(ch, gsn_door_bash);
		}
		else
		{
			act("WHAAAAM!!!  You bash against the $d, but it doesn't budge.", ch, NULL, pexit->keyword, TO_CHAR );
			act("WHAAAAM!!!  $n bashes against the $d, but it holds strong.", ch, NULL, pexit->keyword, TO_ROOM );
			damage(ch, ch, number_range(1, ch->max_hit / 20), gsn_door_bash);
		}
	}
	else
	{
		act("WHAAAAM!!!  You bash against the wall, but it doesn't budge.", ch, NULL, NULL, TO_CHAR );
		act("WHAAAAM!!!  $n bashes against the wall, but it holds strong.", ch, NULL, NULL, TO_ROOM );
		damage(ch, ch, number_range(1, ch->max_hit / 20), gsn_door_bash);
	}
	pop_call();
	return;
}

