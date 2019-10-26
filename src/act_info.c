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

char *  const   where_name      [] =
{
	"   <Used as Light> ",
	"<Worn on L Finger> ",
	"<Worn on R Finger> ",
	"<Worn around Neck> ",
	"<Worn around Neck> ",
	"    <Worn on Body> ",
	"    <Worn on Head> ",
	"    <Worn on Legs> ",
	"    <Worn on Feet> ",
	"   <Worn on Hands> ",
	"    <Worn on Arms> ",
	"  <Worn as Shield> ",
	" <Worn about Body> ",
	"<Worn about Waist> ",
	" <Worn on L Wrist> ",
	" <Worn on R Wrist> ",
	"         <Wielded> ",
	"    <Dual Wielded> ",
	"            <Held> ",
	"           <Heart> "
};


/*
	Local functions.
*/

char	*	format_obj_to_char		args( ( OBJ_DATA *obj, CHAR_DATA *ch, int fShort ) );
void		show_list_to_char		args( ( OBJ_DATA *list, CHAR_DATA *ch, int fShort, bool fShowNothing ) );
void		show_char_to_char_0		args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void		show_char_to_char		args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
void		get_affects_string		(CHAR_DATA *, CHAR_DATA *, char *);
void		create_list			args( ( CHAR_DATA *ch, char *argument, int layout ) );

void		get_string_score_v1 	(CHAR_DATA *, CHAR_DATA *);
void		get_string_status_v1	(CHAR_DATA *, CHAR_DATA *);
void		get_string_reinc_v1 	(CHAR_DATA *, CHAR_DATA *);

char		get_string_score_txt [MAX_STRING_LENGTH];

int lookup_race( char *arg )
{
	int cnt;

	push_call("lookup_race(%p)",arg);

	for (cnt = 0 ; cnt < MAX_RACE ; cnt++)
	{
		if (!str_prefix(arg, race_table[cnt].race_name))
		{
			pop_call();
			return( cnt );
		}
	}
	pop_call();
	return( -1 );
}

int lookup_class( char *arg )
{
	int cnt;

	push_call("lookup_class(%p)",arg);

	for (cnt = 0 ; cnt < MAX_CLASS ; cnt++)
	{
		if (!str_prefix(arg, class_table[cnt].name))
		{
			pop_call();
			return cnt;
		}
	}
	pop_call();
	return -1;
}

int lookup_god( char *arg )
{
	push_call("lookup_god(%p)",arg);

	switch (arg[0])
	{
		case 'n':
			pop_call();
			return 0;
			break;
		case 'c':
			pop_call();
			return 1;
			break;
		case 'o':
			pop_call();
			return 2;
			break;
	}
	pop_call();
	return -1;
}

AREA_DATA *lookup_area( char *arg )
{
	AREA_DATA *pArea;

	push_call("lookup_area(%p)",arg);

	for (pArea = mud->f_area ; pArea ; pArea = pArea->next)
	{
		if (!str_prefix(arg, pArea->name))
		{
			pop_call();
			return pArea;
		}
	}
	pop_call();
	return NULL;
}

char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, int fShort )
{
	static char buf[MAX_STRING_LENGTH], tmp[MAX_STRING_LENGTH];

	push_call("format_obj_to_char(%p,%p,%p)",obj,ch,fShort);

	buf[0] = '\0';

	if (show_build_vnum(ch, obj->pIndexData->vnum))
	{
		sprintf(tmp, "(%d)", obj->pIndexData->vnum);
		sprintf(buf, "%7s ", tmp);
	}

	switch (fShort)
	{
		case 0:
			strcat(buf, capitalize(obj->short_descr));
			break;
		case 1:
			if (obj->long_descr[0] != '\0')
			{
				strcat(buf, obj->long_descr);
			}
			else
			{
				pop_call();
				return buf;
			}
			break;
		case 2:
			strcat(buf, obj->description);
			break;
	}

	strcat(buf, get_color_string(ch, COLOR_TEXT, VT102_BOLD));

	if (IS_OBJ_STAT(obj, ITEM_ETHEREAL))
	{
		strcat(buf, " (Ethereal)");
	}

	if (IS_OBJ_STAT(obj, ITEM_INVIS))
	{
		strcat(buf, " (Invis)");
	}

	if (IS_OBJ_STAT(obj, ITEM_GLOW))
	{
		strcat( buf, " (Glowing)");
	}

	if (IS_OBJ_STAT(obj, ITEM_HUM))
	{
		strcat( buf, " (Humming)");
	}

	if (IS_OBJ_STAT(obj, ITEM_BURNING))
	{
		strcat( buf, " (Burning)");
	}

	if (!IS_NPC(ch))
	{
		if (IS_OBJ_STAT(obj, ITEM_EVIL))
		{
			if (is_affected(ch, gsn_detect_evil))
			{
				strcat(buf, " (Red Aura)");
			}
		}
		if (IS_OBJ_STAT(obj, ITEM_BLESS))
		{
			if (is_affected(ch, gsn_detect_good))
			{
				strcat(buf, " (Gold Aura)");
			}
		}
		if (IS_OBJ_STAT(obj, ITEM_FORGERY))
		{
			if (number_percent() < learned(ch, gsn_detect_forgery))
			{
				check_improve(ch, gsn_detect_forgery);
				strcat( buf, " (Forgery)");
			}
		}
		if (obj->poison != NULL)
		{
			if (number_percent() < learned(ch, gsn_make_poison))
			{
				cat_sprintf(buf, " (Poison %c %c)", obj->poison->for_npc ? 'M' : 'C', obj->poison->poison_type);
			}
		}
	}
	pop_call();
	return buf;
}



/*
	Show a list to a character.
	Can coalesce duplicated items.
*/

void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, int fShort, bool fShowNothing )
{
	char       buf[MAX_STRING_LENGTH], dim[10], bld[10];
	char      *pstrShow;
	OBJ_DATA  *obj, *tobj;
	int        count;

	push_call("show_list_to_char(%p,%p,%p,%p)",list,ch,fShort,fShowNothing);

	if (ch == NULL || ch->desc == NULL)
	{
		pop_call();
		return;
	}

	strcpy(dim, get_color_string(ch, COLOR_OBJECTS, VT102_DIM ));
	strcpy(bld, get_color_string(ch, COLOR_OBJECTS, VT102_BOLD));

	buf[0] = 0;

	/*
		Format the list of objects.
	*/

	for (obj = list ; obj ; obj = obj->next_content)
	{
		if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj))
		{
			for (tobj = obj->prev_content ; tobj ; tobj = tobj->prev_content)
			{
				if (tobj->wear_loc == WEAR_NONE)
				{
					if (tobj->pIndexData->vnum == obj->pIndexData->vnum && !strcmp(tobj->short_descr, obj->short_descr))
					{
						if (can_see_obj(ch, tobj))
						{
							break;
						}
					}
				}
			}
			if (tobj)
			{
				continue;
			}
			pstrShow = format_obj_to_char(obj, ch, fShort);

			if (*pstrShow == '\0')
			{
				continue;
			}

			count = 1;

			for (tobj = obj->next_content ; tobj ; tobj = tobj->next_content)
			{
				if (tobj->wear_loc == WEAR_NONE)
				{
					if (tobj->pIndexData->vnum == obj->pIndexData->vnum && !strcmp(tobj->short_descr, obj->short_descr))
					{
					 	if (can_see_obj(ch, tobj))
					 	{
							count++;
						}
					}
				}
			}

			if (count != 1)
			{
				cat_sprintf(buf, "%s(%2d) %s%s\n\r", dim, count, bld, pstrShow);
			}
			else
			{
				cat_sprintf(buf, "%s     %s%s\n\r", dim, bld, pstrShow);
			}
		}
	}

	if (fShowNothing && buf[0] == 0)
	{
		send_to_char("Nothing.\n\r", ch);
	}

	if (buf[0] != 0)
	{
		send_to_char(buf, ch);
	}

	pop_call();
	return;
}

void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
	char buf[MAX_STRING_LENGTH];

	push_call("show_char_to_char_0(%p,%p)",victim,ch);

	if (!IS_NPC(victim) && victim->desc && !is_mounting(victim) && victim->long_descr[0] != '\0' && victim->position == POS_STANDING && !IS_AFFECTED(ch, AFF_TRUESIGHT))
	{
		ch_printf(ch, "%s\n\r", ansi_translate_def(ch, victim->long_descr, get_color_string(ch, COLOR_MOBILES, VT102_DIM)));
		pop_call();
		return;
	}
	else
	{
		buf[0] = '\0';

		switch (URANGE(0, 10 * victim->hit / UMAX(1, victim->max_hit), 10))
		{
			case  9: strcat(buf, "(Scratched) ");	break;
			case  8: strcat(buf, "(Bruised) ");	break;
			case  7: strcat(buf, "(Cut) ");		break;
			case  6: strcat(buf, "(Injured) ");	break;
			case  5: strcat(buf, "(Wounded) ");	break;
			case  4: strcat(buf, "(Oozing) ");		break;
			case  3: strcat(buf, "(Leaking) ");	break;
			case  2: strcat(buf, "(Bleeding) ");	break;
			case  1: strcat(buf, "(Gushing) ");	break;
			case  0: strcat(buf, "(Dying) ");		break;
		}

		if (!IS_NPC(victim))
		{
			if (IS_SET(victim->act, PLR_WIZINVIS))
			{
				strcat(buf, "(WIZINVIS) ");
			}
			if (IS_SET(victim->act, PLR_WIZCLOAK))
			{
				strcat(buf, "(WIZCLOAK) ");
			}
			if (!victim->desc)
			{
				strcat(buf, "(LINK DEAD) ");
			}
			else if (victim->desc->connected == CON_EDITING)
			{
				strcat(buf, "(WRITING) ");
			}
			if (IS_SET(victim->act, PLR_AFK))
			{
				strcat(buf, "(AFK) ");
			}
			if (IS_SET(victim->act, PLR_KILLER))
			{
				strcat(buf, "(KILLER) ");
			}
			if (IS_SET(victim->act, PLR_THIEF))
			{
				strcat( buf, "(THIEF) ");
			}
			if (IS_SET(victim->act, PLR_OUTCAST))
			{
				strcat(buf, "(OUTCAST) ");
			}
		}
		if (IS_NPC(victim) && show_build_vnum(ch, victim->pIndexData->vnum))
		{
			cat_sprintf(buf, "(%d) ", victim->pIndexData->vnum);
		}
		if (IS_AFFECTED(victim, AFF2_MIRROR_IMAGE))
		{
			strcat(buf, "(Multiple) ");
		}
		if (IS_AFFECTED(victim, AFF_INVISIBLE) || IS_AFFECTED(victim, AFF_IMP_INVISIBLE))
		{
			strcat(buf, "(Invis) ");
		}
		if (IS_AFFECTED(victim, AFF_HIDE))
		{
			strcat(buf, "(Hide) ");
		}
		if (IS_AFFECTED(victim, AFF_STEALTH))
		{
			strcat(buf, "(Stealth) ");
		}
		if (IS_AFFECTED(victim, AFF_CHARM) && ch == victim->master)
		{
			strcat(buf, "(Charmed) ");
		}
		if (IS_AFFECTED(victim, AFF_PASS_DOOR) || rspec_req(victim, RSPEC_PASS_DOOR))
		{
			strcat(buf, "(Translucent) ");
		}
		if (IS_AFFECTED(victim, AFF2_FIRESHIELD))
		{
			strcat(buf, "(Burning) ");
		}
		if (IS_EVIL(victim) && is_affected(ch, gsn_detect_evil))
		{
			strcat(buf, "(Red Aura) ");
		}
		if (IS_GOOD(victim) && is_affected(ch, gsn_detect_good))
		{
			strcat(buf, "(Gold Aura) ");
		}
		if (IS_AFFECTED(victim, gsn_mage_shield))
		{
			strcat(buf, "(Shimmering) ");
		}
		if (IS_AFFECTED(victim, AFF_SANCTUARY))
		{
			strcat(buf, "(White Aura) ");
		}
		if (is_affected(victim, gsn_mana_shield))
		{
			strcat(buf, "(Blue Aura) ");
		}
		if (is_affected(victim, gsn_black_aura))
		{
			strcat(buf, "(Black Aura) ");
		}
		if (is_affected(victim, gsn_magic_mirror))
		{
			strcat(buf, "(Sparkling) ");
		}
		if (CAN_FLY(victim))
		{
			strcat(buf, "(Flying) ");
		}
		if (is_affected(victim, gsn_faerie_fire))
		{
			strcat(buf, "(Pink Aura) ");
		}
		if (is_affected(victim, gsn_mage_shield))
		{
			strcat(buf, "(Shimmering) ");
		}
		if (IS_NPC(victim))
		{
			if (victim->desc && !IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT))
			{
				strcat(buf, "(Possessed) ");
			}
			if (rspec_req(ch, RSPEC_SEE_AGGR) && IS_SET(victim->act, ACT_AGGRESSIVE))
			{
				strcat(buf, "(Aggressive) ");
			}
			if (IS_AFFECTED(victim, AFF_ETHEREAL))
			{
				strcat(buf, "(ETHEREAL) ");
			}

			if (victim->position == POS_STANDING && !is_mounting(victim) && !victim->furniture && victim->long_descr[0] != '\0')
			{
				strcat(buf, victim->long_descr);
				ch_printf(ch, "%s%s\n\r", get_color_string(ch, COLOR_MOBILES, VT102_DIM), justify(buf, get_page_width(ch)));
				pop_call();
				return;
			}
		}
		strcat(buf, capitalize(get_name(victim)));

		switch (victim->position)
		{
			case POS_DEAD:
				strcat(buf, " is DEAD!!");
				break;
			case POS_MORTAL:
				strcat(buf, " is mortally wounded.");
				break;
			case POS_INCAP:
				strcat(buf, " is incapacitated.");
				break;
			case POS_STUNNED:
				strcat(buf, " is lying here stunned.");
				break;
			case POS_SLEEPING:
				if (victim->furniture)
				{
					if (IS_SET(victim->furniture->value[2], SLEEP_ON))
					{
						cat_sprintf(buf, " is sleeping on %s here.", victim->furniture->short_descr);
					}
					else
					{
						cat_sprintf(buf, " is sleeping in %s here.", victim->furniture->short_descr);
					}
				}
				else
				{
					strcat(buf, " is sleeping here.");
				}
				break;
			case POS_RESTING:
				if (victim->furniture)
				{
					if (IS_SET(victim->furniture->value[2], REST_ON))
					{
						cat_sprintf(buf, " is resting on %s here.", victim->furniture->short_descr);
					}
					else
					{
						cat_sprintf(buf, " is resting in %s here.", victim->furniture->short_descr);
					}
				}
				else
				{
					strcat(buf, " is resting here.");
				}
				break;
			case POS_SITTING:
				if (victim->furniture)
				{
					if (IS_SET(victim->furniture->value[2], SIT_ON))
					{
						cat_sprintf(buf, " is sitting on %s here.", victim->furniture->short_descr);
					}
					else
					{
						cat_sprintf(buf, " is sitting in %s here.", victim->furniture->short_descr);
					}
				}
				else
				{
					strcat(buf, " is sitting here.");
				}
				break;
			case POS_FIGHTING:
				if (is_mounting(victim))
				{
					cat_sprintf(buf, " sits mounted on %s here, fighting ", get_name(victim->mounting));
				}
				else
				{
					strcat(buf, " is here, fighting ");
				}
				if (who_fighting(victim) == NULL)
				{
					strcat(buf, "thin air??");
				}
				else if (who_fighting(victim) == ch)
				{
					strcat(buf, "YOU!" );
				}
				else if (victim->in_room == who_fighting(victim)->in_room)
				{
					cat_sprintf(buf, "%s.", PERS(victim->fighting->who, ch));
				}
				else
				{
					strcat(buf, "somone who left??");
				}
				break;
			default:
				if (victim->furniture)
				{
					if (IS_SET(victim->furniture->value[2], STAND_ON))
					{
						cat_sprintf(buf, " is standing on %s here.", victim->furniture->short_descr);
					}
					else
					{
						cat_sprintf(buf, " is standing in %s here.", victim->furniture->short_descr);
					}
				}
				else if (is_mounting(victim))
				{
					cat_sprintf(buf, " sits mounted on %s here.", get_name(victim->mounting));
				}
				else
				{
					strcat(buf, " is here.");
				}
				break;
		}
		ch_printf(ch, "%s%s\n\r", get_color_string(ch, COLOR_MOBILES, VT102_DIM), justify(buf, get_page_width(ch)));
	}
	pop_call();
	return;
}

void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
	char buf[MAX_STRING_LENGTH], buf2[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	CHAR_DATA *fch;
	int iWear;

	push_call("show_char_to_char_1(%p,%p)",victim,ch);

	if (ch != NULL)
	{
		if (!IS_NPC(victim))
		{
			strcpy(buf, capitalize(get_char_description(victim)));

			ch_printf_color(ch, "{300}%s\n\r", justify(get_dynamic_player_description(victim, NULL, buf), 80));
		}
		else if (!mprog_desc_trigger(victim, ch, ""))
		{
			if (victim->description[0] != '\0')
			{
				ch_printf_color(ch, "{300}%s", victim->description);
			}
			else
			{
				act( "{300}You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
			}
		}
	}

	if (ch != NULL)
	{
		strcpy(buf, capitalize(PERS(victim, ch)));
	}
	else
	{
		*buf='\0';
	}

	switch (URANGE(0, 10 * victim->hit / UMAX(1, victim->max_hit), 10))
	{
		case 10: strcat(buf, " is in perfect health");	break;
		case  9: strcat(buf, " is slightly scratched");	break;
		case  8: strcat(buf, " has a few bruises");		break;
		case  7: strcat(buf, " has some cuts");			break;
		case  6: strcat(buf, " has several wounds");		break;
		case  5: strcat(buf, " has many nasty wounds");	break;
		case  4: strcat(buf, " is bleeding freely");		break;
		case  3: strcat(buf, " is covered in blood");	break;
		case  2: strcat(buf, " is leaking guts");		break;
		case  1: strcat(buf, " is almost dead");		break;
		case  0: strcat(buf, " is DYING");				break;
	}

	buf[0] = UPPER(buf[0]);

	if (ch != NULL)
	{
		ch_printf(ch, "%s.\n\r", buf);
	}
	else
	{
		sprintf(buf2, "    %s%s\n\r", capitalize(get_name(victim)), buf);

		for (fch = victim->in_room->first_person ; fch ; fch = fch->next_in_room)
		{
			if (fch->desc && victim != fch)
			{
				if (fch->position > POS_SLEEPING && can_see(fch, victim) && !IS_SET(CH(fch->desc)->act, PLR_DAMAGE))
				{
					if (is_same_group(fch, victim))
					{
						if (!IS_SET(CH(fch->desc)->pcdata->spam, 16))
						{
							ch_printf(fch, "%s%s", get_color_string(fch, COLOR_PARTY_HIT, VT102_DIM), buf2);
						}
					}
					else if (who_fighting(victim) && is_same_group(fch, victim->fighting->who))
					{
						if (!IS_SET(CH(fch->desc)->pcdata->spam, 64) && !IS_SET(CH(fch->desc)->pcdata->spam, 1))
						{
							ch_printf(fch, "%s%s", get_color_string(fch, COLOR_YOU_HIT, VT102_DIM), buf2);
						}
					}
					else if (!IS_SET(CH(fch->desc)->pcdata->spam, 256))
					{
						ch_printf(fch, "%s", buf2);
					}
				}
			}
		}
		pop_call();
		return;
	}

	*buf = '\0';

	for (iWear = 0 ; iWear < MAX_WEAR ; iWear++)
	{
		if ((obj = get_eq_char(victim, iWear)) != NULL && can_see_obj(ch, obj))
		{
			if (strlen(buf) == 0)
			{
				sprintf(buf, "%s is using:\n\r", capitalize(get_name(victim)));
			}
			cat_sprintf(buf, "{078}%s{178}%s\n\r", where_name[iWear], format_obj_to_char(obj, ch, 0));
		}
	}

	if (strlen(buf) == 0)
	{
		sprintf(buf, "%s is using:   Nothing!\n\r", capitalize(get_name(victim)));
	}

	send_to_char_color( buf, ch );

	pop_call();
	return;
}

void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
	CHAR_DATA *rch;

	push_call("show_char_to_char(%p,%p)",list,ch);

	for (rch = list ; rch != NULL ; rch = rch->next_in_room)
	{
		if (rch == ch || !can_see(ch, rch))
		{
			continue;
		}

		if (!can_see_in_room(ch, ch->in_room))
		{
			if (IS_SET(ch->in_room->room_flags, ROOM_SMOKE) && !can_see_smoke(ch))
			{
				continue;
			}
			send_to_char("You see glowing red eyes watching YOU!\n\r", ch );
		}
		else
		{
			if (IS_SET(ch->in_room->room_flags, ROOM_SMOKE) && !can_see_smoke(ch) && !can_see_infra(ch, rch))
			{
				send_to_char("The smoke obscures the outline of a large shape.\n\r", ch);
			}
			else
			{
				show_char_to_char_0(rch, ch);
			}
		}
	}
	pop_call();
	return;
}

bool check_blind( CHAR_DATA *ch )
{
	push_call("check_blind(%p)",ch);

	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT))
	{
		pop_call();
		return TRUE;
	}

	if (IS_AFFECTED(ch, AFF_BLIND))
	{
		send_to_char( "You can't see a thing!\n\r", ch );
		pop_call();
		return FALSE;
	}
	pop_call();
	return TRUE;
}

void show_char_room(CHAR_DATA *ch, ROOM_INDEX_DATA *rm, bool dir, bool range, int vision, int light, int sight)
{
	CHAR_DATA *rch;

	push_call("show_char_room(%p,%p,%p,%p,%p,%p)",ch,rm,dir,vision,light,sight);

	sight -= sector_table[rm->sector_type].sight;

	if (sight < 0)
	{
		pop_call();
		return;
	}

	light = UMIN(light, light + get_room_light(ch, rm, TRUE));

	if (IS_SET(rm->room_flags, ROOM_SOLITARY|ROOM_PRIVATE))
	{
		ch_printf(ch, "%d> A stretch of nothing...\n\r", range);
	}
	else if (vision + light < 0 || !can_see_in_room(ch, rm))
	{
		ch_printf(ch, "%d> A length of darkness...\n\r", range);

		pop_call();
		return;
	}
	else
	{
		ch_printf(ch, "%d> %s\n\r", range, rm->name);

		for (rch = rm->first_person ; rch ; rch = rch->next_in_room)
		{
			if (!can_see(ch, rch) || IS_AFFECTED(rch, AFF_HIDE) || IS_AFFECTED(rch, AFF_STEALTH))
			{
				continue;
			}
			ch_printf(ch, "%s    %s\n\r", get_color_string(ch, COLOR_MOBILES, VT102_BOLD), capitalize(get_name(rch)));

			sight -= race_table[rch->race].height;
		}
	}

	if (!is_valid_exit(ch, rm, dir))
	{
		pop_call();
		return;
	}

	if (!can_use_exit(ch, rm->exit[dir]))
	{
		pop_call();
		return;
	}
	show_char_room(ch, room_index[rm->exit[dir]->to_room], dir, range + 1, vision, light, sight);

	pop_call();
	return;
}


void show_char_dir(CHAR_DATA *ch, bool dir)
{
	EXIT_DATA *pexit;
	int vision, light, sight;

	push_call("show_char_dir(%p,%p)",ch,dir);

	if ((pexit = get_exit(ch->in_room->vnum, dir)) == NULL)
	{
		send_to_char("Nothing special there.\n\r", ch);
		pop_call();
		return;
	}

	if (!is_valid_exit(ch, ch->in_room, dir))
	{
		send_to_char( "Nothing special there.\n\r", ch );
		pop_call();
		return;
	}

	if (pexit->description[0] != '\0')
	{
		ch_printf(ch, "%s\n\r", pexit->description);
	}
	else
	{
		send_to_char("Nothing special there.\n\r", ch);
	}

	if (pexit->keyword[0] != '\0' && !IS_SET(pexit->flags, EX_UNBARRED))
	{
		if (IS_SET(pexit->flags, EX_CLOSED))
		{
			act( "The $d is closed.",					ch, NULL, pexit->keyword, TO_CHAR);
		}
		else if (IS_SET(pexit->flags, EX_BASHED))
		{
			act( "The $d has been bashed from its hinges.",	ch, NULL, pexit->keyword, TO_CHAR);
		}
		else if (IS_SET(pexit->flags, EX_ISDOOR))
		{
			act( "The $d is open.",						ch, NULL, pexit->keyword, TO_CHAR );
		}
	}

	if (!can_use_exit(ch, pexit))
	{
		pop_call();
		return;
	}

	vision = get_vision(ch);
	light  = get_room_light(ch, ch->in_room, TRUE);
	sight  = get_sight(ch);

	show_char_room(ch, room_index[pexit->to_room], dir, 1, vision, light, sight);

	pop_call();
	return;
}

CHAR_DATA *find_char_room(CHAR_DATA *ch, ROOM_INDEX_DATA *rm, bool dir, char *name, int vision, int light, int sight)
{
	CHAR_DATA *rch;
	ROOM_INDEX_DATA *old_room;

	push_call("find_char_room(%p,%p,%p,%p,%p,%p)",ch,rm,dir,vision,light,sight);

	sight -= sector_table[rm->sector_type].sight;

	if (sight < 0)
	{
		pop_call();
		return NULL;
	}

	light = UMIN(light, light + get_room_light(ch, rm, TRUE));

	if (vision + light < 0 || !can_see_in_room(ch, rm))
	{
		pop_call();
		return NULL;
	}

	old_room    = ch->in_room;
	ch->in_room = rm;
	rch         = get_char_room(ch, name);
	ch->in_room = old_room;

	if (rch && rch != ch)
	{
		pop_call();
		return rch;
	}

	if (!is_valid_exit(ch, rm, dir))
	{
		pop_call();
		return NULL;
	}

	if (!can_use_exit(ch, rm->exit[dir]))
	{
		pop_call();
		return NULL;
	}
	pop_call();
	return find_char_room(ch, room_index[rm->exit[dir]->to_room], dir, name, vision, light, sight);
}


CHAR_DATA *find_char_dir(CHAR_DATA *ch, bool dir, char *name)
{
	EXIT_DATA *pexit;
	CHAR_DATA *victim;
	int vision, light, sight;

	push_call("show_char_dir(%p,%p)",ch,dir);

	if ((pexit = get_exit(ch->in_room->vnum, dir)) == NULL)
	{
		pop_call();
		return NULL;
	}

	if (!is_valid_exit(ch, ch->in_room, dir))
	{
		pop_call();
		return NULL;
	}

	if (!can_use_exit(ch, pexit))
	{
		pop_call();
		return NULL;
	}

	vision = get_vision(ch);
	light  = get_room_light(ch, ch->in_room, TRUE);
	sight  = get_sight(ch);

	victim = find_char_room(ch, room_index[pexit->to_room], dir, name, vision, light, sight);

	pop_call();
	return victim;
}


void delete_player(CHAR_DATA *ch)
{
	char buf1[100], buf2[100], name_buf[100];
	PLAYER_GAME *gpl;
	DESCRIPTOR_DATA *d;

	push_call("delete_player(%p)",ch);

	one_argument(ch->name, name_buf);
	strcpy( name_buf, capitalize(name_buf) );

	if (ch->pcdata->clan != NULL)
	{
		if (!strcasecmp(ch->name, ch->pcdata->clan->leader[0]))
		{
			destroy_clan(ch->pcdata->clan);
		}
		else
		{
			--ch->pcdata->clan->members;

			if (is_clan_leader(ch))
			{
				RESTRING(ch->pcdata->clan->leader[ch->pcdata->clan_position], "");
			}
			RESTRING(ch->pcdata->clan_name, "");
			ch->pcdata->clan = NULL;
		}
	}

	if (ch->pcdata->castle)
	{
		del_castle(ch);
	}

	del_data(ch);

	sprintf(buf1, "%s/%c/%c/%s", PLAYER_DIR, tolower(ch->name[0]), tolower(ch->name[1]), name_buf);
	sprintf(buf2, "%s/%c/%c/del/%s", PLAYER_DIR, tolower(ch->name[0]), tolower(ch->name[1]), name_buf);

	remove(buf2);
	rename(buf1, buf2);

	if (ch->desc)
	{
		if (ch->desc->descriptor == -999)
		{
			clear_partial_load(ch);
			pop_call();
			return;
		}

		if (ch->desc->descriptor == -998)
		{
			extract_char(ch);
			pop_call();
			return;
		}

		if (ch->desc->connected < CON_PLAYING)
		{
			extract_char(ch);
			pop_call();
			return;
		}
	}

	if (ch->master != NULL)
	{
		stop_follower (ch);
	}

	do_return(ch, NULL);
	stop_fighting(ch, TRUE);

	for (gpl = mud->f_player ; gpl ; gpl = gpl->next)
	{
		if (gpl->ch->pcdata->reply == ch)
		{
			gpl->ch->pcdata->reply = NULL;
		}
	}

	check_bad_desc(ch);

	if (ch->desc && ch->desc->snoop_by != NULL)
	{
		write_to_buffer(ch->desc->snoop_by, "Your victim has deleted.\n\r", 0);
		ch->desc->snoop_by = NULL;
	}

	vt100off(ch);

	d = ch->desc;

	if (d)
	{
		force_help (d, "goodbye");
		write_to_port (d);
		*d->inbuf = '\0';
	}
	extract_char(ch);

	if (d != NULL)
	{
		d->character = NULL;
		close_socket(d);
	}
	pop_call();
	return;
}

void do_delete( CHAR_DATA *ch, char *arg)
{
	bool PassRqd;
	CHAR_DATA *rch;

	push_call("do_delete(%p,%p)",ch,arg);

	if (IS_NPC(ch) || ch->position == POS_FIGHTING || ch->fighting != NULL)
	{
		send_to_char( "You may not do that now.\n\r", ch);
		pop_call();
		return;
	}

	if (arg == NULL)
	{
		PassRqd = FALSE;
	}
	else
	{
		PassRqd = TRUE;
	}

	if (PassRqd && arg[0] == '\0')
	{
		send_to_char( "You must use a password with this command.\n\rThis command permanently deletes your character!!!!\n\rWarning!   Warning!\n\r", ch);
		pop_call();
		return;
	}

	if (!PassRqd || encrypt64(arg) == ch->pcdata->password)
	{
		if (PassRqd)
		{
			send_to_char( "You have permanently deleted your character!\n\r", ch);
		}
		else
		{
			send_to_char( "Your character has been permanently deleted!\n\r", ch);
			send_to_char( "You have a serious bug that could not be corrected.\n\r",ch);
		}
		save_char_obj(ch, NORMAL_SAVE);

		for (rch = ch->in_room->first_person; rch != NULL; rch = rch->next_in_room)
		{
			if (can_see (rch, ch) && rch != ch && rch->position > POS_SLEEPING)
			{
				ch_printf (rch, "%s has left the game.\n\r", get_name (ch));
			}
		}
		log_printf ("%s has deleted.", get_name(ch));

		delete_player(ch);

		pop_call();
		return;
	}
	else
	{
		send_to_char( "That was not your password.\n\rThis command permanently deletes your character!!!!\n\rWarning!   Warning!\n\r", ch);
	}
	pop_call();
	return;
}

void do_notice( CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *list;
	char buf[MAX_STRING_LENGTH];
	int old_affected_by;

	push_call("do_notice(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (number_percent() < learned(ch, gsn_notice))
	{
		send_to_char("You search the room for creatures.\n\r", ch);

		old_affected_by = ch->affected_by;

		for (list = ch->in_room->first_person ; list != NULL ; list = list->next_in_room)
		{
			SET_BIT(ch->affected_by, AFF_DETECT_INVIS);
			SET_BIT(ch->affected_by, AFF_DETECT_HIDDEN);

			if (ch != list && can_see(ch, list))
			{
				ch->affected_by = old_affected_by;
				SET_BIT(ch->affected_by, AFF_DETECT_HIDDEN);

				if (can_see(ch, list))
				{
					strcpy(buf, capitalize(get_name(list)));
				}
				else
				{
					strcpy(buf, "Someone");
				}
				strcat(buf, " is here.\n\r");
				send_to_char(buf, ch);
			}
		}
		check_improve(ch, gsn_notice);
		ch->affected_by = old_affected_by;
	}
	pop_call();
	return;
}

void do_peek( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char buf1 [MAX_STRING_LENGTH];

	push_call("do_peek(%p,%p)",ch,argument);

	if (ch->desc == NULL)
	{
		pop_call();
		return;
	}

	if (!check_blind(ch))
	{
		pop_call();
		return;
	}

	if (*argument == '\0')
	{
		send_to_char("Peek at whom?\n\r", ch);
		pop_call();
		return;
	}

	if ((victim = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		pop_call();
		return;
	}

	if (can_see(victim, ch) && number_percent() > learned(ch, gsn_glance))
	{
		act( "$n peeks at your belongings.", ch, NULL, victim, TO_VICT   );
		act( "$n peeks at $N's belongings.", ch, NULL, victim, TO_NOTVICT);
		if (learned(ch, gsn_glance) > 0)
		{
			act( "You notice $N notices that you're peeking at $S belongings.", ch, NULL, victim, TO_CHAR);
		}
		else
		{
			act( "You peek at $N's belongings.", ch, NULL, victim, TO_CHAR);
		}
	}
	else if (learned(ch, gsn_glance) > 0)
	{
		act( "You peek at $N's belongings without $M noticing.", ch, NULL, victim, TO_CHAR);
	}
	else
	{
		act( "You peek at $N's belongings.", ch, NULL, victim, TO_CHAR);
	}

	strcpy(buf1, capitalize(get_name(victim)));

	switch(URANGE(0, 10 * victim->hit / UMAX(1, victim->max_hit), 10))
	{
		case 10: strcat(buf1, " is in perfect health");	break;
		case  9: strcat(buf1, " is slightly scratched");	break;
		case  8: strcat(buf1, " has a few bruises");		break;
		case  7: strcat(buf1, " has some cuts");		break;
		case  6: strcat(buf1, " has several wounds");	break;
		case  5: strcat(buf1, " has many nasty wounds");	break;
		case  4: strcat(buf1, " is bleeding freely");	break;
		case  3: strcat(buf1, " is covered in blood");	break;
		case  2: strcat(buf1, " is leaking guts");		break;
		case  1: strcat(buf1, " is almost dead");		break;
		case  0: strcat(buf1, " is DYING");			break;
	}
	if (number_percent() < learned(ch, gsn_greater_peek))
	{
		ch_printf(ch, "%s%s%d%s\n\r", buf1, " and has about ", number_fuzzy(UMAX(1, victim->hit)), " hitpoints left.");
		check_improve(ch, gsn_greater_peek);
	}
	else
	{
		ch_printf(ch, "%s.\n\r", buf1);
	}

	if (number_percent() < learned(ch, gsn_peek))
	{
		ch_printf(ch, "%s is carrying:\n\r", capitalize(get_name(victim)));
		show_list_to_char(victim->first_carrying, ch, 0, TRUE);
	}
	check_improve(ch, gsn_peek);
	pop_call();
	return;
}

void do_look( CHAR_DATA *ch, char *argument )
{
	char buf  [MAX_STRING_LENGTH];
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	char *pdesc;
	int door, cnt, trails;

	push_call("do_look(%p,%p)",ch,argument);

	if (ch->desc == NULL)
	{
		pop_call();
		return;
	}

	if (ch->position < POS_SLEEPING)
	{
		send_to_char( "You can't see anything but stars!\n\r", ch );
		pop_call();
		return;
	}

	if (ch->position == POS_SLEEPING)
	{
		send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
		pop_call();
		return;
	}

	if (!check_blind(ch))
	{
		pop_call();
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if (arg1[0] == '\0' || !strcasecmp( arg1, "auto" ) )
	{
		buf[0] = '\0';

		if (ch->level == 1 && ch->in_room->vnum == ROOM_VNUM_SCHOOL)
		{
			do_help(ch, "newchar");
		}

		if (show_build_vnum(ch, ch->in_room->vnum))
		{
			cat_sprintf(buf, "%s(%d) ", get_color_string(ch, COLOR_PROMPT, VT102_DIM), ch->in_room->vnum);
		}
		cat_sprintf(buf, "%s%s\n\r", get_color_string(ch, COLOR_PROMPT,VT102_BOLD), ch->in_room->name );

		send_to_char(buf, ch);

		if (ch->desc && IS_SET(CH(ch->desc)->act, PLR_AUTOEXIT) && IS_SET(CH(ch->desc)->act, PLR_EXITPOS))
		{
			do_exits( ch, "auto" );
		}

		if (arg1[0] == '\0' || (!IS_NPC(ch) && !IS_SET(ch->act, PLR_BRIEF)))
		{
			if (!can_see_in_room(ch, ch->in_room))
			{
				send_to_char( "It is pitch black ... \n\r", ch );
			}
			else if (IS_SET(ch->in_room->room_flags, ROOM_DYNAMIC))
			{
				ch_printf_color(ch, "{300}%s", justify(get_dynamic_description(ch), 80));
			}
			else
			{
				ch_printf_color(ch, "{300}%s", ch->in_room->description);
			}
		}

		if (ch->desc && IS_SET(CH(ch->desc)->act, PLR_AUTOEXIT) && !IS_SET(CH(ch->desc)->act, PLR_EXITPOS))
		{
			do_exits( ch, "auto" );
		}

		if (IS_SET(ch->in_room->room_flags, ROOM_GLOBE))
		{
			send_to_char( "A cloud of impenetrable darkness enshrouds your surroundings.\n\r", ch);
		}

		if (can_see_in_room(ch, ch->in_room))
		{
			if (IS_SET(ch->in_room->room_flags, ROOM_SMOKE))
			{
				send_to_char( "A cloud of dark purple smoke partially obscures your vision.\n\r", ch);
			}

			if (IS_SET(ch->in_room->room_flags, ROOM_ICE))
			{
				send_to_char( "Sheets of ice covering every surface chill the air around you.\n\r", ch);
			}

			for (trails = cnt = 0 ; cnt < MAX_LAST_LEFT ; cnt++)
			{
				if (IS_SET(ch->in_room->last_left_bits[cnt], TRACK_BLOOD))
				{
					trails++;
				}
			}

			if (trails && trails > 2)
			{
				ch_printf_color(ch, "{118}     Trails of blood lead in multiple directions.\n\r");
			}
			else if (trails)
			{
				for (cnt = 0 ; cnt < MAX_LAST_LEFT ; cnt++)
				{
					if (IS_SET(ch->in_room->last_left_bits[cnt], TRACK_BLOOD))
					{
						ch_printf_color(ch, "{118}     A trail of blood leads %s.\n\r", dir_name[UNSHIFT(ch->in_room->last_left_bits[cnt])]);
					}
				}
			}
		}
		show_list_to_char( ch->in_room->first_content, ch, 1, FALSE );
		show_char_to_char( ch->in_room->first_person,   ch );

		pop_call();
		return;
	}

	if (!strcasecmp(arg1, "i") || !strcasecmp(arg1, "in"))
	{
		if (arg2[0] == '\0')
		{
			send_to_char( "Look in what?\n\r", ch );
			pop_call();
			return;
		}

		if ((obj = get_obj_here(ch, arg2)) == NULL)
		{
			send_to_char( "You do not see that here.\n\r", ch );
			pop_call();
			return;
		}

		switch (obj->item_type)
		{
			default:
				send_to_char( "That is not a container.\n\r", ch );
				break;

			case ITEM_DRINK_CON:
				if (obj->value[1] <= 0)
				{
					send_to_char( "It is empty.\n\r", ch );
					break;
				}
				sprintf(buf, "It's %s half full of a %s liquid.\n\r",
					obj->value[1] <     obj->value[0] / 4 ? "less than" :
					obj->value[1] < 3 * obj->value[0] / 4 ? "about"     : "more than",
					liq_table[obj->value[2]].liq_color	);
				send_to_char( buf, ch );
				break;

			case ITEM_CONTAINER:
			case ITEM_CORPSE_NPC:
			case ITEM_CORPSE_PC:
				if (IS_SET(obj->value[1], CONT_CLOSED))
				{
					send_to_char( "It is closed.\n\r", ch );
					break;
				}
				act( "$p contains:", ch, obj, NULL, TO_CHAR );
				show_list_to_char( obj->first_content, ch, 0, TRUE );
				break;
		}
		pop_call();
		return;
	}

	if ((victim = get_char_room(ch, arg1)) != NULL)
	{
		show_char_to_char_1( victim, ch );
		if (can_see(victim, ch))
		{
			act( "$n looks at you.", ch, NULL, victim, TO_VICT    );
			act( "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
		}
		pop_call();
		return;
	}

	for (obj = ch->first_carrying ; obj ; obj = obj->next_content)
	{
		if (can_see_obj(ch, obj))
		{
			if ((pdesc = get_extra_descr(arg1, obj->pIndexData->first_extradesc)) != NULL)
			{
				ch_printf_color(ch, "{300}%s", pdesc);
				pop_call();
				return;
			}
		}
	}

	for (obj = ch->in_room->first_content ; obj ; obj = obj->next_content)
	{
		if (can_see_obj(ch, obj))
		{
			if ((pdesc = get_extra_descr(arg1, obj->pIndexData->first_extradesc)) != NULL)
			{
				ch_printf_color(ch, "{300}%s", pdesc);
				pop_call();
				return;
			}
		}
	}

	for (victim = ch->in_room->first_person ; victim ; victim = victim->next_in_room)
	{
		if (IS_NPC(victim) && can_see(ch, victim))
		{
			if ((pdesc = get_extra_descr(arg1, victim->pIndexData->first_extradesc)) != NULL)
			{
				ch_printf_color(ch, "{300}%s", pdesc);
				pop_call();
				return;
			}
		}
	}

	if ((pdesc = get_extra_descr(arg1, ch->in_room->first_extradesc)) != NULL)
	{
		ch_printf_color(ch, "{300}%s", pdesc);
		pop_call();
		return;
	}

	if ((obj = get_obj_here(ch, arg1)) != NULL)
	{
		if (obj->description[0] == '\0')
		{
			ch_printf(ch, "You see nothing special about %s.\n\r", obj->short_descr );
		}
		else
		{
			ch_printf_color(ch, "{300}%s", obj->description);
		}
		if (!IS_NPC(ch) && obj->poison && learned(ch, gsn_make_poison))
		{
			ch_printf(ch, "There is a %s %s poison on this object.\n\r",
				obj->poison->instant_damage_high == 0 ? "blue" : obj->poison->constant_damage_high == 0 ? "purple" : "green",
				obj->poison->for_npc ? "murky" : "clear");
		}
		pop_call();
		return;
	}

	if (!strcasecmp(arg1, "n") || !strcasecmp(arg1, "north"))
	{
		door = 0;
	}
	else if (!strcasecmp(arg1, "e") || !strcasecmp(arg1, "east"))
	{
		door = 1;
	}
	else if (!strcasecmp(arg1, "s") || !strcasecmp(arg1, "south"))
	{
		door = 2;
	}
	else if (!strcasecmp(arg1, "w") || !strcasecmp(arg1, "west"))
	{
		door = 3;
	}
	else if (!strcasecmp(arg1, "u") || !strcasecmp(arg1, "up"))
	{
		door = 4;
	}
	else if (!strcasecmp(arg1, "d") || !strcasecmp(arg1, "down"))
	{
		door = 5;
	}
	else
	{
		send_to_char( "You do not see that here.\n\r", ch);
		pop_call();
		return;
	}

	/*
		look direction
	*/


	show_char_dir(ch, door);

	pop_call();
	return;
}

void do_scan( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH],*c,buf[MAX_INPUT_LENGTH];
	char bufc[20];

	push_call("do_scan(%p,%p)",ch,argument);

	if (!check_blind(ch))
	{
		pop_call();
		return;
	}

	one_argument(argument, arg);

	if (arg[0] == '\0')
	{
		strcpy(arg, "neswud");
	}
	else
	{
		arg[6] = '\0';
	}

	strcpy(bufc, get_color_string(ch, COLOR_PROMPT, VT102_BOLD));

	for (c = arg ; *c != '\0' ; c++)
	{
		buf[0] = '\0';

		switch(*c)
		{
			case 'n':
				if (is_valid_exit(ch, ch->in_room, 0))
				{
					sprintf(buf, "%sTo the north:\n\r", bufc);
				}
				break;
			case 'e':
				if (is_valid_exit(ch, ch->in_room, 1))
				{
					sprintf(buf, "%sTo the east:\n\r", bufc);
				}
				break;
			case 's':
				if (is_valid_exit(ch, ch->in_room, 2))
				{
					sprintf(buf, "%sTo the south:\n\r", bufc);
				}
				break;
			case 'w':
				if (is_valid_exit(ch, ch->in_room, 3))
				{
					sprintf(buf, "%sTo the west:\n\r", bufc);
				}
				break;
			case 'u':
				if (is_valid_exit(ch, ch->in_room, 4))
				{
					sprintf(buf, "%sUp:\n\r", bufc);
				}
				break;
			case 'd':
				if (is_valid_exit(ch, ch->in_room, 5))
				{
					sprintf(buf, "%sDown:\n\r", bufc);
				}
				break;
			default:
				ch_printf(ch, "There is no direction abbreviated by '%c'.\n\r", *c);
				break;
		}
		if (buf[0] == '\0')
		{
			continue;
		}
		send_to_char(buf, ch);
		sprintf(buf, "%c", *c);
		do_look(ch, buf);
	}
	pop_call();
	return;
}


void do_examine( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;

	push_call("do_examine(%p,%p)",ch,argument);

	if (ch->desc == NULL)
	{
		pop_call();
		return;
	}

	one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		send_to_char( "Examine what?\n\r", ch );
		pop_call();
		return;
	}

	do_look(ch, arg);

	if ((obj = get_obj_here(ch, arg)) != NULL)
	{
		switch (obj->item_type)
		{
			default:
				break;
			case ITEM_DRINK_CON:
			case ITEM_CONTAINER:
			case ITEM_CORPSE_NPC:
			case ITEM_CORPSE_PC:
				sprintf(buf, "in %s", arg);
				do_look( ch, buf );
		}
	}
	pop_call();
	return;
}

/*
	Thanks to Zrin for auto-exit part.
*/

void do_exits( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
	EXIT_DATA *pexit;
	bool fAuto;
	int door, num_exits = 0;
	char dim[10], bold[10];

	push_call("do_exits(%p,%p)",ch,argument);

	fAuto  = !strcasecmp(argument, "auto");

	if (!check_blind(ch))
	{
		pop_call();
		return;
	}

	strcpy(bold, get_color_string(ch, COLOR_EXITS, VT102_BOLD));
	strcpy( dim, get_color_string(ch, COLOR_EXITS, VT102_DIM ));

	for (buf[0] = '\0', door = 0 ; door <= 5 ; door++)
	{
		if ((pexit = get_exit(ch->in_room->vnum, door)) == NULL)
		{
			continue;
		}
		if (IS_SET(ch->in_room->room_flags, ROOM_SMOKE) && !can_see_smoke(ch))
		{
			continue;
		}
		if (IS_SET(pexit->flags, EX_HIDDEN) && !can_see_hidden(ch))
		{
			continue;
		}
		if (!can_use_exit(ch, pexit))
		{
			continue;
		}
		num_exits++;

		if (fAuto)
		{
			cat_sprintf(buf, "%s%c-%s", bold, toupper(dir_name[door][0]), dim);
		}
		else
		{
			cat_sprintf(buf, "%s%5s - %s", bold, capitalize(dir_name[door]), dim);

			if (show_build_vnum(ch, pexit->to_room))
			{
				cat_sprintf(buf, "%s(%5d) %s", bold, pexit->to_room, dim);
			}
		}

		if (!IS_SET(pexit->flags, EX_CLOSED))
		{
			if (!can_see_in_room(ch, room_index[pexit->to_room]))
			{
				sprintf(buf2, "%-18s", "Too dark to tell");
			}
			else
			{
				sprintf(buf2, "%-18s", room_index[pexit->to_room]->name);
			}
		}
		else if (fAuto)
		{
			if (pexit->keyword[0] != '\0')
			{
				sprintf(buf2, "%-18s", pexit->keyword);
			}
			else
			{
				sprintf(buf2, "%-18s", "door");
			}
		}
		else
		{
			if (pexit->description[0] != '\0')
			{
				strcpy(buf2, pexit->description);
			}
			else
			{
				strcpy(buf2, "You see nothing special.");
			}
		}

		if (fAuto)
		{
			buf2[17] = ' ';
			buf2[18] = '\0';

			if (num_exits % 4 == 0)
			{
				strcat(buf2, "\n\r");
			}
		}
		else
		{
			strcat(buf2, "\n\r");
		}
		strcat(buf, buf2);
	}

	if (!num_exits)
	{
		strcat(buf, "You see no visible exits.");
	}

	if (str_suffix("\n\r", buf))
	{
		strcat(buf,"\n\r");
	}

	send_to_char( buf, ch);
	pop_call();
	return;
}

/*
	New layout for score/status/affects/reinc-status - Scandum 12-06-2002
*/

void do_score( CHAR_DATA *ch, char *argument )
{
	push_call("do_score(%p,%p)",ch,argument);

	get_string_score_v1(ch, ch);

	send_to_char_color( get_string_score_txt, ch );
	pop_call();
	return;
}

void get_string_score_v1( CHAR_DATA *ch, CHAR_DATA *viewer)
{
	char buf0[MAX_STRING_LENGTH];
	char buf1[90], buf2[90], buf3[90], skp1[80], skp2[80];
	char colg[10], colG[10], colw[10], colW[10], colB[10];
	int cnt, leng;

	push_call("get_string_score_v1(%p,%p)",ch,viewer);

	*get_string_score_txt = '\0';

	strcpy(colg, "{028}");
	strcpy(colG, "{128}");
	strcpy(colB, "{148}");
	strcpy(colw, get_color_code(viewer, COLOR_SCORE, VT102_DIM));
	strcpy(colW, get_color_code(viewer, COLOR_SCORE, VT102_BOLD));

	sprintf(buf3, "%s +----------------------------------------------------------------------------+\n\r", colg);
	leng = str_cpy_max(get_string_score_txt, buf3, MAX_STRING_LENGTH);

	sprintf(buf0, "%s |%s    Class%s:%s %-12s %s        Age%s:%s %-12d %s      Level%s:%s %-12d %s|\n\r",
		colg,
		colw, colW, colw, IS_NPC(ch) ? "Monster" : class_table[ch->class].name,
		colw, colW, colw, get_age(ch),
		colw, colW, colw, ch->level,
		colg);
	leng = str_apd_max(get_string_score_txt, buf0, leng, MAX_STRING_LENGTH);

	sprintf(buf0, "%s |%s     Race%s:%s %-12s %s      Hours%s:%s %-12d %s  Practices%s:%s %-12d %s|\n\r",
		colg,
		colw, colW, colw, race_table[ch->race].race_name,
		colw, colW, colw, IS_NPC(ch) ? 0 : ch->pcdata->played/3600,
		colw, colW, colw, IS_NPC(ch) ? 0 : ch->pcdata->practice,
		colg);
	leng = str_apd_max(get_string_score_txt, buf0, leng, MAX_STRING_LENGTH);

	leng = str_apd_max(get_string_score_txt, buf3, leng, MAX_STRING_LENGTH);

	sprintf(buf0, "%s |      %sStr%s: %s%2d%s/%s%2d   %sInt%s: %s%2d%s/%s%2d   %sWis%s: %s%2d%s/%s%2d   %sDex%s: %s%2d%s/%s%2d   %sCon%s: %s%2d%s/%s%2d        %s|\n\r",
		colg,
		colw, colW, colB, get_curr_str(ch), colW, colB, get_max_str(ch),
		colw, colW, colB, get_curr_int(ch), colW, colB, get_max_int(ch),
		colw, colW, colB, get_curr_wis(ch), colW, colB, get_max_wis(ch),
		colw, colW, colB, get_curr_dex(ch), colW, colB, get_max_dex(ch),
		colw, colW, colB, get_curr_con(ch), colW, colB, get_max_con(ch),
		colg);
	leng = str_apd_max(get_string_score_txt, buf0, leng, MAX_STRING_LENGTH);

	leng = str_apd_max(get_string_score_txt, buf3, leng, MAX_STRING_LENGTH);

	sprintf(buf1, "%d%s/%s%d", ch->carry_number, colW, colw, can_carry_n(ch));
	sprintf(buf0, "%s |%s Carrying%s:%s %-11s %s Armor Class%s:%s %-12d %s Experience%s:%s %-12d %s|\n\r",
		colg,
		colw, colW, colw, str_resize(buf1, skp1, -11 - strlen(colW) - strlen(colw)),
		colw, colW, colw, GET_AC(ch),
		colw, colW, colw, IS_NPC(ch) ? 0 : ch->pcdata->exp,
		colg);
	leng = str_apd_max(get_string_score_txt, buf0, leng, MAX_STRING_LENGTH);

	sprintf(buf1, "%d%s/%s%d", ch->carry_weight, colW, colw, can_carry_w(ch));

	sprintf(buf0, "%s |%s   Weight%s:%s %-11s %sSaving Spell%s:%s %-12d %s  Alignment%s:%s %-12d %s|\n\r",
		colg,
		colw, colW, colw, str_resize(buf1, skp1, -11 - strlen(colW) - strlen(colw)),
		colw, colW, colw, GET_SAVING_THROW(ch),
		colw, colW, colw, ch->alignment,
		colg);
	leng = str_apd_max(get_string_score_txt, buf0, leng, MAX_STRING_LENGTH);

	switch(ch->speed)
	{
		case 0: strcpy(buf1, "Walking"); break;
		case 1: strcpy(buf1, "Normal");  break;
		case 2: strcpy(buf1, "Jogging"); break;
		case 3: strcpy(buf1, "Running"); break;
		case 4: strcpy(buf1, "Hasting"); break;
		case 5: strcpy(buf1, "Blazing"); break;
	}
	sprintf(buf0, "%s |%s    Speed%s:%s %-12s %s    Damroll%s:%s %-12d %s       Gold%s:%s %-12s %s|\n\r",
		colg,
		colw, colW, colw, buf1,
		colw, colW, colw, GET_DAMROLL(ch),
		colw, colW, colw, tomoney(ch->gold),
		colg);
	leng = str_apd_max(get_string_score_txt, buf0, leng, MAX_STRING_LENGTH);

	sprintf(buf0, "%s |%s Speaking%s:%s %-12s %s    Hitroll%s:%s %-12d %s      Wimpy%s:%s %-12d %s|\n\r",
		colg,
		colw, colW, colw, IS_NPC(ch) ? race_table[ch->race].race_name : language_table[UNSHIFT(ch->pcdata->speak)].name,
		colw, colW, colw, GET_HITROLL(ch),
		colw, colW, colw, IS_NPC(ch) ? 0 : ch->pcdata->wimpy,
		colg);
	leng = str_apd_max(get_string_score_txt, buf0, leng, MAX_STRING_LENGTH);

	leng = str_apd_max(get_string_score_txt, buf3, leng, MAX_STRING_LENGTH);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	sprintf(buf0, "%s |       ", colg);
	for (cnt = 0 ; cnt < MAX_CLASS ; cnt++)
	{
		cat_sprintf(buf0, "%s%s%s:%s%2d  ", colG, class_table[cnt].who_name, colW, colG, ch->pcdata->mclass[cnt]);
	}
	cat_sprintf(buf0, "     %s|\n\r", colg);
	leng = str_apd_max(get_string_score_txt, buf0, leng, MAX_STRING_LENGTH);

	leng = str_apd_max(get_string_score_txt, buf3, leng, MAX_STRING_LENGTH);

	strcpy(buf1, room_index[ch->pcdata->recall]->name);
	strcpy(buf2, ch->pcdata->auto_command);
	sprintf(buf0, "%s |   %sRecall Room%s:%s %-21s   %sAuto command%s:%s %-21s %s|\n\r", 
		colg,
		colw, colW, colW, str_resize(buf1, skp1, -21),
		colw, colW, colW, str_resize(buf2, skp2, -21),
		colg);
	leng = str_apd_max(get_string_score_txt, buf0, leng, MAX_STRING_LENGTH);

	strcpy(buf1, get_room_index(ch->pcdata->death_room)->name);
	strcpy(buf2, ch->pcdata->auto_flags == AUTO_OFF ? "Off" : ch->pcdata->auto_flags == AUTO_AUTO ? "Auto" : "Quick");

	sprintf(buf0, "%s |   %s Death Room%s:%s %-21s   %s   Auto mode%s:%s %-21s %s|\n\r", 
		colg,
		colw, colW, colW, str_resize(buf1, skp1, -21),
		colw, colW, colW, str_resize(buf2, skp2, -21),
		colg);
	leng = str_apd_max(get_string_score_txt, buf0, leng, MAX_STRING_LENGTH);

	leng = str_apd_max(get_string_score_txt, buf3, leng, MAX_STRING_LENGTH);

	pop_call();
	return;
}


void do_status( CHAR_DATA *ch, char *argument )
{
	push_call("do_status(%p,%p)",ch,argument);

	if (argument[0] == 'r')
	{
		get_string_reinc_v1(ch, ch);
	}
	else
	{
		get_string_status_v1(ch, ch);
	}

	send_to_char_color( get_string_score_txt, ch );
	pop_call();
	return;
}

void get_string_status_v1( CHAR_DATA *ch, CHAR_DATA *viewer)
{
	char buf0[MAX_STRING_LENGTH];
	char buf1[90], buf2[90], buf3[90], buf4[90], skp1[80], skp2[80], skp3[80];
	char colg[10], colw[10], colW[10], colB[10];
	OBJ_DATA *wield;
	int numDice, sizeDice, sizePlus;
	int leng, cnt;

	push_call("get_string_score_v1(%p,%p)",ch,viewer);

	*get_string_score_txt = '\0';

	strcpy(colg, "{028}");
	strcpy(colB, "{148}");
	strcpy(colw, get_color_code(viewer, COLOR_SCORE, VT102_DIM));
	strcpy(colW, get_color_code(viewer, COLOR_SCORE, VT102_BOLD));

	sprintf(buf4, "%s +----------------------------------------------------------------------------+\n\r", colg);
	leng = str_cpy_max(get_string_score_txt, buf4, MAX_STRING_LENGTH);

	sprintf(buf0, "%s |       %sStr%s: %s%2d%s/%s%2d   %sInt%s: %s%2d%s/%s%2d   %sWis%s: %s%2d%s/%s%2d   %sDex%s: %s%2d%s/%s%2d   %sCon%s: %s%2d%s/%s%2d       %s|\n\r",
		colg,
		colw, colW, colB, get_curr_str(ch), colW, colB, get_max_str(ch),
		colw, colW, colB, get_curr_int(ch), colW, colB, get_max_int(ch),
		colw, colW, colB, get_curr_wis(ch), colW, colB, get_max_wis(ch),
		colw, colW, colB, get_curr_dex(ch), colW, colB, get_max_dex(ch),
		colw, colW, colB, get_curr_con(ch), colW, colB, get_max_con(ch),
		colg);
	leng = str_apd_max(get_string_score_txt, buf0, leng, MAX_STRING_LENGTH);

	leng = str_apd_max(get_string_score_txt, buf4, leng, MAX_STRING_LENGTH);

	sprintf(buf1, "%d%s/%s%d", ch->carry_number, colW, colw, can_carry_n(ch));
	sprintf(buf2, "%d%s/%s%d", ch->carry_weight, colW, colw, can_carry_w(ch));
	sprintf(buf0, "%s |%s  Carry Items%s:%s %-11s     %s        Carry Weight%s:%s %-11s            %s|\n\r",
		colg,
		colw, colW, colw, str_resize(buf1, skp1, -11 - strlen(colW) - strlen(colw)),
		colw, colW, colw, str_resize(buf2, skp2, -11 - strlen(colW) - strlen(colw)),
		colg);
	leng = str_apd_max(get_string_score_txt, buf0, leng, MAX_STRING_LENGTH);

	switch (URANGE(-11, ((GET_AC(ch)-100) / 3) / (5+ch->level/5), 0))
	{
		case   0: strcpy( buf1, "Worse than Naked"); break;
		case  -1: strcpy( buf1, "Wearing Clothes "); break;
		case  -2: strcpy( buf1, "Slightly Armored"); break;
		case  -3: strcpy( buf1, "Somewhat Armored"); break;
		case  -4: strcpy( buf1, "Lightly Armored "); break;
		case  -5: strcpy( buf1, "Armored         "); break;
		case  -6: strcpy( buf1, "Well Armored    "); break;
		case  -7: strcpy( buf1, "Strongly Armored"); break;
		case  -8: strcpy( buf1, "Heavily Armored "); break;
		case  -9: strcpy( buf1, "Superbly Armored"); break;
		case -10: strcpy( buf1, "Divinely Armored"); break;
		case -11: strcpy( buf1, "Invincible      "); break;
	}

	for (cnt = GET_AC(ch) ; cnt < 100 && cnt > -100 && strlen(buf1) < 18 ; cnt *= 10)
	{
		strcat(buf1, " ");
	}
	cat_sprintf(buf1, "%s", GET_AC(ch) >= 0 ? " " : "");

	switch (URANGE(-7, (GET_SAVING_THROW(ch)+ch->level/9)/6, 0))
	{
		case  0: strcpy(buf2, "Weak          "); break;
		case -1: strcpy(buf2, "Vulnerable    "); break;
		case -2: strcpy(buf2, "Resistant     "); break;
		case -3: strcpy(buf2, "Steadfast     "); break;
		case -4: strcpy(buf2, "Perseverant   "); break;
		case -5: strcpy(buf2, "Unbending     "); break;
		case -6: strcpy(buf2, "Adamant       "); break;
		case -7: strcpy(buf2, "Invulnerable  "); break;
	}

	for (cnt = GET_SAVING_THROW(ch) ; cnt < 1000 && cnt > -1000 && strlen(buf2) < 18 ; cnt *= 10)
	{
		strcat(buf2, " ");
	}
	cat_sprintf(buf2, "%s", GET_SAVING_THROW(ch) > 0 ? " " : "");

	sprintf(buf0, "%s |  %sArmor Class%s:%s %-16s %s(%s%d%s) %sSaving Spell%s:%s %-14s %s(%s%d%s) %s|\n\r",
		colg,
		colw, colW, colw, buf1, colW, colw, GET_AC(ch), colW,
		colw, colW, colw, buf2, colW, colw, GET_SAVING_THROW(ch), colW,
		colg);
	leng = str_apd_max(get_string_score_txt, buf0, leng, MAX_STRING_LENGTH);

	switch (ch->alignment/175)
	{
		case -5:          strcpy(buf1, "Satanic       "); break;
		case -4:          strcpy(buf1, "Demonic       "); break;
		case -3: case -2: strcpy(buf1, "Evil          "); break;
		case -1:          strcpy(buf1, "Mean          "); break;
		case  0:          strcpy(buf1, "Neutral       "); break;
		case  1:          strcpy(buf1, "Kind          "); break;
		case  2: case  3: strcpy(buf1, "Good          "); break;
		case  4:          strcpy(buf1, "Saintly       "); break;
		case  5:          strcpy(buf1, "Angelic       "); break;
	}

	for (cnt = ch->alignment ; cnt > -1000 && cnt < 1000 && strlen(buf1) < 18 ; cnt *= 10)
	{
		strcat(buf1, " ");
	}
	cat_sprintf(buf1, "%s", ch->alignment > 0 ? " " : "");

	sprintf(buf0, "%s | %s   Practices%s:%s %-23d %s   Alignment%s:%s %-14s %s(%s%d%s) %s|\n\r",
		colg,
		colw, colW, colw, IS_NPC(ch) ? 0 : ch->pcdata->practice,
		colw, colW, colw, buf1, colW, colw, ch->alignment, colW,
		colg);
	leng = str_apd_max(get_string_score_txt, buf0, leng, MAX_STRING_LENGTH); 

	wield = get_eq_char(ch, WEAR_WIELD);

	if (IS_NPC(ch))
	{
		numDice  = get_damnodice(ch);
		sizeDice = get_damsizedice(ch);
		sizePlus = ch->npcdata->damplus + GET_DAMROLL(ch);

		if (wield && wield->item_type == ITEM_WEAPON)
		{
			sizePlus += (wield->value[1] * (1 + (wield->value[2] - 1) / 2.0));
		}
	}
	else
	{
		if (wield && wield->item_type == ITEM_WEAPON)
		{
			numDice  = wield->value[1];
			sizeDice = wield->value[2];
		}
		else
		{
			numDice  = get_damnodice(ch);
			sizeDice = get_damsizedice(ch);
		}
		sizePlus = GET_DAMROLL(ch);
	}
	sprintf(buf2, "%d%sd%s%d%s+%s%d", numDice, colW, colw, sizeDice, colW, colw, sizePlus);
	sprintf(buf1, "%s", str_resize(buf2, skp2, -20 - strlen(colW)*2 - strlen(colw)*2));

	sprintf(buf0, "%s | %s     Hitroll%s:%s %-20d %s        Damroll%s:%s %-20s   %s|\n\r",
		colg,
		colw, colW, colw, GET_HITROLL(ch),
		colw, colW, colw, buf1,
		colg);
	leng = str_apd_max(get_string_score_txt, buf0, leng, MAX_STRING_LENGTH);

	leng = str_apd_max(get_string_score_txt, buf4, leng, MAX_STRING_LENGTH);

	sprintf(buf1, "%s%d%s/%s%d%s: %sHp  ", colB, ch->hit,  colW, colB, ch->max_hit,  colW, colw);
	sprintf(buf2, "%s%d%s/%s%d%s: %sMana", colB, ch->mana, colW, colB, ch->max_mana, colW, colw);
	sprintf(buf3, "%s%d%s/%s%d%s: %sMove", colB, ch->move, colW, colB, ch->max_move, colW, colw);
	sprintf(buf0, "%s |    %-17s         %17s        %17s    %s|\n\r",
		colg,
		str_resize(buf1, skp1, -17 - strlen(colW)*2 - strlen(colw) - strlen(colB)*2),
		str_resize(buf2, skp2, -17 - strlen(colW)*2 - strlen(colw) - strlen(colB)*2),
		str_resize(buf3, skp3,  17 + strlen(colW)*2 + strlen(colw) + strlen(colB)*2),
		colg);
	leng = str_apd_max(get_string_score_txt, buf0, leng, MAX_STRING_LENGTH);

	leng = str_apd_max(get_string_score_txt, buf4, leng, MAX_STRING_LENGTH);

	if (IS_NPC(ch) || !IS_IMMORTAL(ch))
	{
		pop_call();
		return;
	}

	sprintf(buf1, "%s", strlen(ch->pcdata->bamfin) > 0 ? ch->pcdata->bamfin : "appears in a swirling mist.");
	sprintf(buf0, "%s | %s     Poof in%s:%s %-60s %s|\n\r",
		colg,
		colw, colW, colw, strlen(ch->pcdata->bamfin)  > 0 ? str_resize(buf1, skp1, -60 - strlen(buf1) + ansi_strlen(ch->pcdata->bamfin)) : buf1,
		colg);
	strcat(get_string_score_txt, buf0);

	sprintf(buf1, "%s", strlen(ch->pcdata->bamfout) > 0 ? ch->pcdata->bamfout : "leaves in a swirling mist.");
	sprintf(buf0, "%s | %s    Poof out%s:%s %-60s %s|\n\r",
		colg,
		colw, colW, colw, strlen(ch->pcdata->bamfout) > 0 ? str_resize(buf1, skp1, -60 - strlen(buf1) + ansi_strlen(ch->pcdata->bamfout)) : buf1,
		colg);

	strcat(get_string_score_txt, buf0);

	strcat(get_string_score_txt, buf4);

	if (ch->pcdata->a_range_lo == 0)
	{
		pop_call();
		return;
	}

	sprintf(buf1, "%s  Rooms%s: %s%d%s-%s%d", colw, colW, colB, ch->pcdata->a_range_lo, colW, colB, ch->pcdata->a_range_hi);
	sprintf(buf2, "%sObjects%s: %s%d%s-%s%d", colw, colW, colB, ch->pcdata->a_range_lo, colW, colB, ch->pcdata->a_range_hi);
	sprintf(buf3, "%s   Mobs%s: %s%d%s-%s%d", colw, colW, colB, ch->pcdata->a_range_lo, colW, colB, ch->pcdata->a_range_hi);
	sprintf(buf0, "%s |  %-20s       %20s   %20s    %s|\n\r",
		colg,
		str_resize(buf1, skp1, -20 - strlen(colW)*2 - strlen(colw) - strlen(colB)*2),
		str_resize(buf2, skp2, -20 - strlen(colW)*2 - strlen(colw) - strlen(colB)*2),
		str_resize(buf3, skp3,  20 + strlen(colW)*2 + strlen(colw) + strlen(colB)*2),
		colg);
	strcat(get_string_score_txt, buf0);
	strcat(get_string_score_txt, buf4);

	pop_call();
	return;
}

void get_string_reinc_v1( CHAR_DATA *ch, CHAR_DATA *viewer)
{
	char buf0[MAX_STRING_LENGTH];
	char buf1[90], buf3[90], skp1[80];
	char colg[10], colw[10], colW[10], colB[10], colG[10];
	int cnt, leng;

	push_call("get_string_reinc_v1(%p,%p)",ch,viewer);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	*get_string_score_txt = '\0';

	strcpy(colg, "{028}");
	strcpy(colw, get_color_code(ch, COLOR_SCORE, VT102_DIM));
	strcpy(colW, get_color_code(ch, COLOR_SCORE, VT102_BOLD));
	strcpy(colB, "{148}");
	strcpy(colG, "{128}");

	sprintf(buf3, "%s +----------------------------------------------------------------------------+\n\r", colg);
	leng = str_cpy_max(get_string_score_txt, buf3, MAX_STRING_LENGTH);

 	sprintf(buf0, "%s |  %sStr%s:%s %-4d %sInt%s:%s %-4d %sWis%s:%s %-4d %sDex%s:%s %-4d %sCon%s:%s %-4d %sAverage of Stats%s:%s %2.1f  %s|\n\r",
		colg,
		colw, colW, colB, ch->pcdata->perm_str,
		colw, colW, colB, ch->pcdata->perm_int,
		colw, colW, colB, ch->pcdata->perm_wis,
		colw, colW, colB, ch->pcdata->perm_dex,
		colw, colW, colB, ch->pcdata->perm_con,
		colw, colW, colB, (float) (ch->pcdata->perm_str+ch->pcdata->perm_int+ch->pcdata->perm_wis+ch->pcdata->perm_dex+ch->pcdata->perm_con) / 5.0,
		colg);
	leng = str_apd_max(get_string_score_txt, buf0, leng, MAX_STRING_LENGTH);

	leng = str_apd_max(get_string_score_txt, buf3, leng, MAX_STRING_LENGTH);

	for (buf1[0] = '\0', cnt = 0 ; cnt < MAX_RACE ; cnt++)
	{
		if (IS_SHIFT(ch->pcdata->language, cnt))
		{
			cat_sprintf(buf1, "%s ", str_resize(race_table[cnt].race_name, skp1, -3));
		}
	}
	sprintf(buf0, "%s |  %sClass%s:%s %-11s  %sSex%s:%s %-8s  %sRace%s:%s %-11s  %sKnown%s:%s %-11s  %s|\n\r",
 		colg,
 		colw, colW, colG, class_table[ch->class].name,
		colw, colW, colG, ch->sex <= 0 ? "Neutral" : ch->sex == 1 ? "Male" : "Female",
 		colw, colW, colG, race_table[ch->race].race_name,
 		colw, colW, colG, str_resize(buf1, skp1, -11),
 		colg);
	leng = str_apd_max(get_string_score_txt, buf0, leng, MAX_STRING_LENGTH); 

	leng = str_apd_max(get_string_score_txt, buf3, leng, MAX_STRING_LENGTH);

	pop_call();
	return;
}


char *  const   month_name      [] =
{
	"the Winter Falcon",	"the Frost Giant",		"the Old Forces",		"the Grand Struggle",
	"the Soft Rains",		"the Wakening",		"the Golden Dawn",		"the Glittering Star",
	"the Summer Song",		"the Sultry Wind",		"the Sweet Nectar",		"the Lazy Sun",
	"the Dark Tidings",		"the Horned Hunter",	"the Long Shadows", 	"the Ancient Darkness",
};

/* Enhanced AFFECTS command  -  Chaos  4/3/99  */

void do_affects( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];

	push_call("do_affects(%p,%p)",ch,argument);

	get_affects_string(ch, ch, buf);
	send_to_char_color(buf, ch);

	pop_call();
	return;
}

void get_affects_string( CHAR_DATA *ch, CHAR_DATA *viewer, char *outbuf)
{
	AFFECT_DATA *paf;
	char buf0[MAX_STRING_LENGTH];
	char buf1[90], buf2[90], buf3[90], skp1[80], skp2[80];
	char colg[10], colw[10], colW[10], colG[10];
	int leng;

	push_call("get_affects_string(%p,%p,%p,%p)",ch,viewer,outbuf, MAX_STRING_LENGTH);

	*outbuf = '\0';

	strcpy(colg, "{028}");
	strcpy(colw, get_color_code(viewer, COLOR_SCORE, VT102_DIM));
	strcpy(colW, get_color_code(viewer, COLOR_SCORE, VT102_BOLD));
	strcpy(colG, "{128}");

	sprintf(buf3, "%s +----------------------------------------------------------------------------+\n\r", colg);
	leng = str_cpy_max(outbuf, buf3, MAX_STRING_LENGTH);

	if (ch->first_affect != NULL)
	{
		for (paf = ch->first_affect ; paf != NULL ; paf = paf->next)
		{
			sprintf(buf1, "%s%s%s: %s'%s%s%s'",
				colw, IS_SET(skill_table[paf->type].flags, FSKILL_SPELL) ? "Spell" : "Skill",
				colW, colw, colW, skill_table[paf->type].name, colw);

			sprintf(buf2, "modifies %s by %d for %d hours", affect_loc_name(paf->location), paf->modifier, paf->duration);

			sprintf(buf0, "%s | %-27s %-45s %s|\n\r",
				colg,
				str_resize(buf1, skp1, -27 - strlen(colw)*3 - strlen(colW)*2),
				str_resize(buf2, skp2, -46),
				colg);
			leng = str_apd_max(outbuf, buf0, leng, MAX_STRING_LENGTH);
		}
	}
	else
	{
		sprintf(buf0, "%s |%s %-74s %s|\n\r", colg, colW, "None", colg);
		leng = str_apd_max(outbuf, buf0, leng, MAX_STRING_LENGTH);
	}
	leng = str_apd_max(outbuf, buf3, leng, MAX_STRING_LENGTH);

	pop_call();
	return;
}

void do_time( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_INPUT_LENGTH];

	push_call("do_time(%p,%p)",ch,argument);

	sprintf(buf,     "It is %s, the %s day of the Month of %s in the year %d.\n\r\n\r", tocivilian(mud->time_info->hour), numbersuf(mud->time_info->day + 1), month_name[mud->time_info->month], mud->time_info->year);

	cat_sprintf(buf, "Emud started up at: %s\n\r", get_time_string(mud->boot_time));
	cat_sprintf(buf, "The system time is: %s\n\r", get_time_string(mud->current_time));

	send_to_char(buf, ch);

	pop_call();
	return;
}

void do_weather( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_INPUT_LENGTH];

	push_call("do_weather(%p,%p)",ch,argument);

	if (!IS_OUTSIDE(ch))
	{
		ch_printf(ch, "You can't see the weather indoors.\n\r");
		ch_printf(ch, "The temperature is about %d degrees celcius.\n\r", ch->in_room->area->weather_info->temperature);
		pop_call();
		return;
	}

	if (ch->in_room->area->weather_info->temperature < 0)
	{
		switch (ch->in_room->area->weather_info->sky)
		{
			case 0: strcpy( buf, "The sky is clear");		break;
			case 1: strcpy( buf, "The sky is a bit frosty");	break;
			case 2: strcpy( buf, "It is snowing");			break;
			case 3: strcpy( buf, "You are in a blizzard");	break;
		}
	}
	else if (ch->in_room->area->weather_info->temperature < 4)
	{
		switch (ch->in_room->area->weather_info->sky)
		{
			case 0: strcpy( buf, "The sky is clear");		break;
			case 1: strcpy( buf, "The sky is chilling");		break;
			case 2: strcpy( buf, "It is sleeting");			break;
			case 3: strcpy( buf, "You are in a hail storm");	break;
		}
	}
	else
	{
		switch (ch->in_room->area->weather_info->sky)
		{
			case 0: strcpy( buf, "The sky is clear");		break;
			case 1: strcpy( buf, "The sky is cloudy");		break;
			case 2: strcpy( buf, "It is raining");			break;
			case 3: strcpy( buf, "You are in a storm and a");	break;
		}
	}

	if (ch->in_room->area->weather_info->wind_speed > 3)
	{
		strcat(buf, " and a ");

		if (ch->in_room->area->weather_info->temperature < 0)
		{
			strcat(buf, "freezing ");
		}
		else if (ch->in_room->area->weather_info->temperature < 4)
		{
			strcat(buf, "chilling ");
		}
		else if (ch->in_room->area->weather_info->temperature < 10)
		{
			strcat(buf, "cold ");
		}
		else if (ch->in_room->area->weather_info->temperature < 16)
		{
			strcat(buf, "cool ");
		}
		else if (ch->in_room->area->weather_info->temperature < 20)
		{
			strcat(buf, "mild ");
		}
		else if (ch->in_room->area->weather_info->temperature < 25)
		{
			strcat(buf, "warm ");
		}
		else
		{
			strcat(buf, "sultry ");
		}

		if (ch->in_room->area->weather_info->wind_speed < 6)
		{
			cat_sprintf(buf, "%s wind blows", wind_dir_name[ch->in_room->area->weather_info->wind_dir]);
		}
		else if (ch->in_room->area->weather_info->wind_speed < 8)
		{
			cat_sprintf(buf, "%s breeze blows", wind_dir_name[ch->in_room->area->weather_info->wind_dir]);
		}
		else
		{
			cat_sprintf(buf, "%s gust blows", wind_dir_name[ch->in_room->area->weather_info->wind_dir]);
		}
	}

	ch_printf(ch, "%s.\n\rThe temperature is about %d degrees celcius.\n\r", buf, ch->in_room->area->weather_info->temperature);

	if (ch->level < MAX_LEVEL)
	{
		pop_call();
		return;
	}

	ch_printf(ch, "\n\rSky: %d\n\rChange: %d\n\rWind: %d\n\rDir: %d\n\rSunlight: %d\n\rTemp: %d\n\r",
		ch->in_room->area->weather_info->sky,
		ch->in_room->area->weather_info->change,
		ch->in_room->area->weather_info->wind_speed,
		ch->in_room->area->weather_info->wind_dir,
		mud->sunlight,
		ch->in_room->area->weather_info->temperature);

	pop_call();
	return;
}

HELP_DATA *get_help( CHAR_DATA *ch, char *argument )
{
	AREA_DATA *pArea;
	HELP_DATA *pHelp;

	push_call("get_help(%p,%p)",ch,argument);

	for (pArea = mud->f_area ; pArea ; pArea = pArea->next)
	{
		for (pHelp = pArea->first_help ; pHelp ; pHelp = pHelp->next)
		{
			if (pHelp->level > ch->level)
			{
				continue;
			}
			if (is_name(argument, pHelp->keyword))
			{
				pop_call();
				return pHelp;
			}
		}
	}

	for (pArea = mud->f_area ; pArea ; pArea = pArea->next)
	{
		for (pHelp = pArea->first_help ; pHelp ; pHelp = pHelp->next)
		{
			if (pHelp->level > ch->level)
			{
				continue;
			}
			if (is_name_short(argument, pHelp->keyword))
			{
				pop_call();
				return pHelp;
			}
		}
	}
	pop_call();
	return NULL;
}


void do_help( CHAR_DATA *ch, char *argument )
{
	HELP_DATA *pHelp;

	push_call("do_help(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (argument[0] == '\0')
	{
		argument = "mainmenu";
	}

	if ((pHelp = get_help(ch, argument)) == NULL)
	{
		ch_printf(ch, "No help available for '%s'.\n\r", argument);
		pop_call();
		return;
	}

	ch->pcdata->prev_help = ch->pcdata->last_help;
	ch->pcdata->last_help = pHelp;

	SET_BIT(ch->pcdata->interp, INTERP_HELP);

	ch_printf_color(ch, "{300}%s", pHelp->text);

	pop_call();
	return;
}

/*
	Rewrote selection routine and stabilized output - Scandum 16-07-2002
*/

void do_who( CHAR_DATA *ch, char *argument )
{
	push_call("do_who(%p,%p)",ch,argument);
	create_list(ch, argument, 0);
	pop_call();
	return;
}

void do_language( CHAR_DATA *ch, char *argument )
{
	push_call("do_language(%p,%p)",ch,argument);
	create_list(ch, argument, 1);
	pop_call();
	return;
}

void do_multi( CHAR_DATA *ch, char *argument )
{
	push_call("do_multi(%p,%p)",ch,argument);
	create_list(ch, argument, 2);
	pop_call();
	return;
}

void create_list( CHAR_DATA *ch, char *argument, int layout )
{
	CHAR_DATA			*fch		= NULL;
	CLAN_DATA 		*iClan	= NULL;
	AREA_DATA 		*iArea	= NULL;
	CHAR_DATA			*wch		= NULL;
	PLAYER_GAME		*fpl		= NULL;
	char buf[MAX_STRING_LENGTH];
	char tmp[MAX_INPUT_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	bool tong, under;
	int nMatch, nTotal, lrang, hrang;
	int cnt, type, iGod, iRace, iClass;

	char colr[10], colg[10], coly[10], col1[10], colm[10], col2[10], colw[10], col3[10];
	char colR[10], colG[10], colY[10], colB[10], colM[10], colC[10], colW[10];

	push_call("create_list(%p,%p,%d)",ch,argument,layout);

	strcpy(colr, "{018}");
	strcpy(colg, "{028}");
	strcpy(coly, "{038}");
	strcpy(col1, "{002}");
	strcpy(colm, "{058}");
	strcpy(col2, "{134}");
	strcpy(colw, "{078}");
	strcpy(col3, "{088}");

	strcpy(colR, "{118}");
	strcpy(colG, "{128}");
	strcpy(colY, "{138}");
	strcpy(colB, "{148}");
	strcpy(colM, "{158}");
	strcpy(colC, "{168}");
	strcpy(colW, "{178}");

	type		= 0;
	lrang	= 1;
	hrang	= MAX_LEVEL;
	iGod		= iRace	= iClass	= -1;

	argument = one_argument(argument, arg);

	if (!strcasecmp(arg, "clan"))   type = 1;
	if (!strcasecmp(arg, "race"))   type = 2;
	if (!strcasecmp(arg, "class"))  type = 3;
	if (!strcasecmp(arg, "all"))    type = 4;
	if (!strcasecmp(arg, ""))       type = 4;
	if (!strcasecmp(arg, "area"))   type = 5;
	if (!strcasecmp(arg, "god"))    type = 6;
	if (!strcasecmp(arg, "enemy"))  type = 7;
	if (!strcasecmp(arg, "target")) type = 8;
	if (!strcasecmp(arg, "group"))  type = 9;

	switch (type)
	{
		case 0:
			if (is_number(arg))
			{
				lrang = URANGE(1, atol(arg), MAX_LEVEL);
				if (is_number(argument))
				{
					hrang = URANGE(1, atol(argument), MAX_LEVEL);
				}
				else
				{
					hrang = MAX_LEVEL;
				}
				type  = 9;
			}
			else if ((fch = get_player_world(ch, arg)) == NULL)
			{
				send_to_char("No one has that name.\n\r", ch);
				pop_call();
				return;
			}
			break;
		case 1:
			if (strlen(argument) >= 2)
			{
				iClan = get_clan(argument);
			}
			else
			{
				iClan = ch->pcdata->clan;
			}
			if (iClan == NULL)
			{
				send_to_char("You must specify an existing clan.\n\r", ch);
				pop_call();
				return;
			}
			break;
		case 2:
			if (strlen(argument) >= 2)
			{
				iRace = lookup_race(argument);
			}
			else
			{
				iRace = ch->race;
			}
			if (iRace == -1)
			{
				send_to_char("You must specify an existing race.\n\r", ch);
				pop_call();
				return;
			}
			break;
		case 3:
			if (strlen(argument) >= 2)
			{
				iClass = lookup_class(argument);
			}
			else
			{
				iClass = ch->class;
			}
			if (iClass == -1)
			{
				send_to_char("You must specify an existing class.\n\r", ch);
				pop_call();
				return;
			}
			break;
		case 5:
			iArea = ch->in_room->area;
			if (strlen(argument) >= 2)
			{
				iArea = lookup_area(argument);
			}
			if (iArea == NULL)
			{
				send_to_char("You must specify an existing area.\n\r", ch);
				pop_call();
				return;
			}
			break;
		case 6:
			if (strlen(argument) > 0)
			{
				iGod = lookup_god(argument);
			}
			else
			{
				iGod = which_god(ch);
			}
			if (iGod == -1)
			{
				send_to_char("You must specify an existing god.\n\r", ch);
				pop_call();
				return;
			}
			break;
		case 9:
			lrang = (ch->level - 2) * 4/5 + 1;
			hrang = (ch->level + 2) * 4/3 + 1;
			break;
	}

	/*
		Now show matching chars.
	*/

	nMatch = 0;
	nTotal = 0;
	buf[0] = '\0';

	for (fpl = mud->f_player ; fpl ; fpl = fpl->next)
	{
		wch = fpl->ch;

		if (!can_see_world(ch, wch))
		{
			continue;
		}
		else
		{
			nTotal++;
		}

		if (wch->level < lrang || wch->level > hrang)
		{
			continue;
		}

		switch (type)
		{
			case 0: if (wch != fch && wch != ch)		continue; break;
			case 1: if (wch->pcdata->clan != iClan)		continue; break;
			case 2: if (wch->race != iRace)			continue; break;
			case 3: if (wch->class != iClass)			continue; break;
			case 5: if (wch->in_room->area != iArea || IS_AFFECTED(wch, AFF_STEALTH)) continue; break;
			case 6: if (which_god(wch) != iGod)		continue; break;
			case 7: if (which_god(wch) == which_god(ch))	continue; break;
			case 8: if (!can_attack(wch, ch))			continue; break;
		}

		if (nMatch < 200)
		{
			nMatch++;
		}
		else
		{
			continue;
		}

		switch (layout)
		{
			case 0:	/* Who list */
				cat_sprintf(buf, "%s[%2d ", coly, wch->level);

				if (wch->level > MAX_LEVEL - 4)
				{
					switch (wch->level)
					{
						case MAX_LEVEL - 0: cat_sprintf(buf, "%sGOD%s ", colG, coly); break;
						case MAX_LEVEL - 1: cat_sprintf(buf, "%sARC%s ", colG, coly); break;
						case MAX_LEVEL - 2: cat_sprintf(buf, "%sANG%s ", colG, coly); break;
						case MAX_LEVEL - 3: cat_sprintf(buf, "%sLRD%s ", colG, coly); break;
					}
				}
				else
				{
					cat_sprintf(buf, "%s ", class_table[wch->class].who_name);
				}

				if (wch->level > MAX_LEVEL-4)
				{
					cat_sprintf(buf, "---]");
				}
				else
				{
					cat_snprintf(buf, 3, "%s", race_table[wch->race].race_name);
					strcat(buf, "]");
				}

				cat_sprintf(buf, "%s%s%s<%s",	god_table[which_god(wch)].color, god_table[which_god(wch)].who_letter, colg, colG);

				if (IS_SET(wch->act, PLR_AFK))
				{
					cat_sprintf(buf, "%sAFK", colg);
				}
				else if (wch->level >= LEVEL_IMMORTAL || IS_AFFECTED(wch, AFF_TONGUES))
				{
					cat_sprintf(buf, "%sAll", colG);
				}
				else
				{
					cat_sprintf(buf, "%s%s", colG, language_table[UNSHIFT(wch->pcdata->speak)].who_name);
				}

				if (IS_SET(wch->act, PLR_KILLER))
				{
					cat_sprintf(buf, "%s>%sK%s", colg, colr, colW);
				}
				else if (IS_SET(wch->act, PLR_THIEF))
				{
					cat_sprintf(buf, "%s>%sT%s", colg, colm, colW);
				}
				else
				{
					cat_sprintf(buf, "%s> %s", colg, colW);
				}

				sprintf(tmp, "%s%s%-60s                                                  ", wch->name, colw, ansi_strip(ansi_translate(wch->pcdata->title)));

				if (IS_GOD(ch))
				{
					if (wch->pcdata->clan_name[0] != '\0')
					{
						tmp[37+strlen(colw)] = '\0';
						cat_sprintf(tmp, "%s%-10s", col1, wch->pcdata->clan_name);
					}
					else
					{
						tmp[47+strlen(colw)] = '\0';
					}
				}
				else
				{
					if (wch->pcdata->clan_name[0] != '\0')
					{
						tmp[50+strlen(colw)] = '\0';
						cat_sprintf(tmp, "%s%-10s", col1, wch->pcdata->clan_name);
					}
					else
					{
						tmp[50+strlen(colw)] = '\0';
					}
				}

				cat_sprintf(buf, "%s%s", tmp, col2);

				if (IS_GOD(ch))
				{
					if (wch->desc != NULL && !IS_AFFECTED(wch, AFF_STEALTH))
					{
						cat_snprintf(buf, 13, "%-14s", wch->in_room->area->name);
					}
					else if (wch->pcdata->switched)
					{
						cat_snprintf(buf, 13, "  Switched   ");
					}
					else if (wch->desc == NULL)
					{
						cat_snprintf(buf, 13, "  Link Lost  ");
					}
					else
					{
						cat_snprintf(buf, 13, "   Unknown   ");
					}
				}
				cat_sprintf(buf, "%s\n\r", col3);
				break;

			case 1:	/* Language list */
				cat_sprintf(buf, "%s%-13s", colW, wch->name);
				cat_sprintf(buf, "%s[%2d]", coly, wch->level);

				cat_sprintf(buf, "%s%s%s ", god_table[which_god(wch)].color, god_table[which_god(wch)].who_letter, colg);

				tong  = (wch->level >= 90 || IS_AFFECTED(wch, AFF_TONGUES));
				under = (wch->level >= 90 || IS_AFFECTED(wch, AFF_UNDERSTAND));

				for (cnt = 0 ; cnt < MAX_LANGUAGE ; cnt++)
				{
					if (IS_SHIFT(wch->pcdata->language, cnt) || tong || under)
					{
						if (IS_SHIFT(wch->pcdata->speak, cnt) || tong)
						{
							strcat(buf, colG);
						}
						else
						{
							strcat(buf, colg);
						}
						if (IS_SHIFT(wch->pcdata->language, cnt) || (tong && under))
						{
							cat_sprintf(buf, "%s ", language_table[cnt].who_name);
						}
						else
						{
							strcat(buf, "+++ ");
						}
					}
					else
					{
						cat_sprintf(buf, "%s--- ", colg);
					}
				}
				strcat(buf, "\n\r");
				break;

			case 2:	/* Multi class list */
				cat_sprintf(buf, "%s%-13s", colW, wch->name);
				cat_sprintf(buf, "%s[%2d ", coly, wch->level);

				if (wch->level > MAX_LEVEL - 4)
				{
					switch (wch->level)
					{
						case MAX_LEVEL - 0: cat_sprintf(buf, "%sGOD%s]%s", colG, coly, colG); break;
						case MAX_LEVEL - 1: cat_sprintf(buf, "%sARC%s]%s", colG, coly, colG); break;
						case MAX_LEVEL - 2: cat_sprintf(buf, "%sANG%s]%s", colG, coly, colG); break;
						case MAX_LEVEL - 3: cat_sprintf(buf, "%sLRD%s]%s", colG, coly, colG); break;
					}
				}
				else
				{
					cat_sprintf(buf, "%s]", class_table[wch->class].who_name);
				}
				cat_sprintf(buf, "%s%s%s<%s",	god_table[which_god(wch)].color, god_table[which_god(wch)].who_letter, colg, colG);

				cat_sprintf(buf, "%s%s:%s%2d %s%s:%s%2d %s%s:%s%2d %s%s:%s%2d %s%s:%s%2d %s%s:%s%2d %s%s:%s%2d %s%s:%s%2d%s>\n\r",
					class_table[0].who_name, colY, colG, wch->pcdata->mclass[0],
					class_table[1].who_name, colY, colG, wch->pcdata->mclass[1],
					class_table[2].who_name, colY, colG, wch->pcdata->mclass[2],
					class_table[3].who_name, colY, colG, wch->pcdata->mclass[3],
					class_table[4].who_name, colY, colG, wch->pcdata->mclass[4],
					class_table[5].who_name, colY, colG, wch->pcdata->mclass[5],
					class_table[6].who_name, colY, colG, wch->pcdata->mclass[6],
					class_table[7].who_name, colY, colG, wch->pcdata->mclass[7],
					colg);
				break;
		}
	}
	cat_sprintf(buf, "%s%d/%d players.  %s\n\r", coly, nMatch, nTotal, (nMatch != nTotal) ? "This is a partial list." : "");

	send_to_char_color(buf, ch);
	pop_call();
	return;
}

void do_inventory( CHAR_DATA *ch, char *argument )
{
	push_call("do_inventory(%p,%p)",ch,argument);

	send_to_char( "You are carrying:\n\r", ch );
	show_list_to_char( ch->first_carrying, ch, 0, TRUE );
	pop_call();
	return;
}

void do_equipment( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	int iWear;
	char buf[MAX_STRING_LENGTH], dim[10], bld[10];

	push_call("do_equipment(%p,%p)",ch,argument);

	strcpy(dim, get_color_string(ch, COLOR_OBJECTS, VT102_DIM));
	strcpy(bld, get_color_string(ch, COLOR_OBJECTS, VT102_BOLD));

	sprintf(buf, "You are using:\n\r");

	for (iWear = 0 ; iWear < MAX_WEAR ; iWear++)
	{
		if ((obj = get_eq_char(ch, iWear)) == NULL)
		{
			continue;
		}
		cat_sprintf(buf, "%s%s", dim, where_name[iWear]);

		if (can_see_obj(ch, obj))
		{
			cat_sprintf(buf, "%s%s\n\r", bld, format_obj_to_char(obj, ch, 0));
		}
		else
		{
			cat_sprintf(buf, "Something\n\r");
		}
	}
	send_to_char(buf, ch);

	if (strlen(buf) < 20)
	{
		send_to_char("Nothing.\n\r", ch);
	}
	pop_call();
	return;
}

void do_compare( CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *obj1;
	OBJ_DATA *obj2;
	int value, location;
	char *msg;

	push_call("do_compare(%p,%p)",ch,argument);

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0')
	{
		send_to_char("Compare what to what?\n\r", ch);
		pop_call();
		return;
	}

	if ((obj1 = get_obj_carry(ch, arg1)) == NULL)
	{
		send_to_char("You do not carry that item.\n\r", ch);
		pop_call();
		return;
	}

	if (*arg2 == 0)
	{
		for (location = 0 ; location < MAX_WEAR ; location++)
		{
			if (has_wear_location(obj1, location))
			{
				obj2 = get_eq_char(ch, location);

				if (obj2 && can_see_obj(ch, obj2))
				{
					break;
				}
			}
		}

		if (location == MAX_WEAR)
		{
			send_to_char( "You aren't wearing anything comparable.\n\r", ch);
			pop_call();
			return;
		}
	}
	else
	{
		obj2 = get_obj_here(ch, arg2);

		if (obj2 == NULL)
		{
			send_to_char( "You do not have that item.\n\r", ch );
			pop_call();
			return;
		}
	}

	msg   = NULL;
	value = 0;

	if (obj1 == obj2)
	{
		msg = "You compare $p to itself.  It looks about the same.";
	}
	else
	{
		value = compare_obj(ch, obj1, obj2 );

		if (value >= -1 && value <= 1)
		{
			msg = "$p and $P look about the same.";
		}
		else if (value  > 0)
		{
			if (value <= 5)
			{
				msg = "$p looks slightly better than $P.";
			}
			else if (value <= 10)
			{
				msg = "$p looks better than $P.";
			}
			else
			{
				msg = "$p looks a lot better than $P.";
			}
		}
		else
		{
			if (value >= -5)
			{
				msg = "$p looks slightly worse than $P.";
			}
			else if (value >= -10)
			{
				msg = "$p looks worse than $P.";
			}
			else
			{
				msg = "$p looks a lot worse than $P.";
			}
		}
 	}

	act( msg, ch, obj1, obj2, TO_CHAR );
	pop_call();
	return;
}

int compare_obj(CHAR_DATA *ch, OBJ_DATA *obj1, OBJ_DATA *obj2)
{
	int value1, value2;

	push_call("compare_obj(%p,%p)",obj1,obj2);

	value1 = obj_utility_level(ch, obj1);
	value2 = obj_utility_level(ch, obj2);

	pop_call();
	return value1 - value2;
}

void do_credits( CHAR_DATA *ch, char *argument )
{
	push_call("do_credits(%p,%p)",ch,argument);

	do_help( ch, "credits" );

	pop_call();
	return;
}

void do_where( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	PLAYER_GAME *fpl;

	push_call("do_where(%p,%p)",ch,argument);

	one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		strcpy(buf, "{168} Player      {138}  Location                                          {178} Leader\n\r");
		strcat(buf, "{118}--------------------------------------------------------------------------------\n\r");

		for (fpl = mud->f_player ; fpl ; fpl = fpl->next)
		{
			victim = fpl->ch;

			if (!can_see_world(ch, victim))
			{
				continue;
			}

			if (!can_hear(ch, victim))
			{
				continue;
			}

			if (victim->in_room->area != ch->in_room->area)
			{
				continue;
			}

			if (findpath_search_victim(ch, victim, 6) == -1)
			{
				continue;
			}

			cat_sprintf(buf, "{068} %-13s", victim->name);

			cat_snprintf(buf, 56, "{838} %-56s", victim->in_room->name);

			if (victim->leader != NULL && is_same_group(victim, victim->leader) && victim != victim->leader)
			{
				cat_sprintf(buf, "{878} %s\n\r", victim->leader->name);
			}
			else
			{
				cat_sprintf(buf, "{878} -\n\r");
			}
		}

		if (strlen(buf) < 190)
		{
			strcat(buf, "{178} None\n\r");
		}
		send_to_char_color(buf, ch);
		pop_call();
		return;
	}

	victim = findpath_search_name(ch, arg, 6);

	if (victim == NULL)
	{
		ch_printf(ch, "You didn't find '%s'.\n\r", arg);
		pop_call();
		return;
	}

	sprintf(buf, "%s\n\r", capitalize(PERS(victim, ch)));
	cat_sprintf(buf, "%s\n\r", victim->in_room->area->name);
	cat_sprintf(buf, "%s\n\r", victim->in_room->name);

	send_to_char(buf, ch);
	pop_call();
	return;
}

void do_consider( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim, *rch;
	char *msg;
	int diff, all;

	push_call("do_consider(%p,%p)",ch,argument);

	all = FALSE;

 	if (argument[0] == '\0')
	{
		send_to_char( "Consider killing whom?\n\r", ch );
		pop_call();
		return;
	}

	if ((victim = get_char_room(ch, argument)) == NULL)
	{
		if (strcasecmp(argument, "all"))
		{
			send_to_char( "They're not here.\n\r", ch );
			pop_call();
			return;
		}
		all = TRUE;
	}

	for (rch = ch->in_room->first_person ; rch ; rch = rch->next_in_room)
	{
		if (!can_see(ch, rch))
		{
			continue;
		}

		if (!all && victim != rch)
		{
			continue;
		}

		if (all && rch == ch)
		{
			continue;
		}

		victim = rch;

		diff = (victim->level - ch->level) * 10 / (10 + ch->level);

		if (diff <= -10)
		{
			msg = "You could kill $N naked and weaponless.";
		}
		else if (diff <= -5)
		{
			msg = "$N is no match for you.";
		}
		else if (diff <= -2)
		{
			msg = "$N looks like an easy kill.";
		}
		else if (diff <=  1)
		{
			msg = "$N looks like the perfect match.";
		}
		else if (diff <=  4)
		{
			msg = "$N says 'Do you feel lucky, punk?'.";
		}
		else if (diff <=  9)
		{
			msg = "$N laughs at you mercilessly.";
		}
		else if (diff <= 20)
		{
			msg = "$N could kill you naked and weaponless.";
		}
		else
		{
			msg = "Death will thank you for your gift.";
		}
		act( msg, ch, NULL, victim, TO_CHAR );
	}
	if (all && victim == NULL)
	{
		act("You are all by yourself here.", ch, NULL, NULL, TO_CHAR);
	}
	pop_call();
	return;
}

void do_know( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	char *msg;
	int aln;

	push_call("do_know(%p,%p)",ch,argument);

	one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		send_to_char( "Know the alignment of whom?\n\r", ch );
		pop_call();
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char( "You should know they are not here.\n\r", ch );
		pop_call();
		return;
	}

	if (!rspec_req(ch, RSPEC_KNOW_ALIGN))
	{
		act( "You see nothing special about $N.", ch, NULL, victim, TO_CHAR);
		pop_call();
		return;
	}

	aln = victim->alignment;

	if ( aln <= -800 )
		msg = "$N is a devil.";
	else if ( aln <= -600 )
		msg = "$N is very evil.";
	else if ( aln <= -400 )
		msg = "$N evil.";
	else if ( aln <= -200 )
		msg = "$N is slightly evil.";
	else if ( aln <=  0 )
		msg = "$N neutral on the evil side.";
	else if ( aln <=  200 )
		msg = "$N neutral on the good side.";
	else if ( aln <=  400 )
		msg = "$N is slightly good.";
	else if ( aln <=  600 )
		msg = "$N is good.";
	else if ( aln <=  800 )
		msg = "$N is very good.";
	else
		msg = "$N is an Angel.";

	act( msg, ch, NULL, victim, TO_CHAR );
	pop_call();
	return;
}

void set_title( CHAR_DATA *ch, char *title )
{
	char buf[MAX_INPUT_LENGTH];

	push_call("set_title(%p,%p)",ch,title);

	if (IS_NPC(ch))
	{
		bug( "Set_title: NPC.", 0 );
		pop_call();
		return;
	}

	if (TRUE || isalpha(title[0]) || isdigit(title[0]))
	{
		sprintf(buf, " %s", title);
	}
	else
	{
		sprintf(buf, "%s", title);
	}
	buf[60] = '\0';

	STRFREE (ch->pcdata->title );
	ch->pcdata->title = STRALLOC( buf );
	pop_call();
	return;
}

void do_email( CHAR_DATA *ch, char *argument )
{
	push_call("do_email(%p,%p)",ch,argument);

	if ( IS_NPC(ch) || !is_desc_valid( ch ))
	{
		pop_call();
		return;
	}

	if ( argument[0] == '\0' )
	{
		send_to_char( "Change your email address account name to?\n\r", ch );
		pop_call();
		return;
	}

	if (*argument == '.' && *(argument+1) == '\0')
	{
		STRFREE(ch->pcdata->mail_address );
		ch->pcdata->mail_address = STRALLOC( "" );
		send_to_char( "Email address cleared.\n\r", ch );
		pop_call();
		return;
	}

	argument[55] = '\0';

	smash_tilde( argument );
	STRFREE(ch->pcdata->mail_address );
	ch->pcdata->mail_address = STRALLOC( argument );
	ch_printf(ch, "Address set to: %s\n\r", argument );

	pop_call();
	return;
}

void do_html( CHAR_DATA *ch, char *argument )
{
	char *pt;

	push_call("do_html(%p,%p)",ch,argument);

	if (IS_NPC(ch) || !is_desc_valid(ch))
	{
		pop_call();
		return;
	}

	if (argument[0] == '\0')
	{
		send_to_char( "Change your html address to?\n\r", ch );
		pop_call();
		return;
	}

	if (*argument == '.' && *(argument+1) == '\0')
	{
		STRFREE(ch->pcdata->html_address );
		ch->pcdata->html_address = STRALLOC( "" );
		send_to_char( "HTML Home Page address cleared.\n\r", ch );
		pop_call();
		return;
	}

	argument[55] = '\0';

	ch_printf(ch, "Html address set to: %s\n\r", argument);

	for (pt = argument ; *pt != '\0' ; pt++)
	{
		if ((unsigned char) *pt == '~')
		{
			*pt = '*';
		}
	}

	STRFREE(ch->pcdata->html_address );
	ch->pcdata->html_address = STRALLOC( argument );

	pop_call();
	return;
}

void do_title( CHAR_DATA *ch, char *argument )
{
	push_call("do_title(%p,%p)",ch,argument);

	if ( IS_NPC(ch) )
	{
		pop_call();
		return;
	}

	if ( argument[0] == '\0' )
	{
		send_to_char( "Change your title to what?\n\r", ch );
		pop_call();
		return;
	}

	if (argument[0] == '.' && argument[1] == '\0')
	{
		set_title(ch, "");
		send_to_char("Title cleared.\n\r", ch);

		pop_call();
		return;
	}

	argument[60] = '\0';

	smash_tilde( argument );
	set_title( ch, argument );
	send_to_char( "Title set.\n\r", ch );

	pop_call();
	return;
}

void do_unalias( CHAR_DATA *ch, char *argument )
{
	int cnt;
	bool found = FALSE;

	push_call("do_unalias(%p,%p)",ch,argument);

	if (ch->desc == NULL)
	{
		pop_call();
		return;
	}

	smash_tilde( argument );

	if (argument[0] == '\0')
	{
		send_to_char( "Unalias what ?\n\r", ch);
		pop_call();
		return;
	}



	for (cnt = 0 ; cnt < MAX_ALIAS ; cnt++)
	{
		if (!strcasecmp(CH(ch->desc)->pcdata->alias_c[cnt], argument))
		{
			STRFREE(CH(ch->desc)->pcdata->alias[cnt]);
			CH(ch->desc)->pcdata->alias[cnt] = STRALLOC("");

			STRFREE(CH(ch->desc)->pcdata->alias_c[cnt]);
			CH(ch->desc)->pcdata->alias_c[cnt] = STRALLOC("");

			ch_printf(ch, "Alias %s removed.\n\r", argument );
			found = TRUE;
			pop_call();
			return;
		}
	}

	if (!found)
	{
		if (!strcasecmp(argument, "all"))
		{
			for (cnt = 0 ; cnt < MAX_ALIAS ; cnt++)
			{
				RESTRING(CH(ch->desc)->pcdata->alias[cnt], "");
				RESTRING(CH(ch->desc)->pcdata->alias_c[cnt], "");
			}
			ch_printf(ch, "All aliases removed.\n\r");
		}
		else
		{
			ch_printf(ch, "You don't have an alias for %s.\n\r", argument);
		}
	}
	pop_call();
	return;
}

/*
	Sort alphabetically   -  Chaos  4/4/99
*/

void sort_alias( CHAR_DATA *ch )
{
	int cnt1, cnt2;
	int acase;
	char *pt1, *pt2;

	push_call("sort_alias(%p)",ch);

	/* Use simple Sieve filter here */

	for (cnt1 = 0 ; cnt1 < MAX_ALIAS-2 ; cnt1++)
	{
		for (cnt2 = MAX_ALIAS-2 ; cnt2 >= cnt1 ; cnt2--)
		{
			acase = 0;
			if (ch->pcdata->alias[cnt2][0] == '\0')
			{
				if (ch->pcdata->alias[cnt2+1][0] != '\0')
				{
					acase = 1;
				}
				else
				{
					acase = 0;
				}
			}
			else if (ch->pcdata->alias[cnt2+1][0] == '\0')
			{
				acase = -1;
			}
			else
			{
				acase = strcasecmp(ch->pcdata->alias_c[cnt2], ch->pcdata->alias_c[cnt2+1]);
			}

			/* Swap */

			if (acase > 0)
			{
				pt1 = ch->pcdata->alias_c[cnt2];
				pt2 = ch->pcdata->alias  [cnt2];
				ch->pcdata->alias_c[cnt2]	= ch->pcdata->alias_c[cnt2+1];
				ch->pcdata->alias  [cnt2]	= ch->pcdata->alias  [cnt2+1];
				ch->pcdata->alias_c[cnt2+1]	= pt1;
				ch->pcdata->alias  [cnt2+1]	= pt2;
			}
		}
	}
	pop_call();
	return;
}

void do_alias( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char tbuf[MAX_STRING_LENGTH];
	char *pt, *ptt;
	int blen, tot, percent, cnt, found = 4096;
	bool match = FALSE, lowest = FALSE;

	push_call("do_alias(%p,%p)",ch,argument);

	/*
		Re-wrote most of this to get rid of the overrunning buffers and also
		to speed it up a little...Martin
	*/

	if (ch->desc == NULL)
	{
		pop_call();
		return;
	}

	/*
		Colorfull version -  Chaos 4/4/99
	*/

	if (argument[0] == '\0')
	{
		tot = 0;
		blen = str_cpy_max( buf, "{138}Alias list:\n\r", MAX_STRING_LENGTH);

		for (cnt = 0 ; cnt < MAX_ALIAS ; cnt++)
		{
			if (CH(ch->desc)->pcdata->alias[cnt][0] != '\0')
			{
				tot++;
				sprintf(tbuf, "{178}%-12s {038}%s\n\r", CH(ch->desc)->pcdata->alias_c[cnt], CH(ch->desc)->pcdata->alias[cnt] );
				blen = str_apd_max( buf, tbuf, blen, MAX_STRING_LENGTH);
			}
		}
		sprintf( tbuf, "{138}%d Aliases used out of %d available.\n\r", tot, MAX_ALIAS );
		blen = str_apd_max( buf, tbuf, blen, MAX_STRING_LENGTH);
		send_to_char( ansi_translate_text( ch, buf), ch );
		pop_call();
		return;
	}

	smash_tilde( argument );

	argument = one_argument_nolower(argument, buf);

	if (strlen(buf) > 12)
	{
		send_to_char( "You cannot make an alias command longer than 12 characters.\n\r", ch);
		pop_call();
		return;
	}

	for (pt = buf ; *pt != '\0' ; pt++)
	{
		if (!isalpha(*pt))
		{
			send_to_char( "You cannot make an alias command with non-letters.\n\r", ch);
			pop_call();
			return;
		}
	}

	if (argument[0] == '\0')
	{
		for (cnt = 0 ; cnt < MAX_ALIAS ; cnt++)
		{
			if (!strcasecmp(CH(ch->desc)->pcdata->alias_c[cnt], buf))
			{
				ch_printf(ch, "Use unalias to delete your alias for %s (currently %s).\n\r", buf, CH(ch->desc)->pcdata->alias[cnt]);
				pop_call();
				return;
			}
		}
		ch_printf(ch, "Alias %s to what ?\n\r", buf);
		pop_call();
		return;
	}

	if (strlen(argument) > 400)
	{
		send_to_char("You cannot make an alias longer than 400 characters.\n\r", ch);
		pop_call();
		return;
	}

	for (cnt = 0 ; cnt < MAX_ALIAS ; cnt++)
	{
		if (CH(ch->desc)->pcdata->alias[cnt][0] == '\0' && !match)
		{
			if (!lowest)
			{
				found  = cnt;
				lowest = TRUE;
			}
			continue;
		}
		if (!strcasecmp(CH(ch->desc)->pcdata->alias_c[cnt], buf))
		{
			found = cnt;
			match = TRUE;
		}
	}
	if (found != 4096)
	{
		/*
			prevents aliasses like
			alias boo chat %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
			there are people who _love_ to crash the mud thus, and now
			this is one of the answers. We're gonna cap the input length
			of the commands so that there will not happen that kinda things.
			Manwe,
			03/11/1999.
		*/

		for (percent = 0, ptt = argument ; *ptt != '\0' ; ptt++)
		{
			if (*ptt == '%')
			{
				percent++;
			}
			if (percent > 1)
			{
				send_to_char("You may only use one % for every &.\n\r", ch);
				log_printf("%s used an illegal alias: %s", ch->name, argument);
				pop_call();
				return;
			}
			if (*ptt == '&')
			{
				percent = 0;
			}
		}
		STRFREE (CH(ch->desc)->pcdata->alias[found] );
		STRFREE (CH(ch->desc)->pcdata->alias_c[found] );
		CH(ch->desc)->pcdata->alias_c[found] = STRALLOC(lower_all(buf));
		CH(ch->desc)->pcdata->alias[found]   = STRALLOC(argument);

		sort_alias( CH(ch->desc) );
		send_to_char( "Alias set.\n\r" , ch );
		pop_call();
		return;
	}
	send_to_char( "You reached your maximum amount of aliasses.\n\r", ch);
	pop_call();
	return;
}

void do_description( CHAR_DATA *ch, char *argument )
{
	push_call("do_description(%p,%p)",ch,argument);

	if (!ch->desc)
	{
		bug( "do_description: no descriptor", 0 );
		pop_call();
		return;
	}

	switch( ch->pcdata->editmode )
	{
		default:
			bug( "do_description: illegal editmode", 0 );
			pop_call();
			return;

		case MODE_RESTRICTED:
			send_to_char( "You cannot use this command from while editing something else.\n\r",ch);
			pop_call();
			return;

		case MODE_NONE:
			ch->pcdata->editmode = MODE_PERSONAL_DESC;
			ch->pcdata->tempmode  = MODE_NONE;
			start_editing( ch, ch->description );
			pop_call();
			return;

		case MODE_PERSONAL_DESC:
			STRFREE (ch->description );
			ch->description = copy_buffer( ch );
			stop_editing( ch );
			pop_call();
			return;
	}
	pop_call();
	return;
}

void do_report( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_INPUT_LENGTH];

	push_call("do_report(%p,%p)",ch,argument);

	sprintf( buf, "You report: %d/%d hp %d/%d mana %d/%d mv %d xp.\n\r",
		ch->hit,  ch->max_hit,
		ch->mana, ch->max_mana,
		ch->move, ch->max_move,
		IS_NPC(ch) ? 0 : ch->pcdata->exp);

	send_to_char( buf, ch );

	sprintf( buf, "$n reports: %d/%d hp %d/%d mana %d/%d mv %d xp.",
		ch->hit,  ch->max_hit,
		ch->mana, ch->max_mana,
		ch->move, ch->max_move,
		IS_NPC(ch) ? 0 : ch->pcdata->exp);

	act( buf, ch, NULL, NULL, TO_ROOM );

	pop_call();
	return;
}


void do_practice( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char skp[MAX_INPUT_LENGTH];
	int sn, skill_class, mob_level, cnt, cost, practice, adept;

	push_call("do_practice(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (argument[0] == '\0')
	{
		char colw[10], colG[10], colC[10];
		int col, lev;
		char tstc;

		buf[0] = '\0';

		strcpy(colw, ansi_translate_text(ch, "{078}"));
		strcpy(colG, ansi_translate_text(ch, "{128}"));
		strcpy(colC, ansi_translate_text(ch, "{168}"));

		for (col = 0, lev = 1 ; lev <= ch->level ; lev++)
		{
			for (sn = 0 ; sn < MAX_SKILL ; sn++)
			{
				if (skill_table[sn].name == NULL)
				{
					break;
				}

				if ((cnt = multi_pick(ch, sn)) == -1)
				{
					continue;
				}

				if (skill_table[sn].skill_level[cnt] != lev)
				{
					continue;
				}

				if (skill_table[sn].follower != SFOL_NONE)
				{
					sh_int follow = 0;

					switch(ch->pcdata->god)
					{
						case GOD_NEUTRAL:   follow = SFOL_NONE;   break;
						case GOD_CHAOS:     follow = SFOL_CHAOS;  break;
						case GOD_ORDER:     follow = SFOL_ORDER;  break;
						default:            follow = SFOL_NONE;   break;
					}

					if (!HAS_BIT(skill_table[sn].follower, follow))
					{
						continue;
					}
				}

				adept = class_table[cnt].skill_master;

				if (ch->pcdata->mclass[ch->class] < skill_table[sn].skill_level[ch->class])
				{
					tstc = '*'; /* outside current class ... */
				}
				else
				{
					tstc = ' ';
				}
				cat_sprintf(buf, "%s%s%s%c%s%2d%s/%s%2d%s%%",
				colw, str_resize(skill_table[sn].name, skp, 19),
				colG, tstc,
				colC, ch->pcdata->learned[sn], colw,
				colC, adept, colw);

				if (++col % 3 == 0)
				{
					strcat(buf, "\n\r");
				}
			}
		}
		if (col % 3 != 0)
		{
			strcat(buf, "\n\r");
		}

		cat_sprintf(buf, "\n\r%sYou have %s%d%s practice sessions left.         %s* %sOutside current class.\n\r",
			colw, colC, ch->pcdata->practice, colw, colG, colw);

		send_to_char(buf, ch);
	}
	else
	{
		CHAR_DATA *mob;

		if (!IS_AWAKE(ch))
		{
			send_to_char( "In your dreams, or what?\n\r", ch );
			pop_call();
			return;
		}

		for ( mob = ch->in_room->first_person ; mob ; mob = mob->next_in_room )
		{
			if (IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE))
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

		if ((sn = skill_lookup(argument)) < 0)
		{
			send_to_char( "There is no such skill or spell.\n\r", ch );
			pop_call();
			return;
		}

		if (skill_table[sn].follower != SFOL_NONE)
		{
			sh_int follow = 0;

			switch (ch->pcdata->god)
			{
				case GOD_NEUTRAL:   follow = SFOL_NONE;   break;
				case GOD_CHAOS:     follow = SFOL_CHAOS;  break;
				case GOD_ORDER:     follow = SFOL_ORDER;  break;
				default:            follow = SFOL_NONE;   break;
			}

			if (!HAS_BIT(skill_table[sn].follower, follow))
			{
				send_to_char( "You can't practice that.\n\r",ch);
				pop_call();
				return;
			}
		}

		if (HAS_BIT(mob->act, ACT_CLASS))
		{
			skill_class = mob->class;

			if (skill_table[sn].skill_level[skill_class] > 95)
			{
				ch_printf(ch, "%s can only teach you %s abilities.\n\r", get_name(mob), class_table[mob->class].name);

				pop_call();
				return;
			}

			mob_level = mob->pIndexData->level;
		}
		else
		{
			if ((skill_class = multi_pick(ch , sn)) == -1)
			{
				send_to_char( "You can't practice that.\n\r", ch );
				pop_call();
				return;
			}
			mob_level = mob->pIndexData->level / 2;
		}

		if (ch->pcdata->mclass[skill_class] < skill_table[sn].skill_level[skill_class])
		{
			send_to_char("You can't practice that.\n\r", ch);

			pop_call();
			return;
		}

		if (mob_level < skill_table[sn].skill_level[skill_class])
		{
			ch_printf(ch, "%s is too inexperienced to teach you %s.\n\r", capitalize(get_name(mob)), skill_table[sn].name);

			pop_call();
			return;
		}

		if (ch->pcdata->learned[sn] >= class_table[skill_class].skill_master)
		{
			ch_printf(ch, "You are already a master of %s.\n\r", skill_table[sn].name);

			pop_call();
			return;
		}

		practice = ch->pcdata->mclass[ch->class] >= skill_table[sn].skill_level[ch->class] ? 1 : 2;

		if (ch->pcdata->practice < practice)
		{
			send_to_char( "You don't have enough practice sessions left.\n\r", ch );

			pop_call();
			return;
		}

		cost = skill_table[sn].skill_level[skill_class] * skill_table[sn].skill_level[skill_class];

		if (ch->gold < cost)
		{
			ch_printf(ch, "You need %d gold coins to practice %s.\n\r", cost, skill_table[sn].name);

			pop_call();
			return;
		}

		ch->pcdata->practice -= practice;

		ch->gold -= cost;

		if (ch->pcdata->learned[sn] >= class_table[skill_class].skill_adept)
		{
			ch->pcdata->learned[sn] += 1;

			if (ch->pcdata->learned[sn] < class_table[skill_class].skill_master )
			{
				act( "You are already an adept of $T, but you practice some more.", ch, NULL, skill_table[sn].name, TO_CHAR );
				act( "$n practices $T.", ch, NULL, skill_table[sn].name, TO_ROOM );
			}
			else
			{
				act( "You are now a master of $T.", ch, NULL, skill_table[sn].name, TO_CHAR );
				act( "$n is now a master of $T.", ch, NULL, skill_table[sn].name, TO_ROOM );
			}
		}
		else
		{
			ch->pcdata->learned[sn] += int_app[get_curr_int(ch)].learn;

			if (ch->pcdata->learned[sn] < class_table[skill_class].skill_adept)
			{
				act( "You practice $T.", ch, NULL, skill_table[sn].name, TO_CHAR );
				act( "$n practices $T.", ch, NULL, skill_table[sn].name, TO_ROOM );
			}
			else
			{
				ch->pcdata->learned[sn] = class_table[skill_class].skill_adept;

				act( "You are now an adept of $T.", ch, NULL, skill_table[sn].name, TO_CHAR );
				act( "$n is now an adept of $T.", ch, NULL, skill_table[sn].name, TO_ROOM );
			}
		}
	}
	pop_call();
	return;
}

/*
	'Wimpy' originally by Dionysos.
*/

void do_wimpy( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	int wimpy;

	push_call("do_wimpy(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		wimpy = ch->pcdata->wimpy;
	}
	else
	{
		if (is_affected(ch, gsn_fear))
		{
			send_to_char("You are too frightened to do that.\n\r", ch);
			pop_call();
			return;
		}
		if (is_affected(ch, gsn_remove_fear))
		{
			send_to_char("You are too fearless to do that.\n\r", ch);
			pop_call();
			return;
		}
		wimpy = atol( arg );
	}

	if (wimpy < 0)
	{
		send_to_char( "Your courage exceeds your wisdom.\n\r", ch );
		pop_call();
		return;
	}

	if (wimpy > ch->max_hit / 2)
	{
		send_to_char( "Such cowardice ill becomes you.\n\r", ch );
		pop_call();
		return;
	}

	ch->pcdata->wimpy = wimpy;
	ch_printf(ch, "Wimpy is set to %d hit points.\n\r", wimpy );
	pop_call();
	return;
}

void do_password( CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];

	push_call("do_password(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	argument = one_argument_nolower(argument, arg1);
	argument = one_argument_nolower(argument, arg2);
	argument = one_argument_nolower(argument, arg3);

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
	{
		send_to_char( "Syntax: password <old> <new> <new>.\n\r", ch );
		pop_call();
		return;
	}

	if (strcmp(arg2, arg3))
	{
		send_to_char( "Your new password does not verify.\n\rPlease try again.\n\r", ch);
		pop_call();
		return;
	}

	if (encrypt64(arg1) != ch->pcdata->password)
	{
		send_to_char( "Wrong password.  Try again.\n\r", ch );
		pop_call();
		return;
	}

	if (strlen(arg2) < 5)
	{
		send_to_char("New password must be at least five characters long.\n\r", ch );
		pop_call();
		return;
	}

	/*
		No tilde allowed because of player file format.
	*/

	if (!is_valid_password(arg2))
	{
		send_to_char("New password not acceptable, try again.\n\r", ch);
		send_to_char("The password must only contain letters (case sensitive), or numbers.\n\r", ch);
		send_to_char("You are required to include at least one number in the password.\n\r", ch);
		pop_call();
		return;
	}

	ch->pcdata->password = encrypt64(arg2);

	save_char_obj(ch, NORMAL_SAVE);
	send_to_char( "Password changed.\n\r", ch );
	pop_call();
	return;
}

/*
	Lets fool the noobs, rewritten - Scandum 26-05-2002
*/

void do_skills( CHAR_DATA *ch, char *argument )
{
	char buf1[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	int sn, col, cnt, lev, cls;

	push_call("do_skills(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		log_printf("do_skills(%s, %s)", get_name(ch), argument);
		pop_call();
		return;
	}

	for (cnt = 0, cls = ch->class ; cnt < MAX_CLASS ; cnt++)
	{
		if (argument[0] != '\0' && !str_prefix(argument, class_table[cnt].name))
		{
			cls = cnt;
		}
	}

	sprintf(buf1, "{078}");
	sprintf(buf2, "{178} %s {078}", class_table[cls].name);

	for (cnt = 0 ; cnt < 39-strlen(class_table[cls].name)/2 ; cnt++)
	{
		strcat(buf1, "-");
	}
	strcat(buf1, buf2);

	for (cnt = ansi_strlen(buf1) ; cnt < 80 ; cnt++)
	{
		strcat(buf1, "-");
	}
	ch_printf_color(ch, "%s\n\r", buf1);

	for (lev = col = 0, buf1[0] = '\0' ; lev < LEVEL_HERO ; lev++)
	{
		for (sn = 0 ; sn < MAX_SKILL ; sn++)
		{
			if (skill_table[sn].name != NULL)
			{
				if (skill_table[sn].skill_level[cls] == lev)
				{
					if (skill_table[sn].follower == SFOL_NONE)
					{
						sprintf(buf2, "{078}%2d-{178}%-23s", lev, skill_table[sn].name);
					}
					else if (IS_SET(skill_table[sn].follower, SFOL_CHAOS) && ch->pcdata->god == GOD_CHAOS)
					{
						sprintf(buf2, "{078}%2d-{118}%-23s", lev, skill_table[sn].name);
					}
					else if (IS_SET(skill_table[sn].follower, SFOL_ORDER) && ch->pcdata->god == GOD_ORDER)
					{
						sprintf(buf2, "{078}%2d-{168}%-23s", lev, skill_table[sn].name);
					}
					else if (IS_SET(skill_table[sn].follower, SFOL_DUNNO) && IS_GOD(ch))
					{
						sprintf(buf2, "{078}%2d-{128}%-23s", lev, skill_table[sn].name);
					}
					else
					{
						continue;
					}
					if (++col % 3 != 1)
					{
						strcat(buf1, " ");
					}
					strcat(buf1, buf2);

					if (col % 3 == 0)
					{
						ch_printf_color(ch, "%s\n\r", buf1);
						buf1[0] = '\0';
					}
				}
			}
		}
	}
	if (col % 3 != 0)
	{
		ch_printf_color(ch, "%s\n\r", buf1);
	}
	pop_call();
	return;
}

/*
	Contributed by Alander.
*/

void do_commands( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	int cmd, col;

	push_call("do_commands(%p,%p)",ch,argument);

	for (buf[0] = '\0', cmd = col = 0 ; cmd_table[cmd].name[0] != '\0' ; cmd++)
	{
		if (cmd_table[cmd].level < LEVEL_HERO && cmd_table[cmd].level <= get_trust(ch))
		{
			if (IS_SET(cmd_table[cmd].flags, CMD_HIDE))
			{
				continue;
			}
			if (IS_AFFECTED(ch, AFF2_BERSERK) && !IS_SET(cmd_table[cmd].flags, CMD_BERSERK))
			{
				continue;
			}

			if (col % 6 == 0)
			{
				strcat(buf, get_color_string(ch, COLOR_TEXT, VT102_DIM));
			}
			cat_snprintf(buf, 13, "%-13s", cmd_table[cmd].name);
			if (++col % 6 == 0)
			{
				strcat(buf, "\n\r");
			}
		}
	}
	if (col % 6 != 0)
	{
		strcat(buf, "\n\r");
	}
	send_to_char(buf, ch);
	pop_call();
	return;
}

typedef struct chan_list CHAN_LIST;
struct chan_list
{
	CHAR_DATA *ch;
	CHAN_LIST *next;
};

/*  By Chaos   9/20/93   */

void do_level ( CHAR_DATA *ch, char *argument )
{
	int lvn, lvx, cnt, ld;

	push_call("do_level(%p,%p)",ch,argument);

	if( ch->level >= 95 )
	{
		send_to_char( "You are already at the maximum level.\n\r", ch );
		pop_call();
		return;
	}

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	lvn = ch->pcdata->mclass[ch->class]-3;
	lvx = ch->pcdata->mclass[ch->class]+3;
	ld  = ch->level - ch->pcdata->mclass[ch->class];

	if (lvn < 0)
	{
		lvn = 0;
	}
	if (ld == 0 && lvn < 1)
	{
		lvn = 1;
	}
	if (lvx + ld > MAX_LEVEL)
	{
		lvx = MAX_LEVEL - ld;
	}
	if (lvx > 95)
	{
		lvx = 95;
	}
	send_to_char( "Level - Experience Required    - Title\n\r", ch);
	for (cnt = lvn ; cnt <= lvx ; cnt++)
	{
		if (ch->level - ld == cnt)
		{
			ch_printf(ch, " You  - %12d\n\r", ch->pcdata->exp);
		}
		ch_printf(ch, " %3d  - %12d\n\r", cnt+1+ld, exp_level(ch->class,cnt+ld));
	}
	if (ch->level < MAX_LEVEL)
	{
		ch_printf(ch, " You need %d experience points to gain a level.\n\r", exp_level(ch->class, ch->level) - ch->pcdata->exp);
	}
	pop_call();
	return;
}

/* By Chaos 9/20/93  */

void do_clock( CHAR_DATA *ch, char *arg)
{
	char buf[MAX_STRING_LENGTH];
	int clh, clm;

	push_call("do_clock(%p,%p)",ch,arg);

	if( ch->desc->original != NULL)
	{
		pop_call();
		return;
	}

	if( IS_NPC(ch) )
	{
		pop_call();
		return;
	}

	one_argument( arg, buf);
	if (buf[0]=='\0')
	{
		send_to_char( "Usage:   clock <offset>\n\rThis sets the clock to your local time zone.\n\r", ch );
		send_to_char( "         clock mil  - This sets the clock to military time.\n\r" , ch);
		send_to_char( "         clock civ  - This sets the clock to normal am/pm mode.\n\r", ch);

		pop_call();
		return;
	}

	if (!strcasecmp(buf, "mil"))
	{
		ch->pcdata->clock = ch->pcdata->clock % 100 + 100;
		pop_call();
		return;
	}

	if (!strcasecmp(buf, "civ"))
	{
		ch->pcdata->clock = ch->pcdata->clock % 100;
		pop_call();
		return;
	}

	if (!strcasecmp(buf, "cnt"))
	{
		ch->pcdata->clock = ch->pcdata->clock % 100 + 1000;
		pop_call();
		return;
	}

	clh = atol(buf);
	clm = 0;

	if (!is_number(buf))
	{
		sscanf(buf, "%d:%d", &clh, &clm);
	}

	while (clh < 0)
	{
		clh += 24;
	}
	clh = clh % 24;

	while (clm < 0)
	{
		clm += 24;
	}
	clm = clm % 60;

	ch->pcdata->clock = (ch->pcdata->clock / 100) * 100 + clh;
	ch->pcdata->clock = (ch->pcdata->clock % 10000) + clm * 10000;

	pop_call();
	return;
}

void do_class( CHAR_DATA *ch, char *arg)
{
	int cnt;
	char buf[MAX_INPUT_LENGTH];

	push_call("do_class(%p,%p)",ch,arg);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (ch->level % 10 != 0)
	{
		send_to_char("You cannot switch classes now.\n\r", ch );
		pop_call();
		return;
	}

	if (is_affected(ch, gsn_polymorph))
	{
		send_to_char("You may not switch classes in polymorphed form.\n\r", ch);
		pop_call();
		return;
	}

	arg = one_argument_nolower( arg, buf);

	if (buf[0] == '\0')
	{
		strcpy(buf,"You may switch to the following classes: ");

		for (cnt = 0 ; cnt < MAX_CLASS ; cnt++)
		{
			if (race_table[ch->race].race_class[cnt] == 1)
			{
				strcat(buf," ");
				strcat(buf,class_table[cnt].who_name);
			}
		}
		strcat(buf,"\n\r");
		send_to_char( buf, ch);
		pop_call();
		return;
	}

	if (encrypt64(arg) != ch->pcdata->password)
	{
		send_to_char( "You must enter your password after the class name.\n\r",ch);
		pop_call();
		return;
	}

	for (cnt = 0 ; cnt < MAX_CLASS ; cnt++)
	{
		if (!strcasecmp(buf, class_table[cnt].who_name) && race_table[ch->race].race_class[cnt] == 1)
		{
			ch_printf(ch, "Switching class to: %s\n\r", class_table[cnt].who_name);
			ch->class             = cnt;
			ch->pcdata->exp       = exp_level(cnt, ch->level-1)+1;
			ch->pcdata->practice /= 2;

			pop_call();
			return;
		}
	}
	send_to_char("You cannot switch to that class.\n\r", ch);
	pop_call();
	return;
}


void do_config( CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char *arg3;
	char buf[MAX_STRING_LENGTH];
	char bufx[MAX_INPUT_LENGTH];
	int  cnt;

	push_call("do_config(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	argument	= one_argument( argument, arg1 );
	argument	= one_argument( argument, arg2 );
	arg3		= argument;

	if (arg1[0] == '-' || arg1[0] == 'x')
	{
		ch_printf_color(ch, "\n\r{138}Configuration finished.\n\r");

		if (ch->pcdata->last_command)
		{
			STRFREE(ch->pcdata->last_command);
		}
		pop_call();
		return;
	}

	/*
		General Options
	*/

	if (*arg1 == 'a')
	{
		switch (*arg2)
		{
			case 'a':
				TOGGLE_BIT(ch->act, PLR_AUTOEXIT);
				break;
			case 'b':
				TOGGLE_BIT(ch->act, PLR_QUIET);
				break;
			case 'c':
				TOGGLE_BIT(ch->act, PLR_BLANK);
				break;
			case 'd':
				TOGGLE_BIT(ch->act, PLR_BRIEF);
				break;
			case 'e':
				TOGGLE_BIT(ch->act, PLR_PAGER);
				break;
			case 'f':
				TOGGLE_BIT(ch->act, PLR_REPEAT);
				break;
			case 'g':
				TOGGLE_BIT(ch->act, PLR_EXITPOS);
				break;
			case '\0':
				break;
			case 'x':
				do_config(ch, "x");
				pop_call();
				return;
			case '-':
				if (ch->pcdata->last_command)
				{
					STRFREE(ch->pcdata->last_command);
				}
				do_config(ch, "");
				pop_call();
				return;
			default:
				ch_printf_color(ch, "{118}'%c' Is not an option.\n\r", *arg2);

				RESTRING(ch->pcdata->last_command, "config A ");
				pop_call();
				return;
		}

		do_refresh(ch, "");

		strcpy( buf, "{138}      Display Setup Menu\n\r\n\r");

		strcat(  buf, IS_SET(ch->act, PLR_AUTOEXIT)
			? "{158}  (A) {128}[ ON  ] {018}[AUTOEXIT] {038}You automatically see exits.\n\r"
			: "{158}  (A) {028}[ OFF ] {018}[autoexit] {038}You don't automatically see exits.\n\r" );

		strcat(  buf, IS_SET(ch->act, PLR_QUIET)
			? "{158}  (B) {128}[ ON  ] {018}[QUIET   ] {038}You don't see items that spill out of corpses.\n\r"
			: "{158}  (B) {028}[ OFF ] {018}[quiet   ] {038}You see items that spill out of corpses.\n\r");

		strcat( buf,  IS_SET(ch->act, PLR_BLANK)
			? "{158}  (C) {128}[ ON  ] {018}[BLANK   ] {038}You have a blank line before your prompt.\n\r"
			: "{158}  (C) {028}[ OFF ] {018}[blank   ] {038}You have no blank line before your prompt.\n\r" );

		strcat( buf,  IS_SET(ch->act, PLR_BRIEF)
			? "{158}  (D) {128}[ ON  ] {018}[BRIEF   ] {038}You see brief descriptions.\n\r"
			: "{158}  (D) {028}[ OFF ] {018}[brief   ] {038}You see long descriptions.\n\r" );

		strcat( buf,  !IS_SET(ch->act, PLR_PAGER)
			? "{158}  (E) {128}[ ON  ] {018}[PAGER   ]{038} You are using the page pauser.\n\r"
			: "{158}  (E) {028}[ OFF ] {018}[pager   ] {038}You are not using the page pauser.\n\r" );

		strcat( buf,  IS_SET(ch->act, PLR_REPEAT)
			? "{158}  (F) {128}[ ON  ] {018}[REPEAT  ] {038}You see what you've typed in.\n\r"
			: "{158}  (F) {028}[ OFF ] {018}[repeat  ] {038}You don't see what you type in.\n\r");

		strcat( buf,  IS_SET(ch->act, PLR_EXITPOS)
			? "{158}  (G) {128}[ ON  ] {018}[EXITPOS ] {038}Auto exits are above the room description.\n\r"
			: "{158}  (G) {028}[ OFF ] {018}[exitpos ] {038}Auto exits are below the room description.\n\r");

		strcat( buf, "\n{158}  (-) {028}Return\n\r" );

		send_to_char_color(buf, ch );

		RESTRING(ch->pcdata->last_command, "config A ");

		pop_call();
		return;
	}

	/*
		Terminal configuration
	*/

	if (arg1[0] == 'b')
	{
		switch (*arg2)
		{
			case 'a':
				if (!HAS_BIT(ch->pcdata->vt100_flags, VT100_INTERFACE))
				{
					vt100on(ch);
				}
				else
				{
					vt100off(ch);
				}
				break;
			case 'b':
				if (*arg3 == '\0')
				{
					ch_printf_color(ch, "\n\r{128}Enter amount of terminal lines:\n\r");

					STRFREE( ch->pcdata->last_command );
					ch->pcdata->last_command = STRALLOC( "config B B " );
					pop_call();
					return;
				}
				cnt = atol(arg3);
				if (cnt < 15 || cnt > 200)
				{
					ch_printf_color(ch, "\n\r{118}Terminal rows must be between 15 and 200.\n\r" );

					STRFREE( ch->pcdata->last_command );
					ch->pcdata->last_command = STRALLOC( "config B B " );

					pop_call();
					return;
				}
				ch->pcdata->screensize_v = cnt;
				break;
			case 'c':
				if (*arg3 == '\0')
				{
					ch_printf_color(ch, "\n\r{128}Enter amount of terminal columns:\n\r");

					STRFREE( ch->pcdata->last_command );
					ch->pcdata->last_command = STRALLOC( "config B C " );
					pop_call();
					return;
				}
				cnt = atol(arg3);
				if (cnt < 80 || cnt > MAX_TACTICAL_COL)
				{
					ch_printf_color(ch, "\n\r{118}Terminal columns must be between 80 and %d.\n\r", MAX_TACTICAL_COL );

					STRFREE( ch->pcdata->last_command );
					ch->pcdata->last_command = STRALLOC( "config B C " );

					pop_call();
					return;
				}
				ch->pcdata->screensize_h = cnt;
				break;
			case 'd':
				TOGGLE_BIT(ch->pcdata->vt100_flags, VT100_FAST);
				break;
			case 'e':
				TOGGLE_BIT(ch->pcdata->vt100_flags, VT100_HIGHLIGHT);
				break;
			case 'f':
				TOGGLE_BIT(ch->pcdata->vt100_flags, VT100_BEEP);
				break;
			case 'g':
				TOGGLE_BIT(ch->pcdata->vt100_flags, VT100_ECMA48);
				break;
			case 'h':
				TOGGLE_BIT(ch->pcdata->vt100_flags, VT100_BOLD);
				break;
			case 'i':
				TOGGLE_BIT(ch->pcdata->vt100_flags, VT100_UNDERLINE);
				break;
			case 'j':
				TOGGLE_BIT(ch->pcdata->vt100_flags, VT100_FLASH);
				break;
			case 'k':
				TOGGLE_BIT(ch->pcdata->vt100_flags, VT100_REVERSE);
				break;
			case '\0':
				break;
			case 'x':
				do_config(ch, "x");
				pop_call();
				return;
			case '-':
				STRFREE( ch->pcdata->last_command );
				do_config( ch, "" );
				pop_call();
				return;
			default:
				ch_printf_color(ch, "\n\r{118}'%c' Is not an option.\n\r", *arg2 );

				RESTRING(ch->pcdata->last_command, "config B ");

				pop_call();
				return;
		}
		do_refresh(ch, "");

		strcpy( buf, "{138}      Terminal Setup Menu\n\r\n\r");
		strcat( buf, HAS_BIT(ch->pcdata->vt100_flags, VT100_INTERFACE)
			? "{158}  (A) {128}[ ON  ]  {038}VT100 mode is enabled.\n\r"
			: "{158}  (A) {028}[ OFF ]  {038}VT100 mode is disabled.\n\r" );

		cat_sprintf(buf, "{158}  (B) {128}[ %-3d ]  {038}VT100 terminal lines.\n\r",  ch->pcdata->screensize_v);
		cat_sprintf(buf, "{158}  (C) {128}[ %-3d ]  {038}VT100 terminal columns.\n\r", ch->pcdata->screensize_h);

		strcat( buf, IS_SET(ch->pcdata->vt100_flags, VT100_FAST)
			? "{158}  (D) {128}[ ON  ]  {038}VT100 speed is set to FAST.\n\r"
			: "{158}  (D) {028}[ OFF ]  {038}VT100 speed is set to SLOW.\n\r" );

		strcat( buf, IS_SET(ch->pcdata->vt100_flags, VT100_HIGHLIGHT)
			? "{158}  (E) {128}[ ON  ]  {038}'YOU' highlighting is enabled.\n\r"
			: "{158}  (E) {028}[ OFF ]  {038}'YOU' highlighting is disabled.\n\r" );

		strcat( buf, IS_SET(ch->pcdata->vt100_flags, VT100_BEEP)
			? "{158}  (F) {128}[ ON  ]  {038}VT100 Terminal Bell is enabled.\n\r"
			: "{158}  (F) {028}[ OFF ]  {038}VT100 Terminal Bell is disabled.\n\r");

		strcat( buf, IS_SET(ch->pcdata->vt100_flags, VT100_ECMA48)
			? "{158}  (G) {128}[ ON  ]  {038}ISO48 is enabled.\n\r"
			: "{158}  (G) {028}[ OFF ]  {038}ISO48 is disabled.\n\r");

		strcat( buf, IS_SET(ch->pcdata->vt100_flags, VT100_BOLD)
			? "{158}  (H) {128}[ ON  ]  {038}VT100 Bolded Text is enabled.\n\r"
			: "{158}  (H) {028}[ OFF ]  {038}VT100 Bolded Text is disabled.\n\r");

		strcat( buf, IS_SET(ch->pcdata->vt100_flags, VT100_UNDERLINE)
			? "{158}  (I) {128}[ ON  ]  {038}VT100 Underlined Text is enabled.\n\r"
			: "{158}  (I) {028}[ OFF ]  {038}VT100 Underlined Text is disabled.\n\r");

		strcat( buf, IS_SET(ch->pcdata->vt100_flags, VT100_FLASH)
			? "{158}  (J) {128}[ ON  ]  {038}VT100 Flashing Text is enabled.\n\r"
			: "{158}  (J) {028}[ OFF ]  {038}VT100 Flashing Text is disabled.\n\r");

		strcat( buf, IS_SET(ch->pcdata->vt100_flags, VT100_REVERSE)
			? "{158}  (K) {128}[ ON  ]  {038}VT100 Reversed Text is enabled.\n\r"
			: "{158}  (K) {028}[ OFF ]  {038}VT100 Reversed Text is disabled.\n\r");

		strcat( buf, "\n\r{158}  (-) {028}Return\n\r" );

		send_to_char( ansi_translate_text( ch, buf ), ch );

		STRFREE( ch->pcdata->last_command );
		ch->pcdata->last_command = STRALLOC( "config B " );

		pop_call();
		return;
	}

	/* Tactical configuration */

	if (arg1[0] == 'c')
	{
		switch( *arg2 )
		{
			case 'a':
				if (*arg3 == '\0')
				{
					ch_printf_color(ch, "\n\r{128}Enter amount of tactical lines: (1-%d)\n\r", MAX_TACTICAL_ROW);

					STRFREE( ch->pcdata->last_command );
					ch->pcdata->last_command = STRALLOC( "config C A " );

					pop_call();
					return;
				}

				cnt = atol(arg3);

				if (cnt < 1 || cnt > MAX_TACTICAL_ROW)
				{
					ch_printf_color(ch, "\n\r{118}Tactical rows must be between 1 and %d.\n\r", MAX_TACTICAL_ROW );
					STRFREE( ch->pcdata->last_command );
					ch->pcdata->last_command = STRALLOC( "config C A " );
					pop_call();
					return;
				}
				ch->pcdata->tactical_size_v = cnt;
				break;

			case 'b':
				if (*arg3 == '\0')
				{
					ch_printf_color(ch, "\n\r{128}Enter compass width: (5-15, or 0 for OFF)\n\r" );

					STRFREE( ch->pcdata->last_command );
					ch->pcdata->last_command = STRALLOC( "config C B " );

					pop_call();
					return;
				}
				if ((cnt = atol(arg3)) != 0)
				{
					if (cnt < 5 || cnt > 15)
					{
						ch_printf_color(ch, "\n\r{118}Compass width must be between 5 and 15, or 0 to turn off.\n\r");

						STRFREE( ch->pcdata->last_command );
						ch->pcdata->last_command = STRALLOC( "config C B " );

						pop_call();
						return;
					}
				}
				ch->pcdata->tactical_size_c = cnt;
				break;

			case 'c':
				TOG_BIT(ch->pcdata->tactical_flags, TACTICAL_INDEX);
				break;

			case 'd':
				TOG_BIT(ch->pcdata->tactical_flags, TACTICAL_TOP);
				break;

			case 'e':
				if (*arg3 == '\0')
				{
					ch_printf_color(ch, "\n\r{128}Enter your two character index:\n\r" );

					STRFREE( ch->pcdata->last_command );
					ch->pcdata->last_command = STRALLOC( "config C E " );
					pop_call();
					return;
				}
				if (arg3[0] <= ' ' || arg3[0] > 'z' || arg3[1] <= ' ' || arg3[1] > 'z')
				{
					ch_printf_color(ch, "\n\r{118}That is an incorrect index.\n\rChoose a two character index:\n\r");

					STRFREE( ch->pcdata->last_command );
					ch->pcdata->last_command = STRALLOC( "config B E " );

					pop_call();
					return;
				}
				if (arg3[0] == '~')	arg3[0] = '-';
				if (arg3[1] == '~') arg3[1] = '-';

				bufx[0] = arg3[0];
				bufx[1] = arg3[1];
				bufx[2] = '\0';

				STRFREE(ch->pcdata->tactical_index);
				ch->pcdata->tactical_index = STRALLOC(bufx);

				break;

			case 'f':
				TOG_BIT(ch->pcdata->tactical_flags, TACTICAL_EXPTNL);
				break;

			case '\0':
				break;

			case 'x':
				do_config(ch, "x");
				pop_call();
				return;

			case '-':
				STRFREE( ch->pcdata->last_command );
				do_config( ch, "" );
				pop_call();
				return;

			default:
				ch_printf_color(ch, "\n\r{118}'%c' Is not an option.\n\r", *arg2 );

				STRFREE( ch->pcdata->last_command );
				ch->pcdata->last_command = STRALLOC( "config C " );

				pop_call();
				return;
		}
		do_refresh(ch, "");

		strcpy( buf, "{138}      Tactical Setup Menu\n\r\n\r");

		cat_sprintf(buf, "{158}  (A) {128}[ %-3d ]  {038}Tactical terminal lines.\n\r", ch->pcdata->tactical_size_v);

		if (ch->pcdata->tactical_size_c < 4)
		{
			cat_sprintf(buf, "{158}  (B) {028}[ OFF ]  {038}Your tactical compass is off.\n\r" );
		}
		else
		{
			cat_sprintf(buf, "{158}  (B) {128}[ %-3d ]  {038}Tactical compass width.\n\r", ch->pcdata->tactical_size_c);
		}

		strcat(buf, HAS_BIT(ch->pcdata->tactical_flags, TACTICAL_INDEX)
			? "{158}  (C) {128}[ ON  ]  {038}Your tactical indexs are on.\n\r"
			: "{158}  (C) {028}[ OFF ]  {038}Your tactical indexs are off.\n\r");

		strcat(buf, HAS_BIT(ch->pcdata->tactical_flags, TACTICAL_TOP)
			? "{158}  (D) {128}[ ON  ]  {038}Your top stat bar is above the tactical.\n\r"
			: "{158}  (D) {028}[ OFF ]  {038}Your top stat bar is below the tactical.\n\r" );

		cat_sprintf(buf, "{158}  (E) {128}[ %c%c  ]  {038}Your tactical index.\n\r" , ch->pcdata->tactical_index[0], ch->pcdata->tactical_index[1]);

		strcat(buf, HAS_BIT(ch->pcdata->tactical_flags, TACTICAL_EXPTNL)
			? "{158}  (F) {128}[ ON  ]  {038}Your tactical experience is the required.\n\r"
			: "{158}  (F) {028}[ OFF ]  {038}Your tactical experience is the total.\n\r");

		strcat( buf, "\n\r{158}  (-) {028}Return\n\r" );

		send_to_char( ansi_translate_text( ch, buf ), ch );

		STRFREE( ch->pcdata->last_command );
		ch->pcdata->last_command = STRALLOC( "config C " );

		pop_call();
		return;
	}

	/*
		General options
	*/

	if (*arg1 == 'd')
	{
		switch (arg2[0])
		{
			case 'a':
				TOGGLE_BIT(ch->act, PLR_AUTOLOOT);
				break;
			case 'b':
				TOGGLE_BIT(ch->act, PLR_AUTOSAC);
				break;
			case 'c': case 'C':
				TOGGLE_BIT(ch->act, PLR_DAMAGE);
				break;
			case 'd': case 'D':
				TOGGLE_BIT(ch->act, PLR_AUTO_SPLIT);
				break;
			case '\0':
				break;
			case 'x':
				do_config(ch, "x");
				pop_call();
				return;
			case '-':
				STRFREE( ch->pcdata->last_command );
				do_config( ch, "" );
				pop_call();
				return;
			default:
				ch_printf_color(ch, "\n\r{118}'%c' Is not an option.\n\r", *arg2 );

				RESTRING(ch->pcdata->last_command, "config D ");
				pop_call();
				return;
		}

		do_refresh(ch, "");

		strcpy( buf, "{138}      General Options Menu\n\r\n\r");

		strcat( buf,  IS_SET(ch->act, PLR_AUTOLOOT)
			? "{158}  (A) {128}[ ON  ] {018}[AUTOLOOT] {038}You automatically loot corpses.\n\r"
			: "{158}  (A) {028}[ OFF ] {018}[autoloot] {038}You do not automatically loot corpses.\n\r");

		strcat( buf,  IS_SET(ch->act, PLR_AUTOSAC)
			? "{158}  (B) {128}[ ON  ] {018}[AUTOSAC ] {038}You automatically sacrifice corpses.\n\r"
			: "{158}  (B) {028}[ OFF ] {018}[autosac ] {038}You do not automatically sacrifice corpses.\n\r");

		strcat( buf,  !IS_SET(ch->act, PLR_DAMAGE)
			? "{158}  (C) {128}[ ON  ] {018}[DAMAGE  ] {038}You see damage status in combat.\n\r"
			: "{158}  (C) {028}[ OFF ] {018}[damage  ] {038}You do not see damage status in combat.\n\r");

		strcat( buf,  IS_SET(ch->act, PLR_AUTO_SPLIT)
			? "{158}  (D) {128}[ ON  ] {018}[SPLIT   ] {038}You split gold automatically.\n\r"
			: "{158}  (D) {028}[ OFF ] {018}[split   ] {038}You do not split gold automatically.\n\r");

		strcat( buf, "\n\r{158}  (-) {028}Return\n\r" );

		send_to_char_color(buf, ch);

		STRFREE(ch->pcdata->last_command);
		ch->pcdata->last_command = STRALLOC( "config D " );
		pop_call();
		return;
	}

	/*
		Combat Spamming configuration
	*/

	if (arg1[0] == 'e' || arg1[0] == 'E')
	{
		switch( *arg2 )
		{
			case 'a':
				TOGGLE_BIT(ch->pcdata->spam, SPAM_YOU_HIT);
				break;
			case 'b':
				TOGGLE_BIT(ch->pcdata->spam, SPAM_YOU_MISS);
				break;
			case 'c':
				TOGGLE_BIT(ch->pcdata->spam, SPAM_THEY_HIT);
				break;
			case 'd':
				TOGGLE_BIT(ch->pcdata->spam, SPAM_THEY_MISS);
				break;
			case 'e':
				TOGGLE_BIT(ch->pcdata->spam, SPAM_PARTY_HIT);
				break;
			case 'f':
				TOGGLE_BIT(ch->pcdata->spam, SPAM_PARTY_MISS);
				break;
			case 'g':
				TOGGLE_BIT(ch->pcdata->spam, SPAM_THEY_HIT_PARTY);
				break;
			case 'h':
				TOGGLE_BIT(ch->pcdata->spam, SPAM_THEY_MISS_PARTY);
				break;
			case 'i':
				TOGGLE_BIT(ch->pcdata->spam, SPAM_OTHER_HIT);
				break;
			case 'j':
				TOGGLE_BIT(ch->pcdata->spam, SPAM_OTHER_MISS);
				break;
			case 'k':
				TOGGLE_BIT(ch->pcdata->spam, SPAM_PARTY_STATUS);
				break;
			case 'l':
				TOGGLE_BIT(ch->pcdata->spam, SPAM_PARTY_MOVE);
				break;
			case 'm':
				TOGGLE_BIT(ch->pcdata->spam, SPAM_HEAR_UTTER);
				break;
			case 'x':
				do_config(ch, "x");
				pop_call();
				return;
			case '-':
				STRFREE( ch->pcdata->last_command );
				do_config( ch, "" );
				pop_call();
				return;
			case '\0':
				break;
			default:
				ch_printf_color(ch, "\n\r{118}'%c' Is not an option.\n\r", *arg2 );

				RESTRING(ch->pcdata->last_command, "config E ");
				pop_call();
				return;
		}

		do_refresh(ch, "");

		strcpy( buf, "{138}      Combat Spamming Menu\n\r\n\r");

		strcat(  buf, !IS_SET(ch->pcdata->spam, 1)
			? "{158}  (A) {128}[ ON  ] {038}You hit.\n\r"
			: "{158}  (A) {028}[ OFF ] {038}You hit.\n\r");

		strcat(  buf, !IS_SET(ch->pcdata->spam, 2)
			? "{158}  (B) {128}[ ON  ] {038}You miss.\n\r"
			: "{158}  (B) {028}[ OFF ] {038}You miss.\n\r");

		strcat(  buf, !IS_SET(ch->pcdata->spam, 4)
			? "{158}  (C) {128}[ ON  ] {038}They hit you.\n\r"
			: "{158}  (C) {028}[ OFF ] {038}They hit you.\n\r");

		strcat(  buf, !IS_SET(ch->pcdata->spam, 8)
			? "{158}  (D) {128}[ ON  ] {038}They miss you.\n\r"
			: "{158}  (D) {028}[ OFF ] {038}They miss you.\n\r");

		strcat(  buf, !IS_SET(ch->pcdata->spam, 16)
			? "{158}  (E) {128}[ ON  ] {038}Party hits.\n\r"
			: "{158}  (E) {028}[ OFF ] {038}Party hits.\n\r");

		strcat(  buf, !IS_SET(ch->pcdata->spam, 32)
			? "{158}  (F) {128}[ ON  ] {038}Party misses.\n\r"
			: "{158}  (F) {028}[ OFF ] {038}Party misses.\n\r");

		strcat(  buf, !IS_SET(ch->pcdata->spam, 64)
			? "{158}  (G) {128}[ ON  ] {038}They hit party.\n\r"
			: "{158}  (G) {028}[ OFF ] {038}They hit party.\n\r");

		strcat(  buf, !IS_SET(ch->pcdata->spam, 128)
			? "{158}  (H) {128}[ ON  ] {038}They miss party.\n\r"
			: "{158}  (H) {028}[ OFF ] {038}They miss party.\n\r");

		strcat(  buf, !IS_SET(ch->pcdata->spam, 256)
			? "{158}  (I) {128}[ ON  ] {038}Other hit.\n\r"
			: "{158}  (I) {028}[ OFF ] {038}Other hit.\n\r");

		strcat(  buf, !IS_SET(ch->pcdata->spam, 512)
			? "{158}  (J) {128}[ ON  ] {038}Other miss.\n\r"
			: "{158}  (J) {028}[ OFF ] {038}Other miss.\n\r");

		strcat(  buf, IS_SET(ch->pcdata->spam, 1024)
			? "{158}  (K) {128}[ ON  ] {038}Show party status line.\n\r"
			: "{158}  (K) {028}[ OFF ] {038}Show party status line.\n\r");

		strcat(  buf, !IS_SET(ch->pcdata->spam, 2048)
			? "{158}  (L) {128}[ ON  ] {038}Hearing party members move.\n\r"
			: "{158}  (L) {028}[ OFF ] {038}Hearing party members move.\n\r");

		strcat( buf, !IS_SET(ch->pcdata->spam, 4096)
			? "{158}  (M) {128}[ ON  ] {038}Hearing the uttering of spells.\n\r"
			: "{158}  (M) {028}[ OFF ] {038}Hearing the uttering of spells.\n\r");

		strcat( buf, "\n\r{158}  (-) {028}Return\n\r" );

		send_to_char_color(buf, ch);

		RESTRING(ch->pcdata->last_command, "config E ");
		pop_call();
		return;
	}

	if (*arg1 == 'f')
	{
		switch (arg2[0])
		{
			case 'a':
				TOGGLE_BIT(ch->pcdata->channel, CHANNEL_CHAT);
				break;
			case 'b':
				TOGGLE_BIT(ch->pcdata->channel, CHANNEL_FOS);
				break;
			case 'c':
				TOGGLE_BIT(ch->pcdata->channel, CHANNEL_PLAN);
				break;
			case 'd':
				TOGGLE_BIT(ch->pcdata->channel, CHANNEL_TRIVIA);
				break;
			case 'e':
				TOGGLE_BIT(ch->pcdata->channel, CHANNEL_MORPH);
				break;
			case 'f':
				TOGGLE_BIT(ch->pcdata->channel, CHANNEL_BATTLE);
				break;
			case '\0':
				break;
			case 'x':
				do_config(ch, "x");
				pop_call();
				return;
			case '-':
				STRFREE( ch->pcdata->last_command );
				do_config( ch, "" );
				pop_call();
				return;

			default:
				ch_printf_color(ch, "\n\r{118}'%c' Is not an option.\n\r", *arg2 );

				RESTRING(ch->pcdata->last_command, "config F ");
				pop_call();
				return;
		}

		do_refresh(ch, "");

		strcpy( buf, "{138}      Communication Channels Menu\n\r\n\r");

		strcat( buf,  IS_SET(ch->pcdata->channel, CHANNEL_CHAT)
			? "{158}  (A) {128}[ ON  ] {018}[CHAT    ] {038}You hear global chatter.\n\r"
			: "{158}  (A) {028}[ OFF ] {018}[chat    ] {038}You do not hear global chatter.\n\r");

		strcat( buf,  IS_SET(ch->pcdata->channel, CHANNEL_FOS)
			? "{158}  (B) {128}[ ON  ] {018}[FOS     ] {038}You hear the freedom of speech channel.\n\r"
			: "{158}  (B) {028}[ OFF ] {018}[fos     ] {038}You do not hear the freedom of speech channel.\n\r");

		strcat( buf,  IS_SET(ch->pcdata->channel, CHANNEL_PLAN)
			? "{158}  (C) {128}[ ON  ] {018}[PLAN    ] {038}You hear godly plans.\n\r"
			: "{158}  (C) {028}[ OFF ] {018}[plan    ] {038}You do not hear godly plans.\n\r");

		strcat( buf,  IS_SET(ch->pcdata->channel, CHANNEL_TRIVIA)
			? "{158}  (D) {128}[ ON  ] {018}[TRIVIA  ] {038}You hear about trivia games being played.\n\r"
			: "{158}  (D) {028}[ OFF ] {018}[trivia  ] {038}You do not hear about trivia games being played.\n\r");

		strcat( buf,  IS_SET(ch->pcdata->channel, CHANNEL_MORPH)
			? "{158}  (E) {128}[ ON  ] {018}[MORPH   ] {038}You hear about morph games being played.\n\r"
			: "{158}  (E) {028}[ OFF ] {018}[morph   ] {038}You do not hear about morph games being played.\n\r");

		strcat(  buf, IS_SET(ch->pcdata->channel, CHANNEL_BATTLE)
			? "{158}  (F) {128}[ ON  ] {018}[BATTLE  ] {038}You hear about battles being fought.\n\r"
			: "{158}  (F) {028}[ OFF ] {018}[battle  ] {038}You do not hear about battles being fought.\n\r");

		strcat( buf, "\n\r{158}  (-) {028}Return\n\r" );

		send_to_char_color(buf, ch);

		RESTRING(ch->pcdata->last_command, "config F ");
		pop_call();
		return;
	}
	do_refresh(ch, "");

	strcpy( buf, "\n\r{138}      Configuration Main Menu\n\r\n\r");
	strcat( buf, "{158}  (A) {028}Display Setup\n\r" );
	strcat( buf, "{158}  (B) {028}Terminal Setup\n\r" );
	strcat( buf, "{158}  (C) {028}Tactical Setup\n\r" );
	strcat( buf, "{158}  (D) {028}General Options\n\r" );
	strcat( buf, "{158}  (E) {028}Combat Spamming\n\r" );
	strcat( buf, "{158}  (F) {028}Channel Setup\n\r\n\r" );
	strcat( buf, "{158}  (X) {028}Exit\n\r" );
	send_to_char_color(buf, ch );

	RESTRING( ch->pcdata->last_command, "config " );

	pop_call();
	return;
}

void do_disguise( CHAR_DATA *ch, char *argument )
{
	push_call("do_disguise(%p,%p)",ch,argument);

	if (multi(ch, gsn_disguise) == -1)
	{
		pop_call();
		return;
	}

	if (ansi_strlen(argument) == 0 || argument[0] == '\0')
	{
		send_to_char("Disguise yourself as what?\n\r", ch);
		pop_call();
		return;
	}
	if (ansi_strlen(argument) > 80 || strlen(argument) > 480)
	{
		send_to_char("You failed to disguise yourself.\n\r", ch);
		pop_call();
		return;
	}
	if (number_percent() > learned(ch, gsn_disguise))
	{
		send_to_char("You failed to disguise yourself.\n\r", ch);
		pop_call();
		return;
	}
	smash_tilde( argument );

	if (IS_NPC(ch) && ch->long_descr != ch->pIndexData->long_descr)
	{
		STRFREE(ch->long_descr);
	}
	ch->long_descr = STRALLOC(argument);
	send_to_char( "You skillfully disguise yourself.\n\r", ch );
	check_improve(ch, gsn_disguise);
	pop_call();
	return;
}

void do_spy( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	ROOM_INDEX_DATA *old_room;
	int door, level;

	push_call("do_spy(%p,%p)",ch,argument);

	if (multi(ch, gsn_spy) == -1)
	{
		send_to_char("They'd just get annoyed with you.\n\r", ch);
		pop_call();
		return;
	}

	level = multi_skill_level(ch, gsn_spy);

	argument = one_argument(argument, arg);

	if ( arg[0] == '\0' )
	{
		send_to_char( "Spy on whom?\n\r", ch );
		pop_call();
		return;
	}

	victim = get_char_room(ch, arg);

	if (victim == NULL)
	{
		old_room = ch->in_room;

		for (door = 0 ; door < 6 ; door++)
		{
			if (is_valid_exit(ch, old_room, door))
			{
				ch->in_room = room_index[get_exit(old_room->vnum, door)->to_room];

				victim = get_char_room(ch, arg);

				if (victim)
				{
					break;
				}
			}
		}

		ch->in_room = old_room;
	}

	if (victim == ch)
	{
		send_to_char( "You pick up your brain, and put it back on your head.\n\r", ch );
		pop_call();
		return;
	}

	if (victim == NULL || !can_see(ch, victim))
	{
		send_to_char( "They aren't anyway near.\n\r", ch );
		pop_call();
		return;
	}

	if (victim->level > level || number_percent() > learned(ch, gsn_spy))
	{
		send_to_char( "You failed.\n\r", ch );
		pop_call();
		return;
	}

	if (IS_IMMORTAL(victim) && !IS_GOD(ch))
	{
		send_to_char("You failed.\n\r", ch);
		pop_call();
		return;
	}

	if (!str_prefix("st", argument))
	{
		get_string_status_v1(victim, ch);
		send_to_char_color( get_string_score_txt, ch);
	}
	else if (argument == NULL || *argument == '\0' || *argument == 'S' || *argument == 's')
	{
		get_string_score_v1(victim, ch);
		send_to_char_color( get_string_score_txt, ch);
		pop_call();
		return;
	}
	else if (*argument == 'a' || *argument == 'A')
	{
		char buf[MAX_STRING_LENGTH];
		get_affects_string(victim, ch, buf);
		send_to_char_color( buf, ch );
		pop_call();
		return;
	}
	else
	{
		send_to_char("Syntax: spy <target> [affects|score|status]\n\r", ch );
		pop_call();
		return;
	}
	check_improve(ch, gsn_spy);
	pop_call();
	return;
}

int which_god( CHAR_DATA *ch)
{
	push_call("which_god(%p)",ch);

	if (IS_NPC(ch))
	{
		pop_call();
		return 0;
	}

	if (ch->pcdata->god != -1)
	{
		pop_call();
		return(ch->pcdata->god);
	}

	ch->pcdata->god = GOD_NEUTRAL;

	if (get_eq_char(ch, WEAR_HEART))
	{
		switch(get_eq_char(ch, WEAR_HEART)->pIndexData->vnum)
		{
			case 50: ch->pcdata->god = GOD_CHAOS; break;
			case 51: ch->pcdata->god = GOD_ORDER; break;
		}
	}
	pop_call();
	return ch->pcdata->god;
}

void do_timemode( CHAR_DATA *ch, char *argument )
{
	push_call("do_timemode(%p,%p)",ch,argument);

	if (IS_SET(ch->act, PLR_WIZTIME))
	{
		REMOVE_BIT( ch->act, PLR_WIZTIME);
		send_to_char( "Time and Debug info deactivated.\n\r", ch);
	}
	else
	{
		SET_BIT( ch->act, PLR_WIZTIME);
		send_to_char( "Time and Debug info activated.\n\r", ch);
	}
	pop_call();
	return;
}

void do_history( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_INPUT_LENGTH];

	push_call("do_history(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	act ("Your standings:", ch, NULL, NULL, TO_CHAR);
	act ("$n's standings:", ch, NULL, NULL, TO_ROOM);

	if (which_god(ch) != GOD_NEUTRAL && FALSE)
	{
		sprintf(buf, " Enemies Killed: %7d     Enemies that have killed you: %7d\n\r"
		             "Monsters Killed: %7d    Monsters that have killed you: %7d\n\r",
			ch->pcdata->history[HISTORY_KILL_EN],
			ch->pcdata->history[HISTORY_KILL_BY_EN],
			ch->pcdata->history[HISTORY_KILL_NPC],
			ch->pcdata->history[HISTORY_KILL_BY_NPC]);
	}
	else
	{
		sprintf(buf, "Monsters Killed: %7d    Monsters that have killed you: %7d\n\r",
			ch->pcdata->history[HISTORY_KILL_NPC],
			ch->pcdata->history[HISTORY_KILL_BY_NPC]);
	}
	act(buf, ch, NULL, NULL, TO_CHAR);
	act(buf, ch, NULL, NULL, TO_ROOM);

	pop_call();
	return;
}

CHAR_DATA *lookup_char( char *name)
{
	PLAYER_GAME *gpl;

	push_call("lookup_char(%p)",name);

	for (gpl = mud->f_player ; gpl ; gpl = gpl->next)
	{
		if (!strcasecmp(gpl->ch->name, name))
		{
			pop_call();
			return( gpl->ch );
		}
	}
	pop_call();
	return( NULL);
}


void do_usage( CHAR_DATA *ch, char *argument )
{
	PLAYER_GAME *fpl;
	char buf[MAX_STRING_LENGTH];
	bool weekly, average;
	int day, hour, topPlayers, topVal, curVal, val, nPlayer, ave_plr[24];

	push_call("do_usage(%p,%p)",ch,argument);

	weekly  = (argument[0] != '\0' && !str_prefix(argument, "weekly"));
	average = (argument[0] != '\0' && !str_prefix(argument, "average"));

	topPlayers = topVal = nPlayer = 0;

	for (fpl = mud->f_player ; fpl ; fpl = fpl->next)
	{
		if (!can_see_world(ch, fpl->ch))
		{
			continue;
		}
		nPlayer++;
	}

	if (weekly)
	{
		long nPlayers[7];

		for (day = 0 ; day < 7 ; day++)
		{
			nPlayers[day] = 0;
			for (hour = 0 ; hour < 24 ; hour++)
			{
				nPlayers[day] += mud->usage->players[hour][day];
			}
			nPlayers[day] /= 24;

			topPlayers = UMAX(topPlayers, nPlayers[day]);
			topVal     = UMAX(topVal, topPlayers);
		}
		topVal = UMAX(topVal, 10);

		for (buf[0] = '\0', val = 0 ; val < 10 ; val++)
		{
			curVal = topVal - val * (float)((float)topVal/(float)10.0);
			cat_sprintf(buf, "{128}%4d{878}:{838}", curVal);

			for (day = 0 ; day < 7 ; day++)
			{
				if (nPlayers[day] >= curVal)
				{
					if (day == mud->time.tm_wday)
					{
						strcat(buf, "{818}|-| {838}");
					}
					else
					{
						strcat(buf, "|-| ");
					}
				}
				else
				{
					strcat(buf, "    ");
				}
			}
			strcat(buf, "\n\r");
		}
		strcat(buf, "{878}    +---+---+---+---+---+---+---+\n\r{828}     Sun Mon Tue Wed Thu Fri Sat\n\r");
		send_to_char_color(buf, ch);
	}
	else if (average)
	{
		memset(ave_plr, 0, 24 * sizeof(int));

		for (hour = 0 ; hour < 24 ; hour++)
		{
			for (day = 0 ; day < 7 ; day++)
			{
				ave_plr[hour] += mud->usage->players[hour][day];
			}
		}

		for (hour = 0 ; hour < 24 ; hour++)
		{
			ave_plr[hour] /= 7;

			if (topPlayers < ave_plr[hour])
			{
				topPlayers = ave_plr[hour];
			}
		}
		topVal = UMAX(topVal, topPlayers);
		topVal = UMAX(topVal, 10);

		for (buf[0] = '\0', val = 0 ; val < 10 ; val++)
		{
			curVal = topVal - val * (float) ((float)topVal/(float)10.0);
			cat_sprintf(buf, "{128}%4d{878}:{838}", curVal);

			for (hour = 0 ; hour < 24 ; hour++)
			{
				if (hour == mud->time.tm_hour)
				{
					if (ave_plr[hour] >= curVal)
					{
						strcat(buf, "{818}|| {838}");
					}
					else
					{
						strcat(buf, "   ");
					}
				}
				else
				{
					if (ave_plr[hour] >= curVal)
					{
						strcat(buf, "|| ");
					}
					else
					{
						strcat(buf, "   ");
					}
				}
			}
			strcat(buf, "\n\r");
		}
		strcat(buf, "{878}    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+\n\r{128}     00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23\n\r");
		send_to_char_color(buf, ch);
	}
	else
	{
		day = mud->time.tm_wday;

		if (argument[0] && argument[1] && argument[2])
		{
			switch (argument[0]+argument[1]+argument[2])
			{
				case ('s'+'u'+'n'): day = 0; break;
				case ('m'+'o'+'n'): day = 1; break;
				case ('t'+'u'+'e'): day = 2; break;
				case ('w'+'e'+'d'): day = 3; break;
				case ('t'+'h'+'u'): day = 4; break;
				case ('f'+'r'+'i'): day = 5; break;
				case ('s'+'a'+'t'): day = 6; break;
			}
		}

		for (hour = 0 ; hour < 24 ; hour++)
		{
			if (hour == mud->time.tm_hour && day == mud->time.tm_wday)
			{
				if (topPlayers < nPlayer)
				{
					topPlayers = nPlayer;
				}
			}
			else if (topPlayers < mud->usage->players[hour][day])
			{
				topPlayers = mud->usage->players[hour][day];
			}
		}
		topVal = UMAX(topVal, topPlayers);
		topVal = UMAX(topVal, 10);

		for (buf[0] = '\0', val = 0 ; val < 10 ; val++)
		{
			curVal = topVal - val * (float) ((float)topVal/(float)10.0);
			cat_sprintf(buf, "{128}%4d{878}:{838}", curVal);

			for (hour = 0 ; hour < 24 ; hour++)
			{
				if (hour == mud->time.tm_hour && day == mud->time.tm_wday)
				{
					if (nPlayer >= curVal)
					{
						strcat(buf, "{818}|| {838}");
					}
					else
					{
						strcat(buf, "   ");
					}
				}
				else
				{
					if (mud->usage->players[hour][day] >= curVal)
					{
						strcat(buf, "|| ");
					}
					else
					{
						strcat(buf, "   ");
					}
				}
			}
			strcat(buf, "\n\r");
		}
		strcat(buf, "{878}    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+\n\r{128}     00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23\n\r");
		send_to_char_color(buf, ch);
	}
	pop_call();
	return;
}

/*
	Stat commands by Chaos - 1/15/96
*/

void do_strength( CHAR_DATA *ch, char *arg)
{
	int st, stlow, sthigh;

	push_call("do_strength(%p,%p)",ch,arg);

	ch_printf_color(ch, "{178}Strength   Weapon Weight   ToHit Bonus   Dam Bonus   Carry Weight\n\r");

	stlow  = URANGE(0, get_curr_str(ch)-4, 100);
	sthigh = URANGE(0, get_curr_str(ch)+4, 100);

	for (st = stlow ; st <= sthigh ; st++)
	{
		if (get_curr_str(ch) == st)
		{
			ch_printf_color(ch, "{178}-->{078}%2d        %5d          %5d        %5d          %5d\n\r", st, str_app[st].wield,  str_app[st].tohit, str_app[st].todam, str_app[st].carry );
		}
		else
		{
			ch_printf_color(ch, "{078}   {078}%2d        %5d          %5d        %5d          %5d\n\r", st, str_app[st].wield,  str_app[st].tohit, str_app[st].todam, str_app[st].carry );
		}
	}
	pop_call();
	return;
}

void do_wisdom( CHAR_DATA *ch, char *arg)
{
	int st, stlow, sthigh;

	push_call("do_wisdom(%p,%p)",ch,arg);

	ch_printf_color(ch, "{178}Wisdom     Practices per Level     Mana Regeneration     Save Bonus\n\r");

	stlow  = URANGE(0, get_curr_wis(ch)-4, MAX_STAT);
	sthigh = URANGE(0, get_curr_wis(ch)+4, MAX_STAT);

	for (st = stlow ; st <= sthigh ; st++)
	{
		ch_printf_color(ch, "{178}%s{078}%2d            %5d                  %5d              %5d\n\r",
			get_curr_wis(ch) == st ? "->" : "  ",
			st,
			wis_app[st].practice,
			wis_app[st].regen,
			-1 * st / 4);
	}
	pop_call();
	return;
}

void do_intelligence( CHAR_DATA *ch, char *arg)
{
	int st, stlow, sthigh;

	push_call("do_intelligence(%p,%p)",ch,arg);

	ch_printf_color(ch, "{178}Intelligence   Learning Percentage   Concentration Bonus   Mana per Level\n\r");

	stlow  = URANGE(0, get_curr_int(ch)-4, 100);
	sthigh = URANGE(0, get_curr_int(ch)+4, 100);

	for (st = stlow ; st <= sthigh ; st++)
	{
		if (get_curr_int(ch) == st)
		{
			ch_printf_color(ch, "{178}--->{078}%2d               %5d                %5d              %5d\n\r", st, int_app[st].learn,  (st - 13)*2, int_app[st].manap);
		}
		else
		{
			ch_printf_color(ch, "{078}    {078}%2d               %5d                %5d              %5d\n\r", st, int_app[st].learn,  (st - 13)*2, int_app[st].manap);
		}
	}
	ch_printf_color(ch, "{078}Note: Concentration bonus adds to spell's percentage learned at casting time.\n\r");
	pop_call();
	return;
}


void do_dexterity( CHAR_DATA *ch, char *arg)
{
	int st, stlow, sthigh;

	push_call("do_dexterity(%p,%p)",ch,arg);

	ch_printf_color(ch, "{178}Dexterity     Bonus AC     Skill Bonus     Move per Level     Move Regeneration\n\r", ch );

	stlow  = URANGE(0, get_curr_dex(ch)-4, 50);
	sthigh = URANGE(0, get_curr_dex(ch)+4, 50);

	for (st = stlow ; st <= sthigh ; st++)
	{
		if (get_curr_dex(ch) == st)
		{
			ch_printf_color(ch, "{178}-->{078}%2d         %5d         %5d             %5d                %5d\n\r", st, dex_app[st].defensive, dex_app[st].learn, st/3, dex_app[st].regen);
		}
		else
		{
			ch_printf_color(ch, "{078}   {078}%2d         %5d         %5d             %5d                %5d\n\r", st, dex_app[st].defensive, dex_app[st].learn, st/3, dex_app[st].regen);
		}
	}
	ch_printf_color(ch, "{078}Note: Skill bonus adds to skill's percentage learned at executing time.\n\r", ch);
	pop_call();
	return;
}

void do_constitution( CHAR_DATA *ch, char *arg)
{
	int st, stlow, sthigh;

	push_call("do_constitution(%p,%p)",ch,arg);

	ch_printf_color(ch, "{178}Constitution     Hitpoints per Level     Hitpoints Regeneration\n\r", ch);

	stlow  = URANGE(0, get_curr_con(ch)-4, 50);
	sthigh = URANGE(0, get_curr_con(ch)+4, 50);

	for (st = stlow ; st <= sthigh ; st++)
	{
		if (get_curr_con(ch) == st)
		{
			ch_printf_color(ch, "{178}---->{078}%2d           %5d                         %5d\n\r", st, con_app[st].hitp, con_app[st].regen);
		}
		else
		{
			ch_printf_color(ch, "{078}     {078}%2d           %5d                         %5d\n\r", st, con_app[st].hitp, con_app[st].regen);
		}
	}
	pop_call();
	return;
}

void do_glance( CHAR_DATA *ch, char *argument )
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	push_call("do_glance(%p,%p)",ch,argument);

	if (ch->desc == NULL)
	{
		pop_call();
		return;
	}

	if (!check_blind(ch))
	{
		pop_call();
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ((victim = get_char_room(ch, arg1)) != NULL)
	{
		if (number_percent() < learned(ch, gsn_glance))
		{
			act("You look at $N without $M noticing.", ch, NULL, victim, TO_CHAR);

			show_char_to_char_1(victim, ch);

			check_improve(ch, gsn_glance);
		}
		else
		{
			act("You notice $N notices your looking at $M.", ch, NULL, victim, TO_CHAR);
			act("You notice $n glancing at your equipment.", ch, NULL, victim, TO_VICT);
			show_char_to_char_1( victim, ch );
		}
		pop_call();
		return;
	}
	else
	{
		send_to_char("Glance at whom?\n\r",ch);
		pop_call();
		return;
	}
	pop_call();
	return;
}

/*
	Returns a static {color} string - Scandum 29-05-2002
*/

char *get_color_code(CHAR_DATA *ch, int regist, int vt_code)
{
	static char buf[10];

	push_call("get_color_code(%p,%p,%p)",ch,regist,vt_code);

	if (ch->desc == NULL)
	{
		pop_call();
		return "";
	}

	if (!HAS_BIT(CH(ch->desc)->pcdata->vt100_flags, VT100_ANSI) && !IS_SET(CH(ch->desc)->pcdata->vt100_flags, VT100_BOLD) && !IS_SET(CH(ch->desc)->pcdata->vt100_flags, VT100_REVERSE))
	{
		pop_call();
		return "";
	}

	sprintf(buf, "{%d%d%d}", vt_code, CH(ch->desc)->pcdata->color[regist] % 10, CH(ch->desc)->pcdata->color[regist] / 10);

	pop_call();
	return( buf );
}

/*
	Returns a static string -1 as register will ignore colors - Chaos 2/24/95
*/

char *get_color_string( CHAR_DATA *ch, int regist , int vt_code )
{
	static char buf[10];

	push_call("get_color_string(%p,%p,%p)",ch,regist,vt_code);

	if (!ch->desc)
	{
		pop_call();
		return "";
	}

	if (!HAS_BIT(CH(ch->desc)->pcdata->vt100_flags, VT100_ANSI) && !IS_SET(CH(ch->desc)->pcdata->vt100_flags, VT100_BOLD) && !IS_SET(CH(ch->desc)->pcdata->vt100_flags, VT100_REVERSE))
	{
		pop_call();
		return "";
	}

	if (regist >= 0 && regist < COLOR_MAX )
	{
		strcpy(buf, get_color_diff(ch, 8, 8, 8, CH(ch->desc)->pcdata->color[regist] % 10, CH(ch->desc)->pcdata->color[regist] / 10, vt_code));
	}
	else
	{
		strcpy(buf, get_color_diff(ch, 8, 8, 8, 8, 8, vt_code));
	}

	pop_call();
	return( buf );
}

ROOM_INDEX_DATA * RoomDir( CHAR_DATA *ch, ROOM_INDEX_DATA *in_room, int dir, int dist)
{
	ROOM_INDEX_DATA *room;
	EXIT_DATA *pexit;
	int cnt;

	push_call("RoomDir(%p,%p,%p,%p)",ch,in_room,dir,dist);

	cnt  = dist;
	room = in_room;

	while (cnt > 0)
	{
		if ((pexit = get_exit(room->vnum, dir)) == NULL)
		{
			pop_call();
			return NULL;
		}
		if (IS_SET(pexit->flags, EX_CLOSED))
		{
			pop_call();
			return NULL;
		}

		if (IS_SET(pexit->flags, EX_HIDDEN) && !can_see_hidden(ch))
		{
			pop_call();
			return NULL;
		}

		if (!can_use_exit(ch, pexit))
		{
			pop_call();
			return NULL;
		}
		room = room_index[room->exit[dir]->to_room];
		cnt--;
	}
	pop_call();
	return( room );
}

void make_length( char *arg, int len )
{
	int leng, spt, cnt;
	char tbuf[MAX_INPUT_LENGTH];

	push_call("make_length(%p,%p)",arg,len);

	*(arg+len) = '\0';
	leng = strlen( arg );
	spt = (len-leng)/2;
	if( spt > 0 )
	{
		strcpy( tbuf, arg );
		for( cnt=0; cnt<spt; cnt++)
			*(arg+cnt) = ' ';
		strcpy( arg+spt, tbuf );
	}
	leng = strlen( arg );
	for( ; leng<len; leng++)
		*(arg+leng) = ' ';
	*(arg+len) = '\0';

	pop_call();
	return;
}

void strNameRoom( char *buf, ROOM_INDEX_DATA *room )
{
	int offset;
	char *buf3;

	push_call("strNameRoom(%p,%p)",buf,room);

	offset = 0;
	buf3 = room->name;

	if( *buf3=='t' || *buf3=='T' )
	{
		if( *(buf3+1)=='h' || *(buf3+1)=='H' )
		{
			if( *(buf3+2)=='e' || *(buf3+2)=='E' )
			{
				if( *(buf3+3)==' ' )
				{
					offset = 4;
				}
			}
		}
	}
	if( *buf3=='a' || *buf3=='A' )
	{
		if( *(buf3+1)==' ' )
		{
			offset = 2;
		}
	}

	strcpy( buf, capitalize( buf3 + offset) );
	pop_call();
	return;
}

void do_map( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char tbuf1[MAX_INPUT_LENGTH];
	char tbuf3[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *room1, *room2;
	int wwords, wrds;
	int col;
	int cnt;
	int map_range;

	push_call("do_map(%p,%p)",ch,argument);

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

	map_range = URANGE(0, argument[0] ? atoi(argument) : 4, 4);

	wrds = 1 + map_range * 2;
	wwords = (79 - 2*(1+map_range*2))/ wrds;
	wwords = UMIN( wwords, 12 );

	strcpy(buf, "{088}");
	*tbuf3 = '\0';
	room1 = NULL;
	room2 = NULL;

	/* The north and up directions */
	for( cnt=map_range; cnt>0; cnt--)
	{
		room1 = RoomDir( ch, ch->in_room, 0, cnt);
		room2 = RoomDir( ch, ch->in_room, 4, cnt);
		if( room1==NULL && room2==NULL )
			continue;

		/* Correct for centering */
		make_length( tbuf3, (79-(map_range*2+1)*(wwords+2))/2);
		strcat( buf, tbuf3 );

		/* Top alignment */
		make_length( tbuf3, map_range*(wwords+2));
		strcat( buf, tbuf3 );
		if( room1 != NULL )
		{
			make_length( tbuf3, 1+wwords/2);
			strcat( buf, tbuf3 );
			if( is_valid_exit( ch, room1, 0 ) )
				strcat( buf, "|" );
			else
			if( is_valid_exit( ch, room1, 0 ) )
				strcat( buf, "+" );
			else
				strcat( buf, " " );
			make_length( tbuf3, wwords - wwords/2 - 1  );
			strcat( buf, tbuf3 );
			if( is_valid_exit( ch, room1, 4 ) )
				strcat( buf, "/" );
			else
			if( is_valid_exit( ch, room1, 4 ) )
				strcat( buf, "X" );
			else
				strcat( buf, " " );
		}
		else
		{
			make_length( tbuf3, wwords+2 );
			strcat( buf, tbuf3 );
		}

		make_length( tbuf3, (wwords+2)*( cnt-1 ) );
		strcat( buf, tbuf3 );
		if( room2 != NULL )
		{
			make_length( tbuf3, 1+wwords/2);
			strcat( buf, tbuf3 );
			if( is_valid_exit( ch, room2, 0 ) )
				strcat( buf, "|" );
			else
				if( is_valid_exit( ch, room2, 0 ) )
			strcat( buf, "+" );
			else
				strcat( buf, " " );
			make_length( tbuf3, wwords - wwords/2 - 1  );
			strcat( buf, tbuf3 );
			if( is_valid_exit( ch, room2, 4 ) )
				strcat( buf, "/" );
			else
			if( is_valid_exit( ch, room2, 4 ) )
				strcat( buf, "X" );
			else
				strcat( buf, " " );
		}
		strcat( buf, "\n\r" );

		/* Correct for centering */
		make_length( tbuf3, (79-(map_range*2+1)*(wwords+2))/2);
		strcat( buf, tbuf3 );
		/* Center alignment */
		make_length( tbuf3, map_range*(wwords+2));
		strcat( buf, tbuf3 );
		if( room1 != NULL )
		{
			if( is_valid_exit( ch, room1, 3 ) )
				strcat( buf, "-" );
			else
			if( is_valid_exit( ch, room1, 3 ) )
				strcat( buf, "+" );
			else
				strcat( buf, " " );
			col = sector_table[room1->sector_type].color;
			cat_sprintf(buf, "{%d%d%d}", col / 128, col % 8, (col % 128) / 8);
			strNameRoom( tbuf1, room1 );
			make_length( tbuf1, wwords );
			strcat( buf, tbuf1 );
			strcat( buf, "{088}");
			if( is_valid_exit( ch, room1, 1 ) )
				strcat( buf, "-" );
			else
			if( is_valid_exit( ch, room1, 1 ) )
				strcat( buf, "+" );
			else
				strcat( buf, " " );
		}
		else
		{
			make_length( tbuf3, wwords+2 );
			strcat( buf, tbuf3 );
		}
		make_length( tbuf3, (wwords+2)*( cnt-1 ) );
		strcat( buf, tbuf3 );
		if( room2 != NULL )
		{
			if( is_valid_exit( ch, room2, 3 ) )
				strcat( buf, "-" );
			else
			if( is_valid_exit( ch, room2, 3 ) )
				strcat( buf, "+" );
			else
				strcat( buf, " " );
			col = sector_table[room2->sector_type].color;
			cat_sprintf(buf, "{%d%d%d}", col / 128, col % 8, (col % 128) / 8);
			strNameRoom( tbuf1, room2 );
			make_length( tbuf1, wwords );
			strcat( buf, tbuf1 );
			strcat( buf, "{088}");
			if( is_valid_exit( ch, room2, 1 ) )
				strcat( buf, "-" );
			else
			if( is_valid_exit( ch, room2, 1 ) )
				strcat( buf, "+" );
			else
				strcat( buf, " " );
		}
		strcat( buf, "\n\r" );

		/* Correct for centering */
		make_length( tbuf3, (79-(map_range*2+1)*(wwords+2))/2);
		strcat( buf, tbuf3 );

		/* Bottom alignment */
		make_length( tbuf3, map_range*(wwords+2));
		strcat( buf, tbuf3 );
		if( room1 != NULL )
		{
			if( is_valid_exit( ch, room1, 5 ) )
				strcat( buf, "/" );
			else
			if( is_valid_exit( ch, room1, 5 ) )
				strcat( buf, "X" );
			else
				strcat( buf, " " );
			make_length( tbuf3, wwords/2);
			strcat( buf, tbuf3 );
			if( is_valid_exit( ch, room1, 2 ) )
				strcat( buf, "|" );
			else
			if( is_valid_exit( ch, room1, 2 ) )
				strcat( buf, "+" );
			else
				strcat( buf, " " );
			make_length( tbuf3, wwords - wwords/2  );
			strcat( buf, tbuf3 );
		}
		else
		{
			make_length( tbuf3, wwords+2 );
			strcat( buf, tbuf3 );
		}
		make_length( tbuf3, (wwords+2)*( cnt-1 ) );
		strcat( buf, tbuf3 );
		if( room2 != NULL )
		{
			if( is_valid_exit( ch, room2, 5 ) )
				strcat( buf, "/" );
			else
			if( is_valid_exit( ch, room2, 5 ) )
				strcat( buf, "/" );
			else
				strcat( buf, " " );
			make_length( tbuf3, wwords/2);
			strcat( buf, tbuf3 );
			if( is_valid_exit( ch, room2, 2 ) )
				strcat( buf, "|" );
			else
			if( is_valid_exit( ch, room2, 2 ) )
				strcat( buf, "|" );
			else
				strcat( buf, " " );
		}
		strcat( buf, "\n\r" );
	}

	/* The center row, top alignment */

	make_length( tbuf3, (79-(map_range*2+1)*(wwords+2))/2  );
	strcat( buf, tbuf3 );
	for( cnt=map_range; cnt>=(0-map_range); cnt--)
	{
		if( cnt>0 )
			room1 = RoomDir( ch, ch->in_room, 3, cnt);
		else
		if( cnt==0 )
			room1 = ch->in_room;
		else
			room1 = RoomDir( ch, ch->in_room, 1, 0-cnt);
		if( room1 == NULL )
		{
			make_length( tbuf3, wwords + 2 );
			strcat( buf, tbuf3 );
		}
		else
		{
			make_length( tbuf3, wwords/2+1 );
			strcat( buf, tbuf3 );
			if( is_valid_exit( ch, room1, 0 ) )
				strcat( buf, "|" );
			else
			if( is_valid_exit( ch, room1, 0 ) )
				strcat( buf, "+" );
			else
				strcat( buf, " " );
			make_length( tbuf3, wwords - wwords/2 - 1  );
			strcat( buf, tbuf3 );
			if( is_valid_exit( ch, room1, 4 ) )
				strcat( buf, "/" );
			else
			if( is_valid_exit( ch, room1, 4 ) )
				strcat( buf, "X" );
			else
				strcat( buf, " " );
		}
	}
	strcat( buf, "\n\r" );

	/* The center row, center alignment */

	*tbuf3='\0';
	make_length( tbuf3, (79-(map_range*2+1)*(wwords+2))/2);
	strcat( buf, tbuf3 );
	for( cnt=map_range; cnt>=(0-map_range); cnt--)
	{
		if( cnt>0 )
			room1 = RoomDir( ch, ch->in_room, 3, cnt);
		else
		if( cnt==0 )
			room1 = ch->in_room;
		else
			room1 = RoomDir( ch, ch->in_room, 1, 0-cnt);
		if( room1 == NULL )
		{
			make_length( tbuf3, wwords + 2 );
			strcat( buf, tbuf3 );
		}
		else
		{
			if( is_valid_exit( ch, room1, 3 ) )
				strcat( buf, "-" );
			else
			if( is_valid_exit( ch, room1, 3 ) )
				strcat( buf, "+" );
			else
				strcat( buf, " " );
			col = sector_table[room1->sector_type].color;
			cat_sprintf(buf, "{%d%d%d}", col / 128, col % 8, (col % 128) / 8);
			if (ch->in_room == room1)
			{
				strcpy(tbuf1, "You are here");
			}
			else
			{
				strNameRoom(tbuf1, room1);
			}
			make_length( tbuf1, wwords );
			strcat( buf, tbuf1 );
			strcat( buf, "{088}");
			if (is_valid_exit(ch, room1, 1))
			{
				strcat( buf, "-" );
			}
			else if (is_valid_exit(ch, room1, 1))
			{
				strcat( buf, "+" );
			}
			else
			{
				strcat( buf, " " );
			}
		}
	}
	strcat( buf, "\n\r" );

	/* The center row, bottom alignment */

	*tbuf3='\0';
	make_length( tbuf3, (79-(map_range*2+1)*(wwords+2))/2);
	strcat( buf, tbuf3 );
	for( cnt=map_range; cnt>=(0-map_range); cnt--)
	{
		if( cnt>0 )
			room1 = RoomDir( ch, ch->in_room, 3, cnt);
		else
		if( cnt==0 )
			room1 = ch->in_room;
		else
			room1 = RoomDir( ch, ch->in_room, 1, 0-cnt);
		if( room1 == NULL )
		{
			make_length( tbuf3, wwords + 2 );
			strcat( buf, tbuf3 );
		}
		else
		{
			if( is_valid_exit( ch, room1, 5 ) )
				strcat( buf, "/" );
			else
			if( is_valid_exit( ch, room1, 5 ) )
				strcat( buf, "X" );
			else
				strcat( buf, " " );
			make_length( tbuf3, wwords/2 );
			strcat( buf, tbuf3 );
			if( is_valid_exit( ch, room1, 2 ) )
				strcat( buf, "|" );
			else
			if( is_valid_exit( ch, room1, 2 ) )
				strcat( buf, "+" );
			else
				strcat( buf, " " );
			make_length( tbuf3, wwords - wwords/2 );
			strcat( buf, tbuf3 );
		}
	}
	strcat( buf, "\n\r" );

	/* The Down and South directions */
	for( cnt=1; cnt<=map_range; cnt++)
	{
		room1 = RoomDir( ch, ch->in_room, 5, cnt);
		room2 = RoomDir( ch, ch->in_room, 2, cnt);
		if( room1==NULL && room2==NULL )
			continue;

		/* Correct for centering */
		make_length( tbuf3, (79-(map_range*2+1)*(wwords+2))/2);
		strcat( buf, tbuf3 );

		/* Top alignment */
		make_length( tbuf3, (map_range-cnt)*(wwords+2));
		strcat( buf, tbuf3 );
		if( room1 != NULL )
		{
			make_length( tbuf3, 1+wwords/2);
			strcat( buf, tbuf3 );
			if( is_valid_exit( ch, room1, 0 ) )
				strcat( buf, "|" );
			else
			if( is_valid_exit( ch, room1, 0 ) )
				strcat( buf, "+" );
			else
				strcat( buf, " " );
			make_length( tbuf3, wwords - wwords/2 - 1  );
			strcat( buf, tbuf3 );
			if( is_valid_exit( ch, room1, 4 ) )
				strcat( buf, "/" );
			else
			if( is_valid_exit( ch, room1, 4 ) )
				strcat( buf, "X" );
			else
				strcat( buf, " " );
		}
		else
		{
			make_length( tbuf3, wwords+2 );
			strcat( buf, tbuf3 );
		}

		make_length( tbuf3, (wwords+2)*( cnt-1 ) );
		strcat( buf, tbuf3 );
		if( room2 != NULL )
		{
			make_length( tbuf3, 1+wwords/2);
			strcat( buf, tbuf3 );
			if( is_valid_exit( ch, room2, 0 ) )
				strcat( buf, "|" );
			else
			if( is_valid_exit( ch, room2, 0 ) )
				strcat( buf, "+" );
			else
				strcat( buf, " " );
			make_length( tbuf3, wwords - wwords/2 - 1  );
			strcat( buf, tbuf3 );
			if( is_valid_exit( ch, room2, 4 ) )
				strcat( buf, "/" );
			else
			if( is_valid_exit( ch, room2, 4 ) )
				strcat( buf, "X" );
			else
				strcat( buf, " " );
		}
		strcat( buf, "\n\r" );

		/* Correct for centering */
		make_length( tbuf3, (79-(map_range*2+1)*(wwords+2))/2);
		strcat( buf, tbuf3 );

		/* Center alignment */
		make_length( tbuf3, (map_range-cnt)*(wwords+2));
		strcat( buf, tbuf3 );
		if( room1 != NULL )
		{
			if( is_valid_exit( ch, room1, 3 ) )
				strcat( buf, "-" );
			else
			if( is_valid_exit( ch, room1, 3 ) )
				strcat( buf, "+" );
			else
				strcat( buf, " " );
			col = sector_table[room1->sector_type].color;
			cat_sprintf(buf, "{%d%d%d}", col / 128, col % 8, (col % 128) / 8);

			strNameRoom( tbuf1, room1 );
			make_length( tbuf1, wwords );
			strcat( buf, tbuf1 );
			strcat( buf, "{088}");
			if( is_valid_exit( ch, room1, 1 ) )
				strcat( buf, "-" );
			else
			if( is_valid_exit( ch, room1, 1 ) )
				strcat( buf, "+" );
			else
				strcat( buf, " " );
		}
		else
		{
			make_length( tbuf3, wwords+2 );
			strcat( buf, tbuf3 );
		}

		make_length( tbuf3, (wwords+2)*( cnt-1 ) );
		strcat( buf, tbuf3 );
		if( room2 != NULL )
		{
			if( is_valid_exit( ch, room2, 3 ) )
				strcat( buf, "-" );
			else
			if( is_valid_exit( ch, room2, 3 ) )
				strcat( buf, "+" );
			else
				strcat( buf, " " );
			col = sector_table[room2->sector_type].color;
			cat_sprintf(buf, "{%d%d%d}", col / 128, col % 8, (col % 128) / 8);
			strNameRoom( tbuf1, room2 );
			make_length( tbuf1, wwords );
			strcat( buf, tbuf1 );
			strcat( buf, "{088}");
			if( is_valid_exit( ch, room2, 1 ) )
				strcat( buf, "-" );
			else
			if( is_valid_exit( ch, room2, 1 ) )
				strcat( buf, "+" );
			else
				strcat( buf, " " );
		}
		strcat( buf, "\n\r" );

		/* Correct for centering */
		make_length( tbuf3, (79-(map_range*2+1)*(wwords+2))/2);
		strcat( buf, tbuf3 );

		/* Bottom alignment */
		make_length( tbuf3, (map_range-cnt)*(wwords+2));
		strcat( buf, tbuf3 );
		if( room1 != NULL )
		{
			if( is_valid_exit( ch, room1, 5 ) )
				strcat( buf, "/" );
			else
			if( is_valid_exit( ch, room1, 5 ) )
				strcat( buf, "X" );
			else
				strcat( buf, " " );
			make_length( tbuf3, wwords/2);
			strcat( buf, tbuf3 );
			if( is_valid_exit( ch, room1, 2 ) )
				strcat( buf, "|" );
			else
			if( is_valid_exit( ch, room1, 2 ) )
				strcat( buf, "+" );
			else
				strcat( buf, " " );
			make_length( tbuf3, wwords - wwords/2  );
				strcat( buf, tbuf3 );
		}
		else
		{
			make_length( tbuf3, wwords+2 );
			strcat( buf, tbuf3 );
		}

		make_length( tbuf3, (wwords+2)*( cnt-1 ) );
		strcat( buf, tbuf3 );
		if( room2 != NULL )
		{
			if( is_valid_exit( ch, room2, 5 ) )
				strcat( buf, "/" );
			else
			if( is_valid_exit( ch, room2, 5 ) )
				strcat( buf, "X" );
			else
				strcat( buf, " " );
			make_length( tbuf3, wwords/2);
			strcat( buf, tbuf3 );
			if( is_valid_exit( ch, room2, 2 ) )
				strcat( buf, "|" );
			else
			if( is_valid_exit( ch, room2, 2 ) )
				strcat( buf, "+" );
			else
				strcat( buf, " " );
		}
		strcat( buf, "\n\r" );
	}
	send_to_char_color( buf, ch );

	pop_call();
	return;
}

char Most_Type[MOST_MOST][30]=
{
	"Trivia Points",
	"Morph Points",
	"Reincarnations",
	"Killed Enemies",
	"Killed Neutrals",
	"Killed Monsters",
	"Killed by Monsters",
	"Killed by Enemies",
	"Killed by Neutrals",
	"Hit Points",
	"Mana Points",
	"Move Points",
	"Armor Class",
	"Damage Roll",
	"Hit Roll",
	"Castle Cost",
	"Hours",
	"Save Vs Spell"
};

void check_most( CHAR_DATA *ch )
{
	push_call("check_most(%p)",ch);

	if (ch->level > 95)
	{
		pop_call();
		return;
	}

	update_hiscores(ch, MOST_TRIVIA, ch->pcdata->trivia_points, TRUE);
	update_hiscores(ch, MOST_MORPH, ch->pcdata->morph_points, TRUE);
	update_hiscores(ch, MOST_REINCARNATE, ch->pcdata->reincarnation * 100 + ch->level, TRUE);
	update_hiscores(ch, MOST_KILL_EN, ch->pcdata->history[HISTORY_KILL_EN], TRUE);
	update_hiscores(ch, MOST_KILL_PC, ch->pcdata->history[HISTORY_KILL_PC], TRUE);
	update_hiscores(ch, MOST_KILL_NPC, ch->pcdata->history[HISTORY_KILL_NPC], TRUE);
	update_hiscores(ch, MOST_KILL_BY_NPC, ch->pcdata->history[HISTORY_KILL_BY_NPC], TRUE);
	update_hiscores(ch, MOST_KILL_BY_EN, ch->pcdata->history[HISTORY_KILL_BY_EN], TRUE);
	update_hiscores(ch, MOST_KILL_BY_PC, ch->pcdata->history[HISTORY_KILL_BY_PC], TRUE);
	update_hiscores(ch, MOST_HP, ch->max_hit, TRUE);
	update_hiscores(ch, MOST_MANA, ch->max_mana, TRUE);
	update_hiscores(ch, MOST_AC, GET_AC(ch), FALSE);
	update_hiscores(ch, MOST_DR, GET_DAMROLL(ch), TRUE);
	update_hiscores(ch, MOST_HR, GET_HITROLL(ch), TRUE);

	if (ch->pcdata->castle)
	{
		update_hiscores(ch, MOST_CASTLE, ch->pcdata->castle->cost, TRUE);
	}
	update_hiscores(ch, MOST_HOURS, ch->pcdata->played/3600+ch->pcdata->previous_hours, TRUE);
	update_hiscores(ch, MOST_SAVE, GET_SAVING_THROW(ch), FALSE);

	pop_call();
	return;
}

void update_hiscores(CHAR_DATA *ch, int rank, int score, bool cptype)
{
	HISCORE_DATA *hiscore, *hiscore_new;
	HISCORE_LIST *hi_rank;
	bool removed = FALSE;

	push_call("update_hiscore(%p,%p,%p,%p)",ch,rank,score,cptype);

	hi_rank = mud->high_scores[rank];

	for (hiscore = hi_rank->first ; hiscore ; hiscore = hiscore->next)
	{
		if (hiscore->vnum == ch->pcdata->pvnum)
		{
			if (hiscore->score == score)
			{
				pop_call();
				return;
			}
			STRFREE(hiscore->player);
			UNLINK(hiscore, hi_rank->first, hi_rank->last, next, prev);
			FREEMEM(hiscore);
			removed = TRUE;
			break;
		}
	}

	for (hiscore = hi_rank->first ; hiscore ; hiscore = hiscore->next)
	{
		if ((cptype && score > hiscore->score) || (!cptype && score < hiscore->score))
		{
			ALLOCMEM(hiscore_new, HISCORE_DATA, 1);
			hiscore_new->vnum	= ch->pcdata->pvnum;
			hiscore_new->score	= score;
			hiscore_new->player	= STRALLOC(ch->name);
			INSERT_LEFT(hiscore_new, hiscore, hi_rank->first, next, prev);
			break;
		}
	}

	if (hiscore == NULL && removed)
	{
		ALLOCMEM(hiscore_new, HISCORE_DATA, 1);
		hiscore_new->vnum	= ch->pcdata->pvnum;
		hiscore_new->score	= score;
		hiscore_new->player	= STRALLOC(ch->name);
		LINK(hiscore_new, hi_rank->first, hi_rank->last, next, prev);
	}

	if (hiscore != NULL && !removed)
	{
		hiscore = hi_rank->last;

		STRFREE(hiscore->player);
		UNLINK(hiscore, hi_rank->first, hi_rank->last, next, prev);
		FREEMEM(hiscore);
	}

	pop_call();
	return;
}

void do_most( CHAR_DATA *ch, char *argument )
{
	HISCORE_DATA *hiscore;
	char buf[MAX_STRING_LENGTH];
	char hs_buf[2][MAX_HISCORE/2][100];
	int i, x, y, z;

	push_call("do_most(%p,%p)",ch,argument);

	sprintf(buf, "{138}Information on the most of stuff:\n\r\n\r");

	if (*argument == '\0')
	{
		for (x = 0 ; x < MOST_MOST ; x++)
		{
			if (*mud->high_scores[x]->first->player != '\0' && mud->high_scores[x]->first->score != 0)
			{
				switch (x)
				{
					case MOST_CASTLE:
						cat_sprintf(buf, "{038}%20s {128}%11s,000 {178}%s\n\r", Most_Type[x], tomoney(mud->high_scores[x]->first->score), mud->high_scores[x]->first->player);
						break;

					case MOST_REINCARNATE:
						cat_sprintf(buf, "{038}%20s {128}%15d {178}%s\n\r", Most_Type[x], mud->high_scores[x]->first->score/100,  mud->high_scores[x]->first->player);
						break;

					case MOST_KILL_EN:
						continue;
						break;

					default:
						cat_sprintf(buf, "{038}%20s {128}%15d {178}%s\n\r", Most_Type[x], mud->high_scores[x]->first->score,  mud->high_scores[x]->first->player);
						break;
				}
			}
		}
		send_to_char_color(buf, ch);
		pop_call();
		return;
	}

	if (is_number(argument))
	{
		z = atoi(argument);

		sprintf(buf, "{138}Information on the most list ranking %d:\n\r\n\r", z);

		for (x = 0 ; x < MOST_MOST ; x++)
		{
			for (hiscore = mud->high_scores[x]->first, y = 0 ; hiscore && y < MAX_HISCORE ; hiscore = hiscore->next, y++)
			{
				if (y+1 != z)
				{
					continue;
				}
				if (*hiscore->player == '\0')
				{
					continue;
				}
				if (x == MOST_CASTLE)
				{
					cat_sprintf(buf, "{038}%20s {138}%2d: {128}%11s,000 {178}%s\n\r", Most_Type[x], y+1, tomoney(hiscore->score), hiscore->player);
				}
				else if (x == MOST_REINCARNATE)
				{
					cat_sprintf(buf, "{038}%20s {138}%2d: {128}%15d {178}%s\n\r", Most_Type[x], y+1, hiscore->score/100,  hiscore->player);
				}
				else if (x == MOST_KILL_EN)
				{
					continue;
				}
				else
				{
					cat_sprintf(buf, "{038}%20s {138}%2d: {128}%15d {178}%s\n\r", Most_Type[x], y+1, hiscore->score,  hiscore->player);
				}
			}
		}
		if (strlen(buf) > 75)
		{
			send_to_char_color(buf, ch);
		}
		else
		{
			ch_printf(ch, "Most Table '%s' not found.\n\r", argument);
		}
		pop_call();
		return;
	}

	for (x = 0 ; x < MOST_MOST ; x++)
	{
		if (!str_prefix(argument, Most_Type[x]) && x != MOST_KILL_EN)
		{
			break;
		}
	}

	if (x == MOST_MOST)
	{
		argument[MAX_NAME_LENGTH] = '\0';

		sprintf(buf, "{138}Information on the most list ranking of %s:\n\r\n\r", capitalize_name(argument));

		for (x = 0 ; x < MOST_MOST ; x++)
		{
			for (hiscore = mud->high_scores[x]->first, y = 0 ; hiscore && y < MAX_HISCORE ; hiscore = hiscore->next, y++)
			{
				if (strcasecmp(argument, hiscore->player))
				{
					continue;
				}
				if (x == MOST_CASTLE)
				{
					cat_sprintf(buf, "{038}%20s {138}%2d: {128}%11s,000 {178}%s\n\r", Most_Type[x], y+1, tomoney(hiscore->score), hiscore->player);
				}
				else if (x == MOST_REINCARNATE)
				{
					cat_sprintf(buf, "{038}%20s {138}%2d: {128}%15d {178}%s\n\r", Most_Type[x], y+1, hiscore->score/100,  hiscore->player);
				}
				else if (x == MOST_KILL_EN)
				{
					continue;
				}
				else
				{
					cat_sprintf(buf, "{038}%20s {138}%2d: {128}%15d {178}%s\n\r", Most_Type[x], y+1, hiscore->score,  hiscore->player);
				}
			}
		}
		if (strlen(buf) > 75)
		{
			send_to_char_color(buf, ch);
		}
		else
		{
			ch_printf(ch, "Most Table '%s' not found.\n\r", argument);
		}
		pop_call();
		return;
	}

	sprintf(buf, "{038}    /");

	for (z = 0 ; z < 33 - strlen(Most_Type[x]) / 2 ; z++)
	{
		strcat (buf, "=");
	}
	if (strlen(Most_Type[x]) % 2 == 0)
	{
		cat_sprintf(buf, "={138} %s {038}=", Most_Type[x]);
	}
	else
	{
		cat_sprintf(buf, "{138} %s {038}=",  Most_Type[x]);
	}

	for (z = 0 ; z < 33 - strlen(Most_Type[x]) / 2 ; z++)
	{
		strcat (buf, "=");
	}
	cat_sprintf(buf, "\\\n\r");

	cat_sprintf(buf, "{038}    |                                                                      |\n\r");

	z = MAX_HISCORE/2;

	for (hiscore = mud->high_scores[x]->first, i = 0 ; hiscore && i < MAX_HISCORE ; hiscore = hiscore->next, i++)
	{
		if (x == MOST_CASTLE)
		{
			sprintf(hs_buf[i / z][i % z], " | {138}%2d {178}%12s {128}%11s,000 {038}| ", i+1, hiscore->player, tomoney(hiscore->score));
		}
		else if (x == MOST_REINCARNATE)
		{
			sprintf(hs_buf[i / z][i % z], " | {138}%2d {178}%12s {128}%15d {038}| ", i+1, hiscore->player, hiscore->score/100);
		}
		else
		{
			sprintf(hs_buf[i / z][i % z], " | {138}%2d {178}%12s {128}%15d {038}| ", i+1, hiscore->player, hiscore->score);
		}
	}

	for (i = 0 ; i < z ; i++)
	{
		cat_sprintf(buf, "{038}   %s%s\n\r", hs_buf[0][i], hs_buf[1][i]);
	}

	ch_printf_color(ch, "%s{038}    \\======================================================================/\n\r", buf);

	pop_call();
	return;
}

void do_victory_list( CHAR_DATA *ch, char *argument )
{
	char *pt, buf[MAX_STRING_LENGTH];
	int cnt;

	push_call("do_victory_list(%p,%p)",ch,argument);

	sprintf(buf, "{138}             Victor                      Defeated       Date       Time     Year\n\r"
	             "{118}--------------------------------------------------------------------------------\n\r");

	for (cnt = 0 ; cnt < VICTORY_LIST_SIZE ; cnt++)
	{
		if (victory_list[cnt] && strlen(victory_list[cnt]) == 81)
		{
			pt = victory_list[cnt];

			cat_snprintf(buf, 17, "{148}%s",     pt+01);
			cat_snprintf(buf, 21, "{128}%s",     pt+13);
			cat_snprintf(buf, 17, "{148}%s",     pt+29);
			cat_snprintf(buf, 21, "{128}%s",     pt+41);
			cat_snprintf(buf, 16, "{158}%s",     pt+57);
			cat_snprintf(buf, 14, "{178}%s",     pt+68);
			cat_snprintf(buf, 11, "{168}%s\n\r", pt+77);
		}
	}
	send_to_char_color(buf, ch);
	pop_call();
	return;
}

void do_afk( CHAR_DATA *ch, char *argument )
{
	push_call("do_afk(%p,%p)",ch,argument);

	if ( IS_NPC(ch) )
	{
		pop_call();
		return;
	}

	if IS_SET(ch->act, PLR_AFK)
	{
		REMOVE_BIT(ch->act, PLR_AFK);
		send_to_char( "You are no longer afk.\n\r", ch );
		act("$n is no longer afk.", ch, NULL, NULL, TO_ROOM);
		pop_call();
		return;
	}
	else
	{
		SET_BIT(ch->act, PLR_AFK);
		send_to_char( "You are now afk.\n\r", ch );
		act("$n is now afk.", ch, NULL, NULL, TO_ROOM);
		pop_call();
		return;
	}
	pop_call();
	return;
}
