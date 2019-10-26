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

#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/resource.h>
#include "mud.h"

/*
	Local functions.
*/

bool get_obj   args( ( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container, bool fDisplay ) );
bool is_quest  args( ( unsigned char *pt ) );

void do_test( CHAR_DATA *ch, char *argument)
{
	int cnt;

	push_call("do_test(%p,%p)",ch,argument);

	announce_support(ch->desc);

	pop_call();
	return;

	if (*argument == 'a')
	{
		display_appearance_selections(ch, APPEAR_LOOK);

		process_appearance_selections(ch, APPEAR_LOOK, "6");
	}

	if (*argument == 'c')
	{
		for (cnt = 0 ; *cmd_table[cnt].name ; cnt++)
		{
			if (strcmp(cmd_table[cnt].name, cmd_table[cnt+1].name) > 0)
			{
				ch_printf(ch, "%s > %s\n\r", cmd_table[cnt].name, cmd_table[cnt+1].name);
			}
		}
	}

	if (*argument == 'd')
	{
		for (cnt = 0 ; cnt < APPEAR_MAX ; cnt++)
		{
			display_appearance_selections(ch, cnt);
		}
	}

	if (*argument == 'p')
	{
		SET_BIT(mud->flags, MUD_SKIPOUTPUT);

		for (cnt = 0 ; cnt < MAX_PVNUM ; cnt++)
		{
			if (pvnum_index[cnt] == NULL)
			{
				continue;
			}
			if (get_player_world(ch, pvnum_index[cnt]->name) != NULL)
			{
				continue;
			}
			do_pload(ch, pvnum_index[cnt]->name);

			if (argument[1] != 'q' && pvnum_index[cnt])
			{
				do_pquit(ch, pvnum_index[cnt]->name);
			}
		}
		DEL_BIT(mud->flags, MUD_SKIPOUTPUT);
	}

	if (*argument == 'x')
	{
		ch_printf(ch, "Testing: \e[38;5;200mtest\e[38;5;180mtest\e[38;5;160mtest\e[38;5;140mtest\e[38;5;120mtest\e[38;5;100mtest\n");
	}
	pop_call();
	return;
}

void do_appearance(CHAR_DATA *ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	int appearance;

	push_call("do_appearance(%p,%p)",ch,argument);

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	
	if (*arg1 == 0)
	{
		ch_printf(ch, "Syntax: appearance <look|skin|eyes|haircolor|hairtype|hairlength> <argument>\n\r");

		pop_call();
		return;
	}

	if (!str_prefix(arg1, "look"))
	{
		appearance = APPEAR_LOOK;
	}
	else if (!str_prefix(arg1, "skin"))
	{
		appearance = APPEAR_SKINCOLOR;
	}
	else if (!str_prefix(arg1, "eyes"))
	{
		appearance = APPEAR_EYECOLOR;
	}
	else if (!str_prefix(arg1, "haircolor"))
	{
		appearance = APPEAR_HAIRCOLOR;
	}
	else if (!str_prefix(arg1, "hairtype"))
	{
		appearance = APPEAR_HAIRTYPE;
	}
	else if (!str_prefix(arg1, "hairlength"))
	{
		appearance = APPEAR_HAIRLENGTH;
	}
	else
	{
		do_appearance(ch, "");

		pop_call();
		return;
	}

	if (*arg2 == 0)
	{
		display_appearance_selections(ch, appearance);
	}
	else
	{
		process_appearance_selections(ch, appearance, arg2);
	}
	ch_printf(ch, "\n\r");

	pop_call();
	return;
}

void do_wizhelp( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	int cmd, col, level;

	push_call("do_wizhelp(%p,%p)",ch,argument);

	for (level = 96, col = 0, buf[0] = '\0' ; level <= ch->level ; level++)
	{
		for (cmd = 0 ; cmd_table[cmd].name[0] != '\0' ; cmd++)
		{
			if (cmd_table[cmd].level == level && !HAS_BIT(cmd_table[cmd].flags, CMD_HIDE))
			{
				if (col % 5 == 0)
				{
					strcat(buf, get_color_string(ch, COLOR_TEXT, VT102_DIM));
				}
				cat_snprintf(buf, 16, "%d-%-13s", cmd_table[cmd].level, cmd_table[cmd].name);
				if (++col % 5 == 0)
				{
					strcat(buf, "\n\r");
				}
			}
		}
	}
	if (col % 5 != 0)
	{
		strcat(buf, "\n\r");
	}
	send_to_char(buf, ch);
	pop_call();
	return;
}




void do_rstat( CHAR_DATA *ch, char *argument )
{
	int location;
	OBJ_DATA        *obj;
	CHAR_DATA       *rch;
	char arg1[MAX_INPUT_LENGTH];
	char buf1[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];
	char bld [10], dim [10];
	int door;

	push_call("do_rstat(%p,%p)",ch,argument);

	one_argument(argument, arg1);

	strcpy(dim, get_color_string(ch, COLOR_PROMPT,  VT102_DIM ));
	strcpy(bld, get_color_string(ch, COLOR_PROMPT,  VT102_BOLD));

	location = (arg1[0] == '\0') ? ch->in_room->vnum : find_location(ch, arg1);

	if (location == -1)
	{
		send_to_char( "No such location.\n\r", ch );
		pop_call();
		return;
	}

	if (!can_olc_modify(ch, location))
	{
		send_to_char("You can only stat rooms in your allocated range.\n\r", ch);
		pop_call();
		return;
	}

	ch_printf(ch, "%s       Name:%s %s\n\r", dim, bld, room_index[location]->name);
	ch_printf(ch, "%s       Area:%s %s\n\r", dim, bld, room_index[location]->area->name);
	ch_printf(ch, "%s       Vnum:%s %u\n\r", dim, bld, room_index[location]->vnum);
	ch_printf(ch, "%s     Sector:%s %s\n\r", dim, bld, broken_bits(room_index[location]->sector_type, "SECT_", TRUE));
	ch_printf(ch, "%s      Light:%s %d\n\r", dim, bld, room_index[location]->light);

	ch_printf(ch, "%s Room flags: %s%s\n\r", dim, bld, flag_string(room_index[location]->room_flags, r_flags));

	if (room_index[location]->first_extradesc != NULL )
	{
		EXTRA_DESCR_DATA *ed;

		sprintf(buf1, "%sExtra Descs:%s ",      dim, bld);
		sprintf(buf2, "%s,%s ",                 dim, bld);
		for (ed = room_index[location]->first_extradesc ; ed ; ed = ed->next)
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
	for (rch = room_index[location]->first_person ; rch ; rch = rch->next_in_room)
	{
		if (can_see(ch, rch))
		{
			one_argument_nolower(rch->name, buf3);
			if (str_suffix(" ", buf1))
			{
				strcat(buf1, buf2);
			}
			strcat(buf1, buf3);
		}
	}
	ch_printf(ch, "%s\n\r", buf1);

	if (room_index[location]->first_content)
	{
		sprintf(buf1, "%s    Objects:%s ",      dim, bld);
		sprintf(buf2, "%s,%s ",                 dim, bld);
		for (obj = room_index[location]->first_content ; obj ; obj = obj->next_content)
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

		if ((pexit = room_index[location]->exit[door]) != NULL)
		{
			sprintf(buf1,"%s      %5s:%s %5d", dim, capitalize(dir_name[door]), bld, pexit->to_room);
			sprintf(buf2,"%s Key:%s %5d",     dim, bld, pexit->key);
			sprintf(buf3,"%s Flags:%s %s",      dim, bld, flag_string(pexit->flags, exit_flags));

			ch_printf(ch, "%s%s%s\n\r", buf1, buf2, buf3);
		}
	}

	pop_call();
	return;
}

void do_ostat( CHAR_DATA *ch, char *argument )
{
	char buf [MAX_STRING_LENGTH];
	char buf1[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH], buf4[MAX_STRING_LENGTH];
	char bld [10], dim [10];
	char *owner;

	AFFECT_DATA *paf;
	OBJ_DATA    *obj;

	push_call("do_ostat(%p,%p)",ch,argument);

	if (*argument == '\0')
	{
		send_to_char( "Syntax: stat obj <vnum|name>\n\r", ch );
		pop_call();
		return;
	}

	if (is_number(argument))
	{
		if ((obj = get_obj_vnum(ch, argument)) == NULL)
		{
			send_to_char( "No object loaded has that vnum.\n\r", ch );
			pop_call();
			return;
		}
	}
	else
	{
		if ((obj = get_obj_world(ch, argument)) == NULL)
		{
			send_to_char( "No object loaded has that name.\n\r", ch );
			pop_call();
			return;
		}
	}

	if (!can_olc_modify(ch, obj->pIndexData->vnum))
	{
		send_to_char("You can only stat objects in your allocated vnum range.\n\r", ch);
		pop_call();
		return;
	}

	strcpy(dim, get_color_string(ch, COLOR_PROMPT,  VT102_DIM ));
	strcpy(bld, get_color_string(ch, COLOR_PROMPT,  VT102_BOLD));

	ch_printf(ch, "%s    Name:%s %s\n\r", dim, bld, obj->name);
	ch_printf(ch, "%s   Short:%s %s\n\r", dim, bld, obj->short_descr);
	ch_printf(ch, "%s    Long:%s %s\n\r", dim, bld, obj->long_descr);

	if (obj->pIndexData->first_extradesc)
	{
		EXTRA_DESCR_DATA *ed;

		sprintf(buf1, "%sKeywords:%s", dim, bld);

		for (ed = obj->pIndexData->first_extradesc ; ed != NULL ; ed = ed->next)
		{
			strcat(buf1, " ");
			strcat(buf1, ed->keyword);
		}

		ch_printf(ch, "%s\n\r", buf1);
	}

	if (obj->carried_by != NULL)
	{
		ch_printf(ch, "%s Carrier:%s %s\n\r", dim, bld, get_name(obj->carried_by));
	}

	sprintf(buf1, "%s    Vnum:%s %10u",   dim, bld, obj->pIndexData->vnum);
	sprintf(buf2, "%s Index  :%s %30lld", dim, bld, obj->obj_ref_key);
	sprintf(buf3, "%s   Total:%s %10d",   dim, bld, obj->pIndexData->total_objects);

	ch_printf(ch, "%s%s%s\n\r", buf1, buf2, buf3);

	sprintf(buf , "%d/%d", obj->level,  obj_level_estimate(obj->pIndexData));
	sprintf(buf1, "%s   Level:%s %10s", dim, bld, buf);
	sprintf(buf2, "%s    Cost:%s %10d", dim, bld, obj->cost);
	sprintf(buf , "%d+%d", obj->weight, obj->content_weight);
	sprintf(buf3, "%s  Weight:%s %10s", dim, bld, buf);
	sprintf(buf4, "%s    Type:%s %10s", dim, bld, item_type_name(obj));

	ch_printf(ch, "%s%s%s%s\n\r", buf1, buf2, buf3, buf4);

	sprintf(buf1, "%s    Room:%s %10u", dim, bld, obj->in_room ? obj->in_room->vnum : 0);
	sprintf(buf2, "%s  In obj:%s %10u", dim, bld, obj->in_obj ? obj->in_obj->pIndexData->vnum : 0);
	sprintf(buf3, "%s Wearloc:%s %10d", dim, bld, obj->wear_loc);
	sprintf(buf4, "%s In game:%s %10d", dim, bld, obj->pIndexData->total_objects);

	ch_printf(ch, "%s%s%s%s\n\r", buf1, buf2, buf3, buf4);

	sprintf(buf1, "%s  Value0:%s %10d", dim, bld, obj->value[0]);
	sprintf(buf2, "%s  Value1:%s %10d", dim, bld, obj->value[1]);
	sprintf(buf3, "%s  Value2:%s %10d", dim, bld, obj->value[2]);
	sprintf(buf4, "%s  Value3:%s %10d", dim, bld, obj->value[3]);

	ch_printf(ch, "%s%s%s%s\n\r", buf1, buf2, buf3, buf4);

	sprintf(buf1, "%sEnhanced:%s %10s", dim, bld, obj->first_affect ? "true" : "false");
	sprintf(buf2, "%s Altered:%s %10s", dim, bld, HAS_BIT(obj->extra_flags, ITEM_MODIFIED) ? "true" : "false");
	sprintf(buf3, "%s Program:%s %10s", dim, bld, obj->pIndexData->first_oprog ? "true" : "false");
	if ((owner = get_name_pvnum(obj->owned_by)) != NULL)
	{
		sprintf(buf4, "%s   Owner:%s %10s", dim, bld, owner);
	}
	else
	{
		sprintf(buf4, "%s   Owner:%s %10d", dim, bld, obj->owned_by);
	}

	ch_printf(ch, "%s%s%s%s\n\r", buf1, buf2, buf3, buf4);

	sprintf(buf1, "%sSactimer:%s %10d", dim, bld, obj->sac_timer);
	sprintf(buf2, "%s   Timer:%s %10d", dim, bld, obj->timer);

	ch_printf(ch, "%s%s\n\r", buf1, buf2);

	ch_printf(ch, "%sWearflag:%s %s\n\r", dim, bld, flag_string(obj->wear_flags, w_flags));
	ch_printf(ch, "%sItemflag:%s %s\n\r", dim, bld, flag_string(obj->extra_flags, o_flags));

	for (paf = obj->pIndexData->first_affect ; paf != NULL ; paf = paf->next)
	{
		ch_printf(ch, "%s Affects:%s %-22s%s modifies %s by %d for %d hours\n\r", dim, bld, skill_table[(int) paf->type].name, dim, affect_loc_name(paf->location), paf->modifier, paf->duration);
	}
	for (paf = obj->first_affect ; paf != NULL ; paf = paf->next)
	{
		ch_printf(ch, "%s Affects:%s %-22s%s modifies %s by %d for %d hours\n\r", dim, bld, skill_table[(int) paf->type].name, dim, affect_loc_name(paf->location), paf->modifier, paf->duration);
	}

	ch_printf(ch, "%s   Quest:%s %-22s %s#%s\n\r", dim, bld, obj->pIndexData->area->name, dim, quest_bits_to_string(obj->obj_quest));

	if (obj->reset != NULL && obj->reset->obj == obj)
	{
		RESET_DATA *pReset = obj->reset;

		while (pReset != NULL)
		{
			ch_printf(ch, "%s  Reset:%s %c %s %5d %4d %5d\n\r",
				dim, bld, pReset->command, dim,
				pReset->arg1, pReset->arg2, pReset->arg3);

			pReset = pReset->container;
		}
	}
	pop_call();
	return;
}

void do_mstat( CHAR_DATA *ch, char *argument )
{
	char tmp[MAX_STRING_LENGTH];
	char buf1[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];
	char bold[10], dim[10];
	unsigned long num;
	AFFECT_DATA *paf;
	CHAR_DATA   *victim;
	OBJ_DATA    *wield;
	AREA_DATA   *pArea;

	push_call("do_mstat(%p,%p)",ch,argument);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax: stat mob <vnum|name>\n\r", ch);
		pop_call();
		return;
	}

	if (is_number_argument(argument))
	{
		if ((victim = get_mob_vnum(ch, argument)) == NULL)
		{
			send_to_char( "No mobile loaded with that vnum.\n\r", ch );
			pop_call();
			return;
		}
	}
	else if ((victim = get_char_world(ch, argument)) == NULL)
	{
		send_to_char( "No mobile loaded with that name.\n\r", ch );
		pop_call();
		return;
	}

	if (IS_NPC(victim) && !can_olc_modify(ch, victim->pIndexData->vnum))
	{
		send_to_char("You can only stat mobiles in your allocated vnum range.\n\r", ch);
		pop_call();
		return;
	}

	strcpy(dim,   get_color_string(ch, COLOR_PROMPT,  VT102_DIM ));
	strcpy(bold,  get_color_string(ch, COLOR_PROMPT,  VT102_BOLD));
	tmp[0] = '\0';

	if (IS_NPC(victim))
	{
		ch_printf(ch, "%s        Name: %s%s\n\r"  , dim, bold, victim->name);
		ch_printf(ch, "%s       Short: %s%s\n\r"  , dim, bold, victim->short_descr);
		ch_printf(ch, "%s        Long: %s%s\n\r"  , dim, bold, victim->long_descr);

		sprintf(buf1, "%s        Vnum: %s%11u "   , dim, bold, victim->pIndexData->vnum);
		sprintf(buf2, "%s       Level: %s%11d "   , dim, bold, victim->level);
		sprintf(buf3, "%s        Room: %s%11u\n\r", dim, bold, victim->in_room->vnum);

		ch_printf(ch, "%s%s%s", buf1, buf2, buf3);

		sprintf(buf1, "%s   Alignment: %s%11d "   , dim, bold, victim->alignment);
		sprintf(buf2, "%s Armor Class: %s%11d "   , dim, bold, 0 - (mob_armor(victim) + victim->level*4 - 100));
		sprintf(buf3, "%s        Gold: %s%11d\n\r", dim, bold, victim->gold);

		ch_printf(ch, "%s%s%s", buf1, buf2, buf3);

		sprintf(buf1, "%s     Hitroll: %s%11d "   , dim, bold, GET_HITROLL(victim));
		sprintf(tmp,  "%dd%d+%d", get_damnodice(victim), get_damsizedice(victim), GET_DAMROLL(victim) + victim->npcdata->damplus);
		sprintf(buf2, "%s     Damroll: %s%11s "   , dim, bold, tmp);
		if ((wield = get_eq_char(victim, WEAR_WIELD)) != NULL && wield->item_type == ITEM_WEAPON)
		{
			sprintf(tmp, "%dd%d", wield->value[1], wield->value[2]);
		}
		else
		{
			sprintf(tmp, "none");
		}
		sprintf(buf3, "%s      Weapon: %s%11s\n\r", dim, bold, tmp);

		ch_printf(ch, "%s%s%s", buf1, buf2, buf3);

		sprintf(tmp, "%d/%d", victim->hit, victim->max_hit);
		sprintf(buf1, "%s          Hp: %s%11s "   , dim, bold, tmp);
		sprintf(tmp, "%d/%d", victim->mana, victim->max_mana);
		sprintf(buf2, "%s        Mana: %s%11s "   , dim, bold, tmp);
		sprintf(tmp, "%d/%d", victim->move, victim->max_move);
		sprintf(buf3, "%s        Move: %s%11s\n\r", dim, bold, tmp);

		ch_printf(ch, "%s%s%s", buf1, buf2, buf3);

		sprintf(buf1, "%s        Race: %s%11s "   , dim, bold, race_table[victim->race].race_name);
		sprintf(buf2, "%s         Sex: %s%11s "   , dim, bold, victim->sex <= 0 ? "Neutral" : victim->sex <= 1 ? "Male" : "Female");
		sprintf(buf3, "%s    Position: %s%11s\n\r", dim, bold, victim->position == 8 ? "Standing" : victim->position == 7 ? "fighting" : victim->position == 6 ? "sitting" : victim->position == 5 ? "resting" : victim->position == 4 ? "sleeping" : "dying");

		ch_printf(ch, "%s%s%s", buf1, buf2, buf3);

		sprintf(buf1, "%sCarry Number: %s%11d "   , dim, bold, victim->carry_number);
		sprintf(buf2, "%sCarry Weight: %s%11d "   , dim, bold, victim->carry_weight);
		sprintf(buf3, "%sSaving Spell: %s%11d\n\r", dim, bold, GET_SAVING_THROW(victim));

		ch_printf(ch, "%s%s%s", buf1, buf2, buf3);

		sprintf(buf1, "%s   %9s: %s%s\n\r"        , dim, HAS_BIT(victim->act, ACT_WIMPY) ? "Fearing" : "Hating", bold, *victim->npcdata->hate_fear ? victim->npcdata->hate_fear : "none");

		ch_printf(ch, "%s%s%s", buf1, buf2, buf3);

		sprintf(buf1, "%s      Master: %s%11s "   , dim, bold, victim->master   ? victim->master->name        : "none");
		sprintf(buf2, "%s      Leader: %s%11s "   , dim, bold, victim->leader   ? victim->leader->name        : "none");
		sprintf(buf3, "%s    Fighting: %s%11s\n\r", dim, bold, victim->fighting ? victim->fighting->who->name : "none");

		ch_printf(ch, "%s%s%s", buf1, buf2, buf3);

		if (victim->pIndexData->spec_fun != NULL)
		{
			ch_printf(ch, "%s    Fun Spec: %s%s\n\r", dim, bold, spec_name_lookup(victim->pIndexData->spec_fun));
		}
		ch_printf(ch, "%sAction Flags: %s%s\n\r", dim, bold, flag_string(victim->act, act_flags));
		ch_printf(ch, "%sAffect Flags: %s%s\n\r", dim, bold, affect_bit_name(victim->affected_by));
		ch_printf(ch, "%sAffect Flags: %s%s\n\r", dim, bold, affect_bit_name(-victim->affected2_by));

		for (paf = victim->first_affect ; paf != NULL ; paf = paf->next)
		{
			ch_printf(ch, "%s       Spell: %s%-22s%s modifies %s by %d for %d hours\n\r", dim, bold, skill_table[(int) paf->type].name, dim, affect_loc_name(paf->location), paf->modifier, paf->duration);
		}
		ch_printf(ch, "%s       Quest: %s%-22s%s #%s\n\r", dim, bold, victim->pIndexData->area->name, dim, quest_bits_to_string(victim->npcdata->mob_quest));
	}
	else
	{
		ch_printf(ch, "%s        Name: %s%s\n\r"  , dim, bold, victim->name);

		ch_printf(ch, "%s        Vnum: %s%11u %s       Level: %s%11d %s        Room: %s%11u\n\r",
			dim, bold, victim->pcdata->pvnum,
			dim, bold, victim->level,
			dim, bold, victim->in_room->vnum);
		ch_printf(ch, "%s         Age: %s%11d %s       Hours: %s%11d %s       Reinc: %s%11d\n\r",
			dim, bold, get_age(victim),
			dim, bold, victim->pcdata->played/3600 + victim->pcdata->previous_hours,
			dim, bold, victim->pcdata->reincarnation);

		ch_printf(ch, "%s       Class: %s%11s %s  Experience: %s%11d %s   Practices: %s%11d\n\r",
			dim, bold, class_table[victim->class].who_name,
			dim, bold, victim->pcdata->exp,
			dim, bold, victim->pcdata->practice);
		ch_printf(ch, "%s        Race: %s%11s %s         Sex: %s%11s %s    Position: %s%11s\n\r",
			dim, bold, race_table[victim->race].race_name,
			dim, bold, victim->sex <= 0 ? "Neutral" : victim->sex == 1 ? "Male" : "Female",
			dim, bold, victim->position == 8 ? "Standing" : victim->position == 7 ? "fighting" : victim->position == 6 ? "sitting" : victim->position == 5 ? "resting" : victim->position == 4 ? "sleeping" : "dying");

		ch_printf(ch, "%s      Thirst: %s%11d %s        Full: %s%11d %s       Drunk: %s%11d\n\r",
			dim, bold, victim->pcdata->condition[COND_THIRST],
			dim, bold, victim->pcdata->condition[COND_FULL],
			dim, bold, victim->pcdata->condition[COND_DRUNK]);

		ch_printf(ch, "%s  Attributes:  Str:%s %2d/%-2d    %sInt:%s %2d/%-2d  %sWis:%s %2d/%-2d    %sDex:%s %2d/%-2d  %sCon:%s %2d/%-2d\n\r",
			dim, bold, get_curr_str(victim), victim->pcdata->perm_str,
			dim, bold, get_curr_int(victim), victim->pcdata->perm_int,
			dim, bold, get_curr_wis(victim), victim->pcdata->perm_wis,
			dim, bold, get_curr_dex(victim), victim->pcdata->perm_dex,
			dim, bold, get_curr_con(victim), victim->pcdata->perm_con);

		ch_printf(ch, "%s   Alignment: %s%11d %s Armor Class: %s%11d %s        Gold: %s%11d\n\r",
			dim, bold, victim->alignment,
			dim, bold, GET_AC(victim),
			dim, bold, victim->gold);

		if ((wield = get_eq_char(victim, WEAR_WIELD)) != NULL && wield->item_type == ITEM_WEAPON)
		{
			sprintf(tmp, "%dd%d", wield->value[1], wield->value[2]      );
		}
		else
		{
			sprintf(tmp, "%dd%d", get_damnodice(ch), get_damsizedice(ch));
		}
		ch_printf(ch, "%s     Hitroll: %s%11d %s     Damroll: %s%11d %s%s: %s%11s\n\r",
			dim, bold, GET_HITROLL(victim),
			dim, bold, GET_DAMROLL(victim),
			dim, wield ? "      Weapon" : "  Barehanded", bold, tmp);

		sprintf(buf1, "%s    Aging Ac: %s%11d "   , dim, bold, get_age_bonus(victim, RSPEC_AGE_AC));
		sprintf(buf2, "%s    Aging Hr: %s%11d "   , dim, bold, get_age_bonus(victim, RSPEC_AGE_HR));
		sprintf(buf3, "%s    Aging Dr: %s%11d\n\r", dim, bold, get_age_bonus(victim, RSPEC_AGE_DR));

		ch_printf(ch, "%s%s%s", buf1, buf2, buf3);

		sprintf(buf1, "%sCarry Number: %s%11d "   , dim, bold, victim->carry_number);
		sprintf(buf2, "%sCarry Weight: %s%11d "   , dim, bold, victim->carry_weight);
		sprintf(buf3, "%sSaving Spell: %s%11d\n\r", dim, bold, GET_SAVING_THROW(victim));

		ch_printf(ch, "%s%s%s", buf1, buf2, buf3);

		sprintf(tmp, "%d/%d", victim->hit, victim->max_hit);
		sprintf(buf1, "%s          Hp: %s%11s "   , dim, bold, tmp);
		sprintf(tmp, "%d/%d", victim->mana, victim->max_mana);
		sprintf(buf2, "%s        Mana: %s%11s "   , dim, bold, tmp);
		sprintf(tmp, "%d/%d", victim->move, victim->max_move);
		sprintf(buf3, "%s        Move: %s%11s\n\r", dim, bold, tmp);

		ch_printf(ch, "%s%s%s", buf1, buf2, buf3);

		sprintf(buf1, "%s       Wimpy: %s%11d "   , dim, bold, victim->pcdata->wimpy);
		sprintf(buf2, "%sCastle Entra: %s%11d "   , dim, bold, victim->pcdata->castle ? victim->pcdata->castle->entrance : 0);
		sprintf(buf3, "%s Castle Cost: %s%11d\n\r", dim, bold, victim->pcdata->castle ? victim->pcdata->castle->cost / 1000 : 0);

		ch_printf(ch, "%s%s%s", buf1, buf2, buf3);

		sprintf(buf1, "%s      Master: %s%11s "   , dim, bold, victim->master   ? victim->master->name        : "none");
		sprintf(buf2, "%s      Leader: %s%11s "   , dim, bold, victim->leader   ? victim->leader->name        : "none");
		sprintf(buf3, "%s    Fighting: %s%11s\n\r", dim, bold, victim->fighting ? victim->fighting->who->name : "none");

		ch_printf(ch, "%s%s%s", buf1, buf2, buf3);

		ch_printf(ch, "%sAction Flags: %s%s\n\r", dim, bold, flag_string(victim->act, plr_flags));
		ch_printf(ch, "%sAffect Flags: %s%s\n\r", dim, bold, affect_bit_name(victim->affected_by));
		ch_printf(ch, "%sAffect Flags: %s%s\n\r", dim, bold, affect_bit_name(-victim->affected2_by));

		for (paf = victim->first_affect ; paf != NULL ; paf = paf->next)
		{
			ch_printf(ch, "%s       Spell: %s%-22s%s modifies %s by %d for %d hours\n\r", dim, bold, skill_table[(int) paf->type].name, dim, affect_loc_name(paf->location), paf->modifier, paf->duration);
		}

		for (pArea = mud->f_area ; pArea ; pArea = pArea->next)
		{
			num = pArea->low_r_vnum/100;
			if (is_quest(victim->pcdata->quest[num]))
			{
				ch_printf(ch, "%s       Quest: %s%-22s%s #%s\n\r", dim, bold, pArea->name, dim, quest_bits_to_string(victim->pcdata->quest[num]));
			}
		}
	}
	pop_call();
	return;
}

int cnt_op_lines( AREA_DATA *area )
{
	int cnt, vnum;
	OBJ_PROG *oprog;
	MUD_PROG *mprog;
	MP_TOKEN *token;

	push_call("cnt_op_lines(%p)",area);

	for (cnt = 0, vnum = area->low_o_vnum ; vnum <= area->hi_o_vnum ; vnum++)
	{
		if (obj_index[vnum] == NULL)
		{
			continue;
		}

		for (oprog = obj_index[vnum]->first_oprog ; oprog ; oprog = oprog->next)
		{
			cnt++;
		}

		for (mprog = obj_index[vnum]->first_prog ; mprog ; mprog = mprog->next)
		{
			for (token = mprog->first_token ; token ; token = token->next)
			{
				cnt++;
			}
		}
	}
	pop_call();
	return cnt;
}

int cnt_mp_lines( AREA_DATA *area )
{
	int cnt, vnum;
	MUD_PROG *mprog;
	MP_TOKEN *token;

	push_call("cnt_mp_lines(%p)",area);

	for (cnt = 0, vnum = area->low_m_vnum ; vnum <= area->hi_m_vnum ; vnum++)
	{
		if (mob_index[vnum] == NULL)
		{
			continue;
		}

		for (mprog = mob_index[vnum]->first_prog ; mprog ; mprog = mprog->next)
		{
			for (token = mprog->first_token ; token ; token = token->next)
			{
				cnt++;
			}
		}
	}
	pop_call();
	return cnt;
}

int cnt_rp_lines( AREA_DATA *area )
{
	int cnt, vnum;
	MUD_PROG *mprog;
	MP_TOKEN *token;

	push_call("cnt_mp_lines(%p)",area);

	for (cnt = 0, vnum = area->low_r_vnum ; vnum <= area->hi_r_vnum ; vnum++)
	{
		if (room_index[vnum] == NULL)
		{
			continue;
		}

		for (mprog = room_index[vnum]->first_prog ; mprog ; mprog = mprog->next)
		{
			for (token = mprog->first_token ; token ; token = token->next)
			{
				cnt++;
			}
		}
	}
	pop_call();
	return cnt;
}

void do_astat( CHAR_DATA *ch, char *argument )
{
	AREA_DATA *pArea;
	bool found;

	push_call("do_astat(%p,%p)",ch,argument);

	found = FALSE;

	if (argument[0] != '\0')
	{
		for (pArea = mud->f_area ; pArea ; pArea = pArea->next)
		{
			if (!strcasecmp(pArea->filename, argument)
			|| !str_prefix(argument, pArea->name))
			{
				found = TRUE;
				break;
			}
		}

		if (!found)
		{
			if (argument[0] != '\0' )
			{
				send_to_char( "Syntax: stat area [filename|areaname]\n\r", ch );
				pop_call();
				return;
			}
		}
	}
	else
	{
		pArea = ch->in_room->area;
	}

	if (!can_olc_modify(ch, pArea->low_r_vnum))
	{
		ch_printf(ch, "You can only stat areas in your allocated vnum range\n\r");
		pop_call();
		return;
	}

	ch_printf_color( ch, "{078}      name: {178}%s\n\r",		pArea->name);
	ch_printf_color( ch, "{078}  filename: {178}%-20s\n\r",	pArea->filename);
	ch_printf_color( ch, "{078}   authors: {178}%s\n\r",		pArea->authors);
	ch_printf_color( ch, "{078}  resetmsg: {178}%s\n\r",		pArea->resetmsg);
	ch_printf_color( ch, "{078}area flags: {178}%s\n\r",		flag_string(pArea->flags, area_flags));
	ch_printf_color( ch, "{078}     Quest: {178}%s\n\r\n\r",	quest_bits_to_string(pArea->area_quest));
	ch_printf_color( ch, "{078}       age: {178}%5d         {078}           players: {178}%5d\n\r",	pArea->age,			pArea->nplayer);
	ch_printf_color( ch, "{078}     level: {178}%5d         {078}             count: {178}%5d\n\r",	pArea->average_level,	pArea->count);
	ch_printf_color( ch, "{078}    mprogs: {178}%5d         {078}            oprogs: {178}%5d\n\r",	cnt_mp_lines(pArea),	cnt_op_lines(pArea));
	ch_printf_color( ch, "{078}    rprogs: {178}%5d         {078}            tprogs: {178}%5d\n\r", cnt_rp_lines(pArea), cnt_mp_lines(pArea) + cnt_op_lines(pArea) + cnt_rp_lines(pArea));
	ch_printf_color( ch, "{078} low_range: {178}%5d         {078}          hi_range: {178}%5d\n\r",	pArea->olc_range_lo,	pArea->olc_range_hi);
	ch_printf_color( ch, "{078}  low_room: {178}%5d         {078}           hi_room: {178}%5d\n\r",	pArea->low_r_vnum,		pArea->hi_r_vnum);
	ch_printf_color( ch, "{078}   low_obj: {178}%5d         {078}            hi_obj: {178}%5d\n\r",	pArea->low_o_vnum,		pArea->hi_o_vnum);
	ch_printf_color( ch, "{078}   low_mob: {178}%5d         {078}            hi_mob: {178}%5d\n\r",	pArea->low_m_vnum,		pArea->hi_m_vnum);
	ch_printf_color( ch, "{078}soft range: {178}%2d-%2d         {078}        hard range: {178}%2d-%2d\n\r",	pArea->low_soft_range,	pArea->hi_soft_range, pArea->low_hard_range, pArea->hi_hard_range);

	pop_call();
	return;
}


void do_racestat( CHAR_DATA *ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	int race, i;

	push_call("do_racestat(%p,%p)",ch,argument);

	if ((race = lookup_race(argument)) == -1)
	{
		send_to_char("Syntax: stat race <name>\n\r", ch);
		pop_call();
		return;
	}

	ch_printf_color(ch, "{078}     Vnum:{178} %d\n\r", race);
	ch_printf_color(ch, "{078}     Name:{178} %s\n\r", race_table[race].race_name);
	ch_printf_color(ch, "{078}    Stats:{178} STR %2d   DEX %2d   INT %2d   WIS %2d   CON : %2d\n\r",
			race_table[race].race_mod[0],
			race_table[race].race_mod[1],
			race_table[race].race_mod[2],
			race_table[race].race_mod[3],
			race_table[race].race_mod[4]);

	for (buf[0] = '\0', i = 0 ; i < MAX_CLASS ; i++)
	{
		if (race_table[race].race_class[i] == 1)
		{
			cat_sprintf(buf, "%s ", class_table[i].who_name);
		}
	}
	ch_printf_color(ch, "{078}  Classes:{178} %s\n\r", buf);
	ch_printf_color(ch, "{078}    Speed:{178} %s\n\r",
		race_table[race].max_speed == 0 ? "walk"   :
		race_table[race].max_speed == 1 ? "normal" :
		race_table[race].max_speed == 2 ? "jog"    :
		race_table[race].max_speed == 3 ? "run"    :
		race_table[race].max_speed == 4 ? "haste"  :
		race_table[race].max_speed == 5 ? "blaze"  : "unknown");

	ch_printf_color(ch, "{078}Move Rate:{178} %d\n\r", race_table[race].move_rate);
	ch_printf_color(ch, "{078}   Vision:{178} %s\n\r",
		race_table[race].vision == 0 ? "normal"    :
		race_table[race].vision == 1 ? "night"     :
		race_table[race].vision == 2 ? "dark"      : "unknown");

	ch_printf_color(ch, "{078}Modifiers:{178} HP %d  MANA %d  MOVE %d\n\r",
			race_table[race].hp_mod,
			race_table[race].mana_mod,
			race_table[race].move_mod);

	ch_printf_color(ch, "{078}   Reincs:{178} Enters at %d, Leaves at %d\n\r",
			race_table[race].enter,
			race_table[race].leave);

	ch_printf_color(ch, "{078}    Specs:{178} %s\n\r", flag_string(race_table[race].flags, race_specs));

	pop_call();
	return;
}


void do_classstat( CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	int cnt;

	push_call("do_classstat(%p,%p)",ch,argument);

	for (cnt = 0 ; cnt < MAX_CLASS ; cnt++)
	{
		if (is_name_short(argument, (char *) class_table[cnt].name))
		{
			ch_printf(ch, "         Class name: %s\n\r", class_table[cnt].name);

			switch(class_table[cnt].attr_prime)
			{
				case APPLY_STR : strcpy(buf, "Str"); break;
				case APPLY_DEX : strcpy(buf, "Dex"); break;
				case APPLY_INT : strcpy(buf, "Int"); break;
				case APPLY_WIS : strcpy(buf, "Wis"); break;
				case APPLY_CON : strcpy(buf, "Con"); break;
				default :        strcpy(buf, "Unk");break;
			}

			ch_printf(ch, "  Primary attribute: %s", buf);

			switch(class_table[cnt].attr_second)
			{
				case APPLY_STR : strcpy(buf, "Str"); break;
				case APPLY_DEX : strcpy(buf, "Dex"); break;
				case APPLY_INT : strcpy(buf, "Int"); break;
				case APPLY_WIS : strcpy(buf, "Wis"); break;
				case APPLY_CON : strcpy(buf, "Con"); break;
				default :        strcpy(buf, "Unk");break;
			}

			ch_printf(ch, "Secondary attribute: %s", buf);

			ch_printf(ch, "       First weapon: %s\n\r", obj_index[class_table[cnt].weapon]->short_descr);

			ch_printf(ch, "        Skill adept: %d\n\r", class_table[cnt].skill_adept);
			ch_printf(ch, "       Skill master: %d\n\r", class_table[cnt].skill_master);

			ch_printf(ch, "            Hp gain: %d - %d\n\r", class_table[cnt].hp_min, class_table[cnt].hp_max);
			ch_printf(ch, "          Mana gain: %d - %d\n\r", class_table[cnt].mana_min, class_table[cnt].mana_max);

			ch_printf(ch, "        Description: %s\n\r", class_table[cnt].desc);

			ch_printf(ch, "        Class flags: %s\n\r", flag_string(class_table[cnt].spec, class_flags));

			pop_call();
			return;
		}
	}
	send_to_char("Syntax: stat class <name>\n\r",ch);
	pop_call();
	return;
}


void do_slookup( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	char bld[10], dim[10];
	int sn;

	push_call("do_slookup(%p,%p)",ch,argument);

	one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		send_to_char( "Slookup what?\n\r", ch );
		pop_call();
		return;
	}

	strcpy(bld, ansi_translate_text(ch, "{178}"));
	strcpy(dim, ansi_translate_text(ch, "{078}"));

	if (!strcasecmp(arg, "all"))
	{
		for (buf[0] = '\0', sn = 0 ; sn < MAX_SKILL ; sn++)
		{
			if (skill_table[sn].name == NULL)
			{
				break;
			}
			cat_sprintf(buf, "%sSn:%s %3d   %sSlot:%s %3d   %s%s '%s%s%s'\n\r", dim, bld, sn, dim, bld, skill_table[sn].slot, dim, is_spell(sn) ? "Spell" : "Skill", bld, skill_table[sn].name, dim);
		}
		send_to_char(buf, ch);
	}
	else
	{
		if ((sn = skill_lookup(arg)) >= 0)
		{
			ch_printf_color(ch, "{078}Sn:{178} %3d   {078}Slot:{178} %3d   {078}%s '{178}%s{078}'\n\r", sn, skill_table[sn].slot, is_spell(sn) ? "Spell" : "Skill", skill_table[sn].name);
			pop_call();
			return;
		}
		send_to_char("No such skill or spell.\n\r", ch);
	}
	pop_call();
	return;
}


void do_stat( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];

	push_call("do_stat(%p,%p)",ch,argument);

	argument = one_argument(argument, arg);

	if (IS_NPC(ch))
	{
		log_printf("do_stat: used by mob");
		dump_stack();
		pop_call();
		return;
	}

	if (!strcasecmp(arg, "mob"))
	{
		do_mstat(ch, argument);
	}
	else if (!strcasecmp(arg, "obj"))
	{
		do_ostat(ch, argument);
	}
	else if (!strcasecmp(arg, "room"))
	{
		do_rstat(ch, argument);
	}
	else if (!strcasecmp(arg, "area"))
	{
		do_astat(ch, argument);
	}
	else if (IS_GOD(ch) && !strcasecmp(arg, "class"))
	{
		do_classstat(ch, argument);
	}
	else if (IS_GOD(ch) && !strcasecmp(arg, "race"))
	{
		do_racestat(ch, argument);
	}
	else if (IS_GOD(ch) && !strcasecmp(arg, "skill"))
	{
		do_slookup(ch, argument);
	}
	else
	{
		if (IS_GOD(ch))
		{
			send_to_char("Syntax: stat <mob|obj|room|class|race|area|skill> <argument>\n\r", ch);
		}
		else
		{
			send_to_char("Syntax: stat <mob|obj|room|area> <argument>\n\r", ch);
		}
	}
	pop_call();
	return;
}


void do_mpcommands(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	int cmd, col, level;

	push_call("do_mpcommands(%p,%p)",ch,argument);

	for (level = 96, col = 0, buf[0] = '\0' ; level <= MAX_LEVEL ; level++)
	{
		for (cmd = 0 ; cmd_table[cmd].name[0] != '\0'; cmd++)
		{
			if (cmd_table[cmd].level == level
			&& !str_infix("mp", cmd_table[cmd].name)
			&& HAS_BIT(cmd_table[cmd].flags, CMD_HIDE))
			{
				if (col %5 == 0)
				{
					strcat(buf, get_color_string(ch, COLOR_TEXT, VT102_DIM));
				}
				cat_sprintf(buf, "%d-%-13s", cmd_table[cmd].level, cmd_table[cmd].name);
				if (++col % 5 == 0)
				{
					strcat(buf, "\n\r");
				}
			}
		}
	}
	if (col % 5 != 0)
	{
		strcat(buf, "\n\r");
	}
	send_to_char(buf, ch);
	pop_call();
	return;
}


void do_hlist( CHAR_DATA *ch, char *argument )
{
	HELP_DATA *help;
	AREA_DATA *pArea;

	push_call("do_hlist(%p,%p)",ch,argument);

	if (argument[0] == '\0')
	{
		pArea = ch->in_room->area;
	}
	else
	{
		pArea = lookup_area(argument);
	}

	if (pArea && !can_olc_modify(ch, pArea->low_r_vnum))
	{
		ch_printf(ch, "That area is not in your allocated range.\n\r");
		pop_call();
		return;
	}
	else if (pArea == NULL)
	{
		ch_printf(ch, "Syntax: show hlist [areaname]\n\r");
		pop_call();
		return;
	}

	for (help = pArea->first_help ; help ; help = help->next)
	{
		ch_printf_color(ch, "{178}[{078}%2d{178}] {168}%s\n\r",
			help->level,
			help->keyword);
	}
	pop_call();
	return;
}


void do_rlist( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	ROOM_INDEX_DATA *pRoomIndex;
	AREA_DATA *pArea;
	int vnum;

	push_call("do_rlist(%p,%p)",ch,argument);

	if (argument[0] == '\0')
	{
		pArea = ch->in_room->area;
	}
	else
	{
		pArea = lookup_area(argument);
	}

	if (pArea && !can_olc_modify(ch, pArea->low_r_vnum))
	{
		ch_printf(ch, "That area is not in your allocated range.\n\r");
		pop_call();
		return;
	}
	else if (pArea == NULL)
	{
		ch_printf(ch, "Syntax: show rlist [areaname]\n\r");
		pop_call();
		return;
	}

	memset(buf, 0, MAX_STRING_LENGTH);

	for (vnum = pArea->low_r_vnum ; vnum <= pArea->hi_r_vnum ; vnum++)
	{
		if ((pRoomIndex = get_room_index(vnum)) != NULL)
		{
			if (pRoomIndex->area == pArea)
			{
				cat_sprintf(buf, "{178}[{078}%5u{178}] %s\n\r",
					pRoomIndex->vnum,
					pRoomIndex->name);
			}
			if (buf[MAX_STRING_LENGTH * 3 / 4] != '\0')
			{
				break;
			}
		}
	}
	send_to_char_color(buf, ch);
	pop_call();
	return;
}


void do_mlist( CHAR_DATA *ch, char *argument )
{
	char t1[81];
	MOB_INDEX_DATA *pMobIndex;
	AREA_DATA *pArea;
	int vnum;
	bool fAll;
	FILE *mobFile;

	push_call("do_mlist(%p,%p)",ch,argument);

	mobFile = NULL;

	if (argument[0] == '\0')
	{
		pArea = ch->in_room->area;
	}
	else
	{
		pArea = lookup_area(argument);
	}

	if (pArea && !can_olc_modify(ch, pArea->low_r_vnum))
	{
		ch_printf(ch, "That area is not in your allocated range.\n\r");
		pop_call();
		return;
	}
	else if (pArea == NULL)
	{
		if (!IS_GOD(ch))
		{
			ch_printf(ch, "Syntax: show mlist [areaname]\n\r");
			pop_call();
			return;
		}
		else if (strcasecmp(argument, "all"))
		{
			ch_printf(ch, "Syntax: show mlist [areaname|all]\n\r");
			pop_call();
			return;
		}
	}

	fAll = (!pArea && !strcasecmp(argument, "all"));

	if (fAll)
	{
		close_reserve();
		mobFile = my_fopen("mlist.all", "wt", FALSE);
	}

	for (vnum = 0 ; vnum < MAX_VNUM ; vnum++)
	{
		if ((pMobIndex = get_mob_index(vnum)) != NULL)
		{
			if (pMobIndex->area == pArea)
			{
				ch_printf_color(ch, "{178}[{078}%5u{178}] ({078}%3d{178}) {058}%-20s Hp {078}%5d Dam %3d Gold %8d {178}lvl {078}%3d\n\r",
					pMobIndex->vnum,
					pMobIndex->total_mobiles,
					str_resize(pMobIndex->short_descr, t1, -20),
					pMobIndex->hitplus + pMobIndex->hitnodice + (pMobIndex->hitnodice*pMobIndex->hitsizedice - pMobIndex->hitnodice) / 2,
					pMobIndex->damplus + pMobIndex->damnodice + (pMobIndex->damnodice*pMobIndex->damsizedice - pMobIndex->damnodice) / 2,
					pMobIndex->gold,
					pMobIndex->level);
			}
			else if (fAll)
			{
				fprintf(mobFile, "[%5u] %-20s Hp %5d Dam %3d Gold %8d lvl %2d\n",
					pMobIndex->vnum,
					str_resize(capitalize(pMobIndex->player_name), t1, -20),
					pMobIndex->hitplus + pMobIndex->hitnodice + (pMobIndex->hitnodice*pMobIndex->hitsizedice - pMobIndex->hitnodice) / 2,
					pMobIndex->damplus + pMobIndex->damnodice + (pMobIndex->damnodice*pMobIndex->damsizedice - pMobIndex->damnodice) / 2,
					pMobIndex->gold,
					pMobIndex->level);
			}
		}
	}
	if (fAll)
	{
		my_fclose(mobFile);
		open_reserve();
		send_to_char("Written to file: mlist.all\n\r",ch);
	}
	pop_call();
	return;
}


void do_olist( CHAR_DATA *ch, char *argument )
{
	char buf        [MAX_STRING_LENGTH];
	char affectBuf  [MAX_STRING_LENGTH];
	char wearlocBuf [MAX_STRING_LENGTH];
	char itemtypeBuf[MAX_STRING_LENGTH];
	char t1[81], t2[81];

	AFFECT_DATA *paf;
	FILE *objFile;
	OBJ_INDEX_DATA *pObjIndex;
	AREA_DATA *pArea, *lArea;

	int vnum, nMatch, lRange, hRange, range;
	bool fAll, fSort, fBrief;

	push_call("do_olist(%p,%p)",ch,argument);

	lArea = NULL;

	if (argument[0] == '\0')
	{
		pArea = ch->in_room->area;
	}
	else
	{
		pArea = lookup_area(argument);
	}

	if (pArea && !can_olc_modify(ch, pArea->low_r_vnum))
	{
		ch_printf(ch, "That area is not in your allocated range.\n\r");
		pop_call();
		return;
	}
	else if (pArea == NULL)
	{
		if (!IS_GOD(ch))
		{
			ch_printf(ch, "Syntax: show olist [areaname]\n\r");
			pop_call();
			return;
		}
		else
		{
			if (strcasecmp(argument, "all")
			&&  strcasecmp(argument, "sort")
			&&  strcasecmp(argument, "brief"))
			{
				ch_printf(ch, "Syntax: show olist [areaname|all|sort|brief]\n\r");
				pop_call();
				return;
			}
		}
	}

	fAll		= (!pArea && !strcasecmp(argument, "all"));
	fSort	= (!pArea && !strcasecmp(argument, "sort"));
	fBrief	= (!pArea && !strcasecmp(argument, "brief"));

	objFile = NULL;

	if (fAll || fSort || fBrief)
	{
		close_reserve();
		objFile = my_fopen("olist.all", "wt", FALSE );
	}

	nMatch = 0;

	if( !fSort )
	{
		lRange=0;
		hRange=1;
	}
	else
	{
		lRange = 0;
		hRange = 115;
	}

	for (range = lRange ; range < hRange ; range++)
	{
		for (vnum = 0, nMatch = 0 ; nMatch < mud->top_obj_index ; vnum++)
		{
			if ((pObjIndex = get_obj_index(vnum)) != NULL)
			{
				nMatch++;
				if ((fSort && range == pObjIndex->level) || fAll || fBrief || pObjIndex->area == pArea)
				{
					/* Add spot for area name	- Chaos 1/17/96	*/

					if(fAll || fBrief)
					{
						if (lArea != pObjIndex->area)
						{
							if (fBrief)
							{
								lArea = pObjIndex->area;
								fprintf(objFile,"\n%s\n\n", lArea->name);
							}
							else
							{
								lArea = pObjIndex->area;
								fprintf(objFile,"\n[%5d] %s\n\n", lArea->low_r_vnum, lArea->name);
							}
						}
					}

					strcpy(affectBuf,"");

					for (paf = pObjIndex->first_affect ; paf ; paf = paf->next)
					{
						sprintf(t2, "%4d %s", paf->modifier, str_resize(affect_loc_name(paf->location), t1, 3));
						strcat(affectBuf, t2);
						t1[0] = '\0';
					}

					strcpy(itemtypeBuf,"");
					switch (pObjIndex->item_type)
					{
						case ITEM_LIGHT:
							sprintf(t2, "%d hr", pObjIndex->value[2]);
							strcpy(itemtypeBuf,t2);
							break;

						case ITEM_CONTAINER:
							sprintf(t2, "%d lb", pObjIndex->value[0]);
							strcpy(itemtypeBuf,t2);
							break;

						case ITEM_DRINK_CON:
						case ITEM_FOUNTAIN:
							sprintf(t2, "%8s", str_resize(liq_table[pObjIndex->value[2]].liq_name,t1,8));
							strcpy(itemtypeBuf,t2);
							break;

						case ITEM_FOOD:
							sprintf(t2, "%2d hr",pObjIndex->value[0]);
							strcpy(itemtypeBuf,t2);
							break;

						case ITEM_MONEY:
							sprintf(t2, "%d $$",pObjIndex->cost);
							strcpy(itemtypeBuf,t2);
							break;

						case ITEM_WEAPON:
							sprintf(t2,"%2d av", (int)(pObjIndex->value[1]*(1+(pObjIndex->value[2]-1)/2.0)));
							strcpy(itemtypeBuf,t2);
							break;

						case ITEM_ARMOR:
							sprintf(t2, "%2d ac", pObjIndex->value[0] );
							strcpy(itemtypeBuf,t2);
							break;

						case ITEM_AMMO:
							sprintf(t2,"%d av",
							(pObjIndex->value[1]*pObjIndex->value[3])/2);
							strcpy(itemtypeBuf,t2);
							break;
						case ITEM_SCROLL:
						case ITEM_POTION:
						case ITEM_PILL:
						case ITEM_WAND:
						case ITEM_STAFF:
						case ITEM_TREASURE:
						case ITEM_FURNITURE:
						case ITEM_TRASH:
						case ITEM_KEY:
						case ITEM_BOAT:
						case ITEM_CORPSE_NPC:
						case ITEM_CORPSE_PC:
						case ITEM_PORTAL:
						case ITEM_ARTIFACT:
						case ITEM_LOCKPICK:
						case ITEM_TOTEM:
						if (fBrief)
						{
							continue;
						}
						else
						{
							break;
						}
					}

					if (fBrief || fSort)
					{
						strcpy(wearlocBuf,"");

						if (!HAS_BIT(pObjIndex->wear_flags, ITEM_TAKE) && pObjIndex->item_type != ITEM_LIGHT)
						{
							continue;
						}
						else if ( HAS_BIT( pObjIndex->wear_flags, ITEM_WEAR_FINGER ) )
						{
							strcpy( wearlocBuf, "Finger->");
						}
						else if ( HAS_BIT( pObjIndex->wear_flags, ITEM_WEAR_NECK ) )
						{
							strcpy( wearlocBuf, "Neck--->");
						}
						else if ( HAS_BIT( pObjIndex->wear_flags, ITEM_WEAR_BODY ) )
						{
							strcpy( wearlocBuf, "Body--->");
						}
						else if ( HAS_BIT( pObjIndex->wear_flags, ITEM_WEAR_HEAD ) )
						{
							strcpy( wearlocBuf, "Head--->");
						}
						else if ( HAS_BIT( pObjIndex->wear_flags, ITEM_WEAR_LEGS ) )
						{
							strcpy( wearlocBuf, "Legs--->");
						}
						else if ( HAS_BIT( pObjIndex->wear_flags, ITEM_WEAR_FEET ) )
						{
							strcpy( wearlocBuf, "Feet--->");
						}
						else if ( HAS_BIT( pObjIndex->wear_flags, ITEM_WEAR_HANDS ) )
						{
							strcpy( wearlocBuf, "Hands-->");
						}
						else if ( HAS_BIT( pObjIndex->wear_flags, ITEM_WEAR_ARMS ) )
						{
							strcpy( wearlocBuf, "Arms--->");
						}
						else if ( HAS_BIT( pObjIndex->wear_flags, ITEM_WEAR_SHIELD ) )
						{
							strcpy( wearlocBuf, "Shield->");
						}
						else if ( HAS_BIT( pObjIndex->wear_flags, ITEM_WEAR_ABOUT ) )
						{
							strcpy( wearlocBuf, "About-->");
						}
						else if ( HAS_BIT( pObjIndex->wear_flags, ITEM_WEAR_WAIST ) )
						{
							strcpy( wearlocBuf, "Waist-->");
						}
						else if ( HAS_BIT( pObjIndex->wear_flags, ITEM_WEAR_WRIST ) )
						{
							strcpy( wearlocBuf, "Wrist-->");
						}
						else if ( HAS_BIT( pObjIndex->wear_flags, ITEM_WEAR_WIELD ) )
						{
							strcpy( wearlocBuf, "Wield-->");
						}
						else if ( HAS_BIT( pObjIndex->wear_flags, ITEM_WEAR_HOLD ) )
						{
							strcpy( wearlocBuf, "Hold--->");
						}
						else if ( HAS_BIT( pObjIndex->wear_flags, ITEM_TAKE ) )
						{
							continue;
						}
						else
						{
							strcpy( wearlocBuf, "Unknown>");
						}
					}
					if (fBrief || fSort)
					{
						sprintf(buf,"%s %s%25s %8s lvl %3d\n",
							wearlocBuf,
							capitalize( str_resize(pObjIndex->short_descr, t1, -35)),
							affectBuf,
							itemtypeBuf,
							pObjIndex->level);
					}
					else if (fAll)
					{
						sprintf(buf,"[%5u] %s%23s %8s lvl %3d/%3d\n",
							pObjIndex->vnum,
							capitalize(str_resize(pObjIndex->short_descr, t1, -26)),
							affectBuf,
							itemtypeBuf,
							pObjIndex->level,
							obj_level_estimate(pObjIndex));
					}
					else
					{
						sprintf(buf,"{178}[{078}%5u{178}] ({078}%3d{178}) {128}%s{078}%24s %8s {178}lvl {078}%3d{178}/{078}%3d\n",
							pObjIndex->vnum,
							pObjIndex->total_objects,
							str_resize(pObjIndex->short_descr, t1, -19),
							affectBuf,
							itemtypeBuf,
							pObjIndex->level,
							obj_level_estimate(pObjIndex));
					}

					if (!fAll && !fSort && !fBrief)
					{
						strcat( buf, "\r" );
						send_to_char( ansi_translate_text(ch, buf), ch );
					}
					else
					{
						char dummy[4];
						strcpy(dummy,"%s" ); /* to handle objects with %s in their names */
						fprintf(objFile,buf,dummy,dummy,dummy);
					}
				}
			}
		}
	}
	if (fAll || fSort || fBrief)
	{
		my_fclose(objFile);
		open_reserve();
		send_to_char("Written to file: olist.all\n\r",ch);
	}
	pop_call();
	return;
}


void do_owhere( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_INPUT_LENGTH];
	char buf1[MAX_INPUT_LENGTH];
	char buf2[MAX_INPUT_LENGTH];
	char buf3[MAX_INPUT_LENGTH];
	char bold[10], dim[10], objc[10], mobc[10];
	OBJ_DATA *obj;
	OBJ_DATA *in_obj;
	bool found, basic;
	int count,item_vnum;
	int total, total_room, total_npc, total_pc, total_cont, total_progs, total_non_basic;
	int total_pc_cont, total_npc_cont, total_null;

	push_call("do_owhere(%p,%p)",ch,argument);

	strcpy(dim,   get_color_string(ch, COLOR_PROMPT,  VT102_DIM ));
	strcpy(bold,  get_color_string(ch, COLOR_PROMPT,  VT102_BOLD));
	strcpy(objc,  get_color_string(ch, COLOR_OBJECTS, VT102_BOLD));
	strcpy(mobc,  get_color_string(ch, COLOR_MOBILES, VT102_DIM ));

	if (*argument == '\0')
	{
		if (IS_GOD(ch))
		{
			send_to_char("Syntax: show owhere <basic|total|objVnum|objName>\n\r", ch);
		}
		else
		{
			send_to_char("Syntax: show owhere <objVnum|objName>\n\r", ch);
		}
		pop_call();
		return;
	}

	basic = !strcasecmp(argument, "basic");

	if (!strcasecmp(argument, "total"))
	{
		total			= 0;
		total_room		= 0;
		total_npc			= 0;
		total_pc			= 0;
		total_cont		= 0;
		total_pc_cont		= 0;
		total_npc_cont		= 0;
		total_progs		= 0;
		total_null		= 0;
		total_non_basic	= 0;

		for (obj = mud->f_obj ; obj ; obj = obj->next)
		{
			total++;
			if (HAS_BIT(obj->extra_flags, ITEM_MODIFIED))
			{
				total_non_basic++;
			}
			if (obj->in_obj != NULL)
			{
				if (obj->in_obj->carried_by != NULL)
				{
					if (IS_NPC(obj->in_obj->carried_by))
					{
						total_npc_cont++;
					}
					else
					{
						total_pc_cont++;
					}
				}
				else
				{
					total_cont++;
				}
			}
			else if (obj->in_room != NULL)
			{
				total_room++;
			}
			else
			{
				if (obj->carried_by != NULL)
				{
					if (!IS_NPC(obj->carried_by))
					{
						total_pc++;
						if (obj->pIndexData->first_oprog != NULL)
						{
							total_progs++;
						}
					}
					else
					{
						total_npc++;
					}
				}
				else
				{
					total_null++;
				}
			}
		}
		ch_printf(ch, "\n\r");
		ch_printf(ch, "   %s  Totalized list of objects                  \n\r", dim );
		ch_printf(ch, "   %s+----------------------------------+--------+\n\r", bold);
		ch_printf(ch, "   %s| %sObjects carried by players       %s| %s%6d %s|\n\r", bold, dim, bold, dim, total_pc,       bold);
		ch_printf(ch, "   %s| %sObjects carried by mobs          %s| %s%6d %s|\n\r", bold, dim, bold, dim, total_npc,      bold);
		ch_printf(ch, "   %s| %sObjects in containers in room    %s| %s%6d %s|\n\r", bold, dim, bold, dim, total_cont,     bold);
		ch_printf(ch, "   %s| %sObjects in containers on players %s| %s%6d %s|\n\r", bold, dim, bold, dim, total_pc_cont,  bold);
		ch_printf(ch, "   %s| %sObjects in containers on mobs    %s| %s%6d %s|\n\r", bold, dim, bold, dim, total_npc_cont, bold);
		ch_printf(ch, "   %s| %sObjects in rooms                 %s| %s%6d %s|\n\r", bold, dim, bold, dim, total_room,     bold);
		ch_printf(ch, "   %s| %sObjects with active obj programs %s| %s%6d %s|\n\r", bold, dim, bold, dim, total_progs,    bold);
		ch_printf(ch, "   %s| %sObjects in NULL                  %s| %s%6d %s|\n\r", bold, dim, bold, dim, total_null,     bold);
		ch_printf(ch, "   %s| %sTotal Non Basic Objects          %s| %s%6d %s|\n\r", bold, dim, bold, dim, total_non_basic,bold);
		ch_printf(ch, "   %s| %sTotal Objects                    %s| %s%6d %s|\n\r", bold, dim, bold, dim, total,          bold);
		ch_printf(ch, "   %s+----------------------------------+--------+\n\r", bold);

		pop_call();
		return;
	}

	found = FALSE;
	count = 0;

	item_vnum = is_number(argument) ? atol(argument) : -1;

	for (obj = mud->f_obj ; obj && count < 100 ; obj = obj->next)
	{
		if (item_vnum != obj->pIndexData->vnum
		&& !is_name_short(argument, obj->name)
		&& !basic)
		{
			continue;
		}

		if (!can_olc_modify(ch, obj->pIndexData->vnum))
		{
			continue;
		}

		if (basic && !HAS_BIT(obj->extra_flags, ITEM_MODIFIED))
		{
			continue;
		}
		found = TRUE;
		count++;

		for (in_obj = obj ; in_obj->in_obj != NULL && in_obj->in_obj != in_obj; in_obj = in_obj->in_obj)
		{
			;
		}
		sprintf (buf1, "%s[%s%5u%s]%s %s",
			bold, dim,  obj->pIndexData->vnum,
			bold, objc, str_resize(obj->short_descr, buf, -21));

		if (in_obj->carried_by != NULL)
		{
			sprintf (buf2, "%s[%s%5u%s]%s %s",
				bold, dim, in_obj->carried_by->in_room->vnum,
				bold, dim, str_resize(in_obj->carried_by->in_room->name, buf, -16));

			sprintf (buf3, "%s[%s%5u%s]%s %s",
				bold, dim,  IS_NPC(in_obj->carried_by) ? in_obj->carried_by->pIndexData->vnum                  : in_obj->carried_by->pcdata->pvnum,
				bold, mobc, IS_NPC(in_obj->carried_by) ? str_resize(in_obj->carried_by->short_descr, buf, -16) : in_obj->carried_by->name);
		}
		else if (in_obj->in_room == NULL)
		{
			sprintf (buf2, "%s[%s%5u%s]%s %s",
				bold, dim, -1,
				bold, dim, "NULL");

			buf3[0] = '\0';
		}
		else
		{
			sprintf (buf2, "%s[%s%5u%s]%s %s",
				bold, dim, in_obj->in_room->vnum,
				bold, dim, str_resize(in_obj->in_room->name, buf, -16));

			if (in_obj == obj)
			{
				buf3[0] = '\0';
			}
			else if (in_obj == NULL)
			{
				sprintf (buf3, "%s[%s%5d%s]%s %s",
					bold, dim, -1,
					bold, objc, "NULL");
			}
			else
			{
				sprintf (buf3, "%s[%s%5u%s]%s %s",
					bold, dim, in_obj->pIndexData->vnum,
					bold, objc, str_resize(in_obj->short_descr, buf, -16));
			}
		}
		sprintf(buf, "%s %s %s\n\r", buf1, buf2, buf3);
		send_to_char( buf, ch );
	}

	if ( !found )
	{
		send_to_char( "Nothing like that in hell, heaven or earth.\n\r", ch );
	}
	pop_call();
	return;
}


void do_mwhere( CHAR_DATA *ch, char *argument )
{
	char buf1[MAX_INPUT_LENGTH];
	char buf2[MAX_INPUT_LENGTH];
	char bold[10], dim[10], mobc[10];
	CHAR_DATA *victim;
	int count, mob_vnum;

	push_call("do_mwhere(%p,%p)",ch,argument);

	strcpy(dim,   get_color_string(ch, COLOR_PROMPT,  VT102_DIM ));
	strcpy(bold,  get_color_string(ch, COLOR_PROMPT,  VT102_BOLD));
	strcpy(mobc,  get_color_string(ch, COLOR_MOBILES, VT102_DIM ));

	count = 0;

	if (argument[0] == '\0')
	{
		send_to_char( "Syntax: show mwhere <mobVnum|mobName>\n\r", ch );
		pop_call();
		return;
	}

	mob_vnum = atol(argument);

	for (victim = mud->f_char ; count < 100 && victim ; victim = victim->next)
	{
		if (IS_NPC(victim) && can_olc_modify(ch, victim->pIndexData->vnum))
		{
			if (mob_vnum)
			{
				if (victim->pIndexData->vnum != mob_vnum)
				{
					continue;
				}
			}
			else
			{
				if (!is_name_short(argument, victim->name))
				{
					continue;
				}
			}
			count++;
			ch_printf(ch, "%s[%s%5u%s] %s%s %s[%s%5u%s] %s%s\n\r",
				bold,  dim, victim->pIndexData->vnum,
				bold, mobc, str_resize(victim->short_descr, buf1, -31),
				bold,  dim, victim->in_room->vnum,
				bold,  dim, str_resize(victim->in_room->name, buf2, -31));
		}
	}

	if (count == 0)
	{
		if (mob_vnum == 0)
		{
			act("You didn't find any $T.", ch, NULL, argument, TO_CHAR);
		}
		else
		{
			act("No mob loaded with vnum $T.", ch, NULL, argument, TO_CHAR);
		}
	}
	pop_call();
	return;
}


void do_alist( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	AREA_DATA *pArea;
	int vnum;

	push_call("do_alist(%p,%p)",ch,argument);

	if (!IS_GOD(ch))
	{
		ch_printf(ch, "Sorry, Gods only.\n\r");
		pop_call();
		return;
	}

	strcpy(buf, "{088}");

	for (pArea = NULL, vnum = 1 ; vnum < MAX_VNUM ; vnum++)
	{
		if (room_index[vnum] && room_index[vnum]->area != pArea)
		{
			pArea = room_index[vnum]->area;
		}
		else if (mob_index[vnum] && mob_index[vnum]->area != pArea)
		{
			pArea = mob_index[vnum]->area;
		}
		else if (obj_index[vnum] && obj_index[vnum]->area != pArea)
		{
			pArea = obj_index[vnum]->area;
		}
		else
		{
			continue;
		}
		cat_sprintf(buf, "{828} [{878}%5u {828}- {878}%5u{828}] {878}%-30s %-25s\n\r",
			UMIN(pArea->low_r_vnum, UMIN(pArea->low_m_vnum, pArea->low_o_vnum)),
			UMAX(pArea->hi_r_vnum,  UMAX(pArea->hi_m_vnum,  pArea->hi_o_vnum)),
			pArea->filename,
			pArea->name);
	}
	send_to_char_color(buf, ch);
	pop_call();
	return;
}

int unique_players(void)
{
	DESCRIPTOR_DATA *desc;
	int cnt;

	cnt = 0;

	for (desc = mud->f_desc ; desc ; desc = desc->next)
	{
		if (desc->connected <= CON_PLAYING)
		{
			continue;
		}
		cnt++;

		while (desc->next && !strcmp(desc->host, desc->next->host))
		{
			desc = desc->next;
		}
	}
	return cnt;
}

typedef struct	user_list_data		USER_LIST_DATA;

struct user_list_data
{
	USER_LIST_DATA  * next;
	USER_LIST_DATA  * prev;
	DESCRIPTOR_DATA * desc;
};

/*
	This routine Returns which host is greatest by IP numbers for sorting
	Chaos - 3/19/96
*/

int hostcmp( DESCRIPTOR_DATA *d1, DESCRIPTOR_DATA *d2 )
{
	int cnt;

	push_call("hostcmp(%p,%p)",d1,d2);

	if (d1->host == NULL || d2->host == NULL)
	{
		pop_call();
		return 0;
	}

	for (cnt = 0 ; d1->host[cnt] != '\0' ; cnt++)
	{
		if (d1->host[cnt] < d2->host[cnt])
		{
			pop_call();
			return -1;
		}
		if (d1->host[cnt] > d2->host[cnt])
		{
			pop_call();
			return 1;
		}
	}
	pop_call();
	return 0;
}


int count_connects( DESCRIPTOR_DATA *d)
{
	DESCRIPTOR_DATA *dtemp;
	int cnt = 0;

	push_call("count_connects(%p)",d);

	for (dtemp = mud->f_desc ; dtemp ; dtemp = dtemp->next)
	{
		if (d->host && dtemp->host && !strcmp(d->host, dtemp->host))
		{
			cnt++;
		}
	}
	pop_call();
	return cnt;
}


void do_users( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;
	int count, val;
	USER_LIST_DATA *uld, *user_list, *first_uld, *last_uld;

	push_call("do_users(%p,%p)",ch,argument);

	/*
		The complete descriptor list
	*/

	if (*argument != '\0')
	{
		for (count = 0 ; count < MAX_LINKPERPORT + 20 ; count++)
		{
			sprintf(buf, "%3d ", count);

			if (getsockopt(count, SOL_SOCKET, SO_SNDBUF, (char *) &val, (int *) &val) < 0)
			{
				cat_sprintf(buf, "SN NA    ");
			}
			else
			{
				cat_sprintf(buf, "SN %-5d ", val);
			}

			if (getsockopt(count, SOL_SOCKET, SO_RCVBUF, (char *) &val, (int *) &val) < 0)
			{
				cat_sprintf(buf, "RC NA   ");
			}
			else
			{
				cat_sprintf(buf, "RC %-4d ", val);
			}

			if (getsockopt(count, SOL_SOCKET, SO_REUSEADDR, (char *) &val, (int *) &val) < 0)
			{
				cat_sprintf(buf, "RU N ");
			}
			else
			{
				cat_sprintf(buf, "RU %-1d ", val);
			}

		/*
			if (getsockopt(count, SOL_SOCKET, SO_DEBUG, (char *) &val, (unsigned int *) &val) < 0)
			{
				cat_sprintf(buf, "DE N ");
			}
			else
			{
				cat_sprintf(buf, "DE %-1d ", val);
			}
		*/

			if (getsockopt(count, SOL_SOCKET, SO_KEEPALIVE, (char *) &val, (int *) &val) < 0)
			{
				cat_sprintf(buf, "KE N ");
			}
			else
			{
				cat_sprintf(buf, "KE %-1d ", val);
			}

			if (getsockopt(count, SOL_SOCKET, SO_LINGER, (char *) &val, (int *) &val) < 0)
			{
				cat_sprintf(buf, "LG N ");
			}
			else
			{
				cat_sprintf(buf, "LG %-1d ", val);
			}

			if (getsockopt(count, SOL_SOCKET, SO_TYPE, (char *) &val, (int *) &val) < 0)
			{
				cat_sprintf(buf, "TY N ");
			}
			else
			{
				cat_sprintf(buf, "TY %-1d ", val);
			}

			cat_sprintf(buf, "OWN %-2d FLG %-5x ", fcntl(count, F_GETOWN), fcntl(count, F_GETFL) >= 0 ? fcntl(count, F_GETFL) : 0);

			for (d = mud->f_desc ; d ; d = d->next)
			{
				if (d->character && is_desc_valid(d->character))
				{
					if (d->descriptor == count)
					{
						cat_sprintf(buf, "%-3d %s", d->connected, d->character->name);
						break;
					}
				}
			}
			ch_printf(ch, "%s\n\r", buf);
		}
		pop_call();
		return;
	}

	/*
		Sorted by ip with test for multiplaying - Scandum 18-07-2002
	*/

	first_uld = last_uld = NULL;

	for (d = mud->f_desc ; d ; d = d->next)
	{
		for (uld = first_uld ; uld ; uld = uld->next)
		{
			if (hostcmp(d, uld->desc) == -1)
			{
				ALLOCMEM(user_list, USER_LIST_DATA, 1);
				user_list->desc = d;
				INSERT_LEFT(user_list, uld, first_uld, next, prev);
				break;
			}
		}
		if (uld == NULL)
		{
			ALLOCMEM(user_list, USER_LIST_DATA, 1);
			user_list->desc = d;
			LINK(user_list, first_uld, last_uld, next, prev);
		}
	}

	buf[0] = '\0';

	for (uld = first_uld ; uld ; uld = uld->next)
	{
		sprintf(buf, "{178}[{078}%3d %4d{178}] {178}%14s{078}@{178}%-17s {118}%3s {078}%-14s\n\r",
			uld->desc->descriptor,
			uld->desc->connected,
			CH(uld->desc) ? CH(uld->desc)->name : "(none)",
			uld->desc->host,
			count_connects(uld->desc) > 5 ? "MP" : "",
			uld->desc->mth->terminal_type);

			send_to_char_color(buf, ch);
	}

	while ((uld = first_uld) != NULL)
	{
		UNLINK(uld, first_uld, last_uld, next, prev);
		FREEMEM(uld);
	}

	pop_call();
	return;
}


void do_roomfragment( CHAR_DATA *ch, char *argument )
{
	int cnt;
	char buf[MAX_STRING_LENGTH];

	push_call("do_roomfragment(%p,%p)",ch,argument);

	strcpy( buf, "Room Fragmentation chart:\n\r");
	strcat( buf, "    0 1 2 3 4 5 6 7 8 9\n\r" );
	for (cnt = 0 ; cnt < MAX_VNUM/100 ; cnt++)
	{
		if (cnt % 10 == 0)
		{
			cat_sprintf(buf, "%2d  ",cnt/10);
		}
		if (room_index[cnt*100+1] != NULL)
		{
			strcat(buf, "X ");
		}
		else
		{
			strcat(buf, "  ");
		}
		if (cnt % 10 == 9)
		{
			strcat(buf, "\n\r");
		}
	}
	send_to_char(buf, ch);
	pop_call();
	return;
}


void do_termlist (CHAR_DATA * ch, char *arg)
{
	PLAYER_GAME *gpl;
	char buf[MAX_STRING_LENGTH];

	push_call("do_termlist(%p,%p)",ch,arg);

	for (buf[0] = '\0', gpl = mud->f_player ; gpl ; gpl = gpl->next)
	{
		cat_sprintf(buf, "{078}%15s {188}[{078}%2d{188}] [{078}%3d{188}] ", gpl->ch->name, gpl->ch->pcdata->screensize_v, gpl->ch->pcdata->screensize_h);

		cat_sprintf(buf, "{078}%s  %s  ", HAS_BIT(gpl->ch->pcdata->vt100_flags, VT100_INTERFACE) ? "VT100" : "VTOFF", HAS_BIT(gpl->ch->pcdata->vt100_flags, VT100_ANSI) ? "ANSI" : "MONO");

		if (gpl->ch->desc && gpl->ch->desc->mth->mccp2)
		{
			cat_sprintf(buf, "MCCP2 %4.1f%%", 100 - 100.0 * gpl->ch->desc->mth->mccp2->total_out / UMAX(1, gpl->ch->desc->mth->mccp2->total_in));
		}
		else
		{
			cat_sprintf(buf, "           ");
		}
		cat_sprintf(buf, "%4d  %s\n\r", gpl->ch->pcdata->idle, gpl->ch->desc ? gpl->ch->desc->mth->terminal_type : "");
	}
	send_to_char_color(buf, ch);
	pop_call();
	return;
}


void do_file( CHAR_DATA *ch, char *argument)
{
	CHAR_DATA  *victim;
	CRIME_DATA *record;
	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	char colG[10], colY[10], colC[10], colW[10];
	int number, cnt;
	bool loaded;

	push_call("do_file(%p,%p)",ch,argument);

	strcpy(colG, ansi_translate_text(ch, "{128}"));
	strcpy(colY, ansi_translate_text(ch, "{138}"));
	strcpy(colC, ansi_translate_text(ch, "{168}"));
	strcpy(colW, ansi_translate_text(ch, "{178}"));

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0')
	{
		send_to_char("Usage : file <victim> [<record #>]\n\r", ch);
		pop_call();
		return;
	}

	if ((victim = lookup_char(arg1)) == NULL)
	{
		if ((victim = start_partial_load(ch, arg1)) == NULL)
		{
			pop_call();
			return;
		}
		loaded = TRUE;
	}
	else
	{
		loaded = FALSE;
	}

	if (arg2[0] != '\0')
	{
		number = atoi(arg2);
	}
	else
	{
		number = -1;
	}

	if (victim->pcdata->first_record == NULL)
	{
		ch_printf(ch, "%s has no criminal record.\n\r", victim->name);
		if (loaded)
		{
			clear_partial_load(victim);
		}
		pop_call();
		return;
	}

	for (cnt = 0, buf[0] = '\0', record = victim->pcdata->first_record ; record ; record = record->next)
	{
		if (number == -1 || ++cnt == number)
		{
			cat_sprintf(buf, "%s%s %shas been arrested on %s%s %sby %s%s%s.\n\r", colY, victim->name, colW, colG, get_time_string(record->date), colW, colY, record->arrester, colW);

			cat_sprintf(buf, "%s%s %shas been imprisoned for %s%d %sdays, %s%d %shours and %s%d %sminutes.\n\r",
				colY, victim->name, colW,
				colG, record->jailtime / 86400, colW,
				colG, record->jailtime % 86400 / 3600, colW,
				colG, record->jailtime % 86400 % 3600 / 60, colW);

			if (record->released == TRUE)
			{
				cat_sprintf(buf, "%s%s %shas been released from this punishment by %s%s%s.\n\r",
					colY, victim->name, colW,
					colY, record->releaser, colW);
			}
			else
			{
				int time_left = victim->pcdata->jailtime - (mud->current_time - victim->pcdata->jaildate);
				if (time_left > 0)
				{
					cat_sprintf(buf, "%s%s %swill be autoreleased in %s%d %sdays, %s%d %shours and %s%d %sminutes.\n\r",
						colY, victim->name, colW,
						colG, time_left / 86400, colW,
						colG, time_left % 86400 / 3600, colW,
						colG, time_left % 86400 % 3600 / 60, colW);
				}
				else
				{
					cat_sprintf(buf, "%s%s %sis waiting to be auto released by the system.\n\r",
						colY, victim->name, colW);
				}
			}
			cat_sprintf(buf, "%sFile information:%s\n\r%s\n\r",
				colC, colW, record->crime_record);
		}
	}

	if (number != -1 && number > cnt)
	{
		sprintf(buf, "%s hasn't committed that many crimes.\n\r", victim->name);
	}
	send_to_char(buf, ch);

	if (loaded)
	{
		clear_partial_load(victim);
	}
	pop_call();
	return;
}


void do_showcurve(CHAR_DATA *ch, char *argument)
{
	int cnt, i, req_exp, dif_exp, prev_exp;
	char buf[MAX_STRING_LENGTH];

	push_call("do_showcurve(%p,%p)",ch,argument);

	cnt = i = req_exp = dif_exp = prev_exp = 0;

	if (argument[0] == '\0')
	{
		ch_printf(ch, "Show the learning curve of which class?\n\r");
		pop_call();
		return;
	}

	buf[0] = 0;

	if (!strcasecmp(argument, "npc"))
	{
		for (i = 1 ; i <= 300 ; i++)
		{
			req_exp  = exp_level(CLASS_MONSTER, i);
			dif_exp  = req_exp - prev_exp;
			prev_exp = req_exp;
			cat_sprintf(buf, "{078}Level {138}%3d{178}: {078}%12d {138}- {078}%12d exp\n\r", i, req_exp, dif_exp);
		}
		send_to_char_color(buf, ch);

		pop_call();
		return;
	}

	for (cnt = 0 ; cnt < MAX_CLASS ; cnt++)
	{
		if (is_name_short(argument, (char *) class_table[cnt].who_name))
		{
			for (i = 1 ; i <= 95 ; i++)
			{
				req_exp  = exp_level(cnt, i);
				dif_exp  = req_exp - prev_exp;
				prev_exp = req_exp;
				cat_sprintf(buf, "{078}Level {138}%2d{178}: {078}%12d {138}- {078}%12d exp\n\r", i, req_exp, dif_exp);
			}
			break;
		}
	}

	if (cnt == MAX_CLASS)
	{
		send_to_char("Syntax: show expcurve <ran|gla|mar|nin|ele|ill|mon|nec|npc>\n\r", ch);
	}
	else
	{
		send_to_char_color(buf, ch);
	}
	pop_call();
	return;
}


void do_show( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];

	push_call("do_show(%p,%p)",ch,argument);

	argument = one_argument(argument, arg);

	if (IS_NPC(ch))
	{
		log_printf("do_show: used by mob");
		dump_stack();
		pop_call();
		return;
	}

	if (!strcasecmp(arg, "mpcommands"))
	{
		do_mpcommands(ch, argument);
	}
	else if (!strcasecmp(arg, "rlist"))
	{
		do_rlist(ch, argument);
	}
	else if (!strcasecmp(arg, "mlist"))
	{
		do_mlist(ch, argument);
	}
	else if (!strcasecmp(arg, "olist"))
	{
		do_olist(ch, argument);
	}
	else if (!strcasecmp(arg, "hlist"))
	{
		do_hlist(ch, argument);
	}
	else if (!strcasecmp(arg, "mwhere"))
	{
		do_mwhere(ch, argument);
	}
	else if (!strcasecmp(arg, "owhere"))
	{
		do_owhere(ch, argument);
	}
	else if (IS_GOD(ch) && !strcasecmp(arg, "alist"))
	{
		do_alist(ch, argument);
	}
	else if (IS_GOD(ch) && !strcasecmp(arg, "openfiles"))
	{
		do_openfiles(ch, argument);
	}
	else if (IS_GOD(ch) && !strcasecmp(arg, "cpu"))
	{
		do_cpu(ch, argument);
	}
	else if (IS_GOD(ch) && !strcasecmp(arg,"memory"))
	{
		do_memory(ch, argument);
	}
	else if (IS_GOD(ch) && !strcasecmp(arg, "terms"))
	{
		do_termlist(ch, argument);
	}
	else if (IS_GOD(ch) && !strcasecmp(arg, "users"))
	{
		do_users(ch, argument);
	}
	else if (IS_GOD(ch) && !strcasecmp(arg, "log"))
	{
		do_llog(ch, argument);
	}
	else if (IS_GOD(ch) && !strcasecmp(arg, "world"))
	{
		do_roomfragment(ch, argument);
	}
	else if (IS_GOD(ch) && !strcasecmp(arg, "file"))
	{
		do_file(ch, argument);
	}
	else if (IS_GOD(ch) && !strcasecmp(arg, "plist"))
	{
		do_lookup(ch, argument);
	}
	else if (IS_GOD(ch) && !strcasecmp(arg, "attack"))
	{
		do_attack(ch, argument);
	}
	else if (IS_GOD(ch) && !strcasecmp(arg, "exp"))
	{
		do_showcurve(ch, argument);
	}
	else if (IS_GOD(ch) && !strcasecmp(arg, "stack"))
	{
		do_stack(ch, argument);
	}
	else
	{
		if (IS_GOD(ch))
		{
			send_to_char("Syntax: show <field> <argument>\n\r\n\r", ch);
			send_to_char("Field being one of:\n\r", ch);
			send_to_char("  world log users terms memory cpu hash openfiles file exp attack\n\r", ch);
			send_to_char("  plist rlist alist mlist olist hlist mwhere owhere mpcommands stack\n\r", ch);
		}
		else
		{
			send_to_char("Syntax: show <rlist|mlist|olist|hlist|mwhere|owhere|mpcommands> <argument>\n\r", ch);
		}
	}
	pop_call();
	return;
}


void do_bamfin( CHAR_DATA *ch, char *argument )
{
	push_call("do_bamfin(%p,%p)",ch,argument);

	if (*argument == '\0')
	{
		send_to_char("Set poof in to what?\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_NPC(ch))
	{
		smash_tilde(argument);
		STRFREE(ch->pcdata->bamfin);
		ch->pcdata->bamfin = STRALLOC(argument);
		ch_printf_color(ch, "Poof in set to: %s\n\r", argument);
	}
	pop_call();
	return;
}


void do_bamfout( CHAR_DATA *ch, char *argument )
{
	push_call("do_bamfout(%p,%p)",ch,argument);

	if (*argument == '\0')
	{
		send_to_char("Set poof out to what?\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_NPC(ch))
	{
		smash_tilde( argument );
		STRFREE(ch->pcdata->bamfout );
		ch->pcdata->bamfout = STRALLOC( argument );
		ch_printf_color(ch, "Poof out set to: %s\n\r", argument);
	}
	pop_call();
	return;
}


void do_poof( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];

	push_call("do_poof(%p,%p)",ch,argument);

	argument = one_argument(argument, arg);

	if (!strcasecmp(arg, "in"))
	{
		do_bamfin(ch, argument);
	}
	else if (!strcasecmp(arg, "out"))
	{
		do_bamfout(ch, argument);
	}
	else
	{
		ch_printf(ch, "Syntax: poof <in|out> <string>\n\r");
	}
	pop_call();
	return;
}


void do_arrest( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int jailtime, multiplier, pvnum;

	push_call("do_arrest(%p,%p)",ch,argument);

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	pvnum = 0;

	if (ch->pcdata->editmode == MODE_NONE)
	{
		if (*arg1 == '\0')
		{

			sprintf(buf, "List of arrested players:\n\r\n\r");

			for (pvnum = 0 ; pvnum < MAX_PVNUM ; pvnum++)
			{
				if (pvnum_index[pvnum] == NULL)
				{
					continue;
				}
				if (HAS_BIT(pvnum_index[pvnum]->flags, PVNUM_ARRESTED))
				{
					cat_sprintf(buf, "%-16s", pvnum_index[pvnum]->name);
				}
			}
			ch_printf(ch, "%s\n\r", justify(buf, 80));

			pop_call();
			return;
		}

		if (*arg2 == '\0')
		{
			send_to_char("Usage: arrest <name> <number> <hours|days|months|years>\n\r\n\r", ch);		
	
			pop_call();
			return;
		}
	
		if ((pvnum = get_pvnum_name(arg1)) == -1)
		{
			send_to_char("That player does not exists.\n\r", ch);
			pop_call();
			return;
		}

		if (pvnum < 100)
		{
			send_to_char("You cannot arrest that person.\n\r", ch);
			pop_call();
			return;
		}

		if (HAS_BIT(pvnum_index[pvnum]->flags, PVNUM_ARRESTED))
		{
			send_to_char("They've already been arrested.\n\r", ch);
			pop_call();
			return;
		}

		if ((jailtime = atoi(arg2)) <= 0)
		{
			send_to_char("You must enter a correct duration.\n\r", ch);
			pop_call();
			return;
		}

		switch (*argument)
		{
			case 'h':	case 'H': multiplier = 3600;		break;
			case 'd':	case 'D': multiplier = 86400;		break;
			case 'w': case 'W': multiplier = 604800;	break;
			case 'm':	case 'M': multiplier = 2635200;	break;
			case 'y': case 'Y': multiplier = 31536000;	break;

			default:
				send_to_char("You must specify 'years', 'months', 'weeks', 'hours' or 'days'.\n\r", ch);
				pop_call();
				return;
				break;
		}

		if (multiplier * jailtime > 31536000)
		{
			send_to_char("The maximum sentence is 1 year.\n\r", ch);
			pop_call();
			return;
		}

		if (pvnum_index[pvnum]->ch == NULL)
		{
			do_pload(ch, pvnum_index[pvnum]->name);
		}

		if (pvnum_index[pvnum] == NULL || pvnum_index[pvnum]->ch == NULL)
		{
			send_to_char("Failed to find pvnum data.\n\r", ch);
			pop_call();
			return;
		}

		victim = pvnum_index[pvnum]->ch;

		log_printf("%s has arrested %s", ch->name, victim->name);

		if (victim->fighting != NULL)
		{
			stop_fighting( victim, TRUE );
		}
		char_to_room(victim, ROOM_VNUM_JAIL);

		SET_BIT(pvnum_index[pvnum]->flags, PVNUM_ARRESTED);
		victim->pcdata->jailtime = jailtime * multiplier;
		victim->pcdata->jaildate = mud->current_time;

		ch_printf(victim, "%s has arrested you.\n\r", get_name(ch));
		ch_printf(ch, "%s has been arrested.\n\r", get_name(victim));

		save_players();
	}

	switch (ch->pcdata->editmode)
	{
		default :
			bug("do_arrest() : illegal editmode");
			pop_call();
			return;

		case MODE_RESTRICTED :
			send_to_char("You cannot arrest someone while editing something.\n\r",ch);
			pop_call();
			return;

		case MODE_NONE :
			ch->pcdata->edit_ptr = pvnum_index[pvnum];
			ch->pcdata->editmode = MODE_CRIME_RECORD;
			ch->pcdata->tempmode = MODE_NONE;
			start_editing(ch, ch->pcdata->edit_buf);
			pop_call();
			return;

		case MODE_CRIME_RECORD :
			add_crime(ch, ch->pcdata->edit_ptr);
			stop_editing(ch);
			pop_call();
			return;
	}
	pop_call();
	return;
}

void add_crime (CHAR_DATA *ch, PVNUM_DATA *pvnum)
{
	CHAR_DATA *victim;
	CRIME_DATA *new_record;

	push_call("add_crime(%p,%p)",ch,pvnum);

	if (pvnum->ch == NULL)
	{
		do_pload(ch, pvnum->name);
	}

	victim = pvnum->ch;

	ALLOCMEM(new_record, CRIME_DATA, 1);

	new_record->crime_record = copy_buffer(ch);
	new_record->arrester     = STRALLOC(ch->name);
	new_record->date         = mud->current_time;
	new_record->level        = UMIN(ch->level, MAX_LEVEL);
	new_record->jailtime     = victim->pcdata->jailtime;

	LINK(new_record, victim->pcdata->first_record, victim->pcdata->last_record, next, prev);

	pop_call();
	return;
}



void do_release( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA  *victim;
	CRIME_DATA *record;

	push_call("do_release(%p,%p)",ch,argument);

	if ((victim = get_player_world(ch, argument)) == NULL)
	{
		send_to_char("That character could not be found.\n\r", ch);
		pop_call();
		return;
	}

	if (!HAS_BIT(pvnum_index[victim->pcdata->pvnum]->flags, PVNUM_ARRESTED))
	{
		send_to_char("That player is not imprisoned.\n\r", ch);
		pop_call();
		return;
	}

	record = victim->pcdata->last_record;

	if (record && record->level >= ch->level)
	{
		if (record->level == MAX_LEVEL && !strcasecmp(ch->name, record->arrester))
		{
			send_to_char("Overriding your own sentence!\n\r", ch);
		}
		else
		{
			ch_printf(ch, "You may not release %s", capitalize(victim->name));
			pop_call();
			return;
		}
	}

	if (record)
	{
		record->released = TRUE;
		STRFREE(record->releaser);
		record->releaser = STRALLOC(ch->name);
	}

	act( "$n has been released.", victim, NULL, NULL, TO_ROOM );
	char_from_room( victim );
	char_to_room( victim, ROOM_VNUM_TEMPLE);
	act( "$n has been released.", victim, NULL, NULL, TO_ROOM );
	do_look( victim, "auto" );
	act( "$n has released you.", ch, NULL, victim, TO_VICT );

	DEL_BIT(pvnum_index[victim->pcdata->pvnum]->flags, PVNUM_ARRESTED);
	victim->pcdata->jailtime = 0;
	victim->pcdata->jaildate = 0;

	ch_printf(ch, "%s has been released.\n\r", get_name(victim));

	save_players();

	pop_call();
	return;
}


void auto_release( CHAR_DATA *ch )
{
	CRIME_DATA *record;

	push_call("auto_release(%p)",ch);

	record = ch->pcdata->last_record;

	if (record)
	{
		STRFREE(record->releaser);
		record->released = TRUE;
		record->releaser = STRALLOC("The System");
	}

	act( "$n has been released.", ch, NULL, NULL, TO_ROOM );
	char_from_room( ch );
	char_to_room( ch, ROOM_VNUM_TEMPLE);
	act( "$n has been released.", ch, NULL, NULL, TO_ROOM );
	do_look( ch, "auto" );

	DEL_BIT(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_ARRESTED);
	ch->pcdata->jailtime = 0;
	ch->pcdata->jaildate = 0;

	send_to_char("You have been released from the dungeons.\n\r", ch);

	save_players();
	pop_call();
	return;
}


void do_deny( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	char buf[MAX_STRING_LENGTH];
	int pvnum;

	push_call("do_deny(%p,%p)",ch,argument);

	if (argument[0] == '\0')
	{
		sprintf(buf, "List of denied players:\n\r\n\r");

		for (pvnum = 0 ; pvnum < MAX_PVNUM ; pvnum++)
		{
			if (pvnum_index[pvnum] == NULL)
			{
				continue;
			}
			if (HAS_BIT(pvnum_index[pvnum]->flags, PVNUM_DENIED))
			{
				cat_sprintf(buf, "%-16s", pvnum_index[pvnum]->name);
			}
		}
		ch_printf(ch, "%s\n\r", justify(buf, 80));
		pop_call();
		return;
	}

	if ((pvnum = get_pvnum_name(argument)) == -1)
	{
		send_to_char("That player does not exist.\n\r", ch);
		pop_call();
		return;
	}

	if (pvnum < 100)
	{
		send_to_char( "You failed.\n\r", ch );
		pop_call();
		return;
	}

	victim = get_char_pvnum(pvnum);

	TOG_BIT(pvnum_index[pvnum]->flags, PVNUM_DENIED);

	if (HAS_BIT(pvnum_index[pvnum]->flags, PVNUM_DENIED))
	{
		send_to_char( "They have been denied access.\n\r", ch );

		log_printf("%s has been denied by %s", pvnum_index[pvnum]->name, ch->name);

		if (victim)
		{
			send_to_char("You have been denied access.\n\r", victim);
			do_quit(victim, NULL);
		}
	}
	else
	{
		send_to_char( "They are no longer denied.\n\r", ch );

		log_printf("%s has been undenied by %s", pvnum_index[pvnum]->name, ch->name);
	}
	save_players();

	pop_call();
	return;
}

void do_disconnect( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;
	int port;

	push_call("do_disconnect(%p,%p)",ch,argument);

	one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		send_to_char( "Disconnect whom?\n\r", ch );
		pop_call();
		return;
	}

	if ((port = atol(arg)) == 0)
	{
		if ((victim = get_player_world(ch, arg)) == NULL)
		{
			send_to_char( "They aren't here.\n\r", ch );
			pop_call();
			return;
		}

		if (victim->desc == NULL)
		{
			act( "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
			pop_call();
			return;
		}

		for (d = mud->f_desc ; d ; d = d->next)
		{
			if (is_desc_valid(d->character) && d == victim->desc)
			{
				close_socket(d);
				send_to_char( "Ok.\n\r", ch );
				pop_call();
				return;
			}
		}
		send_to_char( "Descriptor of that name not found!\n\r", ch );
		pop_call();
		return;
	}

	for (d = mud->f_desc ; d ; d = d->next)
	{
		if (d->descriptor == port)
		{
			close_socket(d);
			send_to_char( "Ok.\n\r", ch);
			pop_call();
			return;
		}
	}
	send_to_char( "Descriptor of that number not found!\n\r", ch );
	pop_call();
	return;
}

void do_pardon( CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	push_call("do_pardon(%p,%p)",ch,argument);

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if (arg1[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char( "Syntax: pardon <player> <killer|thief|outcast>.\n\r", ch );
		pop_call();
		return;
	}

	if ((victim = get_player_world(ch, arg1)) == NULL)
	{
		send_to_char( "They aren't here.\n\r", ch );
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "killer"))
	{
		TOG_BIT(victim->act, PLR_KILLER);

		if (HAS_BIT(victim->act, PLR_KILLER))
		{
			victim->pcdata->killer_played = victim->pcdata->played;
			send_to_char( "Killer flag added.\n\r", ch );
			send_to_char( "You are now a KILLER!\n\r", victim );
		}
		else
		{
			send_to_char( "Killer flag removed.\n\r", ch );
			send_to_char( "You are no longer a KILLER.\n\r", victim );
		}
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "thief"))
	{
		TOG_BIT(victim->act, PLR_THIEF);

		if (HAS_BIT(victim->act, PLR_THIEF))
		{
	 		send_to_char( "Thief flag added.\n\r", ch );
			send_to_char( "You are now a THIEF!\n\r", victim );
		}
		else
		{
			send_to_char( "Thief flag removed.\n\r", ch );
			send_to_char( "You are no longer a THIEF.\n\r", victim );
		}
		pop_call();
		return;
    }

	if (!strcasecmp(arg2, "outcast"))
	{
		TOG_BIT(victim->act, PLR_OUTCAST);

		if (HAS_BIT(victim->act, PLR_OUTCAST))
		{
			send_to_char( "Outcast flag added.\n\r", ch );
			send_to_char( "You are now an OUTCAST!\n\r", victim );
		}
		else
		{
			send_to_char( "Outcast flag removed.\n\r", ch );
			send_to_char( "You are no longer an OUTCAST.\n\r", victim );
		}
		pop_call();
		return;
	}
	send_to_char( "Syntax: pardon <player> <killer|thief|outcast>.\n\r", ch );
	pop_call();
	return;
}


void do_echo( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;

	push_call("do_echo(%p,%p)",ch,argument);

	if (argument[0] == '\0')
	{
		if (ch != NULL)
		{
			send_to_char( "Echo what?\n\r", ch );
		}
		pop_call();
		return;
	}

	for (d = mud->f_desc ; d ; d = d->next)
	{
		if (is_desc_valid(d->character) && (d->connected == CON_PLAYING || d->connected == CON_EDITING))
		{
			if ((!IS_NPC(d->character) && (d->character->level == MAX_LEVEL) && (ch != NULL)))
			{
				ch_printf_color(d->character, "{178}[{078}%s{178}]\n\r", ch->name);
			}
			ch_printf_color(d->character, "%s\n\r", ansi_justify(argument, get_page_width(d->character)));
		}
	}
	pop_call();
	return;
}

int find_location( CHAR_DATA *ch, char *arg )
{
	CHAR_DATA *victim;

	push_call("find_location(%p,%p)",ch,arg);

	if (is_number(arg))
	{
		if (get_room_index(atol(arg)) == NULL)
		{
			pop_call();
			return -1;
		}
		pop_call();
		return atol(arg);
	}

	if ((victim = get_player_world(ch, arg)) != NULL)
	{
		pop_call();
		return victim->in_room->vnum;
	}

	if ((victim = get_char_world(ch, arg)) != NULL)
	{
		pop_call();
		return victim->in_room->vnum;
	}

	pop_call();
	return -1;
}

void do_transfer( CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int location;
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;

	push_call("do_transfer(%p,%p)",ch,argument);

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if (arg1[0] == '\0')
	{
		send_to_char( "Transfer whom (and where)?\n\r", ch );
		pop_call();
		return;
	}

	if (IS_NPC(ch))
	{
		log_printf("[%u] Mob using transfer", ch->pIndexData->vnum);
		pop_call();
		return;
	}

	if (!strcasecmp(arg1, "all") && IS_GOD(ch))
	{
		for (d = mud->f_desc ; d ; d = d->next)
		{
			if (d->connected >= CON_PLAYING
			&&  is_desc_valid(d->character)
			&&  d->character != ch
			&&  d->character->in_room != NULL
			&&  d->character->in_room != ch->in_room
			&&  can_see_world(ch, d->character))
			{
				char buf[MAX_STRING_LENGTH];
				sprintf(buf, "%s %s", d->character->name, arg2);
				do_transfer(ch, buf);
			}
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
		if ((location = find_location(ch, arg2)) == -1)
		{
			send_to_char( "No such location.\n\r", ch );
			pop_call();
			return;
		}

		if (!can_olc_modify(ch, location))
		{
			send_to_char( "That location is not in your allocated vnum range.\n\r", ch);
			pop_call();
			return;
		}

		if (ch->level < LEVEL_IMMORTAL && room_is_private(room_index[location]))
		{
			send_to_char( "That room is private right now.\n\r", ch );
			pop_call();
			return;
		}
	}

	if ((victim = get_char_world(ch, arg1)) == NULL)
	{
		send_to_char( "They aren't here.\n\r", ch );
		pop_call();
		return;
	}

	if (victim->in_room == NULL)
	{
		send_to_char( "They are in limbo.\n\r", ch );
		pop_call();
		return;
	}

	if (victim->fighting != NULL)
	{
		stop_fighting( victim, TRUE );
	}
	act( "$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM );
	char_from_room( victim );
	char_to_room( victim, location );
	act( "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );
	if (ch != victim)
	{
		act( "$n has transferred you.", ch, NULL, victim, TO_VICT );
	}
	do_look( victim, "auto" );

	pop_call();
	return;
}

void do_at( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	int location, original;
	CHAR_DATA *mount;
	OBJ_DATA  *furniture;

	push_call("do_at(%p,%p)",ch,argument);

	argument = one_argument( argument, arg );

	if (IS_NPC(ch))
	{
		log_printf( "[%u] Mob using at: %s", ch->pIndexData->vnum, argument);
		pop_call();
		return;
	}

	if (arg[0] == '\0' || argument[0] == '\0')
	{
		send_to_char( "At where what?\n\r", ch );
		pop_call();
		return;
	}

	if ((location = find_location(ch, arg)) == -1)
	{
		send_to_char( "No such location.\n\r", ch );
		pop_call();
		return;
	}

	if (!can_olc_modify(ch, location))
	{
		send_to_char("That destination is not in your allocated vnum range.\n\r", ch);
		pop_call();
		return;
	}

	furniture = ch->furniture;
	mount	= ch->mounting;
	original  = ch->in_room->vnum;

	char_from_room( ch );
	char_to_room( ch, location);

	interpret(ch, argument);

	/*
		See if 'ch' still exists before continuing!
		Handles 'at XXXX quit' case.
	*/

	if (ch->in_room)
	{
		char_from_room(ch);
		char_to_room(ch, original);
		ch->furniture = furniture;
		ch->mounting  = mount;
	}
	pop_call();
	return;
}




void do_mfind( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	MOB_INDEX_DATA *pMobIndex;
	int nMatch, vnum;

	push_call("do_mfind(%p,%p)",ch,argument);

	one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		send_to_char( "mob find what?\n\r", ch );
		pop_call();
		return;
	}

	for (nMatch = 0, vnum = 0 ; vnum < MAX_VNUM ; vnum++)
	{
		if ((pMobIndex = get_mob_index(vnum)) != NULL)
		{
			if (!can_olc_modify(ch, pMobIndex->vnum))
			{
				continue;
			}
			if (is_name_list(arg, pMobIndex->player_name) && ++nMatch)
			{
				ch_printf_color(ch, "{178}[{078}%5u{178}] %s%s\n\r",
					pMobIndex->vnum,
					get_color_string(ch, COLOR_MOBILES, VT102_DIM),
					capitalize(pMobIndex->short_descr));
			}
		}
	}

	if (nMatch)
	{
		ch_printf( ch, "Number of matches: %d\n", nMatch );
	}
	else
	{
		send_to_char( "Nothing like that in hell, heaven, or earth.\n\r", ch );
	}
	pop_call();
	return;
}


void do_ofind( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_INDEX_DATA *pObjIndex;
	int nMatch, vnum;

	push_call("do_ofind(%p,%p)",ch,argument);

	one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		send_to_char( "Obj find what?\n\r", ch );
		pop_call();
		return;
	}

	for (nMatch = 0, vnum = 0 ; vnum < MAX_VNUM ; vnum++)
	{
		if ((pObjIndex = get_obj_index(vnum)) != NULL)
		{
			if (!can_olc_modify(ch, pObjIndex->vnum))
			{
				continue;
			}
			if (is_name_list(arg, pObjIndex->name) && ++nMatch)
			{
				ch_printf_color(ch, "{178}[{078}%5u{178}] %s%s\n\r",
					pObjIndex->vnum,
					get_color_string(ch, COLOR_OBJECTS, VT102_BOLD),
					capitalize(pObjIndex->short_descr));
			}
		}
	}
	if (nMatch)
	{
		ch_printf(ch, "Number of matches: %d\n", nMatch);
	}
	else
	{
		send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
	}
	pop_call();
	return;
}


void do_find( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];

	push_call("do_find(%p,%p)",ch,argument);

	argument = one_argument(argument, arg);

	if (!strcasecmp(arg, "mob"))
	{
		do_mfind(ch, argument);
	}
	else if (!strcasecmp(arg, "obj"))
	{
		do_ofind(ch, argument);
	}
	else
	{
		ch_printf(ch, "Syntax: find <mob|obj> <name>\n\r");
	}
	pop_call();
	return;
}


void do_reboo( CHAR_DATA *ch, char *argument )
{
	push_call("do_reboo(%p,%p)",ch,argument);
	send_to_char( "If you want to REBOOT, spell it out.\n\r", ch );
	pop_call();
	return;
}

void do_reboot( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	PLAYER_GAME *pch, *pch_next;

	push_call("do_reboot(%p,%p)",ch,argument);

	if (argument[0] == 'S' || argument[0] == 's')
	{
		sprintf(buf, "Reboot scheduled after everyone is off the Realm.\n\rCompliments of %s\n\r", ch->name);
		do_echo( ch, buf );
		SET_BIT(mud->flags, MUD_EMUD_REBOOT);
		pop_call();
		return;
	}

	if (argument[0] == 'N' || argument[0] == 'n')
	{
		DESCRIPTOR_DATA *d;
		DESCRIPTOR_DATA *d_next;

		save_world();

		for (d = mud->f_desc; d ; d = d_next)
		{
			d_next = d->next;

			if (!d->character || d->connected < 0)
			{
				write_to_descriptor (d, "\n\rSorry, Emud is Rebooting. Try again in one minute.\n\r", 0);
				close_socket(d);
			}
		}

		log_string("Saving Players...");

		for (pch = mud->f_player ; pch ; pch = pch_next)
		{
			pch_next = pch->next;

			if (pch->ch != ch)
			{
				send_to_char( "The game is now rebooting and should be back up in one minute.\n\r", pch->ch );
				do_quit(pch->ch, NULL);
			}
		}
		do_quit(ch, NULL);

		SET_BIT(mud->flags, MUD_EMUD_DOWN);
		SET_BIT(mud->flags, MUD_EMUD_BOOTING);

		pop_call();
		return;
	}
	send_to_char( "You must specify 'now' or 'soon'.\n\r", ch);
	pop_call();
	return;
}

void do_shutdow( CHAR_DATA *ch, char *argument )
{

	push_call("do_shutdow(%p,%p)",ch,argument);
	send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
	pop_call();
	return;
}

void do_shutdown( CHAR_DATA *ch, char *argument )
{
	PLAYER_GAME *pch, *pch_next;
	FILE *fp;

	push_call("do_shutdown(%p,%p)",ch,argument);

	if (!IS_GOD(ch))
	{
		log_printf("%s tried to shutdown the mud.", get_name(ch));
		dump_stack();
		pop_call();
		return;
	}

	if (tolower(argument[0]) != 'n')
	{
		send_to_char( "You must specify 'now' after the command.\n\r", ch);
		pop_call();
		return;
	}

	do_echo(NULL, "The game is shutting down now. Please try to connect in 15 minutes again.");

	save_world();

	log_string("Saving Players...");

	for (pch = mud->f_player ; pch ; pch = pch_next)
	{
		pch_next = pch->next;

		if (pch->ch != ch)
		{
			send_to_char("The game is now shut down.  Try again in 15 minutes.\n\r", pch->ch);
			do_quit(pch->ch, NULL);
		}
	}

	fp = my_fopen(SHUTDOWN_FILE, "w", FALSE);

	fprintf(fp, "Shutdown by %s.\n", ch->name);

	my_fclose(fp);

	do_quit(ch, NULL);

	my_fclose_all();

	SET_BIT(mud->flags, MUD_EMUD_DOWN);
	SET_BIT(mud->flags, MUD_EMUD_BOOTING);

	pop_call();
	return;
}


void do_snoop( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;
	bool notify;

	push_call("do_snoop(%p,%p)",ch,argument);

	argument = one_argument(argument, arg);

	if (arg[0] == '\0')
	{
		victim = ch;
	}
	else if ((victim = get_player_world(ch, arg)) == NULL)
	{
		send_to_char( "They aren't here.\n\r", ch );
		pop_call();
		return;
	}

	if (tolower(argument[0]) == 'n')
	{
		notify = TRUE;
	}
	else
	{
		notify = FALSE;
	}

	if (victim->desc == NULL)
	{
		send_to_char( "No descriptor to snoop.\n\r", ch );
		pop_call();
		return;
	}

	log_god_printf("%s: snoop %s %s", ch->name, arg, notify ? "(notified)" : "");

	if (victim == ch)
	{
		send_to_char( "Cancelling all snoops.\n\r", ch );

		for (d = mud->f_desc ; d ; d = d->next)
		{
			if (d->snoop_by == ch->desc)
			{
				d->snoop_by = NULL;
			}
		}
		pop_call();
		return;
	}

	if (victim->desc->snoop_by == ch->desc)
	{
		act( "Cancelling snoop of $N.", ch, NULL, victim, TO_CHAR );
		if (notify)
		{
			ch_printf(victim, "%s is no longer snooping you.\n\r", get_name(ch));
		}
		victim->desc->snoop_by = NULL;
		pop_call();
		return;
	}

	if (victim->desc->snoop_by != NULL )
	{
		send_to_char( "Busy already.\n\r", ch );
		pop_call();
		return;
	}

	if (get_trust(victim) == MAX_LEVEL)
	{
		send_to_char( "You failed.\n\r", ch );
		pop_call();
		return;
	}

	if (ch->desc != NULL)
	{
		for (d = ch->desc->snoop_by ; d != NULL ; d = d->snoop_by)
		{
			if (d->character == victim || d->original == victim)
			{
				send_to_char( "No snoop loops.\n\r", ch );
				pop_call();
				return;
			}
		}
	}

	victim->desc->snoop_by = ch->desc;
	act( "You are now snooping $N.", ch, NULL, victim, TO_CHAR );
	if (notify)
	{
		ch_printf(victim, "%s is now snooping you.\n\r", get_name(ch));
	}
	pop_call();
	return;
}

void do_switch( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	push_call("do_switch(%p,%p)",ch,argument);

	one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		send_to_char( "Switch into whom?\n\r", ch );
		pop_call();
		return;
	}

	if (ch->desc == NULL)
	{
		pop_call();
		return;
	}

	if (ch->desc->original != NULL)
	{
		send_to_char( "You are already switched.\n\r", ch );
		pop_call();
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL)
	{
		send_to_char( "They aren't here.\n\r", ch );
		pop_call();
		return;
	}

	if (!IS_NPC(victim))
	{
		send_to_char( "You may not switch into a player's character.\n\r", ch );
		pop_call();
		return;
	}

	if (victim == ch)
	{
		send_to_char( "Ok.\n\r", ch );
		pop_call();
		return;
	}

	if (victim->desc != NULL)
	{
		send_to_char( "Character in use.\n\r", ch );
		pop_call();
		return;
	}

	ch->desc->character  = victim;
	ch->desc->original   = ch;
	victim->desc         = ch->desc;
	ch->desc             = NULL;
	ch->pcdata->switched = TRUE;
	send_to_char( "Ok.\n\r", victim );

	pop_call();
	return;
}


void do_return( CHAR_DATA *ch, char *argument )
{
	push_call("do_return(%p,%p)",ch,argument);

	if (ch->desc == NULL)
	{
		pop_call();
		return;
	}

	if (ch->desc->original == NULL)
	{
		if (argument != NULL)
		{
			send_to_char( "You are already yourself.\n\r", ch );
		}
		pop_call();
		return;
	}

	if (argument != NULL)
	{
		send_to_char( "You return to your original body.\n\r", ch );
		morph_update(ch, NULL);
	}

	ch->desc->character					= ch->desc->original;
	ch->desc->character->timer			= 0;
	ch->desc->character->pcdata->switched	= FALSE;
	ch->desc->original					= NULL;
	ch->desc->character->desc			= ch->desc;
	ch->desc							= NULL;

	/*
		The time to die is at hand - Scandum 27-04-2002
	*/

	if (IS_NPC(ch))
	{
		raw_kill(ch);
	}
	pop_call();
	return;
}


void do_mload( CHAR_DATA *ch, char *argument )
{
	MOB_INDEX_DATA *pMobIndex;
	CHAR_DATA *victim;

	push_call("do_mload(%p,%p)",ch,argument);

	if (!is_number(argument))
	{
		ch_printf(ch, "Syntax: load mob <vnum>\n\r");
	}
	else if (!can_olc_modify(ch, atoi(argument)))
	{
		ch_printf(ch, "Vnum: %d is not in your allocated range.\n\r", atoi(argument));
	}
	else if ((pMobIndex = get_mob_index(atol(argument))) == NULL)
	{
		ch_printf(ch, "No mob has that vnum.\n\r");
	}
	else
	{
		victim = create_mobile( pMobIndex );
		char_to_room( victim, ch->in_room->vnum );
		act( "$N takes shape in the center of the room.", ch, NULL, victim, TO_ROOM );
		act( "$N takes shape in the center of the room.", ch, NULL, victim, TO_CHAR );
	}
	pop_call();
	return;
}

void do_oload( CHAR_DATA *ch, char *argument )
{
	OBJ_INDEX_DATA *pObjIndex;
	OBJ_DATA *obj;

	push_call("do_oload(%p,%p)",ch,argument);

	if (!is_number(argument))
	{
		ch_printf(ch, "Syntax: load obj <vnum>\n\r");
	}
	else if (!can_olc_modify(ch, atoi(argument)))
	{
		ch_printf(ch, "Vnum: %d is not in your allocated range.\n\r", atoi(argument));
	}
	else if ((pObjIndex = get_obj_index(atol(argument))) == NULL)
	{
		ch_printf(ch, "No obj has that vnum.\n\r");
	}
	else
	{
		obj = create_object( pObjIndex, 0);
		obj_to_char(obj, ch);
		act( "You create $p.", ch, obj, NULL, TO_CHAR );
	}
	pop_call();
	return;
}

void do_load( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];

	push_call("do_load(%p,%p)",ch,argument);

	argument = one_argument(argument, arg);

	if (argument[0] == '\0' || arg[0] == '\0')
	{
		ch_printf(ch, "Syntax: load <obj|mob> <vnum>\n\r");
		pop_call();
		return;
	}

	log_god_printf("(G) %s: load %s %s", ch->name, arg, argument);

	if (!strcasecmp(arg, "mob"))
	{
		do_mload(ch, argument);
	}
	else if (!strcasecmp(arg, "obj"))
	{
		do_oload(ch, argument);
	}
	else
	{
		do_load(ch, "");
	}
	pop_call();
	return;
}

void do_purge( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	int room, room_start;
	bool quite;

	push_call("do_purge(%p,%p)",ch,argument);

	if (argument != NULL)
	{
		quite = FALSE;
		one_argument( argument, arg );
	}
	else
	{
		quite = TRUE;
		strcpy(arg, "");
	}

	if (!strcasecmp(arg, "area"))
	{
		arg[0] = '\0';
		room_start = ch->in_room->vnum;

		for (room = room_index[room_start]->area->low_r_vnum ; room <= room_index[room_start]->area->hi_r_vnum ; room++)
		{
			if (room_index[room] == NULL)
			{
				continue;
			}
			if (room_index[room]->area != room_index[room_start]->area)
			{
				break;
			}
			char_from_room(ch);
			char_to_room(ch, room);
			do_purge(ch, NULL);
		}
		char_from_room(ch);
		char_to_room(ch, room_start);
		send_to_char( "You have purged the Area.\n\r", ch);
		pop_call();
		return;
	}

	if (arg[0] == '\0'  || arg == NULL)
	{
		CHAR_DATA *vnext;
		OBJ_DATA  *obj_next;

		for (victim = ch->in_room->first_person ; victim != NULL ; victim = vnext)
		{
			vnext = victim->next_in_room;
			if (IS_NPC(victim) && victim != ch)
			{
				junk_mob(victim);
			}
		}

		for (obj = ch->in_room->first_content ; obj != NULL ; obj = obj_next)
		{
			obj_next = obj->next_content;
			if (obj->item_type != ITEM_CORPSE_PC)
			{
				junk_obj( obj );
			}
		}
		if (!quite)
		{
			act( "$n purges the room!", ch, NULL, NULL, TO_ROOM);
			send_to_char( "Ok.\n\r", ch );
		}
		pop_call();
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char( "They aren't here.\n\r", ch );
		pop_call();
		return;
	}

	if (!IS_NPC(victim))
	{
		send_to_char( "Not on PC's.\n\r", ch );
		pop_call();
		return;
	}

	act( "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
	junk_mob(victim);
	pop_call();
	return;
}

void do_advance( CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int level;
	int iLevel;

	push_call("do_advance(%p,%p)",ch,argument);

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
	{
		send_to_char( "Syntax: advance <char> <level>.\n\r", ch );
		pop_call();
		return;
	}

	if ((victim = get_char_room(ch, arg1)) == NULL)
	{
		send_to_char( "That player is not here.\n\r", ch);
		pop_call();
		return;
	}

	if (IS_NPC(ch))
	{
		log_printf("%s tried to advance %s.", ch->name, victim->name);
		pop_call();
		return;
	}

	if (IS_NPC(victim))
	{
		send_to_char( "Not on NPC's.\n\r", ch );
		pop_call();
		return;
	}

	if ((level = atol(arg2)) < 1 || level > MAX_LEVEL)
	{
		send_to_char( "Level must be 1 to 99.\n\r", ch );
		pop_call();
		return;
	}

	if (level > get_trust(ch))
	{
		send_to_char( "Limited to your trust level.\n\r", ch );
		pop_call();
		return;
	}

	if (!IS_GOD(ch))
	{
		send_to_char("You are not allowed to advance.\n\r",ch);
		pop_call();
		return;
	}

	if (level < victim->level)
	{
		int sn;

    		if (IS_GOD(victim))
    		{
			send_to_char("You cannot lower the level of a God.\n\r", ch);
    			pop_call();
    			return;
		}

		send_to_char( "Lowering a player's level!\n\r", ch );
		send_to_char( "**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r", victim );
		victim->level       = 1;
		victim->pcdata->exp = 0;
		victim->max_hit     = victim->pcdata->actual_max_hit  = 20;
		victim->max_mana    = victim->pcdata->actual_max_mana = 100;
		victim->max_move    = victim->pcdata->actual_max_move = 100;

		roll_race(victim);

		for (sn = 0 ; sn < MAX_SKILL ; sn++)
		{
			victim->pcdata->learned[sn] = 0;
		}
		for (sn = 0 ; sn < MAX_CLASS ; sn++)
		{
			victim->pcdata->mclass[sn] = 0;
		}
		victim->pcdata->practice = 21;
		victim->pcdata->mclass[victim->class] = 1;
	}
	else
	{
		send_to_char( "Raising a player's level!\n\r", ch );
		send_to_char( "**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", victim );
	}

	for (iLevel = victim->level ; iLevel < level ; iLevel++)
	{
		advance_level( victim, FALSE );
	}
	victim->hit      = victim->max_hit;
	victim->mana     = victim->max_mana;
	victim->move     = victim->max_move;

	sub_player (victim);
	add_player (victim);
	save_char_obj (victim, NORMAL_SAVE);
	save_char_obj (victim, BACKUP_SAVE);

	pop_call();
	return;
}

void do_trust( CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int level;

	push_call("do_trust(%p,%p)",ch,argument);

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
	{
		send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
		pop_call();
		return;
	}

	if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
	{
		send_to_char( "That player is not here.\n\r", ch);
		pop_call();
		return;
	}

	if ( ( level = atol( arg2 ) ) < 0 || level > MAX_LEVEL )
	{
		send_to_char( "Level must be 0 (reset) or 1 to 99.\n\r", ch );
		pop_call();
		return;
	}

	if ( level > get_trust( ch ) )
	{
		send_to_char( "Limited to your trust.\n\r", ch );
		pop_call();
		return;
	}

	victim->trust = level;
	pop_call();
	return;
}

void do_restore( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	push_call("do_restore(%p,%p)",ch,argument);

	if (argument[0] == '\0')
	{
		send_to_char( "Restore whom?\n\r", ch );
		pop_call();
		return;
	}

	if ((victim = get_char_room_even_blinded(ch, argument)) == NULL)
	{
		if (!IS_NPC(ch))
		{
			send_to_char( "They aren't here.\n\r", ch );
		}
		else
		{
			log_build_printf(ch->pIndexData->vnum, "do_restore: target not found: %s", argument);
		}
		pop_call();
		return;
	}

	victim->hit  = victim->max_hit;
	victim->mana = victim->max_mana;
	victim->move = victim->max_move;
	update_pos( victim );
	act( "$n has restored you.", ch, NULL, victim, TO_VICT );
	send_to_char( "Ok.\n\r", ch );

	pop_call();
	return;
}


void do_freeze( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];
	int pvnum;

	push_call("do_freeze(%p,%p)",ch,argument);

	if (argument[0] == '\0')
	{
		sprintf(buf, "List of frozen players:\n\r\n\r");

		for (pvnum = 0 ; pvnum < MAX_PVNUM ; pvnum++)
		{
			if (pvnum_index[pvnum] == NULL)
			{
				continue;
			}
			if (HAS_BIT(pvnum_index[pvnum]->flags, PVNUM_FROZEN))
			{
				cat_sprintf(buf, "%-16s", pvnum_index[pvnum]->name);
			}
		}
		ch_printf(ch, "%s\n\r", justify(buf, 80));
		pop_call();
		return;
	}

	if ((pvnum = get_pvnum_name(argument)) == -1)
	{
		send_to_char("That player does not exists.\n\r", ch);
		pop_call();
		return;
	}

	if (pvnum < 100)
	{
		send_to_char( "You failed.\n\r", ch );
		pop_call();
		return;
	}

	victim = get_char_pvnum(pvnum);

	TOG_BIT(pvnum_index[pvnum]->flags, PVNUM_FROZEN);

	if (HAS_BIT(pvnum_index[pvnum]->flags, PVNUM_FROZEN))
	{
		send_to_char( "They have been frozen.\n\r", ch );
		log_printf("%s has been frozen by %s", get_name_pvnum(pvnum), ch->name);

		if (victim)
		{
			send_to_char( "You have been frozen.\n\r", victim);
		}
	}
	else
	{
		send_to_char("They are no longer frozen.\n\r", ch);
		log_printf("%s has been un-frozen by %s", get_name_pvnum(pvnum), ch->name);

		if (victim)
		{
			send_to_char( "You are no longer frozen.\n\r", victim);
		}
	}
	save_players();

	pop_call();
	return;
}


void do_mute( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	int pvnum;

	push_call("do_mute(%p,%p)",ch,argument);

	if (argument[0] == '\0')
	{
		sprintf(buf, "List of muted players:\n\r\n\r");

		for (pvnum = 0 ; pvnum < MAX_PVNUM ; pvnum++)
		{
			if (pvnum_index[pvnum] == NULL)
			{
				continue;
			}
			if (HAS_BIT(pvnum_index[pvnum]->flags, PVNUM_MUTED))
			{
				cat_sprintf(buf, "%-16s", pvnum_index[pvnum]->name);
			}
		}
		ch_printf(ch, "%s\n\r", justify(buf, 80));
		pop_call();
		return;
	}

	if ((pvnum = get_pvnum_name(argument)) == -1)
	{
		send_to_char("That player does not exists.\n\r", ch);
		pop_call();
		return;
	}

	if (pvnum < 100)
	{
		send_to_char( "You failed.\n\r", ch );
		pop_call();
		return;
	}

	TOG_BIT(pvnum_index[pvnum]->flags, PVNUM_MUTED);

	if (HAS_BIT(pvnum_index[pvnum]->flags, PVNUM_MUTED))
	{
		send_to_char( "They have been muted.\n\r", ch );

		log_printf("%s has been muted by %s", get_name_pvnum(pvnum), ch->name);
	}
	else
	{
		send_to_char( "They are no longer muted.\n\r", ch );

		log_printf("%s has been un-muted by %s", get_name_pvnum(pvnum), ch->name);
	}
	save_players();

	pop_call();
	return;
}


void do_log( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_INPUT_LENGTH];
	int pvnum;

	push_call("do_log(%p,%p)",ch,argument);

	if (argument[0] == '\0')
	{
		sprintf(buf, "List of logged players:\n\r\n\r");

		for (pvnum = 0 ; pvnum < MAX_PVNUM ; pvnum++)
		{
			if (pvnum_index[pvnum] == NULL)
			{
				continue;
			}
			if (HAS_BIT(pvnum_index[pvnum]->flags, PVNUM_LOGGED))
			{
				cat_sprintf(buf, "%-16s", pvnum_index[pvnum]->name);
			}
		}
		ch_printf(ch, "%s\n\r", justify(buf, 80));
		pop_call();
		return;
	}

	if ((pvnum = get_pvnum_name(argument)) == -1)
	{
		send_to_char("That player does not exists.\n\r", ch);
		pop_call();
		return;
	}

	TOG_BIT(pvnum_index[pvnum]->flags, PVNUM_LOGGED);

	if (HAS_BIT(pvnum_index[pvnum]->flags, PVNUM_LOGGED))
	{
		send_to_char( "They have been logged.\n\r", ch );

		log_printf("%s has been logged by %s", get_name_pvnum(pvnum), ch->name);
	}
	else
	{
		send_to_char( "They are no longer logged.\n\r", ch );

		log_printf("%s has been un-logged by %s", get_name_pvnum(pvnum), ch->name);
	}
	save_players();

	pop_call();
	return;
}

void do_silence( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];
	int pvnum;

	push_call("do_silence(%p,%p)",ch,argument);

	if (argument[0] == '\0')
	{
		sprintf(buf, "List of silenced players:\n\r\n\r");

		for (pvnum = 0 ; pvnum < MAX_PVNUM ; pvnum++)
		{
			if (pvnum_index[pvnum] == NULL)
			{
				continue;
			}
			if (HAS_BIT(pvnum_index[pvnum]->flags, PVNUM_SILENCED))
			{
				cat_sprintf(buf, "%-16s", pvnum_index[pvnum]->name);
			}
		}
		ch_printf(ch, "%s\n\r", justify(buf, 80));
		pop_call();
		return;
	}

	if ((pvnum = get_pvnum_name(argument)) == -1)
	{
		send_to_char("That player does not exist.\n\r", ch);
		pop_call();
		return;
	}

	if (pvnum < 100)
	{
		send_to_char( "You failed.\n\r", ch );
		pop_call();
		return;
	}

	TOG_BIT(pvnum_index[pvnum]->flags, PVNUM_SILENCED);

	victim = get_char_pvnum(pvnum);

	if (HAS_BIT(pvnum_index[pvnum]->flags, PVNUM_SILENCED))
	{
		send_to_char( "They have been silenced.\n\r", ch );

		log_printf("%s has been silenced by %s", get_name_pvnum(pvnum), ch->name);
		if (victim)
		{
			send_to_char("You have been silenced.\n\r", victim);
		}
	}
	else
	{
		send_to_char( "They are no longer silenced.\n\r", ch );

		log_printf("%s has been un-silenced by %s", get_name_pvnum(pvnum), ch->name);

		if (victim)
		{
			send_to_char("You are no longer silenced.\n\r", victim);
		}
	}
	save_players();

	pop_call();
	return;
}


void do_peace( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *rch;

	push_call("do_peace(%p,%p)",ch,argument);

	if (*argument != '\0')
	{
		if ((rch = get_char_room_even_blinded(ch, argument)) != NULL)
		{
			stop_fighting(rch, TRUE);

			stop_hate_fear(rch);
		}
		pop_call();
		return;
	}

	for (rch = ch->in_room->first_person ; rch ; rch = rch->next_in_room)
	{
		stop_fighting(rch, FALSE);
		stop_hate_fear(rch);
	}

	if (!IS_NPC(ch))
	{
		act( "$n booms 'PEACE!'", ch, NULL, NULL, TO_ROOM );
		send_to_char( "You boom 'PEACE!'\n\r", ch );
	}
	pop_call();
	return;
}

void do_ban( CHAR_DATA *ch, char *argument )
{
	BAN_DATA *pban;
	char skp[80];

	push_call("do_ban(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (argument[0] == '\0')
	{
		send_to_char("List of banned sites:\n\r", ch);

		for (pban = mud->f_ban ; pban ; pban = pban->next)
		{
			ch_printf_color(ch, "{158}%15s {138}banned on {118}%s {138}by {128}%s\n\r",
				pban->name,
				str_resize(get_time_string(pban->date), skp, -24),
				pban->banned_by);
		}
		pop_call();
		return;
	}

	for (pban = mud->f_ban ; pban ; pban = pban->next)
	{
		if (!strcasecmp(argument, pban->name))
		{
			UNLINK(pban, mud->f_ban, mud->l_ban, next, prev);
			STRFREE(pban->name);
			STRFREE(pban->banned_by);
			save_sites();
			send_to_char("Siteban removed.\n\r", ch );
			pop_call();
			return;
		}
	}

	ALLOCMEM(pban, BAN_DATA, 1);
	pban->name		= STRALLOC( argument );
	pban->banned_by	= STRALLOC( ch->name );
	pban->date		= mud->current_time;
	LINK(pban, mud->f_ban, mud->l_ban, next, prev);

	save_sites();

	log_printf("The site: %s has been banned by %s", argument, ch->name);

	send_to_char("Siteban set.\n\r", ch );

	pop_call();
	return;
}


void do_nban( CHAR_DATA *ch, char *argument )
{
	BAN_DATA *pban;
	char skp[80];

	push_call("do_nban(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (argument[0] == '\0')
	{
		send_to_char("List of newbie banned sites:\n\r", ch);

		for (pban = mud->f_nban ; pban ; pban = pban->next)
		{
			ch_printf_color(ch, "{158}%15s {138}banned on {118}%s {138}by {128}%s\n\r",
				pban->name,
				str_resize(get_time_string(pban->date), skp, -24),
				pban->banned_by);
		}
		pop_call();
		return;
	}

	for (pban = mud->f_nban ; pban ; pban = pban->next)
	{
		if (!strcasecmp(argument, pban->name))
		{
			UNLINK(pban, mud->f_nban, mud->l_nban, next, prev);
			STRFREE(pban->name);
			STRFREE(pban->banned_by);
			save_nsites();
			send_to_char("Newbie Siteban removed.\n\r", ch );
			pop_call();
			return;
		}
	}

	ALLOCMEM(pban, BAN_DATA, 1);
	pban->name		= STRALLOC( argument );
	pban->banned_by	= STRALLOC( ch->name );
	pban->date		= mud->current_time;
	LINK(pban, mud->f_nban, mud->l_nban, next, prev);

	save_nsites();

	log_printf("The site: %s has been banned by %s", argument, ch->name);

	send_to_char("Siteban set.\n\r", ch );

	pop_call();
	return;
}

void do_wizlock( CHAR_DATA *ch, char *argument )
{
	push_call("do_wizlock(%p,%p)",ch,argument);

	TOG_BIT(mud->flags, MUD_WIZLOCK);

	if (HAS_BIT(mud->flags, MUD_WIZLOCK))
	{
		send_to_char( "Game wizlocked.\n\r", ch );
		log_printf("do_wizlock(%s) *locked*", get_name(ch));
	}
	else
	{
		send_to_char( "Game un-wizlocked.\n\r", ch );
		log_printf("do_wizlock(%s) *unlocked*", get_name(ch));
	}
	pop_call();
	return;
}



/*
	Thanks to Grodyn for pointing out bugs in this function.
*/

void do_force( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];

	push_call("do_force(%p,%p)",ch,argument);

	argument = one_argument( argument, arg );

	if (arg[0] == '\0' || argument[0] == '\0')
	{
		send_to_char( "Force whom to do what?\n\r", ch );
		pop_call();
		return;
	}

	str_cpy_max(buf, argument, 80);

	if (!strcasecmp(arg, "all"))
	{
		PLAYER_GAME *vch, *vch_next;

		for (vch = mud->f_player ; vch ; vch = vch_next)
		{
			vch_next = vch->next;

			if (vch->ch != ch)
			{
				if (get_trust(vch->ch) < get_trust(ch))
				{
					act("$n forces you to '$t'.", ch, argument, vch->ch, TO_VICT);
					interpret( vch->ch, buf );
				}
			}
		}
	}
	else
	{
		CHAR_DATA *victim;

		if ((victim = get_char_world(ch, arg)) == NULL)
		{
			send_to_char( "They aren't here.\n\r", ch );
			pop_call();
			return;
		}

		if (victim == ch)
		{
			send_to_char( "Aye aye, right away!\n\r", ch );
			pop_call();
			return;
		}

		if (IS_NPC(victim) && ch->level < MAX_LEVEL && !HAS_BIT(mud->flags, MUD_EMUD_REALGAME))
		{
			send_to_char( "You can't force a mobile.\n\r", ch );
			pop_call();
			return;
		}

		if (!IS_NPC(victim) && victim->level >= ch->level)
		{
			send_to_char( "Do it yourself!\n\r", ch );
			pop_call();
			return;
		}

		act( "$n forces you to '$t'.", ch, argument, victim, TO_VICT );
		interpret( victim, buf );
	}
	send_to_char( "Ok.\n\r", ch );
	pop_call();
	return;
}

/*
	New routines by Dionysos.
*/

void do_invis( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	PLAYER_GAME *gpl;

	push_call("do_invis(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (ch->level < MAX_LEVEL || *argument == '\0')
	{
		victim = ch;
	}
	else if ((victim = get_player_world(ch, argument)) == NULL)
	{
		victim = ch;
	}
	if (HAS_BIT(victim->act, PLR_WIZINVIS) )
	{
		act( "$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM );
		send_to_char( "You slowly fade back into existence.\n\r", victim);
	}
	else
	{
		for (gpl = mud->f_player ; gpl ; gpl = gpl->next)
		{
			if (gpl->ch->pcdata->reply == victim)
			{
				gpl->ch->pcdata->reply = NULL;
			}
		}
		act( "$n slowly fades into thin air.", victim, NULL, NULL, TO_ROOM );
		send_to_char( "You slowly vanish into thin air.\n\r", victim );
	}
	TOG_BIT(victim->act, PLR_WIZINVIS);

	pop_call();
	return;
}

void do_cloak( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	push_call("do_cloak(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (ch->level < MAX_LEVEL || *argument == '\0')
	{
		victim = ch;
	}
	else if ((victim = get_player_world(ch, argument)) == NULL)
	{
		victim = ch;
	}

	if (HAS_BIT(victim->act, PLR_WIZCLOAK))
	{
		act( "$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM);
		send_to_char( "You slowly fade back into existence.\n\r", victim);
	}
	else
	{
		act( "$n slowly fades into thin air.", victim, NULL, NULL, TO_ROOM );
		send_to_char( "You slowly vanish into thin air.\n\r", victim );
	}
	TOG_BIT(victim->act, PLR_WIZCLOAK);

	pop_call();
	return;
}

void do_buildlight( CHAR_DATA *ch, char *argument )
{
	push_call("do_buildlight(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (HAS_BIT(ch->act, PLR_BUILDLIGHT))
	{
		send_to_char("Build light mode off.\n\r", ch);
	}
	else
	{
		send_to_char("Build light mode on.\n\r", ch);
	}

	TOG_BIT(ch->act, PLR_BUILDLIGHT);

	pop_call();
	return;
}

void do_holylight( CHAR_DATA *ch, char *argument )
{
	push_call("do_holylight(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (HAS_BIT(ch->act, PLR_HOLYLIGHT))
	{
		send_to_char( "Holy light mode off.\n\r", ch );
	}
	else
	{
		send_to_char( "Holy light mode on.\n\r", ch );
	}
	TOG_BIT(ch->act, PLR_HOLYLIGHT);

	pop_call();
	return;
}

void do_hearlog( CHAR_DATA *ch, char *argument )
{
	push_call("do_hearlog(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (HAS_BIT(ch->act, PLR_HEARLOG))
	{
		send_to_char( "No longer listening to log file.\n\r", ch );
	}
	else
	{
		send_to_char( "You are listening to the log file.\n\r", ch );
	}
	TOG_BIT(ch->act, PLR_HEARLOG);

	pop_call();
	return;
}

void do_connect( CHAR_DATA *ch, char *argument )
{
	int dest_vnum;
	int door_num;
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];

	push_call("do_connect(%p,%p)",ch,argument);

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	strcpy( arg3, argument );

	if (arg1[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char("Syntax: connect <direction> <destination vnum> [both]\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && !can_olc_modify(ch, ch->in_room->vnum))
	{
		send_to_char("This room is not in your allocated range.\n\r", ch);
		pop_call();
		return;
	}

	door_num  = direction_door(arg1);
	dest_vnum = atol( arg2 );

	if (door_num < 0 || door_num > 5 )
	{
		if (IS_NPC(ch))
		{
			log_printf( "[%u] Invalid direction: <connect %s %s>", ch->pIndexData->vnum, arg1, arg2 );
		}
		send_to_char( "Invalid direction.\n\r",ch);
		pop_call();
		return;
	}

	if (get_room_index(dest_vnum) == NULL && dest_vnum != -1)
	{
		if (IS_NPC(ch))
		{
			log_printf( "[%u] Invalid destination vnum: <connect %s %s>", ch->pIndexData->vnum, arg1, arg2 );
		}
		send_to_char( "A room with that vnum does not exist.\n\r",ch);
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && dest_vnum != -1 && !can_olc_modify(ch, dest_vnum))
	{
		ch_printf(ch, "That vnum is not in your allocated range.\n\r");
		pop_call();
		return;
	}

	set_exit(ch->in_room->vnum, door_num, dest_vnum);

	if (arg3[0] == 'b' || arg3[0] == 'B')
	{
		set_exit( dest_vnum, rev_dir[door_num], ch->in_room->vnum);
	}
	pop_call();
	return;
}

extern void ListCheck(void);

void do_test1( CHAR_DATA *ch, char *argument )
{
	log_printf("do_test1(%p,%p)",ch,argument);

	send_to_char( "Siteban saving.\n\r", ch );
	save_sites();
}


void do_tick( CHAR_DATA *ch, char *argument)
{
	int cnt, ticks;

	push_call("do_tick(%p,%p)",ch,argument);

	ticks = atol(argument);

	for (cnt = 0 ; cnt < ticks * PULSE_TICK ; cnt++)
	{
		update_handler();
	}
	ch_printf(ch, "Time advances %d mud %s.\n\r", ticks, short_to_name("hour", ticks));

	pop_call();
	return;
}

char *broken_bits( long long number, char *vector, bool linear )
{
	long long bit, cnt;
	bool found;
	static char broken_string[MAX_INPUT_LENGTH];

	push_call("broken_bits(%p,%p,%p)",number,vector,linear);

	if (HAS_BIT(mud->flags, MUD_STRAIGHTNUMBERS))
	{
		sprintf(broken_string, "%lld" , number);
		pop_call();
		return broken_string;
	}

	found	= FALSE;
	bit		= 0;

	broken_string[0] = '\0';

	if (!linear)
	{
		while (bit <= number && number >= 0)
		{
			if (HAS_BIT(number, bit))
			{
				for (cnt = mud->bitvector_ref[*vector - 'A'] ; cnt < MAX_BITVECTOR ; cnt++)
				{
					if (bit == bitvector_table[cnt].value)
					{
						if (!str_prefix(vector, bitvector_table[cnt].name))
						{
							if (found)
							{
								strcat( broken_string, "|" );
							}
							strcat(broken_string, bitvector_table[cnt].name);
							found = TRUE;
							break;
						}
					}
				}
				if (cnt == MAX_BITVECTOR)
				{
					if (found)
					{
						strcat(broken_string, "|" );
					}
					log_printf("broken_bits: %s not found %lld", vector, bit);
					cat_sprintf(broken_string, "%lld", bit);
					found = TRUE;
				}
			}
			if (bit != 0)
			{
				bit *= 2;
			}
			else
			{
				bit++;
			}
		}
	}
	else
	{
		for (cnt = mud->bitvector_ref[*vector - 'A'] ; cnt < MAX_BITVECTOR ; cnt++)
		{
			if (number == bitvector_table[cnt].value)
			{
				if (vector[0] == bitvector_table[cnt].name[0] && !str_prefix(vector, bitvector_table[cnt].name))
				{
					strcat(broken_string, bitvector_table[cnt].name);
					found = TRUE;
					break;
				}
			}
		}
	}
	if (!found)
	{
		sprintf(broken_string, "%lld", number);
	}
	pop_call();
  	return( broken_string);
}

void do_destroy( CHAR_DATA *ch, char * arg )
{
	char arg1[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	HISCORE_DATA *hiscore;
	int cnt;

	push_call("do_destroy(%p,%p)",ch,arg);

	arg = one_argument(arg, arg1);

	if (!IS_GOD(ch))
	{
		log_printf("security leak");
		dump_stack();
		pop_call();
		return;
	}

	if (arg1[0] == '\0')
	{
		send_to_char("Syntax: destroy <castle|clan|player|hiscores|victories> <name>\n\r", ch );
		pop_call();
		return;
	}

	if (!strcasecmp(arg1, "castle"))
	{
		if ((victim = get_player_world(ch, arg)) == NULL)
		{
			send_to_char( "Hmmm...can't find them.\n\r", ch );
			pop_call();
			return;
		}

		if (victim->pcdata->castle == NULL)
		{
			send_to_char( "They don't have a castle structure.\n\r", ch );
			pop_call();
			return;
		}

		del_castle(victim);
		ch_printf(ch, "Deleted %s's castle.\n\r", get_name(victim));
	}

	if (!strcasecmp(arg1, "clan"))
	{
		CLAN_DATA *clan;
		char buf[MAX_INPUT_LENGTH];

		if ((clan = get_clan(arg)) == NULL)
		{
			send_to_char( "That clan does not exist!\n\r", ch );
			pop_call();
			return;
		}

		if (ch->level < MAX_LEVEL)
		{
			send_to_char( "You are not allowed to destroy clans.\n\r", ch );
			pop_call();
			return;
		}
		sprintf(buf, "The clan of %s has been disbanded by the gods!", clan->name);
		do_echo(NULL, buf);

		destroy_clan(clan);

		pop_call();
		return;
	}

	if (!strcasecmp(arg1, "player"))
	{
		if ((victim = get_player_world(ch, arg)) == NULL)
		{
			send_to_char("Hmmm...can't find them.\n\r", ch);
			pop_call();
			return;
		}

		if (IS_GOD(victim))
		{
			send_to_char( "You cannot destroy gods.\n\r", ch);
			pop_call();
			return;
		}

		sprintf(arg1, "%s has sent %s's soul to oblivion.", get_name(ch), victim->name);
		do_echo(ch, arg1);

		delete_player(victim);

		pop_call();
		return;
	}

	if (!strcasecmp(arg1, "hiscores"))
	{
		for (cnt = 0 ; cnt < MOST_MOST ; cnt++)
		{
			for (hiscore = mud->high_scores[cnt]->first ; hiscore ; hiscore = hiscore->next)
			{
				RESTRING(hiscore->player, "");
				hiscore->vnum = 0;
				hiscore->score = 0;
			}
		}
		pop_call();
		return;
	}

	if (!strcasecmp(arg1, "victories"))
	{
		for (cnt = 0 ; cnt < VICTORY_LIST_SIZE ; cnt++)
		{
			strcpy(victory_list[cnt], "Z");
		}
		save_victors();
		pop_call();
		return;
	}

	do_destroy(ch, "");
	pop_call();
	return;
}


void describe_object_program( CHAR_DATA *ch, OBJ_PROG *prg )
{
	push_call("describe_object_program(%p,%p)",ch,prg);

	switch (prg->trigger)
	{
		case TRIG_COMMAND:		ch_printf_color(ch, "{078}<{178}%3d{078}> [{178}%3d%%{078}] {138}TRIG_COMMAND {178}%s\n\r",		prg->index, prg->percentage, cmd_table[prg->cmd].name);	break;
		case TRIG_UNKNOWN:		ch_printf_color(ch, "{078}<{178}%3d{078}> [{178}%3d%%{078}] {138}TRIG_UNKNOWN {178}%s\n\r",		prg->index, prg->percentage, prg->unknown);	break;
		case TRIG_TICK:		ch_printf_color(ch, "{078}<{178}%3d{078}> [{178}%3d%%{078}] {138}TRIG_TICK\n\r",				prg->index, prg->percentage);				break;
		case TRIG_VOID:		ch_printf_color(ch, "{078}<{178}%3d{078}> [{178}----{078}] {138}TRIG_VOID\n\r",					prg->index);							break;
		case TRIG_HIT:			ch_printf_color(ch, "{078}<{178}%3d{078}> [{178}%3d%%{078}] {138}TRIG_HIT\n\r",					prg->index, prg->percentage);				break;
		case TRIG_DAMAGE:		ch_printf_color(ch, "{078}<{178}%3d{078}> [{178}%3d%%{078}] {138}TRIG_DAMAGE\n\r",				prg->index, prg->percentage);				break;
		case TRIG_WEAR:		ch_printf_color(ch, "{078}<{178}%3d{078}> [{178}%3d%%{078}] {138}TRIG_WEAR\n\r",				prg->index, prg->percentage);				break;
		case TRIG_REMOVE:		ch_printf_color(ch, "{078}<{178}%3d{078}> [{178}%3d%%{078}] {138}TRIG_REMOVE\n\r",				prg->index, prg->percentage);				break;
		case TRIG_ROOM_COMMAND:	ch_printf_color(ch, "{078}<{178}%3d{078}> [{178}%3d%%{078}] {138}TRIG_ROOM_COMMAND {178}%s\n\r",	prg->index, prg->percentage, cmd_table[prg->cmd].name);	break;
		case TRIG_WEAR_COMMAND:	ch_printf_color(ch, "{078}<{178}%3d{078}> [{178}%3d%%{078}] {138}TRIG_WEAR_COMMAND {178}%s\n\r",	prg->index, prg->percentage, cmd_table[prg->cmd].name);	break;
		case TRIG_ROOM_UNKNOWN:	ch_printf_color(ch, "{078}<{178}%3d{078}> [{178}%3d%%{078}] {138}TRIG_ROOM_UNKNOWN {178}%s\n\r",	prg->index, prg->percentage, prg->unknown);	break;
		case TRIG_WEAR_UNKNOWN:	ch_printf_color(ch, "{078}<{178}%3d{078}> [{178}%3d%%{078}] {138}TRIG_WEAR_UNKNOWN {178}%s\n\r",	prg->index, prg->percentage, prg->unknown);	break;
		default: 				ch_printf_color(ch, "{078}<{178}%3d{078}> [{178}%3d%%{078}] {118}ERROR\n\r",					prg->index, prg->percentage);				break;
	}
	switch (prg->obj_command)
	{
		case OPROG_ECHO:			ch_printf_color(ch, "{148}ECHO         {178}%s\n\r",							prg->argument );																	break;
		case OPROG_GOD_COMMAND:		ch_printf_color(ch, "{158}GOD COMMAND  {178}%s\n\r",							prg->argument );																	break;
		case OPROG_GOD_ARGUMENT:		ch_printf_color(ch, "{158}GOD ARGUMENT {178}%s\n\r",							prg->argument );																	break;
		case OPROG_COMMAND:			ch_printf_color(ch, "{158}COMMAND      {178}%s\n\r",							prg->argument );																	break;
		case OPROG_ARGUMENT:		ch_printf_color(ch, "{158}ARGUMENT     {178}%s\n\r",							prg->argument );																	break;
		case OPROG_QUEST_SET:		ch_printf_color(ch, "{168}SET QUEST    {178}%d %d %d\n\r",						prg->quest_offset, prg->quest_bits, prg->if_value );										break;
		case OPROG_QUEST_ADD:		ch_printf_color(ch, "{168}ADD QUEST    {178}%d %d %d\n\r",						prg->quest_offset, prg->quest_bits, prg->if_value );										break;
		case OPROG_PLAYER_QUEST_IF:	ch_printf_color(ch, "{128}IF P QUEST   {178}%d %d %c %d {078}[{178}%d %d{078}]\n\r",	prg->quest_offset, prg->quest_bits, prg->if_symbol, prg->if_value, prg->if_true, prg->if_false );	break;
		case OPROG_OBJECT_QUEST_IF:	ch_printf_color(ch, "{128}IF O QUEST   {178}%d %d %c %d {078}[{178}%d %d{078}]\n\r",	prg->quest_offset, prg->quest_bits, prg->if_symbol, prg->if_value, prg->if_true, prg->if_false );	break;
		case OPROG_IF_HAS_OBJECT:	ch_printf_color(ch, "{128}IF HAS OBJ   {178}%d {078}[{178}%d %d{078}]\n\r",			prg->if_value, prg->if_true, prg->if_false );											break;
		case OPROG_IF:				ch_printf_color(ch, "{128}IF CHECK     {178}%s %c %d {078}[{178}%d %d{078}]\n\r", 	broken_bits(prg->if_check, "OIF_", TRUE), prg->if_symbol, prg->if_value, prg->if_true, prg->if_false);	break;
		case OPROG_APPLY:			ch_printf_color(ch, "{148}APPLY TEMP   {178}%s %d\n\r",							broken_bits(prg->if_check, "OAPPLY_", TRUE), prg->if_value );								break;
		case OPROG_JUNK:			ch_printf_color(ch, "{118}JUNK ITEM\n\r");																													break;
		default:					ch_printf_color(ch, "{118}ERROR\n\r");																														break;
	}
	pop_call();
	return;
}

void mpdebug(CHAR_DATA *ch, MP_TOKEN *token)
{
	char buf[100];

	push_call("mpdebug(%p,%p)",ch,token);

	sprintf(buf, "%.*s", 2 * token->level, "                                        ");

	switch (token->type)
	{
		case MPTOKEN_SOCIAL:
			ch_printf_color(ch, "{178}%s%s %s\n\r", buf, social_table[token->value].name, token->string);
			break;
		case MPTOKEN_COMMAND:
			ch_printf_color(ch, "{178}%s%s %s\n\r", buf, cmd_table[token->value].name, token->string);
			break;
		case MPTOKEN_IF:
			ch_printf_color(ch, "{128}%sif %s\n\r", buf, token->string);
			break;
		case MPTOKEN_AND:
			ch_printf_color(ch, "{128}%sand %s\n\r", buf, token->string);
			break;
		case MPTOKEN_OR:
			ch_printf_color(ch, "{128}%sor %s\n\r", buf, token->string);
			break;
		case MPTOKEN_ELSE:
			ch_printf_color(ch, "{128}%selse\n\r", buf);
			break;
		case MPTOKEN_ELSEIF:
			ch_printf_color(ch, "{128}%selseif %s\n\r", buf, token->string);
			break;
		case MPTOKEN_ENDIF:
			ch_printf_color(ch, "{128}%sendif\n\r", buf);
			break;
		case MPTOKEN_BREAK:
			ch_printf_color(ch, "{118}%sbreak\n\r", buf);
			break;
		case MPTOKEN_SWITCH:
			ch_printf_color(ch, "{158}%sswitch %s\n\r", buf, token->string);
			break;
		case MPTOKEN_CASE:
			ch_printf_color(ch, "{158}%scase %s\n\r", buf, token->string);
			break;
		case MPTOKEN_DEFAULT:
			ch_printf_color(ch, "{158}%sdefault\n\r", buf);
			break;
		case MPTOKEN_ENDSWITCH:
			ch_printf_color(ch, "{158}%sendswitch\n\r", buf);
			break;
		default:
			ch_printf_color(ch, "{118}%sunknown %s\n\r", buf, token->string);
			break;
	}
	pop_call();
	return;
}

void do_program(CHAR_DATA *ch, MUD_PROG *prog, char type)
{
	MP_TOKEN *token;

	push_call("do_program(%p,%p)",ch,prog);

	while (prog)
	{
		switch (type)
		{
			case 'm':
				ch_printf_color(ch, "{138}>%s %s~\n\r", mprog_type_to_name(prog->type), prog->arglist);
				break;
			case 'o':
				ch_printf_color(ch, "{138}>%s %s~\n\r", oprog_type_to_name(prog->type), prog->arglist);
				break;
			case 'r':
				ch_printf_color(ch, "{138}>%s %s~\n\r", rprog_type_to_name(prog->type), prog->arglist);
				break;
		}

		for (token = prog->first_token ; token ; token = token->next)
		{
			mpdebug(ch, token);
		}
		prog = prog->next;
	}
	pop_call();
	return;
}

void do_oprogram( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_INDEX_DATA *obj;
	OBJ_PROG *oprog;

	push_call("do_oprogram(%p,%p)",ch,argument);

	one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		send_to_char( "Syntax: oprog <name|vnum>\n\r", ch );
		pop_call();
		return;
	}

	if (is_number(arg) && get_obj_index(atoi(arg)) != NULL)
	{
		obj = get_obj_index(atoi(arg));
	}
	else
	{
		if (get_obj_world(ch, arg) == NULL)
		{
			send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
			pop_call();
			return;
		}
		obj = get_obj_world(ch, arg)->pIndexData;
	}

	if (!can_olc_modify(ch, obj->vnum))
	{
		send_to_char("That object is not in your allocated vnum range.\n\r", ch);
		pop_call();
		return;
	}

	ch_printf_color(ch, "{078}[{178}%5u{078}] {128}%s\n\r", obj->vnum, obj->short_descr);

	if (obj->first_prog)
	{

		do_program(ch, obj->first_prog, 'o');
		pop_call();
		return;
	}

	if (obj->first_oprog == NULL)
	{
		ch_printf_color(ch, "{078}This object has no program.\n\r");
		pop_call();
		return;
	}

	oprog = obj->first_oprog;

	while (oprog != NULL)
	{
		describe_object_program(ch, oprog);
		oprog = oprog->next;
	}
	pop_call();
	return;
}

void do_mprogram( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	MOB_INDEX_DATA *mind;

	push_call("do_mprogram(%p,%p)",ch,argument);

	one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		send_to_char("Mprog whom?\n\r", ch );
		pop_call();
		return;
	}

	if (is_number(arg) && get_mob_index(atoi(arg)) != NULL)
	{
		mind = get_mob_index(atoi(arg));
	}
	else	if ((victim = get_char_world(ch, arg)) == NULL)
	{
		send_to_char( "They aren't here.\n\r", ch );
		pop_call();
		return;
	}
	else if (!IS_NPC(victim))
	{
		send_to_char( "Players have no mobile programs.\n\r", ch);
		pop_call();
		return;
	}
	else
	{
		mind = victim->pIndexData;
	}

	if (!can_olc_modify(ch, mind->vnum))
	{
		send_to_char("That mobile is not in your allocated range.\n\r", ch);
		pop_call();
		return;
	}

	ch_printf_color(ch, "{078}[{178}%5u{078}] {058}%s\n\r", mind->vnum, mind->short_descr);

	do_program(ch, mind->first_prog, 'm');

	pop_call();
	return;
}

void do_rprogram( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *room;

	push_call("do_rprogram(%p,%p)",ch,argument);

	one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		room = ch->in_room;
	}
	else if (is_number(arg))
	{
		room = get_room_index(atoi(arg));
	}
	else
	{
		send_to_char("Syntax: prog room [vnum]\n\r", ch);
		pop_call();
		return;
	}

	if (room == NULL)
	{
		send_to_char("Invalid room vnum\n\r", ch);
		pop_call();
		return;
	}

	if (!can_olc_modify(ch, room->vnum))
	{
		send_to_char("That room is not in your allocated range.\n\r", ch);
		pop_call();
		return;
	}

	ch_printf_color(ch, "{078}[{178}%5u{078}] {058}%s\n\r", room->vnum, room->name);

	do_program(ch, room->first_prog, 'r');

	pop_call();
	return;
}

void do_shutoff( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_INDEX_DATA *obj;
	MOB_INDEX_DATA *mob;

	push_call("do_shutoff(%p,%p)",ch,argument);

	argument = one_argument(argument, arg);

	if (argument[0] == '\0' || arg[0] == '\0')
	{
		send_to_char( "Syntax: prog shutoff <mob|obj> <vnum>\n\r", ch );
		pop_call();
		return;
	}

	if (!can_olc_modify(ch, atoi(argument)))
	{
		ch_printf(ch, "That vnum is not in your allocated range.\n\r");
	}
	else if (!strcasecmp(arg, "obj"))
	{
		if ((obj = get_obj_index(atoi(argument))) != NULL)
		{
			ch_printf(ch, "Oprog index for obj %u erased.\n\r", obj->vnum);
			obj->oprogtypes = 0;
		}
		else
		{
			ch_printf(ch, "There no such object.\n\r");
		}
	}
	else if (!strcasecmp(arg, "mob"))
	{
		if ((mob = get_mob_index(atoi(argument))) != NULL)
		{
			ch_printf(ch, "Mprog index for mob %u erased.\n\r", mob->vnum);
			mob->progtypes = 0;
		}
		else
		{
			ch_printf(ch, "There is no such mobile.\n\r");
		}
	}
	else
	{
		do_shutoff(ch, "");
	}
	pop_call();
	return;
}


void do_prog( CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	push_call("do_prog(%p,%p)",ch,argument);

	argument = one_argument(argument, arg);

	if (!strcasecmp(arg, "mob"))
	{
		do_mprogram(ch, argument);
	}
	else if (!strcasecmp(arg, "obj"))
	{
		do_oprogram(ch, argument);
	}
	else if (!strcasecmp(arg, "room"))
	{
		do_rprogram(ch, argument);
	}
	else if (!strcasecmp(arg, "shutoff"))
	{
		do_shutoff(ch, argument);
	}
	else
	{
		ch_printf(ch, "Syntax: prog <mob|obj|shutoff> <argument>\n\r");
	}
	pop_call();
	return;
}

void do_pload( CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *fch;
	DESCRIPTOR_DATA *d;
	bool exists;

	push_call("do_pload(%p,%p)",ch,argument);

	if (!check_parse_name(argument, FALSE))
	{
		send_to_char( "Either you chose a name with less than 3 characters, with a non-ascii character,\n\ror it was the name of a monster.\n\rIllegal name, try another.\n\r", ch );
		pop_call();
		return;
	}

	fch = lookup_char( argument );

	if (fch != NULL)
	{
		send_to_char( "This character is already loaded.\n\r", ch );
		pop_call();
		return;
	}

	for (d = mud->f_desc ; d ; d = d->next)
	{
		if (is_desc_valid(d->character))
		{
			if (!strcasecmp(argument, d->character->name))
			{
				act("$E was in the process of logging on. You close $S session.", ch, NULL, d->character, TO_CHAR);
				close_socket(d);
			}
		}
	}

	ALLOCMEM(d, DESCRIPTOR_DATA, 1);
	d->original	= NULL;
	d->descriptor	= -998;  /* Special case for pload loads */
	exists		= load_char_obj( d, argument );
	fch			= d->character;

	if (!exists)
	{
		ch_printf(ch, "The character named '%s' cannot be found.\n\r", argument);

		if (d != NULL)
		{
			d->character = NULL;
			d->original  = NULL;
		}
		if (fch != NULL)
		{
			fch->desc = NULL;
			extract_char(fch);
		}
		if (d != NULL)
		{
			FREEMEM(d);
		}
		pop_call();
		return;
	}

	d->connected = CON_PLAYING;
	add_char(fch);
	add_player(fch);
	fch->desc = NULL;

	if (fch->in_room != NULL)
	{
		char_to_room( fch, fch->in_room->vnum );
	}
	else
	{
		char_to_room( fch, ch->in_room->vnum );
	}
	send_to_char( "You load the character.\n\r", ch );

	enter_game(fch);

	d->character = NULL;
	d->original  = NULL;

	FREEMEM(d);

	pop_call();
	return;
}


void do_pquit( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	push_call("do_pquit(%p,%p)",ch,argument);

	if ((victim = get_player_world(ch, argument)) == NULL)
	{
		send_to_char( "That player does not exist in the realm.\n\r", ch );
		pop_call();
		return;
	}

	if (IS_NPC(victim))
	{
		send_to_char( "You are trying to pquit a mobile!\n\r", ch);
		pop_call();
		return;
	}

	if (victim->level >= ch->level && is_desc_valid(victim))
	{
		send_to_char( "You cannot force that player to quit.\n\r", ch);
		pop_call();
		return;
	}

	ch_printf(ch, "You forced %s to quit.\n\r", victim->name);
	do_quit(victim, NULL);

	pop_call();
	return;
}

void do_fixpass (CHAR_DATA* ch, char* argument)
{
	CHAR_DATA *victim;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];

	push_call("do_fixpass(%p,%p)",ch,argument);

	/*
		one_argument smashes case, using one_argument_nolower - Scandum
	*/

	argument = one_argument_nolower(argument, arg1);
	argument = one_argument_nolower(argument, arg2);
	argument = one_argument_nolower(argument, arg3);

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
	{
		send_to_char("Syntax: fixpass <character> <new pass> <new pass>.\n\r", ch );
		pop_call();
		return;
	}

	if ((victim = get_player_world(ch, arg1)) == NULL)
	{
		send_to_char("They aren't here (better pload them).\n\r", ch );
		pop_call();
		return;
	}

	if (victim->level >= ch->level && victim != ch)
	{
		send_to_char("They can fix their own password.\n\r", ch );
		pop_call();
		return;
	}

	if (strcmp(arg2, arg3))
	{
		send_to_char("The new password does not verify. Please try again.\n\r", ch);
		pop_call();
		return;
	}

	if (!is_valid_password(arg2))
	{
		send_to_char("New password not acceptable, try again.\n\r", ch);
		pop_call();
		return;
	}

	victim->pcdata->password = encrypt64(arg2);

	save_char_obj(victim, NORMAL_SAVE);

	ch_printf(ch, "Ok. %s has had their password changed.\n\r", victim->name);

	pop_call();
	return;
}

void do_rename (CHAR_DATA* ch, char* argument)
{
	char old_name[MAX_INPUT_LENGTH],
	new_name[MAX_INPUT_LENGTH],
	strsave [MAX_INPUT_LENGTH];
	CHAR_DATA* victim;
	FILE* file;

	push_call("do_rename(%p,%p)",ch,argument);

	argument = one_argument(argument, old_name);
	argument = one_argument(argument, new_name);

	if (old_name[0] == '\0')
	{
		send_to_char ("Rename who?\n\r",ch);
		pop_call();
		return;
	}

	if (new_name[0] == '\0')
	{
		send_to_char ("Rename them to what?\n\r",ch);
		pop_call();
		return;
	}

	if ((victim = get_player_world(ch, old_name)) == NULL)
	{
		send_to_char ("There is no such player online.\n\r",ch);
		pop_call();
		return;
	}

	if (victim->level >= ch->level && victim != ch)
	{
		send_to_char ("They can change their own name.\n\r", ch);
		pop_call();
		return;
	}

	if (!check_parse_name(new_name, TRUE))
	{
		send_to_char ("The new name is illegal.\n\r",ch);
		pop_call();
		return;
	}

	sprintf(strsave, "%s/%c/%c/%s", PLAYER_DIR, tolower(new_name[0]), tolower(new_name[1]), capitalize_name(new_name));

	close_reserve();

	file = my_fopen(strsave, "r", TRUE);

	if (file)
	{
		send_to_char ("A player with that name already exists!\n\r",ch);

		my_fclose(file);
		open_reserve();

		pop_call();
		return;
	}

	open_reserve();

	if (get_player_world(ch, new_name))
	{
		send_to_char ("A player with that name is currently playing.\n\r",ch);
		pop_call();
		return;
	}

	save_char_obj(victim, BACKUP_SAVE);

	sprintf(strsave, "%s/%c/%c/%s", PLAYER_DIR, tolower(victim->name[0]), tolower(victim->name[1]), capitalize_name(victim->name));

	STRFREE(victim->name);
	victim->name = STRALLOC(capitalize_name(new_name));

	RESTRING(pvnum_index[victim->pcdata->pvnum]->name, victim->name);

	save_char_obj(victim, NORMAL_SAVE);

	unlink(strsave);

	ch_printf(ch, "Character renamed to %s.\n\r", victim->name);

	if (ch != victim)
	{
		ch_printf(victim, "%s has renamed you to %s.\n\r", get_name(ch), victim->name);
	}
	pop_call();
	return;
}

int direction_door( char *txt )
{
	push_call("direction_door(%p)",txt);

	if (txt == NULL || *txt == '\0')
	{
		pop_call();
		return -1;
	}

	if (is_number(txt))
	{
		if (atoi(txt) < 0 || atoi(txt) > 5)
		{
			pop_call();
			return -1;
		}
		pop_call();
		return atoi(txt);
	}

	if (!strcasecmp(txt, "n") || !strcasecmp(txt, "north"))
	{
		pop_call();
		return 0;
	}
	if (!strcasecmp(txt, "e") || !strcasecmp(txt, "east"))
	{
		pop_call();
		return 1;
	}
	if (!strcasecmp(txt, "s") || !strcasecmp(txt, "south"))
	{
		pop_call();
		return 2;
	}
	if (!strcasecmp(txt, "w") || !strcasecmp(txt, "west"))
	{
		pop_call();
		return 3;
	}
	if (!strcasecmp(txt, "u") || !strcasecmp(txt, "up"))
	{
		pop_call();
		return 4;
	}
	if (!strcasecmp(txt, "d") || !strcasecmp(txt, "down"))
	{
		pop_call();
		return 5;
	}
	pop_call();
	return -1;
}


/* REVERT command.  Restore a player's backup file.  Presto 8/98 */

void do_revert(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char back_name[MAX_STRING_LENGTH], real_name[MAX_STRING_LENGTH];
	char victimname[MAX_STRING_LENGTH];
	int  i, length;
	CHAR_DATA *victim;
	FILE *back_file, *real_file;

	push_call("do_revert(%p,%p)",ch,argument);

	if (ch->level < MAX_LEVEL)
	{
		send_to_char("You may not do that.\n\r", ch);
		pop_call();
		return;
	}

	argument = one_argument(argument, arg);

	if (*arg == '\0')
	{
		send_to_char("Usage: revert <name>\n\r", ch);
		pop_call();
		return;
	}

	/* See if they're logged in. */

	if ((victim = get_player_world(ch, arg)) == NULL)
	{
		/* They aren't logged in. */
	}
	else
	{
		if (victim == ch)
		{
			send_to_char("You may not revert yourself.\n\r", ch);
			pop_call();
			return;
		}
		if (IS_NPC(victim))
		{
			send_to_char("You may not revert a mob.\n\r", ch);
			pop_call();
			return;
		}

		char_from_room(victim);
		char_to_room(victim, ROOM_VNUM_TEMPLE);
		send_to_char("Your backup file is being restored. Quitting...\n\r", victim);
		send_to_char("Logging out player...\n\r", ch);
		strcpy(arg, victim->name);
		do_quit(victim, NULL);
	}

	/* See if the backup pfile exists. */

	strcpy(victimname, capitalize_name(arg));
	length = strlen(victimname);

	for (i = 1 ; i < length ; i++)
	{
		victimname[i] = tolower(victimname[i]);
	}


	sprintf(back_name, "%s/%c/%c/del/%s", PLAYER_DIR, tolower(victimname[0]), tolower(victimname[1]), victimname);

	if ((back_file = my_fopen(back_name, "r", FALSE)) == NULL)
	{
		sprintf(back_name, "%s/%c/%c/bak/%s", PLAYER_DIR, tolower(victimname[0]), tolower(victimname[1]), victimname);

		if ((back_file = my_fopen(back_name, "r", FALSE)) == NULL)
		{
			send_to_char("Could not open player's backup file.\n\r", ch);

			pop_call();
			return;
		}
	}
	send_to_char("Player's backup file is open...\n\r", ch);
		
	sprintf(real_name, "%s/%c/%c/%s",     PLAYER_DIR, tolower(victimname[0]), tolower(victimname[1]), victimname);

	if ((real_file = my_fopen(real_name, "w", FALSE)) == NULL)
	{
		send_to_char("Could not open player's real file.\n\r", ch);
		my_fclose(back_file);

		pop_call();
		return;
	}
	else
	{
		send_to_char("Player's real file is open...\n\r", ch);
	}
	i = fgetc(back_file);

	while (i != EOF)
	{
		fprintf(real_file, "%c", i);
		i = fgetc(back_file);
	}

	my_fclose(real_file);
	my_fclose(back_file);

	remove(back_name);

	send_to_char("Revert was successful.\n\r", ch);

	pop_call();
	return;
}


void set_quest_bits( unsigned char **pointer, unsigned int offset, unsigned int bits, unsigned int value )
{
	unsigned int cnt;
	unsigned int bytec, byteb, bitval, cbitval;
	unsigned char *pt;

	push_call("set_quest_bits(%p,%p,%p,%p)",pointer,offset,bits,value);

	if (offset + bits >= 8 * MAX_QUEST_BYTES || bits > 32)
	{
		log_printf("set_quest_bits: invalid bit range: %u %u %u", offset, bits, value);
		dump_stack();
		pop_call();
		return;
	}

	pt = (*pointer);

	if (pt == NULL)
	{
		if (value == 0)
		{
			pop_call();
			return;
		}
		ALLOCMEM(pt, bool, MAX_QUEST_BYTES);
		*pointer = pt;
	}

	for (cnt = 0 ; cnt < bits ; cnt++, offset++)
	{
		bytec   = offset / 8;
		byteb   = 1 << (offset % 8);
		cbitval = (*(pt + bytec)) & byteb;
		bitval  = value % 2;

		value /= 2;

		if (bitval == 0)
		{
			if (cbitval != 0)
			{
				*(pt+bytec) -= byteb;
			}
		}
		else
		{
			if (cbitval == 0)
			{
				*(pt+bytec) += byteb;
			}
		}
	}

	if (!is_quest(pt))
	{
		FREEMEM( pt );
		*pointer = NULL;
	}
	pop_call();
	return;
}

int get_quest_bits(unsigned char *pt, unsigned int offset, unsigned int bits)
{
	unsigned int cnt, value, refer;
	unsigned int bytec, byteb, cbitval;

	push_call("get_quest_bits(%p,%p,%p)",pt,offset,bits);

	if (pt == NULL)
	{
		pop_call();
		return 0;
	}

	if (offset + bits >= 8 * MAX_QUEST_BYTES || bits > 32)
	{
		log_printf("get_quest_bits: invalid bit range: %u %u", offset, bits);
		dump_stack();
		pop_call();
		return 0;
	}

	refer  = 1;
	value  = 0;

	for (cnt = 0 ; cnt < bits ; cnt++, offset++)
	{
		bytec   = offset / 8;
		byteb   = 1 << (offset % 8);
		cbitval = (*(pt+bytec)) & byteb;
		if (cbitval > 0)
		{
			value += refer;
		}
		refer *= 2;
	}
	pop_call();
	return(value);
}

char *quest_bits_to_string( unsigned char *pt)
{
	static char quest_bit_buf[100];
	sh_int cnt;

	push_call("quest_bits_to_string(%p)",pt);

	quest_bit_buf[0] = '\0';

	for (cnt = MAX_QUEST_BYTES-1 ; cnt >= 0 ; cnt--)
	{
		if (pt != NULL)
		{
			cat_sprintf(quest_bit_buf, "%02X", *(pt+cnt));
		}
		else
		{
			strcat(quest_bit_buf, "00" );
		}

		if (cnt % 2 == 0 && cnt > 0)
		{
			strcat(quest_bit_buf, "-");
		}
	}
	pop_call();
	return( quest_bit_buf );
}

bool is_quest( register unsigned char *pt )
{
	bool cnt;

	push_call("is_quest(%p)",pt);

	if (pt == NULL)
	{
		pop_call();
		return( FALSE );
	}

	for (cnt = 0 ; cnt < MAX_QUEST_BYTES ; cnt++)
	{
		if (*(pt+cnt) != 0)
		{
			pop_call();
			return( TRUE );
		}
	}
	pop_call();
	return( FALSE );
}

void do_oset( CHAR_DATA *ch, char *argument )
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int value;

	push_call("do_oset(%p,%p)",ch,argument);

	smash_tilde( argument );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	strcpy( arg3, argument );

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
	{
		ch_printf(ch, "Syntax: set obj <field> <argument>\n\r");
		ch_printf(ch, "  Field being one of:\n\r");
		ch_printf(ch, "    v0 v1 v2 v3 name short long desc extra setextra clrextra\n\r");
		ch_printf(ch, "    wear level weight cost timer sactimer quest randquest   \n\r");
		pop_call();
		return;
	}

	if ((obj = get_obj_here(ch, arg1)) == NULL)
	{
		ch_printf(ch, "Object not found.\n\r");
		pop_call();
		return;
	}

	value = atol(arg3);

	if ( !strcasecmp( arg2, "value0" ) || !strcasecmp( arg2, "v0" ) )
	{
		obj->value[0] = value;
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "value1" ) || !strcasecmp( arg2, "v1" ) )
	{
		obj->value[1] = value;
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "value2" ) || !strcasecmp( arg2, "v2" ) )
	{
		obj->value[2] = value;
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "value3" ) || !strcasecmp( arg2, "v3" ) )
	{
		obj->value[3] = value;
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "extra" ) )
	{
		obj->extra_flags = value;
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "setextra" ) )
	{
		SET_BIT(obj->extra_flags, value);
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "clrextra" ) )
	{
		DEL_BIT(obj->extra_flags, value);
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "wear" ) )
	{
		obj->wear_flags = value;
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

	if ( !strcasecmp( arg2, "name" ) )
	{
		STRFREE (obj->name );
		if( !strcasecmp( arg3, "null") )
		{
			obj->name = STRALLOC(obj->pIndexData->name);
		}
		else
		{
			obj->name  = STRALLOC( arg3 );
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

	if ( !strcasecmp( arg2, "long" ) )
	{
		STRFREE (obj->long_descr );
		if (!strcasecmp( arg3, "null"))
		{
			obj->long_descr = STRALLOC(obj->pIndexData->long_descr);
		}
		else
		{
			obj->long_descr = STRALLOC( arg3 );
		}
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "desc" ) )
	{
		STRFREE (obj->description);
		if (!strcasecmp( arg3, "null") )
		{
			obj->description = STRALLOC(obj->pIndexData->description);
		}
		else
		{
			obj->description = STRALLOC( arg3 );
		}
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "quest" ) )
	{
		int firstBit,len;

		if (sscanf(arg3,"%d %d %d",&firstBit,&len,&value)!=3)
		{
			ch_printf(ch, "Bad parameters <set obj quest %s>", arg3);
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
			ch_printf(ch, "Bad parameters <set obj randquest %s>", arg3);
			pop_call();
			return;
		}
		SET_BIT(obj->extra_flags, ITEM_MODIFIED);
		value = number_bits( len );
		set_quest_bits( &obj->obj_quest, firstBit, len, value);

		pop_call();
		return;
	}
	do_oset(ch, "");
	pop_call();
	return;
}


void do_mset( CHAR_DATA *ch, char *argument )
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
		ch_printf(ch, "Syntax: set mob <target> <field> <argument>\n\r");
		ch_printf(ch, "  Field being one of:\n\r");
		ch_printf(ch, "    quest questr randquest randquestr sex level exp gold align\n\r");
		ch_printf(ch, "    hp mana move thirst drunk full name short long desc\n\r");

		pop_call();
		return;
	}

	if ((victim = get_char_room(ch, arg1)) == NULL)
	{
		ch_printf(ch, "They aren't here.\n\r");
		pop_call();
		return;
	}

	value = is_number(arg3) ? atol(arg3) : -1;

	if ( !strcasecmp( arg2, "quest" ) )
	{
		int firstBit,len;

		if (sscanf(arg3,"%d %d %d",&firstBit,&len,&value) != 3)
		{
			ch_printf(ch, "Syntax: set mob <target> quest <offset> <numbits> <value>\n\r");
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
			ch_printf(ch, "Syntax: set mob <target> questr <vnum> <offset> <numbits> <value>\n\r");
			pop_call();
			return;
		}
		if (vnum < 0 || vnum >= MAX_VNUM || room_index[vnum]==NULL)
		{
			ch_printf(ch, "Room [%u] not found.\n\r", vnum);
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
			ch_printf(ch, "Syntax: randquest <offset> <numbits>\n\r");
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

		if (sscanf(arg3,"%d %d %d",&vnum,&firstBit,&len)!=3)
		{
			ch_printf(ch, "Syntax: set mob <target> randquestr <vnum> <offset> <numbits>\n\r");
			pop_call();
			return;
		}

		if (vnum < 0 || vnum >= MAX_VNUM || room_index[vnum] == NULL)
		{
			ch_printf(ch, "Vnum [%u] not found.\n\r", vnum);
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
			set_quest_bits( &victim->pcdata->quest[room_index[vnum]->area->low_r_vnum/100], firstBit, len, value );
		}
		pop_call();
		return;
	}

	if ( !strcasecmp( arg2, "sex" ) )
	{
		if ( value < 0 || value > 2 )
		{
			pop_call();
			return;
		}
		victim->sex = value;

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

	if (!strcasecmp(arg2, "hp"))
	{
		if (!IS_NPC(victim))
		{
			value = URANGE(1, value, 99999);
			victim->pcdata->actual_max_hit += value - victim->max_hit;
			victim->max_hit = value;
		}
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "mana"))
	{
		if (!IS_NPC(victim))
		{
			value = URANGE(1, value, 32000);
			victim->pcdata->actual_max_mana += value - victim->max_mana;
			victim->max_mana = value;
		}
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "move"))
	{
		if (!IS_NPC(victim))
		{
			value = URANGE(1, value, 32000);
			victim->pcdata->actual_max_move += value - victim->max_move;
			victim->max_move = value;
		}
		pop_call();
		return;
	}

	if (!strcasecmp(arg2, "align"))
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
		log_printf("[%u] Unknown argument: <mpmset %s %s %s>", ch->pIndexData->vnum, arg1, arg2, arg3);
	}
	pop_call();
	return;
}


void do_pset( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int value;

	push_call("do_pset(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	smash_tilde( argument );

	argument = one_argument(argument, arg);

	if (arg[0] == '\0' || argument[0] == '\0')
	{
		ch_printf(ch, "Syntax: set player <player> <field> <argument>\n\r");
		ch_printf(ch, "  Field being one of:\n\r");
		ch_printf(ch, "    god class race wis con int dex str\n\r");
		pop_call();
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		ch_printf(ch, "They aren't here.\n\r");
		pop_call();
		return;
	}

	argument = one_argument(argument, arg);
	value    = is_number(argument) ? atoi(argument) : -1;

	if (IS_NPC(victim))
	{
		ch_printf(ch, "Not on mobs.\n\r");
		pop_call();
		return;
	}

	if (!strcasecmp(arg, "god"))
	{
		if (lookup_god(argument) == -1)
		{
			ch_printf(ch, "No such god.\n\r");
		}
		else
		{
			victim->pcdata->god = lookup_god(argument);
		}
	}
	else if (!strcasecmp(arg, "class"))
	{
		if (lookup_class(argument) == -1)
		{
			ch_printf(ch, "No such class.\n\r");
		}
		else
		{
			victim->class = lookup_class(argument);
		}
	}
	else if (!strcasecmp(arg, "race"))
	{
		if (lookup_race(argument) == -1)
		{
			ch_printf(ch, "No such race.\n\r");
		}
		else
		{
			victim->race = lookup_race(argument);
		}
	}
	else if (!strcasecmp(arg, "str"))
	{
		if (value < 3 || value > 99)
		{
			send_to_char( "Strength range is 3 to 99.\n\r", ch );
		}
		else
		{
			victim->pcdata->perm_str = value;
		}
	}
	else if (!strcasecmp(arg, "int"))
	{
		if (value < 3 || value > 99)
		{
			send_to_char( "Intelligence range is 3 to 99.\n\r", ch );
		}
		else
		{
			victim->pcdata->perm_int = value;
		}
	}
	else if (!strcasecmp(arg, "wis"))
	{
		if (value < 3 || value > 99)
		{
			send_to_char( "Wisdom range is 3 to 99.\n\r", ch );
		}
		else
		{
			victim->pcdata->perm_wis = value;
		}
	}
	else if (!strcasecmp(arg, "dex"))
	{
		if ( value < 3 || value > 99 )
		{
			send_to_char( "Dexterity range is 3 to 99.\n\r", ch );
		}
		else
		{
			victim->pcdata->perm_dex = value;
		}
	}
	else if (!strcasecmp(arg, "con"))
	{
		if (value < 3 || value > 99)
		{
			send_to_char( "Constitution range is 3 to 99.\n\r", ch );
		}
		else
		{
			victim->pcdata->perm_con = value;
		}
	}
	else
	{
		do_pset(ch, "");
	}
	pop_call();
	return;
}


void do_sset( CHAR_DATA *ch, char *argument )
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int value;
	int sn;
	bool fAll;

	push_call("do_sset(%p,%p)",ch,argument);

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
	{
		send_to_char("Syntax: set skill <player> <all|skill|spell> <value>\n\r", ch);
		pop_call();
		return;
	}

	if ((victim = get_player_world(ch, arg1)) == NULL)
	{
		send_to_char( "They aren't here.\n\r", ch );
		pop_call();
		return;
	}

	fAll = !strcasecmp(arg2, "all");

	if (!fAll && skill_lookup(arg2) < 0)
	{
		send_to_char( "No such skill or spell.\n\r", ch );
		pop_call();
		return;
	}

	if (!is_number(arg3))
	{
		send_to_char( "Value must be numeric.\n\r", ch );
		pop_call();
		return;
	}

	value = atol(arg3);

	if (value < 0 || value > 100)
	{
		send_to_char( "Value range is 0 to 100.\n\r", ch );
		pop_call();
		return;
	}

	if (fAll)
	{
		for (sn = 0 ; sn < MAX_SKILL; sn++ )
		{
			if (skill_table[sn].name != NULL)
			{
				victim->pcdata->learned[sn] = value;
			}
		}
	}
	else
	{
		victim->pcdata->learned[skill_lookup(arg2)] = value;
	}
	pop_call();
	return;
}


void do_set( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];

	push_call("do_set(%p,%p)",ch,argument);

	argument = one_argument(argument, arg);

	if (IS_NPC(ch))
	{
		log_printf("do_set: used by mob");
		dump_stack();
		pop_call();
		return;
	}

	log_god_printf("(G) Log %s: set %s %s", ch->name, arg, argument);

	if (!strcasecmp(arg, "mob"))
	{
		do_mset(ch, argument);
	}
	else if (!strcasecmp(arg, "obj"))
	{
		do_oset(ch, argument);
	}
	else if (!strcasecmp(arg, "skill"))
	{
		do_sset(ch, argument);
	}
	else if (!strcasecmp(arg, "player"))
	{
		do_pset(ch, argument);
	}
	else
	{
		send_to_char("Syntax: set <mob|obj|skill|player> <argument>\n\r", ch);
	}
	pop_call();
	return;
}
