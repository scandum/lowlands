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
#include <math.h>

/*
	Local functions.
*/

bool			get_obj			args( ( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container, bool fDisplay ) );
bool			put_obj			args( ( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container, bool fDisplay ) );
bool			drop_obj			args( ( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container, bool fDisplay ) );
bool			sacrifice_obj		args( ( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container, bool fDisplay ) );
int			get_cost			args( ( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy ) );
OBJ_DATA	*	find_char_corpse	args( ( CHAR_DATA *ch, bool ) );

/*
	Determines if ch can wear an object based on class - Scandum 06-07-2002
*/

bool class_allowed(OBJ_DATA *obj, CHAR_DATA *ch, bool fDisplay)
{
	bool forbidden = FALSE;

	push_call("class_allowed(%p,%p,%p)",obj,ch,fDisplay);

	if (obj->pIndexData->class_flags == 0)
	{
		pop_call();
		return TRUE;
	}

	switch (ch->class)
	{
 		case CLASS_RANGER:       forbidden = !IS_SET(obj->pIndexData->class_flags, FLAG_CLASS_RANGER);       break;
 		case CLASS_GLADIATOR:    forbidden = !IS_SET(obj->pIndexData->class_flags, FLAG_CLASS_GLADIATOR);    break;
 		case CLASS_MARAUDER:     forbidden = !IS_SET(obj->pIndexData->class_flags, FLAG_CLASS_MARAUDER);     break;
 		case CLASS_NINJA:        forbidden = !IS_SET(obj->pIndexData->class_flags, FLAG_CLASS_NINJA);        break;
 		case CLASS_ELEMENTALIST: forbidden = !IS_SET(obj->pIndexData->class_flags, FLAG_CLASS_ELEMENTALIST); break;
 		case CLASS_ILLUSIONIST:  forbidden = !IS_SET(obj->pIndexData->class_flags, FLAG_CLASS_ILLUSIONIST);  break;
 		case CLASS_MONK:         forbidden = !IS_SET(obj->pIndexData->class_flags, FLAG_CLASS_MONK);         break;
 		case CLASS_NECROMANCER:  forbidden = !IS_SET(obj->pIndexData->class_flags, FLAG_CLASS_NECROMANCER);  break;
 	}

  	if (forbidden)
 	{
 		if (fDisplay)
		{
			act("You are of the wrong class to use $p.", ch, obj, NULL, TO_CHAR);
		}
		pop_call();
		return FALSE;
	}
	pop_call();
	return TRUE;
}


bool could_dual( CHAR_DATA *ch )
{
	push_call("could_dual(%p)",ch);

	if (learned(ch, gsn_dual_wield))
	{
		pop_call();
		return TRUE;
	}

	if (rspec_req(ch, RSPEC_MULTI_ARMS))
	{
		pop_call();
		return TRUE;
	}
	pop_call();
	return FALSE;
}


bool can_dual( CHAR_DATA *ch, int fDisplay )
{
	push_call("can_dual(%p,%d)",ch,fDisplay);

	if (!could_dual(ch))
	{
		pop_call();
		return FALSE;
	}
/*
	if (get_eq_char(ch, WEAR_DUAL_WIELD))
	{
		if (fDisplay)
		{
			send_to_char( "You are already wielding two weapons!\n\r", ch );
		}
		pop_call();
		return FALSE;
	}
*/
	if (rspec_req(ch, RSPEC_MULTI_ARMS))
	{
		pop_call();
		return TRUE;
	}

	if (get_eq_char(ch, WEAR_SHIELD))
	{
		if (fDisplay)
		{
			send_to_char( "You cannot dual wield while holding a shield!\n\r", ch );
		}
		pop_call();
		return FALSE;
	}

	if (get_eq_char(ch, WEAR_HOLD))
	{
		if (fDisplay)
		{
			send_to_char( "You cannot dual wield while holding something!\n\r", ch );
		}
		pop_call();
		return FALSE;
	}
	pop_call();
	return TRUE;
}


int count_total_objects( OBJ_DATA *container )
{
	int count;
	OBJ_DATA *obj;

	push_call("count_total_objects(%p)",container);

	for (count = 0, obj = container->first_content ; obj ; obj = obj->next_content)
	{
		count++;
	}
	pop_call();
	return( count );
}


int reinc_eq_level( CHAR_DATA *ch )
{
	if (IS_NPC(ch) || ch->pcdata->reincarnation == 0)
	{
		return ch->level;
	}

	return (ch->level * (1.0 + UMIN(10, ch->pcdata->reincarnation) * 0.058));
}


int obj_eq_level( CHAR_DATA *ch, OBJ_DATA *obj )
{
	if (IS_NPC(ch) || ch->pcdata->reincarnation == 0)
	{
		return obj->level;
	}

	return ceilf(obj->level / (1.0 + UMIN(10, ch->pcdata->reincarnation) * 0.058));
}

bool get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container, bool fDisplay )
{
	CHAR_DATA *rch;

	push_call("get_obj(%p,%p,%p,%p)",ch,obj,container,fDisplay);

	if (!IS_SET(obj->wear_flags, ITEM_TAKE))
	{
		if (fDisplay)
		{
			send_to_char( "You cannot take that.\n\r", ch );
		}
		pop_call();
		return FALSE;
	}

	if (obj->item_type == ITEM_FURNITURE)
	{
		for (rch = ch->in_room->first_person ; rch ; rch = rch->next_in_room)
		{
			if (rch->furniture == obj)
			{
				if (fDisplay)
				{
					act("$N seems to be using $p", ch, obj, rch, TO_CHAR);
				}
				pop_call();
				return FALSE;
			}
		}
	}

	if (ch->carry_number >= can_carry_n(ch))
	{
		if (fDisplay)
		{
			act("You cannot carry that many items.", ch, NULL, NULL, TO_CHAR);
		}
		pop_call();
		return FALSE;
	}

	if (container == NULL || container->carried_by != ch)
	{
		if (!IS_IMMORTAL(ch))
		{
			if (ch->carry_weight + get_obj_weight(obj) > can_carry_w(ch))
			{
				if (fDisplay)
				{
					act( "You cannot carry that much weight.", ch, NULL, NULL, TO_CHAR);
				}
				pop_call();
				return FALSE;
			}
		}
	}

	obj_to_char(obj, ch);

	if (fDisplay)
	{
		if (container)
		{
			act( "You get $p from $P.", ch, obj, container, TO_CHAR );
			act( "$n gets $p from $P.", ch, obj, container, TO_ROOM );
		}
		else
		{
			act( "You get $p.", ch, obj, container, TO_CHAR );
			act( "$n gets $p.", ch, obj, container, TO_ROOM );
		}
	}

	if (!IS_NPC(ch) && IS_SET(obj->pIndexData->oprogtypes, TRIG_GET))
	{
		OBJ_PROG *prg;

		for (prg = obj->pIndexData->first_oprog ; prg ; prg = prg->next)
		{
			if (prg->trigger == TRIG_GET && number_percent() <= prg->percentage)
			{
				start_object_program( ch, obj, prg, "");
			}
		}
	}

	oprog_get_trigger(ch, obj);

	if (obj && obj->item_type == ITEM_MONEY)
	{
		give_gold(ch, obj->value[0]);
		junk_obj(obj);
	}
	pop_call();
	return TRUE;
}

bool put_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container, bool fDisplay )
{
	push_call("put_obj(%p,%p,%p,%p)",ch,obj,container,fDisplay);

	if (obj == container)
	{
		if (fDisplay)
		{
			send_to_char( "You cannot fold it into itself.\n\r", ch );
		}
		pop_call();
		return FALSE;
	}

	if (!can_drop_obj(ch, obj))
	{
		if (fDisplay)
		{
			send_to_char( "You cannot let go of it.\n\r", ch );
		}
		pop_call();
		return FALSE;
	}

	if (get_obj_weight(obj) + container->content_weight > container->value[0])
	{
		if (fDisplay)
		{
			send_to_char( "It won't fit.\n\r", ch);
		}
		pop_call();
		return FALSE;
	}

	if (count_total_objects(container) >= (container->value[3] ? container->value[3] : MAX_OBJECTS_IN_CONTAINER))
	{
		if (fDisplay)
		{
			send_to_char( "There are too many items there already.\n\r", ch);
		}
		pop_call();
		return FALSE;
	}

	if (obj->item_type == ITEM_CONTAINER)
	{
		if (fDisplay)
		{
			send_to_char( "You cannot put a container in a container.\n\r", ch);
		}
		pop_call();
		return FALSE;
	}

	if (!IS_NPC(ch) && obj_eq_level(ch, container) - 5 > ch->level && container->in_room == NULL)
	{
		if (fDisplay)
		{
			ch_printf(ch, "You must be level %d to use this container.\n\r", obj_eq_level(ch, container) - 5);
		}
		pop_call();
		return FALSE;
	}

	obj_to_obj( obj, container );

	if (fDisplay)
	{
		act( "You put $p in $P.", ch, obj, container, TO_CHAR );
		act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
	}
	pop_call();
	return TRUE;
}


bool drop_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container, bool fDisplay )
{
	push_call("drop_obj(%p,%p,%p,%p)",ch,obj,container,fDisplay);

	if (!can_drop_obj(ch, obj))
	{
		if (fDisplay)
		{
			send_to_char( "You cannot let go of it.\n\r", ch );
		}
		pop_call();
		return FALSE;
	}

	obj->sac_timer = OBJ_SAC_TIME;

	if (fDisplay)
	{
		act( "You drop $p.", ch, obj, NULL, TO_CHAR );
		act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
	}
	obj_to_room( obj, ch->in_room->vnum );

	if (!IS_NPC(ch) && IS_SET(obj->pIndexData->oprogtypes, TRIG_DROP))
	{
		OBJ_PROG *prg;

		for (prg = obj->pIndexData->first_oprog ; prg ; prg = prg->next)
		{
			if (prg->trigger == TRIG_DROP && number_percent() <= prg->percentage)
			{
				start_object_program( ch, obj, prg, "");
			}
		}
	}

	oprog_drop_trigger(ch, obj);

	pop_call();
	return TRUE;
}

bool sacrifice_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container, bool fDisplay )
{
	CHAR_DATA	*rch;

	push_call("sacrifice_obj(%p,%p,%p,%p)",ch,obj,container,fDisplay);

	if (!IS_SET(obj->wear_flags, ITEM_TAKE))
	{
		if (fDisplay)
		{
			act( "$p is not an acceptable sacrifice.", ch, obj, NULL, TO_CHAR);
		}
		pop_call();
		return FALSE;
	}

	if (obj->item_type == ITEM_CORPSE_PC)
	{
		if (IS_NPC(ch) || ch->pcdata->pvnum != obj->owned_by)
		{
			if (fDisplay)
			{
				act("$p is not yours to sacrifice.", ch, obj, NULL, TO_CHAR);
			}
			pop_call();
			return FALSE;
		}
	}

	if (obj->item_type == ITEM_FURNITURE)
	{
		for (rch = ch->in_room->first_person ; rch ; rch = rch->next_in_room)
		{
			if (rch->furniture == obj)
			{
				break;
			}
		}
		if (rch)
		{
			if (fDisplay)
			{
				act("$N seems to be using $p", ch, obj, rch, TO_CHAR);
			}
			pop_call();
			return FALSE;
		}
	}

	give_gold(ch, obj->cost / 100 + 1);

	ch_printf(ch, "%s give%s you %d gold coin%s for %s.\n\r", capitalize(get_god_name(which_god(ch))), which_god(ch) ? "s" : "", obj->cost/100+1, (obj->cost/100+1) != 1 ? "s" : "", obj->short_descr);
	act("$n sacrifices $p to $T.", ch, obj, get_god_name(which_god(ch)), TO_ROOM);

	oprog_sacrifice_trigger(ch, obj);

	if (!IS_NPC(ch) && IS_SET(obj->pIndexData->oprogtypes, TRIG_SACRIFICE))
	{
		OBJ_PROG *prg;

		for (prg = obj->pIndexData->first_oprog ; prg ; prg = prg->next)
		{
			if (prg->trigger == TRIG_SACRIFICE && number_percent() <= prg->percentage)
			{
				start_object_program( ch, obj, prg, "");
			}
		}
	}

	if (obj->first_content)
	{
		while (obj->first_content)
		{
			obj->first_content->sac_timer = OBJ_SAC_TIME;

			for (rch = ch->in_room->first_person ; rch ; rch = rch->next_in_room)
			{
				if (IS_NPC(rch) || !IS_SET(rch->act, PLR_QUIET))
				{
					act("  $p falls out onto the floor.", rch, obj->first_content, NULL, TO_CHAR);
				}
			}
			obj_to_room(obj->first_content, ch->in_room->vnum);
		}
	}

	if (!IS_NPC(ch) && obj == ch->pcdata->corpse)
	{
		ch->pcdata->corpse = find_char_corpse(ch, TRUE);
	}

	junk_obj(obj);

	pop_call();
	return TRUE;
}


void corpse_loot( CHAR_DATA *ch )
{
	push_call("corpse_loot(%p)",ch);

	while (ch->pcdata->corpse->first_content)
	{
		obj_to_char(ch->pcdata->corpse->first_content, ch);
	}

	act( "You get everything from $p.",ch, ch->pcdata->corpse, NULL, TO_CHAR );
	act( "$n gets everything from $p.",ch, ch->pcdata->corpse, NULL, TO_ROOM );

	junk_obj(ch->pcdata->corpse);

	ch->pcdata->corpse = find_char_corpse(ch, TRUE);

	save_char_obj(ch, NORMAL_SAVE);

	pop_call();
	return;
}


void do_get( CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf1[MAX_INPUT_LENGTH];
	char buf2[MAX_INPUT_LENGTH];

	OBJ_DATA *obj			= NULL;
	OBJ_DATA *obj_next		= NULL;
	OBJ_DATA *container		= NULL;
	OBJ_DATA *found_obj		= NULL;

	bool clutter			= FALSE;
	bool shrtd			= FALSE;

	int amount			=  1;
	int cnt				=  0;
	int cnt_not_got		=  0;
	int split				=  0;

	push_call("do_get(%p,%p)",ch,argument);

	if (!check_blind(ch))
	{
		pop_call();
		return;
	}

	/*
		special approach for player corpses.
	*/

	if (!IS_NPC(ch) && ch->pcdata->corpse && ch->in_room->vnum == ch->pcdata->corpse_room)
	{
		corpse_loot(ch);
		pop_call();
		return;
	}

	if (argument == NULL)
	{
		argument	= "all npc_corpse";
		split	= UMAX(1, ch->gold);
	}

	if (*argument == 0)
	{
		send_to_char("Get what?\n\r", ch);
		pop_call();
		return;
	}

	argument = one_argument( argument, arg1 );

	if (is_number(arg1))
	{
		amount	= UMAX(1, atol(arg1));
		argument	= one_argument( argument, arg1 );
	}

	argument = one_argument( argument, arg2 );

	if (arg2[0] == '\0')
	{
		if (!IS_NPC(ch) && ch->fighting != NULL)
		{
			send_to_char( "You are too busy to pick anything up.\n\r", ch );
			pop_call();
			return;
		}

		if (strcasecmp(arg1, "all") && str_prefix("all.", arg1))
		{
			if ((obj = get_obj_list(ch, arg1, ch->in_room->first_content)) == NULL)
			{
				act("You see no $T here.", ch, NULL, short_to_name(arg1, amount), TO_CHAR);
				pop_call();
				return;
			}

			if (amount == 1)
			{
				get_obj(ch, obj, NULL, TRUE);
				pop_call();
				return;
			}

			for ( ; shrtd <= 1 && !found_obj ; shrtd++)
			{
				for (obj = ch->in_room->first_content ; obj ; obj = obj_next)
				{
					obj_next = obj->next_content;

					if (!can_see_obj(ch, obj) || !IS_SET(obj->wear_flags, ITEM_TAKE))
					{
						continue;
					}

					if (!shrtd && !is_name(arg1, obj->name))
					{
						continue;
					}

					if (shrtd  && !is_name_short(arg1, obj->name))
					{
						continue;
					}

					if (found_obj == NULL)
					{
						found_obj = obj;
					}

					if (obj->short_descr != found_obj->short_descr)
					{
						continue;
					}

					if (get_obj(ch, obj, NULL, FALSE))
					{
						cnt++;
					}
					else
					{
						cnt_not_got++;
					}

					if (cnt >= amount)
					{
						break;
					}
				}
			}
			if (found_obj)
			{
				sprintf(buf1, "You get %d %s.", cnt, short_to_name(found_obj->short_descr, cnt));
				sprintf(buf2, "$n gets %d %s.", cnt, short_to_name(found_obj->short_descr, cnt));
				act(buf1, ch, found_obj, NULL, TO_CHAR);
				act(buf2, ch, found_obj, NULL, TO_ROOM);
			}
			else
			{
				do_get(ch, arg1);
			}
		}
		else
		{
			/*
				'get all' or 'get all.object'
			*/

			for ( ; shrtd <= 1 && !found_obj ; shrtd++)
			{
				for (obj = ch->in_room->first_content ; obj ; obj = obj_next)
				{
					obj_next = obj->next_content;

					if (!can_see_obj(ch, obj) || !IS_SET(obj->wear_flags, ITEM_TAKE))
					{
						continue;
					}

					if (!shrtd && arg1[3] && !is_name(&arg1[4], obj->name))
					{
						continue;
					}

					if (shrtd  && arg1[3] && !is_name_short(&arg1[4], obj->name))
					{
						continue;
					}

					if (found_obj == NULL)
					{
						found_obj = obj;
					}

					if (obj->short_descr != found_obj->short_descr)
					{
						clutter = TRUE;
					}

					if (get_obj(ch, obj, NULL, FALSE))
					{
						cnt++;
					}
					else
					{
						cnt_not_got++;
					}
				}
			}
			if (found_obj == NULL)
			{
				if (arg1[3] == '\0')
				{
					send_to_char( "You see nothing here.\n\r", ch );
				}
				else
				{
					act( "You see no $T here.", ch, NULL, short_to_name(&arg1[4], 0), TO_CHAR );
				}
			}
			else
			{
				if (cnt_not_got > 0)
				{
					if (arg1[3] == '\0' || clutter)
					{
						ch_printf(ch, "You get all but %d of the items.\n\r", cnt_not_got);
						act("$n gets some items.", ch, NULL, NULL, TO_ROOM);
					}
					else
					{
						sprintf(buf1, "You get %d %s.", cnt, short_to_name(found_obj->short_descr, cnt));
						sprintf(buf2, "$n gets %d %s.", cnt, short_to_name(found_obj->short_descr, cnt));
						act(buf1, ch, found_obj, NULL, TO_CHAR);
						act(buf2, ch, found_obj, NULL, TO_ROOM);
					}
				}
				else
				{
					if (clutter)
					{
						if (arg1[3] == '\0')
						{
							act( "You get everything in the room.", ch, NULL, NULL, TO_CHAR);
							act( "$n gets everything in the room.", ch, NULL, NULL, TO_ROOM);
						}
						else
						{
							act("You get all the $T in the room.", ch, NULL, short_to_name(&arg1[4], cnt), TO_CHAR);
							act("$n gets all the $T in the room.", ch, NULL, short_to_name(&arg1[4], cnt), TO_ROOM);
						}
					}
					else
					{
						sprintf(buf1, "You get every %s in the room.", short_to_name(found_obj->short_descr, 1));
						sprintf(buf2, "$n gets every %s in the room.", short_to_name(found_obj->short_descr, 1));
						act(buf1, ch, found_obj, NULL, TO_CHAR);
						act(buf2, ch, found_obj, NULL, TO_ROOM);
					}
				}
			}
		}
		pop_call();
		return;
	}
	else
	{
		/*
			"get <all|all.object|item> container"
		*/

		if ((container = get_obj_here(ch, arg2)) == NULL)
		{
			act( "You see no $T here.", ch, NULL, arg2, TO_CHAR );
			pop_call();
			return;
		}

		if (container->carried_by == NULL && !IS_NPC(ch) && ch->fighting != NULL)
		{
			send_to_char( "You are too busy to pick anything up.\n\r", ch );
			pop_call();
			return;
		}

		switch (container->item_type)
		{
			case ITEM_CONTAINER:
				break;
			case ITEM_CORPSE_NPC:
				if (container->owned_by && !pvnum_in_group(ch, container->owned_by))
				{
					send_to_char( "You have no right to those items!\n\r", ch);
					pop_call();
					return;
				}
				break;
			case ITEM_CORPSE_PC:
				if (IS_NPC(ch) || container->owned_by != ch->pcdata->pvnum)
				{
					send_to_char( "You have no right to those items.\n\r", ch );
					pop_call();
					return;
				}
				break;
			default:
				send_to_char( "That's not a container.\n\r", ch );
				pop_call();
				return;
		}

		if (IS_SET(container->value[1], CONT_CLOSED))
		{
			act( "$p is closed.", ch, container, NULL, TO_CHAR );
			pop_call();
			return;
		}

		if (strcasecmp(arg1, "all") && str_prefix("all.", arg1))
		{
			/*
				"get object container"
			*/

			if ((obj = get_obj_list(ch, arg1, container->first_content)) == NULL)
			{
				act("You see nothing like that in $p.", ch, container, NULL, TO_CHAR);
				pop_call();
				return;
			}

			if (amount == 1)
			{
				get_obj(ch, obj, container, TRUE);
				pop_call();
				return;
			}

			for ( ; shrtd <= 1 && !found_obj ; shrtd++)
			{
				for (obj = container->first_content ; obj ; obj = obj_next)
				{
					obj_next = obj->next_content;

					if (!can_see_obj(ch, obj) || !IS_SET(obj->wear_flags, ITEM_TAKE))
					{
						continue;
					}

					if (!shrtd && !is_name(arg1, obj->name))
					{
						continue;
					}

					if (shrtd  && !is_name_short(arg1, obj->name))
					{
						continue;
					}

					if (found_obj == NULL)
					{
						found_obj = obj;
					}

					if (obj->short_descr != found_obj->short_descr)
					{
						continue;
					}

					if (get_obj(ch, obj, container, FALSE))
					{
						cnt++;
					}
					else
					{
						cnt_not_got++;
					}

					if (cnt >= amount)
					{
						break;
					}
				}
			}
			if (found_obj)
			{
				sprintf(buf1, "You get %d %s from $p.", cnt, short_to_name(found_obj->short_descr, cnt));
				sprintf(buf2, "$n gets %d %s from $p", cnt, short_to_name(found_obj->short_descr, cnt));
				act(buf1, ch, container, NULL, TO_CHAR);
				act(buf2, ch, container, NULL, TO_ROOM);
			}
			else
			{
				cat_sprintf(arg1, " %s", arg2);

				do_get(ch, arg1);
			}
		}
		else
		{
			/*
				"get all container" or "get all.object container"
			*/

			for ( ; shrtd <= 1 && !found_obj ; shrtd++)
			{
				for (obj = container->first_content ; obj ; obj = obj_next)
				{
					obj_next = obj->next_content;

					if (!can_see_obj(ch, obj) || !IS_SET(obj->wear_flags, ITEM_TAKE))
					{
						continue;
					}

					if (!shrtd && arg1[3] && !is_name(&arg1[4], obj->name))
					{
						continue;
					}

					if (shrtd  && arg1[3] && !is_name_short(&arg1[4], obj->name))
					{
						continue;
					}

					if (split)
					{
						if (!IS_SET(ch->act, PLR_AUTOLOOT) && obj->item_type != ITEM_MONEY)
						{
							continue;
						}
					}

					if (found_obj == NULL)
					{
						found_obj = obj;
					}

					if (obj->short_descr != found_obj->short_descr)
					{
						clutter = TRUE;
					}

					if (get_obj(ch, obj, container, FALSE))
					{
						cnt++;
					}
					else
					{
						cnt_not_got++;
					}
				}
			}

			if (found_obj == NULL)
			{
				if (!split)
				{
					if (arg1[3] == '\0')
					{
						act( "You see nothing in $p.", ch, container, NULL, TO_CHAR );
					}
					else
					{
						act("You see nothing like that in $p.", ch, container, NULL, TO_CHAR );
					}
				}
			}
			else
			{
				if (cnt_not_got)
				{
					if (arg1[3] == '\0' || clutter)
					{
						ch_printf(ch, "You get all but %d of the items from %s.\n\r", cnt_not_got, container->short_descr);
						act("$n gets some items from $p.",ch,container,NULL,TO_ROOM);
					}
					else
					{
						sprintf(buf1, "You get %d %s from $p.", cnt, short_to_name(found_obj->short_descr, cnt));
						sprintf(buf2, "$n gets %d %s from $p.", cnt, short_to_name(found_obj->short_descr, cnt));
						act(buf1, ch, container, NULL, TO_CHAR);
						act(buf2, ch, container, NULL, TO_ROOM);
					}
				}
				else if (split && cnt == 1)
				{
					sprintf(buf1, "You get %s from $p.", found_obj->short_descr);
					sprintf(buf2, "$n gets %s from $p.", found_obj->short_descr);
					act(buf1, ch, container, NULL, TO_CHAR);
					act(buf2, ch, container, NULL, TO_ROOM);
				}
				else if (clutter)
				{
					if (arg1[3] == '\0')
					{
						act( "You get everything from $p.",ch, container, NULL, TO_CHAR );
						act( "$n gets everything from $p.",ch, container, NULL, TO_ROOM );
					}
					else
					{
						act( "You get all $T from $p.",ch, container, short_to_name(&arg1[4], 0), TO_CHAR );
						act( "$n gets all $T from $p.",ch, container, short_to_name(&arg1[4], 0), TO_ROOM );
					}
				}
				else
				{
					sprintf(buf1, "You get every %s from $p.", short_to_name(found_obj->short_descr, 1));
					sprintf(buf2, "$n gets every %s from $p.", short_to_name(found_obj->short_descr, 1));
					act(buf1, ch, container, NULL, TO_CHAR);
					act(buf2, ch, container, NULL, TO_ROOM);
				}

				if (IS_SET(ch->act, PLR_AUTO_SPLIT) && (ch->gold - split) >= 10 && split)
				{
					CHAR_DATA *gch;

					for (gch = ch->in_room->first_person ; gch ; gch = gch->next_in_room)
					{
						if (gch != ch && is_same_group(gch, ch) && !IS_NPC(gch))
						{
							sprintf(buf1, "%d", ch->gold - split);
							do_split(ch, buf1);
							break;
						}
					}
				}
			}
		}
	}
	pop_call();
	return;
}


void do_put( CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf1[MAX_INPUT_LENGTH];
	char buf2[MAX_INPUT_LENGTH];

	OBJ_DATA *container		= NULL;
	OBJ_DATA *found_obj		= NULL;
	OBJ_DATA *obj			= NULL;
	OBJ_DATA *obj_next		= NULL;

	bool shrtd			= FALSE;
	bool clutter			= FALSE;

	int amount			= 1;
	int cnt				= 0;
	int cnt_not_put		= 0;

	push_call("do_put(%p,%p)",ch,argument);

	argument = one_argument( argument, arg1 );

	if (is_number(arg1))
	{
		amount	= UMAX(1, atol(arg1));
		argument	= one_argument( argument, arg1 );
	}

	argument = one_argument( argument, arg2 );

	if (*arg1 == 0 || *arg2 == 0)
	{
		send_to_char( "Put what in what?\n\r", ch );
		pop_call();
		return;
	}

	if ((container = get_obj_here(ch, arg2)) == NULL)
	{
		act( "You see no $T here.", ch, NULL, arg2, TO_CHAR );
		pop_call();
		return;
	}

	if (container->item_type != ITEM_CONTAINER)
	{
		act("$p is not a container.", ch, container, NULL, TO_CHAR );
		pop_call();
		return;
	}

	if (IS_SET(container->value[1], CONT_CLOSED))
	{
		act( "$p is closed.", ch, container, NULL, TO_CHAR );
		pop_call();
		return;
	}

	if (strcasecmp(arg1, "all") && str_prefix("all.", arg1))
	{
		/*
			'put obj container'
		*/

		if ((obj = get_obj_carry(ch, arg1)) == NULL)
		{
			send_to_char( "You do not have that item.\n\r", ch );
			pop_call();
			return;
		}

		if (amount == 1)
		{
			put_obj(ch, obj, container, TRUE);
			pop_call();
			return;
		}

		for ( ; shrtd <= 1 && !found_obj ; shrtd++)
		{
			for (obj = ch->first_carrying ; obj ; obj = obj_next)
			{
				obj_next = obj->next_content;

				if (!can_see_obj(ch, obj) || obj->wear_loc != WEAR_NONE)
				{
					continue;
				}

				if (!shrtd && !is_name(arg1, obj->name))
				{
					continue;
				}

				if (shrtd  && !is_name_short(arg1, obj->name))
				{
					continue;
				}

				if (found_obj == NULL)
				{
					found_obj = obj;
				}

				if (obj->short_descr != found_obj->short_descr)
				{
					continue;
				}

				if (put_obj(ch, obj, container, FALSE))
				{
					cnt++;
				}
				else
				{
					cnt_not_put++;
				}

				if (cnt >= amount)
				{
					break;
				}
			}
		}
		if (found_obj)
		{
			sprintf(buf1, "You put %d %s into $p.", cnt, short_to_name(found_obj->short_descr, cnt));
			sprintf(buf2, "$n puts %d %s into $p.", cnt, short_to_name(found_obj->short_descr, cnt));
			act(buf1, ch, container, NULL, TO_CHAR);
			act(buf2, ch, container, NULL, TO_ROOM);
		}
		else
		{
			do_put(ch, arg1);
		}
		pop_call();
		return;
	}
	else
	{
		/*
			'put all container' or 'put all.obj container'
		*/

		for ( ; shrtd <= 1 && !found_obj ; shrtd++)
		{
			for (obj = ch->first_carrying ; obj ; obj = obj_next)
			{
				obj_next = obj->next_content;

				if (!can_see_obj(ch, obj) || obj->wear_loc != WEAR_NONE || obj == container)
				{
					continue;
				}

				if (!shrtd && arg1[3] && !is_name(&arg1[4], obj->name))
				{
					continue;
				}

				if (shrtd  && arg1[3] && !is_name_short(&arg1[4], obj->name))
				{
					continue;
				}

				if (found_obj == NULL)
				{
					found_obj = obj;
				}

				if (obj->short_descr != found_obj->short_descr)
				{
					clutter = TRUE;
				}

				if (put_obj(ch, obj, container, FALSE))
				{
					cnt++;
				}
				else
				{
					cnt_not_put++;
				}
			}
		}

		if (found_obj)
		{
			if (cnt_not_put > 0)
			{
				if (arg1[3] == '\0' || clutter)
				{
					sprintf(buf1, "You put all but %d of the items into $P.", cnt_not_put);
					sprintf(buf2, "$n puts some items into $P.");
				}
				else
				{
					sprintf(buf1, "You put %d %s into $P.", cnt, short_to_name(found_obj->short_descr, cnt));
					sprintf(buf2, "$n puts %d %s into $P.", cnt, short_to_name(found_obj->short_descr, cnt));
				}
			}
			else if (clutter)
			{
				if (arg1[3] == 0)
				{
					sprintf(buf1, "You put everything into $P.");
					sprintf(buf2, "$n puts everything into $P.");
				}
				else
				{
					sprintf(buf1, "You put all %s into $P.", short_to_name(&arg1[4], 0));
					sprintf(buf2, "$n puts all %s into $P.", short_to_name(&arg1[4], 0));
				}
			}
			else
			{
				sprintf(buf1, "You put all your %s into $P.", short_to_name(found_obj->short_descr, 0));
				sprintf(buf2, "$n puts all $s %s into $P.", short_to_name(found_obj->short_descr, 0));
			}
			act(buf1, ch, found_obj, container, TO_CHAR);
			act(buf2, ch, found_obj, container, TO_ROOM);
		}
		else
		{
			act( "You don't have $T.", ch, obj, arg1, TO_CHAR );
		}
	}
	pop_call();
	return;
}

void do_drop( CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];

	char buf1[MAX_INPUT_LENGTH];
	char buf2[MAX_INPUT_LENGTH];

	OBJ_DATA *obj			= NULL;
	OBJ_DATA *found_obj		= NULL;
	OBJ_DATA *obj_next		= NULL;

	bool shrtd			= FALSE;
	bool clutter			= FALSE;

	int amount			= 1;
	int cnt				= 0;
	int cnt_not_dropped		= 0;

	push_call("do_drop(%p,%p)",ch,argument);

	argument = one_argument( argument, arg1 );

	if (is_number(arg1))
	{
		amount	= UMAX(1, atol(arg1));
		argument	= one_argument( argument, arg1 );
	}

	if (*arg1 == 0)
	{
		send_to_char( "Drop what?\n\r", ch );
		pop_call();
		return;
	}

	/*
		'drop object'
	*/

	if (strcasecmp(arg1, "all") && str_prefix("all.", arg1))
	{
		if ((obj = get_obj_carry(ch, arg1)) == NULL)
		{
			send_to_char( "You do not have that item.\n\r", ch );
			pop_call();
			return;
		}

		if (amount == 1)
		{
			drop_obj(ch, obj, NULL, TRUE);
			pop_call();
			return;
		}

		for ( ; shrtd <= 1 && !found_obj ; shrtd++)
		{
			for (obj = ch->first_carrying ; obj ; obj = obj_next)
			{
				obj_next = obj->next_content;

				if (!can_see_obj(ch, obj) || obj->wear_loc != WEAR_NONE)
				{
					continue;
				}

				if (!shrtd && !is_name(arg1, obj->name))
				{
					continue;
				}

				if (shrtd  && !is_name_short(arg1, obj->name))
				{
					continue;
				}

				if (found_obj == NULL)
				{
					found_obj = obj;
				}

				if (obj->short_descr != found_obj->short_descr)
				{
					continue;
				}

				if (drop_obj(ch, obj, NULL, FALSE))
				{
					cnt++;
				}
				else
				{
					cnt_not_dropped++;
				}

				if (cnt >= amount)
				{
					break;
				}
			}
		}
		if (found_obj)
		{
			sprintf(buf1, "You drop %d %s.", cnt, short_to_name(found_obj->short_descr, cnt));
			sprintf(buf2, "$n drops %d %s.", cnt, short_to_name(found_obj->short_descr, cnt));
			act(buf1, ch, found_obj, NULL, TO_CHAR);
			act(buf2, ch, found_obj, NULL, TO_ROOM);
		}
		else
		{
			do_drop(ch, arg1);
		}
		pop_call();
		return;
	}
	else
	{
		/*
			'drop all' or 'drop all.object'
		*/

		for ( ; shrtd <= 1 && !found_obj ; shrtd++)
		{
			for (obj = ch->first_carrying ; obj ; obj = obj_next)
			{
				obj_next = obj->next_content;

				if (!can_see_obj(ch, obj) || obj->wear_loc != WEAR_NONE)
				{
					continue;
				}

				if (!shrtd && arg1[3] && !is_name(&arg1[4], obj->name))
				{
					continue;
				}

				if (shrtd  && arg1[3] && !is_name_short(&arg1[4], obj->name))
				{
					continue;
				}

				if (found_obj == NULL)
				{
					found_obj = obj;
				}

				if (obj->short_descr != found_obj->short_descr)
				{
					clutter = TRUE;
				}

				if (drop_obj(ch, obj, NULL, FALSE))
				{
					cnt++;
				}
				else
				{
					cnt_not_dropped++;
				}
			}
		}

		if (found_obj)
		{
			if (cnt_not_dropped > 0)
			{
				if (arg1[3] == '\0' || clutter)
				{
					sprintf(buf1, "You drop all but %d of the items.", cnt_not_dropped);
					sprintf(buf2, "$n drops some items.");
				}
				else
				{
					sprintf(buf1, "You drop %d %s.", cnt, short_to_name(found_obj->short_descr, cnt));
					sprintf(buf2, "$n drops %d %s..", cnt, short_to_name(found_obj->short_descr, cnt));
				}
			}
			else if (clutter)
			{
				if (arg1[3] == 0)
				{
					sprintf(buf1, "You drop everything.");
					sprintf(buf2, "$n drops everything.");
				}
				else
				{
					sprintf(buf1, "You drop all your %s.", short_to_name(&arg1[4], 0));
					sprintf(buf2, "$n drops all $s %s.",   short_to_name(&arg1[4], 0));
				}
			}
			else
			{
				sprintf(buf1, "You drop all your %s.", short_to_name(found_obj->short_descr, 0));
				sprintf(buf2, "$n drops all $s %s.",   short_to_name(found_obj->short_descr, 0));
			}
			act(buf1, ch, found_obj, NULL, TO_CHAR);
			act(buf2, ch, found_obj, NULL, TO_ROOM);
		}
		else
		{
			if (!found_obj)
			{
				if (arg1[3] == '\0' )
				{
					act( "You are not carrying anything.", ch, NULL, NULL, TO_CHAR );
				}
				else
				{
					act( "You are not carrying any $T.", ch, NULL, short_to_name(&arg1[4], 0), TO_CHAR );
				}
			}
		}
	}
	pop_call();
	return;
}

void do_give( CHAR_DATA *ch, char *argument )
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA  *obj;

	push_call("do_give(%p,%p)",ch,argument);

	if (!IS_NPC(ch) && who_fighting(ch) && !IS_NPC(ch->fighting->who))
	{
		send_to_char( "You are too busy to give anything away.\n\r", ch );
		pop_call();
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( arg1[0] == '\0' || arg2[0] == '\0' )
	{
		send_to_char( "Give what to whom?\n\r", ch );
		pop_call();
		return;
	}

	if ( is_number( arg1 ) )
	{
		/* 'give NNNN coins victim' */

		int amount = atol(arg1);

		if (amount <= 0 || (strcasecmp(arg2, "coins") && strcasecmp(arg2, "coin")))
		{
			send_to_char( "Sorry, you cannot do that.\n\r", ch );
			pop_call();
			return;
		}

		argument = one_argument( argument, arg2 );
		if ( arg2[0] == '\0' )
		{
			send_to_char( "Give what to whom?\n\r", ch );
			pop_call();
			return;
		}

		if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
		{
			send_to_char( "They aren't here.\n\r", ch );
			pop_call();
			return;
		}

		if (!IS_NPC(victim) && who_fighting(victim) && !IS_NPC(victim->fighting->who))
		{
			send_to_char( "They are too busy to take anything right now.\n\r", ch );
			pop_call();
			return;
		}

		if ( ch->gold < amount )
		{
			send_to_char( "You haven't got that much gold.\n\r", ch );
			pop_call();
			return;
		}

		if (amount + victim->gold > victim->level * 1000000)
		{
			amount = victim->level * 1000000 - victim->gold;
		}

		ch->gold     -= amount;

		if (!IS_NPC(victim))
		{
			victim->gold += amount;
		}

		ch_printf(ch, "You give %s %d gold coins.\n\r", get_name(victim), amount);
		ch_printf(victim, "%s gives you %d gold coins.\n\r", capitalize(get_name(ch)), amount);
		act( "$n gives $N some gold.",  ch, NULL, victim, TO_NOTVICT );

		mprog_bribe_trigger(victim,ch,amount);

		pop_call();
		return;
	}

	if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
	{
		send_to_char( "You do not have that item.\n\r", ch );

		pop_call();
		return;
	}

	if ( obj->wear_loc != WEAR_NONE )
	{
		send_to_char( "You must remove it first.\n\r", ch );
		pop_call();
		return;
	}

	if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
	{
		send_to_char( "They aren't here.\n\r", ch );
		pop_call();
		return;
	}

	if (!can_drop_obj(ch, obj) && !IS_IMMORTAL(ch))
	{
		send_to_char( "You cannot let go of it.\n\r", ch );
		pop_call();
		return;
	}

	if (!IS_IMMORTAL(ch))
	{
		if (victim->carry_number >= can_carry_n(victim))
		{
			act( "$N cannot carry that many items.", ch, NULL, victim, TO_CHAR );
			pop_call();
			return;
		}
	}

	if (!IS_IMMORTAL(ch))
	{
		if (victim->carry_weight + get_obj_weight( obj ) > can_carry_w( victim ) )
		{
			act( "$N cannot carry that much weight.", ch, NULL, victim, TO_CHAR );
			pop_call();
			return;
		}
	}
	if (!IS_IMMORTAL(ch))
	{
		if ( !can_see_obj( victim, obj ) )
		{
			act( "$N cannot see it.", ch, NULL, victim, TO_CHAR );
			pop_call();
			return;
		}
	}

	act( "You give $p to $N.", ch, obj, victim, TO_CHAR    );
	act( "$n gives you $p.",   ch, obj, victim, TO_VICT    );
	act( "$n gives $p to $N.", ch, obj, victim, TO_NOTVICT );

	if (obj->item_type == ITEM_MONEY)
	{
		give_gold(victim, obj->value[0]);
		junk_obj(obj);
	}
	else
	{
		obj_to_char(obj, victim);

		if (!IS_OBJ_STAT(obj, ITEM_FORGERY))
		{
			mprog_give_trigger(victim,ch,obj);
		}
	}
	pop_call();
	return;
}


void do_fill( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	OBJ_DATA *fountain;
	bool found;

	push_call("do_fill(%p,%p)",ch,argument);

	one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		send_to_char( "Fill what?\n\r", ch );
		pop_call();
		return;
	}

	if ((obj = get_obj_carry(ch, arg)) == NULL)
	{
		send_to_char( "You do not have that item.\n\r", ch );
		pop_call();
		return;
	}

	found = FALSE;

	for (fountain = ch->in_room->first_content ; fountain ; fountain = fountain->next_content)
	{
		if (fountain->item_type == ITEM_FOUNTAIN)
		{
			found = TRUE;
			break;
		}
	}

	if (!found)
	{
		send_to_char( "There is no fountain here!\n\r", ch );
		pop_call();
		return;
	}

	if (obj->item_type != ITEM_DRINK_CON)
	{
		send_to_char( "You cannot fill that.\n\r", ch );
		pop_call();
		return;
	}

	if (obj->value[1] != 0 && obj->value[2] != fountain->value[2])
	{
		send_to_char( "There is already another liquid in it.\n\r", ch );
		pop_call();
		return;
	}

	if (obj->value[1] >= obj->value[0])
	{
		send_to_char( "Your container is full.\n\r", ch );
		pop_call();
		return;
	}

	act( "You fill $p.", ch, obj, NULL, TO_CHAR );
	obj->value[2] = fountain->value[2];
	obj->value[1] = obj->value[0];
	SET_BIT(obj->extra_flags, ITEM_MODIFIED);
	pop_call();
	return;
}

void do_drink( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int amount;
	int liquid;
	int cnt;

	push_call("do_drink(%p,%p)",ch,argument);

	if (!check_blind(ch))
	{
		pop_call();
		return;
	}

	one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		for (obj = ch->in_room->first_content ; obj ; obj = obj->next_content)
		{
			if (obj->item_type == ITEM_FOUNTAIN)
			{
				break;
			}
		}

		if (rspec_req(ch,RSPEC_VAMPIRIC))
		{
			for (cnt = 0 ; cnt < MAX_LAST_LEFT ; cnt++)
			{
				if (IS_SET(ch->in_room->last_left_bits[cnt], TRACK_BLOOD))
				{
					/* vampires drink blood-trails too */
					send_to_char( "You lick blood from the ground.\n\r", ch );
					act( "$n licks blood from the ground.", ch, NULL, NULL, TO_ROOM );
					gain_condition( ch, COND_FULL,   2);
					gain_condition( ch, COND_THIRST, 2);

					REMOVE_BIT(ch->in_room->last_left_bits[cnt], TRACK_BLOOD);
					pop_call();
					return;
				}
			}
		}

		if (obj == NULL)
		{
			send_to_char( "Drink what?\n\r", ch );
			pop_call();
			return;
		}
	}
	else
	{
		if ((obj = get_obj_here(ch, arg)) == NULL)
		{
			send_to_char( "You cannot find it.\n\r", ch );

			pop_call();
			return;
		}
	}

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
	{
		send_to_char( "You fail to reach your mouth.  *Hic*\n\r", ch );
		pop_call();
		return;
	}

	switch (obj->item_type)
	{
		default:
			send_to_char( "You cannot drink from that.\n\r", ch );
			pop_call();
			return;
		case ITEM_DRINK_CON:
		case ITEM_FOUNTAIN:
			if (obj->item_type == ITEM_DRINK_CON && obj->value[1] <= 0)
			{
				send_to_char( "It is already empty.\n\r", ch );
				pop_call();
				return;
			}

			if ((liquid = obj->value[2]) >= LIQ_MAX)
			{
				log_printf( "do_drink(%s, i%d) bad liquid number %d.", get_name(ch), obj->pIndexData->vnum, liquid);
				liquid = obj->value[2] = 0;
			}

			act( "You drink $T from $p.", ch, obj, liq_table[liquid].liq_name, TO_CHAR );
			act( "$n drinks $T from $p.", ch, obj, liq_table[liquid].liq_name, TO_ROOM );

			if (obj->item_type == ITEM_DRINK_CON)
			{
				amount = UMIN(obj->value[1], number_range(1, 2));
			}
			else
			{
				amount = number_range(1, 2);
			}

			if (!rspec_req(ch,RSPEC_VAMPIRIC))
			{
				if (!IS_NPC(ch))
				{
					if (ch->pcdata->condition[COND_DRUNK] <= 10)
					{
						gain_condition( ch, COND_DRUNK,  amount * liq_table[liquid].liq_affect[COND_DRUNK] );
						if (ch->pcdata->condition[COND_DRUNK] > 10)
						{
							send_to_char( "You feel drunk.\n\r", ch );
						}
					}
					if (ch->pcdata->condition[COND_FULL] <= 70)
					{
						gain_condition( ch, COND_FULL,   amount * liq_table[liquid].liq_affect[COND_FULL] );
						if (ch->pcdata->condition[COND_FULL] > 70)
						{
							send_to_char( "You are full.\n\r", ch );
						}
					}
					if (ch->pcdata->condition[COND_THIRST] <= 70)
					{
						gain_condition( ch, COND_THIRST, amount * liq_table[liquid].liq_affect[COND_THIRST] );
						if (ch->pcdata->condition[COND_THIRST] > 70)
						{
							send_to_char( "You do not feel thirsty.\n\r", ch );
						}
					}
				}
				if (obj->value[3] != 0)
				{
					/* The shit was poisoned ! */

					AFFECT_DATA af;

					send_to_char( "You choke and gag.\n\r", ch );
					act( "$n chokes and gags.", ch, NULL, NULL, TO_ROOM );
					af.type      = gsn_poison;
					af.duration  = 3 * amount;
					af.location  = APPLY_NONE;
					af.modifier  = 0;
					af.bittype   = AFFECT_TO_CHAR;
					af.bitvector = AFF_POISON;
					affect_join( ch, &af );
				}
			}
			else
			{
				if (liquid == LIQ_BLOOD)
				{
					gain_condition(ch, COND_FULL,   5);
					gain_condition(ch, COND_THIRST, 5);
				}
				else if (liq_table[liquid].liq_affect[COND_DRUNK] > 0)
				{
					AFFECT_DATA af;

					act ("$n starts to smoke.", ch, NULL, NULL, TO_ROOM);
					act ("You start to smoke.", ch, NULL, NULL, TO_CHAR);

					af.type 		= gsn_poison;
					af.duration	= 2 * amount;
					af.location	= APPLY_NONE;
					af.modifier	= 0;
					af.bittype	= AFFECT_TO_CHAR;
					af.bitvector	= AFF_POISON;
					affect_join( ch, &af);
				}
			}
			if (obj->item_type == ITEM_DRINK_CON)
			{
				SET_BIT(obj->extra_flags, ITEM_MODIFIED);
				obj->value[1] -= amount;
				if (obj->value[1] <= 0 && obj->value[0] <= 2)
				{
					send_to_char( "The empty container vanishes.\n\r", ch );
					junk_obj( obj );
				}
			}
			break;
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
			if (rspec_req(ch, RSPEC_VAMPIRIC))
			{
				if (obj->level > 1)
				{
					int wanted = 24;

					if (!IS_NPC(ch))
					{
						wanted = 48 - ch->pcdata->condition[COND_THIRST];
					}

					if ((wanted = UMIN(wanted, obj->level)) == 0)
					{
						ch_printf(ch, "You are too full to drink from %s.\n\r", obj->short_descr);
					}
					else
					{
						gain_condition(ch, COND_FULL,   wanted);
						gain_condition(ch, COND_THIRST, wanted);

						if (ch->hit < ch->max_hit)
						{
							int heal = UMIN(wanted, ch->max_hit-ch->hit);
							ch_printf(ch,"You drink blood from %s, restoring %d hp.\n\r", obj->short_descr, heal);
							ch->hit += heal;
						}
						else
						{
							ch_printf(ch,"You drink blood from %s.\n\r",obj->short_descr);
						}
						obj->level -= wanted;
						act( "$n drinks blood from the $p.", ch, obj, NULL, TO_ROOM );
					}
					/*
						the corpse remains, it's only drained!
					*/
				}
				else
				{
					ch_printf(ch,"There's no blood left in %s.\n\r", obj->short_descr);
				}
				break;
			}
			else
			{
				send_to_char("You cannot drink from that.\n\r",ch);
				break;
			}
	}
	pop_call();
	return;
}

/* Forage for Gnomes - Chaos 8/17/98 */
/* or other foraging races */

void do_forage( CHAR_DATA *ch, char *argument )
{

	push_call("do_forage(%p,%p)",ch,argument);

	if (IS_NPC(ch) || !rspec_req(ch, RSPEC_FORAGE))
	{
		send_to_char( "You have no idea how to find food in the wilderness.\n\r", ch );
		pop_call();
		return;
	}

	if (ch->in_room == NULL
	|| (ch->in_room->sector_type != SECT_FIELD
	&&  ch->in_room->sector_type != SECT_FOREST
	&&  ch->in_room->sector_type != SECT_HILLS
	&&  ch->in_room->sector_type != SECT_MOUNTAIN ))
	{
		send_to_char( "There is no food to be found here.\n\r", ch );
		pop_call();
		return;
	}

	send_to_char( "You find enough food and water to satisfy yourself for the day.\n\r", ch );

	gain_condition(ch, COND_FULL, 24);
	gain_condition(ch, COND_THIRST, 24);

	pop_call();
	return;
}

void do_eat( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int level;

	push_call("do_eat(%p,%p)",ch,argument);

	one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		send_to_char( "Eat what?\n\r", ch );
		pop_call();
		return;
	}

	if ((obj = get_obj_carry(ch, arg)) == NULL)
	{
		send_to_char( "You do not have that item.\n\r", ch );
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && ch->fighting && ch->class != CLASS_NINJA)
	{
		send_to_char( "You may not do that while fighting.\n\r", ch);
		pop_call();
		return;
	}

	if (obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL)
	{
		send_to_char( "That's not edible.\n\r", ch );
		pop_call();
		return;
	}

	if (obj->item_type == ITEM_PILL)
	{
		level = obj->value[0];

		if (level > reinc_eq_level(ch))
		{
			level = reinc_eq_level(ch);
		}

		if (!HAS_BIT(class_table[ch->class].spec, CFLAG_EAT))
		{
			level /= 2;
		}
	}

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_FULL] > 40 && obj->item_type != ITEM_PILL)
	{
		send_to_char( "You are too full to eat more.\n\r", ch );
		pop_call();
		return;
	}

	if (ch->fighting != NULL && number_percent() > get_curr_dex(ch) * 2)
	{
		act( "You crumble $p.", ch, obj, NULL, TO_CHAR );
		act( "$n crumbles $p.", ch, obj, NULL, TO_ROOM );
		wait_state( ch, PULSE_VIOLENCE);
		junk_obj( obj );
		pop_call();
		return;
	}
	act( "You eat $p.", ch, obj, NULL, TO_CHAR );
	act( "$n eats $p.",  ch, obj, NULL, TO_ROOM );

	switch ( obj->item_type )
	{
		case ITEM_FOOD:
			if (!IS_NPC(ch) && !IS_UNDEAD(ch) && !rspec_req(ch, RSPEC_VAMPIRIC))
			{
				int condition;

				condition = ch->pcdata->condition[COND_FULL];
				gain_condition( ch, COND_FULL, obj->value[0] );
				if ( condition == 0 && ch->pcdata->condition[COND_FULL] > 0 )
				{
					send_to_char( "You are no longer hungry.\n\r", ch );
				}
				else if ( ch->pcdata->condition[COND_FULL] > 40 )
				{
					send_to_char( "You are full.\n\r", ch );
				}
			}

			switch(obj->value[1])
			{
				case FOOD_GARLIC:
					if (rspec_req(ch,RSPEC_VAMPIRIC))
					{
						AFFECT_DATA af;
						act ("$n starts to smoke and sizzle.",ch,0,0,TO_ROOM);
						send_to_char("You smoke and sizzle.\n\r",ch);

						af.type = gsn_poison;
						af.duration = 3 * obj->value[0];
						af.location = APPLY_NONE;
						af.modifier = 0;
						af.bittype   = AFFECT_TO_CHAR;
						af.bitvector = AFF_POISON;
						affect_join(ch,&af);
					}
					break;
			}

			if (obj->value[3] != 0 && !rspec_req(ch, RSPEC_VAMPIRIC))
			{
				/* The shit was poisoned! */

				AFFECT_DATA af;

				send_to_char( "You choke and gag.\n\r", ch );
				act( "$n chokes and gags.", ch, 0, 0, TO_ROOM );

				af.type      = gsn_poison;
				af.duration  = 2 * obj->value[0];
				af.location  = APPLY_NONE;
				af.modifier  = 0;
				af.bittype   = AFFECT_TO_CHAR;
				af.bitvector = AFF_POISON;
				affect_join( ch, &af );
			}
			break;

		case ITEM_PILL:
			obj_cast_spell( obj->value[1], obj->value[0], ch, ch, obj );
			obj_cast_spell( obj->value[2], obj->value[0], ch, ch, obj );
			obj_cast_spell( obj->value[3], obj->value[0], ch, ch, obj );
			wait_state(ch, PULSE_VIOLENCE);
			break;
	}
	junk_obj( obj );
	pop_call();
	return;
}

/*
	Remove an object.
*/

bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace , bool fDisplay )
{
	OBJ_DATA *obj;

	push_call("remove_obj(%p,%p,%p,%p)",ch,iWear,fReplace,fDisplay);

	if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	{
		pop_call();
		return TRUE;
	}

	if (!fReplace && ch->carry_number >= can_carry_n(ch))
	{
		act( "You cannot carry that many items.", ch, NULL, NULL, TO_CHAR);
		pop_call();
		return FALSE;
	}

	if ( !fReplace )
	{
		pop_call();
		return FALSE;
	}

	if (IS_SET(obj->extra_flags, ITEM_NOREMOVE))
	{
		if (fDisplay)
		{
			act( "You cannot remove $p.", ch, obj, NULL, TO_CHAR );
		}
		pop_call();
		return FALSE;
	}

	unequip_char( ch, obj );

	if (fDisplay)
	{
		act( "You stop using $p.", ch, obj, NULL, TO_CHAR );
		act( "$n stops using $p.", ch, obj, NULL, TO_ROOM );
	}

	oprog_remove_trigger(ch, obj);

	pop_call();
	return TRUE;
}

/*
	Wear one object.
	Optional replacement of existing objects.
	Big repetitive code, ick.
*/

bool wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace, int location, bool fDisplay )
{
	OBJ_DATA *obj2, *tmpobj;

	push_call("wear_obj(%p,%p,%p,%p,%p)",ch,obj,fReplace,location,fDisplay);

	if (reinc_eq_level(ch) < obj->level)
	{
		if (fDisplay)
		{
			ch_printf(ch, "You must be level %d to use this object.\n\r", obj_eq_level(ch, obj));
			act( "$n tries to use $p, but is too inexperienced.",	ch, obj, NULL, TO_ROOM );
		}
		pop_call();
		return FALSE;
	}

	if ((obj2 = get_obj_wear_vnum(ch, obj->pIndexData->vnum)) != NULL)
	{
		if (obj2->pIndexData == obj->pIndexData)
		{
			if (fDisplay)
			{
				act("Wearing more than one $t will not help you.", ch, short_to_name(obj->short_descr, 1), NULL, TO_CHAR);
			}
			pop_call();
			return FALSE;
		}
	}

	if (obj->owned_by)
	{
		if (IS_NPC(ch) || ch->pcdata->pvnum != obj->owned_by)
		{
			if (fDisplay)
			{
				send_to_char( "You cannot wear someone else's equipment!\n\r", ch);
			}
			pop_call();
			return FALSE;
		}
	}

	if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch))
	||  (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch))
	||  (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch)))
	{
		if (fDisplay)
		{
			act( "You are zapped by $p.", ch, obj, NULL, TO_CHAR );
			act( "$n is zapped by $p.",  ch, obj, NULL, TO_ROOM );
		}
		pop_call();
		return FALSE;
	}

	if (!class_allowed(obj, ch, fDisplay))
	{
		pop_call();
		return FALSE;
	}

	if ((location == WEAR_LIGHT || location == -1) && obj->item_type == ITEM_LIGHT)
	{
		if ( !remove_obj( ch, WEAR_LIGHT, fReplace , fDisplay) )
		{
			pop_call();
			return FALSE;
		}

		if (fDisplay)
		{
			act( "You light $p and hold it.",  ch, obj, NULL, TO_CHAR );
			act( "$n lights $p and holds it.", ch, obj, NULL, TO_ROOM );
		}
		equip_char( ch, obj, WEAR_LIGHT );

		pop_call();
		return TRUE;
	}

	if ((location == WEAR_HEART || location == -1) && CAN_WEAR(obj, ITEM_WEAR_HEART))
	{
		OBJ_DATA *oldchain = get_eq_char(ch, WEAR_HEART);

		if (oldchain)
		{
			log_god_printf("%s replaced god chain [%u] with god chain [%u]", ch->name, oldchain->pIndexData->vnum, obj->pIndexData->vnum);

			junk_obj(oldchain);
		}

		if (!remove_obj(ch, WEAR_HEART, fReplace, fDisplay))
		{
			pop_call();
			return FALSE;
		}

		equip_char(ch, obj, WEAR_HEART);

		if (!IS_NPC(ch))
		{
			ch->pcdata->god = -1;
			ch->pcdata->god = which_god(ch);
		}

		if (fDisplay)
		{
			act( "$p becomes one with your heart.", ch, obj, NULL, TO_CHAR );
			act( "$p becomes one with $n's heart.", ch, obj, NULL, TO_ROOM );
		}
		pop_call();
		return TRUE;
	}

	if ((location == WEAR_FINGER_L || location == WEAR_FINGER_R || location == -1) && CAN_WEAR(obj, ITEM_WEAR_FINGER))
	{
		if (location == -1)
		{
			if (get_eq_char(ch, WEAR_FINGER_L) != NULL
			&&  get_eq_char(ch, WEAR_FINGER_R) != NULL
			&&  !remove_obj(ch, WEAR_FINGER_L, fReplace , fDisplay)
			&&  !remove_obj(ch, WEAR_FINGER_R, fReplace , fDisplay))
			{
				pop_call();
				return FALSE;
			}
		}

		if (location == WEAR_FINGER_L)
		{
			if (get_eq_char(ch, WEAR_FINGER_L) != NULL &&  !remove_obj(ch, WEAR_FINGER_L, fReplace , fDisplay))
			{
				pop_call();
				return FALSE;
			}
		}

		if (location == WEAR_FINGER_R)
		{
			if (get_eq_char(ch, WEAR_FINGER_R) != NULL && !remove_obj(ch, WEAR_FINGER_R, fReplace , fDisplay))
			{
				pop_call();
				return FALSE;
			}
		}

		if (rspec_req(ch, RSPEC_MULTI_ARMS))
		{
			if (fDisplay)
			{
				send_to_char("You cannot wear rings.\n\r", ch);
			}
			pop_call();
			return FALSE;
		}

		if (location == WEAR_FINGER_L || location == -1)
		{
			if (get_eq_char(ch, WEAR_FINGER_L) == NULL)
			{
				equip_char(ch, obj, WEAR_FINGER_L);

				if (fDisplay)
				{
					act( "You slip $p onto your left finger.",  ch, obj, NULL, TO_CHAR );
					act( "$n slips $p onto $s left finger.",    ch, obj, NULL, TO_ROOM );
				}
				pop_call();
				return TRUE;
			}
		}

		if (location == WEAR_FINGER_R || location == -1)
		{
			if (get_eq_char(ch, WEAR_FINGER_R) == NULL)
			{
				equip_char( ch, obj, WEAR_FINGER_R );

				if (fDisplay)
				{
					act( "You slip $p onto your right finger.", ch, obj, NULL, TO_CHAR );
					act( "$n slips $p onto $s right finger.",   ch, obj, NULL, TO_ROOM );
				}
				pop_call();
				return TRUE;
			}
		}

		bug( "Wear_obj: no free finger.", 0 );

		if (fDisplay)
		{
			send_to_char( "You already wear two rings.\n\r", ch );
		}
		pop_call();
		return FALSE;
	}

	if ((location == WEAR_NECK_1 || location == WEAR_NECK_2 || location == -1) && CAN_WEAR(obj, ITEM_WEAR_NECK))
	{
		if (location == -1)
		{
			if (get_eq_char(ch, WEAR_NECK_1) != NULL
			&&  get_eq_char(ch, WEAR_NECK_2) != NULL
			&&  !remove_obj(ch, WEAR_NECK_1, fReplace , fDisplay)
			&&  !remove_obj(ch, WEAR_NECK_2, fReplace , fDisplay))
			{
				pop_call();
				return FALSE;
			}
		}
		if (location == WEAR_NECK_1)
		{
			if (get_eq_char(ch, WEAR_NECK_1) != NULL && !remove_obj(ch, WEAR_NECK_1, fReplace , fDisplay))
			{
				pop_call();
				return FALSE;
			}
		}
		if (location == WEAR_NECK_2)
		{
			if (get_eq_char(ch, WEAR_NECK_2) != NULL && !remove_obj(ch, WEAR_NECK_2, fReplace , fDisplay))
			{
				pop_call();
				return FALSE;
			}
		}

		if (location == WEAR_NECK_1 || location == -1)
		{
			if (get_eq_char(ch, WEAR_NECK_1) == NULL)
			{
				equip_char( ch, obj, WEAR_NECK_1 );

				if (fDisplay)
				{
					act( "You fasten $p around your neck.", ch, obj, NULL, TO_CHAR );
					act( "$n fastens $p around $s neck.",   ch, obj, NULL, TO_ROOM );
				}
				pop_call();
				return TRUE;
			}
		}

		if (location == WEAR_NECK_2 || location == -1)
		{
			if (get_eq_char(ch, WEAR_NECK_2) == NULL)
			{
				equip_char( ch, obj, WEAR_NECK_2 );
				if (fDisplay)
				{
					act( "You fasten $p around your neck.", ch, obj, NULL, TO_CHAR );
					act( "$n fastens $p around $s neck.",   ch, obj, NULL, TO_ROOM );
				}
				pop_call();
				return TRUE;
			}
		}
		bug( "Wear_obj: no free neck.", 0 );

		if (fDisplay)
		{
			send_to_char( "You already wear two neck items.\n\r", ch );
		}
		pop_call();
		return FALSE;
	}

	if ((location == WEAR_BODY || location == -1) && CAN_WEAR(obj, ITEM_WEAR_BODY))
	{
		if (!remove_obj(ch, WEAR_BODY, fReplace, fDisplay))
		{
			pop_call();
			return FALSE;
		}
		equip_char( ch, obj, WEAR_BODY );

		if (fDisplay)
		{
			act( "You wear $p on your body.", ch, obj, NULL, TO_CHAR );
			act( "$n wears $p on $s body.",   ch, obj, NULL, TO_ROOM );
		}


		pop_call();
		return TRUE;
	}

	if ((location == WEAR_HEAD || location == -1) && CAN_WEAR(obj, ITEM_WEAR_HEAD))
	{
		if ( !remove_obj( ch, WEAR_HEAD, fReplace , fDisplay) )
		{
			pop_call();
			return FALSE;
		}
		equip_char( ch, obj, WEAR_HEAD );

		if ( fDisplay )
		{
			act( "You place $p on your head.", ch, obj, NULL, TO_CHAR );
			act( "$n places $p on $s head.",   ch, obj, NULL, TO_ROOM );
		}
		pop_call();
		return TRUE;
	}

	if ((location == WEAR_LEGS || location == -1) && CAN_WEAR(obj, ITEM_WEAR_LEGS))
	{
		if (!remove_obj(ch, WEAR_LEGS, fReplace , fDisplay))
		{
			pop_call();
			return FALSE;
		}
		equip_char( ch, obj, WEAR_LEGS );

		if (fDisplay)
		{
			act( "You slip into $p.", ch, obj, NULL, TO_CHAR );
			act( "$n slips into $p.", ch, obj, NULL, TO_ROOM );
		}

		pop_call();
		return TRUE;
	}

	if ((location == WEAR_FEET || location == -1) && CAN_WEAR(obj, ITEM_WEAR_FEET))
	{
		if (!remove_obj(ch, WEAR_FEET, fReplace, fDisplay))
		{
			pop_call();
			return FALSE;
		}
		equip_char(ch, obj, WEAR_FEET);

		if (fDisplay)
		{
			act( "You pull on $p.", ch, obj, NULL, TO_CHAR );
			act( "$n pulls on $p.",   ch, obj, NULL, TO_ROOM );
		}
		pop_call();
		return TRUE;
	}

	if ((location == WEAR_HANDS || location == -1) && CAN_WEAR(obj, ITEM_WEAR_HANDS))
	{
		if ( !remove_obj( ch, WEAR_HANDS, fReplace , fDisplay) )
		{
			pop_call();
			return FALSE;
		}
		equip_char( ch, obj, WEAR_HANDS );

		if (fDisplay)
		{
			act( "You slip $p over your hands.", ch, obj, NULL, TO_CHAR );
			act( "$n slips $p over $s hands.",   ch, obj, NULL, TO_ROOM );
		}

		pop_call();
		return TRUE;
	}

	if ((location == WEAR_ARMS || location == -1) && CAN_WEAR(obj, ITEM_WEAR_ARMS))
	{
		if ( !remove_obj( ch, WEAR_ARMS, fReplace , fDisplay) )
		{
			pop_call();
			return FALSE;
		}
		equip_char( ch, obj, WEAR_ARMS );

		if (fDisplay)
		{
			act( "You wear $p on your arms.", ch, obj, NULL, TO_CHAR );
			act( "$n wears $p on $s arms.",   ch, obj, NULL, TO_ROOM );
		}
		pop_call();
		return TRUE;
	}

	if ((location == WEAR_ABOUT || location == -1) && CAN_WEAR(obj, ITEM_WEAR_ABOUT))
	{
		if ( !remove_obj( ch, WEAR_ABOUT, fReplace , fDisplay) )
		{
			pop_call();
			return FALSE;
		}
		equip_char( ch, obj, WEAR_ABOUT );

		if (fDisplay)
		{
			act( "You wrap $p about your body.", ch, obj, NULL, TO_CHAR );
			act( "$n wraps $p about $s body.",   ch, obj, NULL, TO_ROOM );
		}
		pop_call();
		return TRUE;
	}

	if ((location == WEAR_WAIST || location == -1) && CAN_WEAR(obj, ITEM_WEAR_WAIST))
	{
		if (!remove_obj( ch, WEAR_WAIST, fReplace , fDisplay) )
		{
			pop_call();
			return FALSE;
		}
		equip_char( ch, obj, WEAR_WAIST );

		if (fDisplay)
		{
			act( "You fasten $p around your waist.", ch, obj, NULL, TO_CHAR );
			act( "$n fastens $p around $s waist.",   ch, obj, NULL, TO_ROOM );
		}
		pop_call();
		return TRUE;
	}

	if ((location == WEAR_WRIST_L || location == WEAR_WRIST_R || location == -1) && CAN_WEAR(obj, ITEM_WEAR_WRIST))
	{
		if (location == -1)
		{
			if (get_eq_char(ch, WEAR_WRIST_L) != NULL
			&&  get_eq_char(ch, WEAR_WRIST_R) != NULL
			&&  !remove_obj(ch, WEAR_WRIST_L, fReplace , fDisplay)
			&&  !remove_obj(ch, WEAR_WRIST_R, fReplace , fDisplay))
			{
				pop_call();
				return FALSE;
			}
		}
		if (location == WEAR_WRIST_L)
		{
			if (get_eq_char(ch, WEAR_WRIST_L) != NULL && !remove_obj(ch, WEAR_WRIST_L, fReplace , fDisplay))
			{
				pop_call();
				return FALSE;
			}
		}
		if (location == WEAR_WRIST_R)
		{
			if (get_eq_char(ch, WEAR_WRIST_R) != NULL && !remove_obj(ch, WEAR_WRIST_R, fReplace , fDisplay))
			{
				pop_call();
				return FALSE;
			}
		}

		if (location == WEAR_WRIST_L || location == -1)
		{
			if ( get_eq_char( ch, WEAR_WRIST_L ) == NULL )
			{
				equip_char( ch, obj, WEAR_WRIST_L );
	
				if (fDisplay)
				{
					act( "You slide $p over your left wrist.", ch, obj, NULL, TO_CHAR );
					act( "$n slides $p over $s left wrist.",   ch, obj, NULL, TO_ROOM );
				}
				pop_call();
				return TRUE;
			}
		}

		if (location == WEAR_WRIST_R || location == -1)
		{
			if ( get_eq_char( ch, WEAR_WRIST_R ) == NULL )
			{
				equip_char( ch, obj, WEAR_WRIST_R );

				if( fDisplay )
				{
					act( "You slide $p over your right wrist.", ch, obj, NULL, TO_CHAR );
					act( "$n slides $p over $s right wrist.",   ch, obj, NULL, TO_ROOM );
				}
				pop_call();
				return TRUE;
			}
		}

		bug( "Wear_obj: no free wrist.", 0 );

		if ( fDisplay )
		{
			send_to_char( "You already wear two wrist items.\n\r", ch );
		}
		pop_call();
		return FALSE;
	}

	if ((location == WEAR_SHIELD || location == -1) && CAN_WEAR( obj, ITEM_WEAR_SHIELD ) )
	{
		if (get_eq_char(ch, WEAR_DUAL_WIELD) && !rspec_req(ch, RSPEC_MULTI_ARMS))
		{
			if (fDisplay)
			{
				send_to_char("You cannot use a shield AND two weapons!\n\r", ch);
			}
			pop_call();
			return FALSE;
		}

		if (!remove_obj(ch, WEAR_SHIELD, fReplace , fDisplay))
		{
			pop_call();
			return FALSE;
		}

		if (!IS_NPC(ch))
		{
			if (!cflag(ch, CFLAG_SHIELD))
			{
				if (fDisplay)
				{
					send_to_char( "You cannot use that kind of armor.\n\r", ch);
				}
				pop_call();
				return FALSE;
			}
		}
		equip_char( ch, obj, WEAR_SHIELD );

		if (fDisplay)
		{
			act( "You strap $p to your arm.", ch, obj, NULL, TO_CHAR );
			act( "$n straps $p to $s arm.",   ch, obj, NULL, TO_ROOM );
		}
		pop_call();
		return TRUE;
	}

	if ((location == WEAR_WIELD || location == -1) && CAN_WEAR(obj, ITEM_WEAR_WIELD))
	{
		int skill;

		if (get_obj_weight(obj) > str_app[get_curr_str(ch)].wield)
		{
			if (fDisplay)
			{
				send_to_char( "It is too heavy for you to wield.\n\r", ch );
			}
			pop_call();
			return FALSE;
		}

		if ((tmpobj = get_eq_char(ch, WEAR_DUAL_WIELD)) != NULL)
		{
			if (get_obj_weight(obj) + get_obj_weight(tmpobj) > str_app[get_curr_str(ch)].wield)
			{
				if (fDisplay)
				{
					act("$p is too heavy for you to dual-wield alongside $P.", ch, obj, tmpobj, TO_CHAR);
				}
				pop_call();
				return FALSE;
			}
		}

		if (!remove_obj(ch, WEAR_WIELD, fReplace, fDisplay))
		{
			pop_call();
			return FALSE;
		}

		equip_char( ch, obj, WEAR_WIELD );

		if (fDisplay)
		{
			act( "You wield $p.", ch, obj, NULL, TO_CHAR );
			act( "$n wields $p.", ch, obj, NULL, TO_ROOM );
		}

		if (!fDisplay)
		{
			pop_call();
			return TRUE;
		}

		if (obj->item_type == ITEM_WEAPON)
		{
			skill = weapon_skill(ch, obj->value[0]);

			switch (skill / 25)
			{
				case 5: act("$p feels like a part of you.",      ch, obj, NULL, TO_CHAR); break;
				case 4: act("You are mastered with $p.",         ch, obj, NULL, TO_CHAR); break;
				case 3: act("You are skilled with $p.",          ch, obj, NULL, TO_CHAR); break;
				case 2: act("$p feels clumsy in your hands.",    ch, obj, NULL, TO_CHAR); break;
				case 1: act("You fumble and almost drop $p.",    ch, obj, NULL, TO_CHAR); break;
				case 0: act("You wonder which end is up on $p.", ch, obj, NULL, TO_CHAR); break;
			}
		}
		pop_call();
		return TRUE;
	}

	if ((location == WEAR_DUAL_WIELD || location == -1) && CAN_WEAR(obj, ITEM_WEAR_WIELD))
	{
		int skill;

		if (could_dual(ch))
		{
			if ((tmpobj = get_eq_char(ch, WEAR_WIELD)) != NULL && get_eq_char(ch, WEAR_DUAL_WIELD))
			{
				if (get_obj_weight(obj) + get_obj_weight(tmpobj) > str_app[get_curr_str(ch)].wield)
				{
					if (fDisplay)
					{
						act("$p is too heavy for you to dual-wield alongside $P.", ch, obj, tmpobj, TO_CHAR);
					}
					pop_call();
					return FALSE;
				}

			}

			if (get_obj_weight(obj) > str_app[get_curr_str(ch)].wield / 2)
			{
				if (fDisplay)
				{
					act("$p is too heavy for you to dual-wield.", ch, obj, NULL, TO_CHAR);
				}
				pop_call();
				return FALSE;
			}

			if (!rspec_req(ch, RSPEC_MULTI_ARMS))
			{
				if (!remove_obj(ch, WEAR_SHIELD, fReplace, fDisplay))
				{
					pop_call();
					return FALSE;
				}

				if (!remove_obj(ch, WEAR_HOLD, fReplace, fDisplay))
				{
					pop_call();
					return FALSE;
				}
			}

			if (!remove_obj(ch, WEAR_DUAL_WIELD, fReplace, fDisplay))
			{
				pop_call();
				return FALSE;
			}
			equip_char( ch, obj, WEAR_DUAL_WIELD );

			if (fDisplay)
			{
				act( "You wield $p in your off-hand.", ch, obj, NULL, TO_CHAR );
				act( "$n wields $p in $s off-hand.", ch, obj, NULL, TO_ROOM );
			}

			if (!fDisplay)
			{
				pop_call();
				return TRUE;
			}

			if (obj->item_type == ITEM_WEAPON)
			{
				skill = weapon_skill(ch, obj->value[0]);

				switch (skill / 25)
				{
					case 5: act("$p feels like a part of you.",      ch, obj, NULL, TO_CHAR); break;
					case 4: act("You are mastered with $p.",         ch, obj, NULL, TO_CHAR); break;
					case 3: act("You are skilled with $p.",          ch, obj, NULL, TO_CHAR); break;
					case 2: act("$p feels clumsy in your hands.",    ch, obj, NULL, TO_CHAR); break;
					case 1: act("You fumble and almost drop $p.",    ch, obj, NULL, TO_CHAR); break;
					case 0: act("You wonder which end is up on $p.", ch, obj, NULL, TO_CHAR); break;
				}
				pop_call();
				return TRUE;
			}
			pop_call();
			return FALSE;
		}
		else
		{
			if (fDisplay)
			{
				send_to_char( "You cannot dual-wield weapons.\n\r", ch);
			}
			pop_call();
			return FALSE;
		}
	}

	if ((location == WEAR_HOLD || location == -1) && CAN_WEAR(obj, ITEM_WEAR_HOLD))
	{
		if (get_eq_char(ch, WEAR_DUAL_WIELD) && !rspec_req(ch, RSPEC_MULTI_ARMS))
		{
			if (fDisplay)
			{
				send_to_char("You cannot hold something AND two weapons!\n\r", ch);
			}
			pop_call();
			return FALSE;
		}
		if (!remove_obj(ch, WEAR_HOLD, fReplace , fDisplay))
		{
			pop_call();
			return FALSE;
		}
		equip_char( ch, obj, WEAR_HOLD );

		if (fDisplay)
		{
			act( "You hold $p in your hand.", ch, obj, NULL, TO_CHAR );
			act( "$n holds $p in $s hand.",   ch, obj, NULL, TO_ROOM );
		}
		pop_call();
		return TRUE;
	}

	if (fReplace && location != -1 && fDisplay)
	{
		act("You cannot wear $p in that location.", ch, obj, NULL, TO_CHAR);
	}
	else if (fReplace && fDisplay)
	{
		act("You cannot wear $p.", ch, obj, NULL, TO_CHAR);
	}
	pop_call();
	return FALSE;
}


void do_wear( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	char local[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	OBJ_PROG *prg;
	int location;

	push_call("do_wear(%p,%p)",ch,argument);

	argument=one_argument( argument, arg );
	one_argument( argument, local );

	if (arg[0] == '\0')
	{
		send_to_char( "Wear what?\n\r", ch );
		pop_call();
		return;
	}

	if (!strcasecmp(arg, "all"))
	{
		OBJ_DATA *obj_next, *obj_old;
		bool foundi;

		foundi  = FALSE;
		obj_old = NULL;

		for (obj = ch->first_carrying ; obj ; obj = obj_next)
		{
			/* Now always picks best and also lights - Scandum */

			obj_next = obj->next_content;

			if (obj->carried_by == NULL)
			{
				break;
			}
/*
			if (obj->item_type == ITEM_LIGHT)
			{
				obj_old = get_eq_char(ch, WEAR_LIGHT);

				if ((obj_old == NULL || compare_obj(ch, obj_old, obj) < 0)
				&& !(IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch))
				&& !(IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch))
				&& !(IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch)))
				{
					if (wear_obj(ch, obj, TRUE, -1, FALSE))
					{
						foundi = TRUE;

						oprog_wear_trigger(ch, obj);

						if (!IS_NPC(ch) && obj->carried_by && IS_SET(obj->pIndexData->oprogtypes, TRIG_WEAR))
						{
							for (prg = obj->pIndexData->first_oprog ; prg ; prg = prg->next)
							{
								if (prg->trigger == TRIG_WEAR && number_percent() <= prg->percentage)
								{
									start_object_program( ch, obj, prg, "");
								}
							}
						}
						continue;
					}
				}
			}
*/
			if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj))
			{
				for (location = 0 ; location < MAX_WEAR ; location++)
				{
					if (has_wear_location(obj, location))
					{
						obj_old = get_eq_char(ch, location);

						if ((obj_old == NULL || compare_obj(ch, obj_old, obj) < 0)
						&& !(IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch))
						&& !(IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch))
						&& !(IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch)))
						{
							if (wear_obj(ch, obj, TRUE, location, FALSE))
							{
								foundi = TRUE;

								oprog_wear_trigger(ch, obj);

								if (!IS_NPC(ch) && obj->carried_by && IS_SET(obj->pIndexData->oprogtypes, TRIG_WEAR))
								{
									for (prg = obj->pIndexData->first_oprog ; prg ; prg = prg->next)
									{
										if (prg->trigger == TRIG_WEAR && number_percent() <= prg->percentage)
										{
											start_object_program( ch, obj, prg, "");
										}
									}
								}
								break;
							}
						}
					}
				}
			}
		}

		if (foundi)
		{
			act( "You wear the best equipment that you have.",ch,NULL,NULL,TO_CHAR);
			act( "$n wears all $s equipment.", ch, NULL, NULL, TO_ROOM);
		}
		else
		{
			send_to_char( "You wear nothing new.\n\r", ch);
		}
		pop_call();
		return;
	}
	else
	{
		if ((obj = get_obj_carry(ch, arg)) == NULL)
		{
			ch_printf(ch, "You do not have the item '%s'.\n\r", arg);
			pop_call();
			return;
		}
		location=-1;
		if (!strcasecmp(local, "light"))	location = WEAR_LIGHT;
		if (!strcasecmp(local, "finger"))	location = WEAR_FINGER_L;
		if (!strcasecmp(local, "1.finger"))     location = WEAR_FINGER_R;
		if (!strcasecmp(local, "2.finger"))     location = WEAR_FINGER_R;
		if (!strcasecmp(local, "neck"))		location = WEAR_NECK_1;
		if (!strcasecmp(local, "1.neck"))	location = WEAR_NECK_2;
		if (!strcasecmp(local, "2.neck"))	location = WEAR_NECK_2;
		if (!strcasecmp(local, "body"))		location = WEAR_BODY;
		if (!strcasecmp(local, "head"))		location = WEAR_HEAD;
		if (!strcasecmp(local, "legs"))		location = WEAR_LEGS;
		if (!strcasecmp(local, "feet"))		location = WEAR_FEET;
		if (!strcasecmp(local, "hands"))	location = WEAR_HANDS;
		if (!strcasecmp(local, "arms"))		location = WEAR_ARMS;
		if (!strcasecmp(local, "shield"))	location = WEAR_SHIELD;
		if (!strcasecmp(local, "about"))	location = WEAR_ABOUT;
		if (!strcasecmp(local, "waist"))	location = WEAR_WAIST;
		if (!strcasecmp(local, "wrist"))	location = WEAR_WRIST_L;
		if (!strcasecmp(local, "1.wrist"))	location = WEAR_WRIST_R;
		if (!strcasecmp(local, "2.wrist"))	location = WEAR_WRIST_R;
		if (!strcasecmp(local, "wield"))	location = WEAR_WIELD;
		if (!strcasecmp(local, "dual"))         location = WEAR_DUAL_WIELD;
		if (!strcasecmp(local, "hold"))		location = WEAR_HOLD;
		if (!strcasecmp(local, "heart"))	location = WEAR_HEART;

		if (wear_obj(ch, obj, TRUE, location, TRUE))
		{
			oprog_wear_trigger(ch, obj);

			if (!IS_NPC(ch) && IS_SET(obj->pIndexData->oprogtypes, TRIG_WEAR))
			{
				for (prg = obj->pIndexData->first_oprog ; prg ; prg = prg->next)
				{
					if (prg->trigger == TRIG_WEAR && number_percent() <= prg->percentage)
					{
						start_object_program( ch, obj, prg, "");
					}
				}
			}
		}
	}
	pop_call();
	return;
}

void do_remove( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_PROG *prg;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	push_call("do_remove(%p,%p)",ch,argument);

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Remove what?\n\r", ch );

		pop_call();
		return;
	}
	/* all added by Trom */

	if (!strcasecmp(arg, "all"))
	{
		act( "You remove all your equipment.",ch,NULL,NULL,TO_CHAR);
		act( "$n removes all $s equipment.", ch, NULL, NULL, TO_ROOM);

		for (obj = ch->first_carrying ; obj != NULL ; obj = obj_next)
		{
			obj_next = obj->next_content;

			if (obj->carried_by == NULL)
			{
				break;
			}

			if (obj->wear_loc != WEAR_NONE)
			{
				if (remove_obj(ch, obj->wear_loc,TRUE, FALSE))
				{
					if (!IS_NPC(ch) && obj->carried_by && IS_SET(obj->pIndexData->oprogtypes, TRIG_REMOVE))
					{
						for (prg = obj->pIndexData->first_oprog ; prg != NULL ; prg = prg->next)
						{
							if (prg->trigger == TRIG_REMOVE && number_percent() <= prg->percentage)
							{
								start_object_program( ch, obj, prg, "");
							}
						}
					}
				}
			}
		}
	}
	else
	{
		if ((obj = get_obj_wear( ch, arg ) ) == NULL )
		{
			ch_printf(ch, "You do not have the item '%s'.\n\r", arg);
			pop_call();
			return;
		}
		if (remove_obj( ch, obj->wear_loc, TRUE , TRUE))
		{
			if (!IS_NPC(ch) && IS_SET(obj->pIndexData->oprogtypes, TRIG_REMOVE))
			{
				for (prg = obj->pIndexData->first_oprog ; prg != NULL ; prg = prg->next)
				{
					if (prg->trigger == TRIG_REMOVE && number_percent() <= prg->percentage)
					{
						start_object_program( ch, obj, prg, "");
					}
				}
			}
		}
	}
	pop_call();
	return;
}


void do_sacrifice( CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];

	OBJ_DATA *obj			= NULL;
	OBJ_DATA *obj_next		= NULL;

	bool	found			= FALSE;
	bool shrtd			= FALSE;

	int amount			= 1;
	int cnt				= 0;

	push_call("do_sacrifice(%p,%p)",ch,argument);

	argument = one_argument( argument, arg1 );

	if (is_number(arg1))
	{
		amount	= UMAX(1, atol(arg1));
		argument	= one_argument( argument, arg1 );
	}

	if (*arg1 == 0)
	{
		send_to_char("Sacrifice what?\n\r", ch);
		pop_call();
		return;
	}

	if (!check_blind(ch))
	{
		pop_call();
		return;
	}

	if (strcasecmp(arg1, "all") && str_prefix("all.", arg1))
	{
		if ((obj = get_obj_list(ch, arg1, ch->in_room->first_content)) == NULL)
		{
			send_to_char( "You cannot find it.\n\r", ch );
			pop_call();
			return;
		}

		for ( ; shrtd <= 1 && !found ; shrtd++)
		{
			for (obj = ch->in_room->first_content ; obj ; obj = obj_next)
			{
				obj_next = obj->next_content;

				if (!can_see_obj(ch, obj))
				{
					continue;
				}

				if (!shrtd && !is_name(arg1, obj->name))
				{
					continue;
				}

				if (shrtd  && !is_name_short(arg1, obj->name))
				{
					continue;
				}

				found = TRUE;

				if (obj->first_content)
				{
					obj_next = NULL;
				}

				if (sacrifice_obj(ch, obj, NULL, TRUE))
				{
					if (++cnt >= amount)
					{
						break;
					}
				}
			}
		}
		pop_call();
		return;
	}
	else
	{
		for ( ; shrtd <= 1 && !found ; shrtd++)
		{
			for (obj = ch->in_room->first_content ; obj ; obj = obj_next)
			{
				obj_next = obj->next_content;

				if (!can_see_obj(ch, obj) || !IS_SET(obj->wear_flags, ITEM_TAKE))
				{
					continue;
				}

				if (!shrtd && arg1[3] && !is_name(&arg1[4], obj->name))
				{
					continue;
				}

				if (shrtd  && arg1[3] && !is_name_short(&arg1[4], obj->name))
				{
					continue;
				}

				found = TRUE;

				if (obj->first_content)
				{
					obj_next = NULL;
				}

				sacrifice_obj(ch, obj, NULL, TRUE);
			}
		}
		if (found == FALSE)
		{
			if (arg1[3] == '\0')
			{
				send_to_char( "You see nothing here.\n\r", ch );
			}
			else
			{
				act( "You see no $T here.", ch, NULL, short_to_name(&arg1[4], 0), TO_CHAR );
			}
		}
	}
	pop_call();
	return;
}


void do_quaff( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int level;

	push_call("do_quaff(%p,%p)",ch,argument);

	if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_char( "You cannot do that here.\n\r", ch);

		pop_call();
		return;
	}

	if (!IS_NPC(ch) && ch->fighting && ch->class != CLASS_MARAUDER)
	{
		send_to_char( "You may not do that while fighting.\n\r", ch);
		pop_call();
		return;
	}

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Quaff what?\n\r", ch );

		pop_call();
		return;
	}

	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
		send_to_char( "You do not have that potion.\n\r", ch );

		pop_call();
		return;
	}

	if ( obj->item_type != ITEM_POTION )
	{
		send_to_char( "You can quaff only potions.\n\r", ch );

		pop_call();
		return;
	}

	level = obj->value[0];

	if (level > reinc_eq_level(ch))
	{
		level = reinc_eq_level(ch);
	}

	if (!cflag(ch, CFLAG_QUAFF))
	{
		level /= 2;
	}

	if (ch->fighting && number_percent() > get_curr_dex(ch) * 2)
	{
		act( "You break $p.", ch, obj, NULL ,TO_CHAR );
		act( "$n breaks $p.", ch, obj, NULL, TO_ROOM );
	}
	else
	{
		act( "You quaff $p.", ch, obj, NULL ,TO_CHAR );
		act( "$n quaffs $p.", ch, obj, NULL, TO_ROOM );

		obj_cast_spell( obj->value[1], level, ch, ch, obj );
		obj_cast_spell( obj->value[2], level, ch, ch, obj );
		obj_cast_spell( obj->value[3], level, ch, ch, obj );
	}

	junk_obj(obj);

	wait_state(ch, PULSE_VIOLENCE * 2/3);

	pop_call();
	return;
}


void do_recite( CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *scroll;
	OBJ_DATA *obj;

	push_call("do_recite(%p,%p)",ch,argument);

	if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_char( "You cannot do that here.\n\r", ch);
		pop_call();
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ((scroll = get_obj_carry(ch, arg1)) == NULL)
	{
		send_to_char("You do not have that scroll.\n\r", ch);
		pop_call();
		return;
	}

	if (scroll->item_type != ITEM_SCROLL)
	{
		send_to_char( "You can recite only scrolls.\n\r", ch );
		pop_call();
		return;
	}

	if (scroll->level > reinc_eq_level(ch))
	{
		send_to_char( "You are too low a level to recite that scroll!\n\r", ch);
		pop_call();
		return;
	}

	obj = NULL;

	if (arg2[0] == '\0')
	{
		victim = ch;
	}
	else
	{
		if ((victim = get_char_room (ch, arg2)) == NULL
		&&  (obj = get_obj_here(ch, arg2)) == NULL)
		{
			send_to_char("You cannot find it.\n\r", ch);
			pop_call();
			return;
		}
	}

	if (ch->fighting && number_percent() > learned(ch, gsn_scroll_mastery))
	{
		if (number_percent() > learned(ch, gsn_recite) / 4)
		{
			send_to_char("You tear the scroll in two in the heat of combat.\n\r", ch);
			junk_obj(scroll);
			pop_call();
			return;
		}
	}
	else if (ch->fighting)
	{
		check_improve(ch, gsn_scroll_mastery);
	}
	else
	{
		if (number_percent() > learned(ch, gsn_recite))
		{
			switch (number_bits(2))
			{
				case 0:	send_to_char("Something in your throat prevents you from reciting the proper phrase.\n\r", ch);	break;
				case 1:	send_to_char("You bumble your words causing the spell to fizzle\n\r", ch);						break;
				case 2:	send_to_char("You mispronounce a syllable.\n\r", ch);										break;
				case 3:	send_to_char("You hesitate mid-way through the reciting and nothing happens.\n\r", ch);			break;
			}
			junk_obj(scroll);
			pop_call();
			return;
		}
		check_improve(ch, gsn_recite);
	}

	act( "You recite $p.", ch, scroll, NULL, TO_CHAR );
	act( "$n recites $p.", ch, scroll, NULL, TO_ROOM );

	wait_state(ch, skill_table[gsn_recite].beats);

	obj_cast_spell(scroll->value[1], scroll->value[0], ch, victim, obj);
	obj_cast_spell(scroll->value[2], scroll->value[0], ch, victim, obj);
	obj_cast_spell(scroll->value[3], scroll->value[0], ch, victim, obj);

	junk_obj(scroll);

	pop_call();
	return;
}

void do_brandish( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	OBJ_DATA *staff;
	int sn;

	push_call("do_brandish(%p,%p)",ch,argument);

	if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_char( "You cannot do that here.\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && (ch->fighting || ch->position == POS_FIGHTING))
	{
		send_to_char( "You may not do that while fighting.\n\r", ch);
		pop_call();
		return;
	}

	if ((staff = get_eq_char(ch, WEAR_HOLD)) == NULL)
	{
		send_to_char( "You hold nothing in your hand.\n\r", ch );
		pop_call();
		return;
	}

	if (staff->item_type != ITEM_STAFF)
	{
		send_to_char( "You can brandish only with a staff.\n\r", ch );
		pop_call();
		return;
	}

	if (staff->level > reinc_eq_level(ch))
	{
		send_to_char( "You are too low a level to brandish that staff!\n\r", ch);
		pop_call();
		return;
	}

	if ((sn = staff->value[3]) < 0 || sn >= MAX_SKILL || skill_table[sn].spell_fun == 0)
	{
		bug( "Do_brandish in staff %d: bad sn %d.", staff->pIndexData->vnum, sn );
		pop_call();
		return;
	}

	wait_state(ch, skill_table[gsn_brandish].beats);

	if (staff->value[2] > 0)
	{
		act( "You brandish $p.",  ch, staff, NULL, TO_CHAR );
		act( "$n brandishes $p.", ch, staff, NULL, TO_ROOM );

		if (number_percent() > learned(ch, gsn_brandish))
		{
			act("You fail to invoke $p.",  ch, staff, NULL, TO_CHAR);
			act("...and nothing happens.", ch, staff, NULL, TO_ROOM);
		}
		else
		{
			for (vch = ch->in_room->first_person ; vch ; vch = vch_next)
			{
				vch_next    = vch->next_in_room;

				switch (skill_table[sn].target)
				{
					default:
						bug("Do_brandish: bad target for sn %d.", sn);
						pop_call();
						return;

					case TAR_IGNORE:
						if (vch != ch)
						{
							continue;
						}
						break;

					case TAR_CHAR_OFFENSIVE:
						if (!IS_NPC(vch) && who_fighting(ch) && ch->fighting->who != vch)
						{
							continue;
						}
						break;

					case TAR_CHAR_DEFENSIVE:
						if (IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch))
						{
							continue;
						}
						break;

					case TAR_CHAR_SELF:
						if (vch != ch)
						{
							continue;
						}
						break;
				}
				obj_cast_spell(staff->value[3], staff->value[0], ch, vch, staff);
			}
			check_improve(ch, gsn_brandish);
		}
	}

	if (--staff->value[2] <= 0)
	{
		act( "Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR );
		act( "$n's $p blazes bright and is gone.", ch, staff, NULL, TO_ROOM );
		junk_obj( staff );
	}
	pop_call();
	return;
}


void do_zap( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *wand;
	OBJ_DATA *obj;

	push_call("do_zap(%p,%p)",ch,argument);

	if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_char( "You cannot do that here.\n\r", ch);
		pop_call();
		return;
	}

	if ((ch->fighting != NULL || ch->position == POS_FIGHTING))
	{
		send_to_char( "You may not do that while fighting.\n\r", ch);
		pop_call();
		return;
	}

	one_argument( argument, arg );

	if ((wand = get_eq_char(ch, WEAR_HOLD)) == NULL)
	{
		send_to_char( "You hold nothing in your hand.\n\r", ch );
		pop_call();
		return;
	}

	if (wand->item_type != ITEM_WAND)
	{
		send_to_char( "You can zap only with a wand.\n\r", ch );
		pop_call();
		return;
	}

	if (arg[0] == '\0' && ch->fighting == NULL)
	{
		send_to_char( "Zap whom or what?\n\r", ch );
		pop_call();
		return;
	}

	obj = NULL;

	if (arg[0] == '\0')
	{
		if (ch->fighting != NULL)
		{
			victim = who_fighting(ch);
		}
		else
		{
			send_to_char( "Zap whom or what?\n\r", ch );
			pop_call();
			return;
		}
	}
	else
	{
		if ((victim = get_char_room (ch, arg)) == NULL
		&&  (obj = get_obj_here(ch, arg)) == NULL)
		{
			send_to_char( "You cannot find it.\n\r", ch );
			pop_call();
			return;
		}
	}

	wait_state(ch, skill_table[gsn_zap].beats);

	if (wand->value[2] > 0)
	{
		if (victim != NULL)
		{
			act( "You zap $N with $p.", ch, wand, victim, TO_CHAR );
			act( "$n zaps $N with $p.", ch, wand, victim, TO_ROOM );
		}
		else
		{
			act( "You zap $P with $p.", ch, wand, obj, TO_CHAR );
			act( "$n zaps $P with $p.", ch, wand, obj, TO_ROOM );
		}

		if (number_percent() > learned(ch, gsn_zap))
		{
			act("Your efforts with $p produce only smoke and sparks.", ch, wand, NULL, TO_CHAR);
			act("$n's efforts with $p produce only smoke and sparks.", ch, wand, NULL, TO_ROOM);
		}
		else
		{
			obj_cast_spell(wand->value[3], wand->value[0], ch, victim, obj);
			check_improve(ch, gsn_zap);
		}
	}

	if (--wand->value[2] <= 0)
	{
		act( "Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR );
		act( "$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM );
		junk_obj( wand );
	}
	pop_call();
	return;
}

void do_steal( CHAR_DATA *ch, char *argument )
{
	char buf  [MAX_STRING_LENGTH];
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	int percent, lag;

	push_call("do_steal(%p,%p)",ch,argument);

	if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_char( "You cannot do that here.\n\r", ch);
		pop_call();
		return;
	}

	if (find_keeper(ch) != NULL)
	{
		send_to_char( "You cannot steal in a shop!\n\r", ch);
		pop_call();
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if (arg1[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char( "Steal what from whom?\n\r", ch );
		pop_call();
		return;
	}

	if ((victim = get_char_room(ch, arg2)) == NULL)
	{
		send_to_char( "They aren't here.\n\r", ch );
		pop_call();
		return;
	}

	if (victim == ch)
	{
		send_to_char( "That's pointless.\n\r", ch );
		pop_call();
		return;
	}

	if (victim->fighting || victim->position == POS_FIGHTING)
	{
		send_to_char( "Your victim is moving so fast, you cannot steal from them!\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_NPC(victim) && (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM)))
	{
		if (!can_attack( ch, victim ))
		{
			send_to_char( "You may not steal from them.\n\r", ch);
			pop_call();
			return;
		}

		if (victim->hit < victim->max_hit / 2)
		{
			send_to_char( "They are too suspicious to allow a theft right now.\n\r", ch);
			pop_call();
			return;
		}

	}

	if (IS_AFFECTED( victim, AFF_CHARM) )
	{
		send_to_char( "You cannot steal from someone's pet.\n\r", ch);
		pop_call();
		return;
	}

	if (check_recently_fought(ch, victim))
	{
		ch_printf(ch, "%s was recently fought.  Try later.\n\r", capitalize(victim->short_descr));
		pop_call();
		return;
	}

	if (!IS_NPC(victim) && number_percent() < learned(victim, gsn_guard))
	{
		wait_state(ch, skill_table[gsn_steal].beats * 2);

		send_to_char("They notice your attempt and brush your hand away.\n\r", ch);
		act("$n just tried to steal $t from you.\n\r", ch, arg1, victim, TO_VICT);
		check_improve(victim, gsn_guard);
		pop_call();
		return;
	}

	if (IS_AWAKE(victim))
	{
		percent = number_range(1, 200) - victim->distracted;
	}
	else
	{
		percent = number_range(1, 100);
	}

	if (percent > learned(ch, gsn_steal))
	{
		send_to_char( "Oops.\n\r", ch );
		act( "$n tried to steal from you.\n\r", ch, NULL, victim, TO_VICT    );
		act( "$n tried to steal from $N.\n\r",  ch, NULL, victim, TO_NOTVICT );

		lag = victim->wait;
		sprintf( buf, "%s is a bloody thief!", get_name( ch ) );
		do_shout( victim, buf );
		victim->wait = lag;

		wait_state(ch, skill_table[gsn_steal].beats * 2);

		if (!IS_NPC(ch))
		{
			if (IS_NPC(victim))
			{
				multi_hit( victim, ch, TYPE_UNDEFINED );
			}
			else if (!IS_NPC(ch))
			{
				if (!IS_SET(ch->act, PLR_THIEF))
				{
					SET_BIT(ch->act, PLR_THIEF);
					send_to_char( "*** You are now a THIEF!! ***\n\r", ch );
					save_char_obj(ch, NORMAL_SAVE);
				}

			}
		}
		pop_call();
		return;
	}

	wait_state(ch, skill_table[gsn_steal].beats);

	if ( !strcasecmp( arg1, "coin"  )
	||   !strcasecmp( arg1, "coins" )
	||   !strcasecmp( arg1, "gold"  ) )
	{
		int amount;

		amount = victim->gold / number_range(10, 100);

		if (amount <= 0)
		{
			send_to_char( "You couldn't get any gold.\n\r", ch );

			pop_call();
			return;
		}
		ch->gold     += amount;
		victim->gold -= amount;

		ch_printf(ch, "Bingo!  You got %d gold coins.\n\r", amount );

		if (!IS_NPC(victim) && number_percent() < victim->pcdata->learned[gsn_guard])
		{
			act("You notice $n taking $s hand out of your gold collection.", ch,NULL,victim,TO_VICT);
		}
		check_improve(ch, gsn_steal);
		pop_call();
		return;
	}

	if ((obj = get_obj_carry_even_invis(victim, arg1)) == NULL)
	{
		send_to_char( "You cannot find it.\n\r", ch );
		pop_call();
		return;
	}

	if ( !can_drop_obj( ch, obj )
	||   IS_SET(obj->extra_flags, ITEM_INVENTORY)
	||   obj->item_type == ITEM_CONTAINER
	||   obj->level > reinc_eq_level(ch))
	{
		send_to_char( "You cannot pry it away.\n\r", ch );
		pop_call();
		return;
	}

	if (ch->carry_number >= can_carry_n(ch))
	{
		send_to_char( "You have your hands full.\n\r", ch );
		pop_call();
		return;
	}

	if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
	{
		send_to_char( "You cannot carry that much weight.\n\r", ch );
		pop_call();
		return;
	}

	obj_from_char( obj );
	obj_to_char( obj, ch );
	send_to_char( "Ok.\n\r", ch );
	if (!IS_NPC(victim) && number_percent() < victim->pcdata->learned[gsn_guard])
	{
		act("You notice $n has your $p in $s hands.", ch,obj,victim,TO_VICT);
	}
	check_improve(ch, gsn_steal);
	pop_call();
	return;
}

/*
 * Shopping commands.
 */

CHAR_DATA *find_keeper( CHAR_DATA *ch )
{
	CHAR_DATA *keeper;

	push_call("find_keeper(%p)",ch);

	for (keeper = ch->in_room->first_person ; keeper ; keeper = keeper->next_in_room)
	{
		if (IS_NPC(keeper) && keeper->pIndexData->pShop)
		{
			break;
		}
	}
	pop_call();
	return keeper;
}



int get_cost( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy )
{
	SHOP_DATA *pShop;
	int cost;

	push_call("get_cost(%p,%p,%p)",keeper,obj,fBuy);

	pShop = keeper->pIndexData->pShop;

	if (fBuy)
	{
		cost = obj->cost * pShop->profit_buy  / 100;
	}
	else
	{
		OBJ_DATA *obj2;
		int itype;

		cost = 0;
		for (cost = itype = 0 ; itype < MAX_TRADE ; itype++)
		{
			if (obj->item_type == pShop->buy_type[itype])
			{
				cost = obj->cost * pShop->profit_sell / 100;
				break;
			}
		}

		for (obj2 = keeper->first_carrying ; obj2 ; obj2 = obj2->next_content)
		{
			if (obj->pIndexData == obj2->pIndexData)
			{
				cost = (cost*9)/10;
			}
		}
	}

	pop_call();
	return cost;
}

int bargain( CHAR_DATA *ch, int cost )
{
	push_call("bargain(%p,%p)",ch,cost);

	cost = cost * (100 - learned(ch, gsn_bargain) / 4) / 100;

	pop_call();
	return UMAX(1, cost);
}

void do_buy( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];

	push_call("do_buy(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	argument = one_argument(argument, arg);

	if (arg[0] == '\0')
	{
		send_to_char("Buy what?\n\r", ch);
		pop_call();
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP))
	{
		CHAR_DATA *keeper;
		CHAR_DATA *pet;
		ROOM_INDEX_DATA *pRoomIndexNext;
		ROOM_INDEX_DATA *in_room;

		if (IS_NPC(ch))
		{
			pop_call();
			return;
		}

		if ((keeper = find_keeper(ch)) == NULL)
		{
			send_to_char("There is nothing for sale here.\n\r",ch);
			pop_call();
			return;
		}

		pRoomIndexNext = get_room_index(ch->in_room->vnum + 1);

		if (pRoomIndexNext == NULL)
		{
			bug("Do_buy: bad pet shop at vnum %u.", ch->in_room->vnum);
			send_to_char("There is nothing for sale here.\n\r", ch);
			pop_call();
			return;
		}

		if (get_pets(ch) > 0)
		{
			ch_printf(ch, "%s%s tells you 'Sorry, only one pet at a time.'\n\r", get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(keeper->short_descr));
			pop_call();
			return;
		}
		in_room = ch->in_room;
		char_from_room(ch);
		char_to_room(ch, pRoomIndexNext->vnum);
		pet = get_char_room(ch, arg);
		char_from_room(ch);
		char_to_room(ch, in_room->vnum);

		if (pet == NULL || pet->reset == NULL || pet->reset->arg3 != pRoomIndexNext->vnum)
		{
			ch_printf(ch, "%s%s tells you 'Sorry, you cannot buy that here.'\n\r", get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(keeper->short_descr));
			pop_call();
			return;
		}

		if (ch->gold < bargain(ch, 100 + pet->level * pet->level))
		{
			ch_printf(ch, "%s%s tells you 'Sorry, you cannot afford that pet.'\n\r", get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(keeper->short_descr));
			pop_call();
			return;
		}

		if (ch->level < pet->level)
		{
			ch_printf(ch, "%s%s tells you 'Sorry, you're not ready for that pet.'\n\r", get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(keeper->short_descr));
			pop_call();
			return;
		}

		ch->gold -= bargain(ch, 100 + pet->level * pet->level);
		pet       = create_mobile( pet->pIndexData );
		SET_BIT(pet->affected_by, AFF_CHARM);

		argument = one_argument(argument, arg);

		sprintf(buf, "%sA neck tag says 'I belong to %s'.\n\r", pet->description, get_name(ch));
		STRFREE(pet->description );
		pet->description = STRALLOC( buf );

		char_to_room( pet, ch->in_room->vnum );
		add_follower( pet, ch );
		ch_printf(ch, "%s%s tells you 'Enjoy your pet.'\n\r", get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(keeper->short_descr));
		act( "$n bought $N as a pet.", ch, NULL, pet, TO_ROOM );

		check_improve(ch, gsn_bargain);
		pop_call();
		return;
	}
	else
	{
		CHAR_DATA *keeper;
		OBJ_DATA *obj, *found_obj;
		int cost, amount, total_cost, cnt;

		obj		= NULL;
		found_obj	= NULL;
		amount	= 1;

		if ((keeper = find_keeper(ch)) == NULL)
		{
			send_to_char("There is nothing to purchase here.\n\r", ch);
			pop_call();
			return;
		}

		/*
			handle morgue buying corpses
		*/

		if (!IS_NPC(ch) && IS_SET(ch->in_room->room_flags, ROOM_MORGUE) && is_name("corpse", arg))
		{
			if ((obj = find_char_corpse(ch, FALSE)) != NULL)
			{
				if (obj->in_room == ch->in_room)
				{
					ch_printf(ch,"%s%s tells you 'But you have already bought your corpse!'\n\r", get_color_string(ch, COLOR_SPEACH, VT102_BOLD), capitalize(keeper->short_descr));
					pop_call();
					return;
				}

				cost = bargain(ch, obj->value[3]);

				ch_printf(ch,"%s%s tells you 'That'll be %d gold coins.'\n\r",get_color_string(ch, COLOR_SPEACH, VT102_BOLD), capitalize(keeper->short_descr), cost);

				if (cost > ch->gold)
				{
					ch_printf(ch,"%s%s tells you 'I'm afraid you cannot afford to buy your corpse.'\n\r",get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(keeper->short_descr));
					pop_call();
					return;
				}

				ch->gold -= cost;

				act("$n disappears and returns with your corpse.", keeper, NULL, ch, TO_VICT);
				act("$n disappears and returns with $N's corpse.", keeper, NULL, ch, TO_NOTVICT);

				ch->pcdata->corpse_room = ch->in_room->vnum;
				obj_to_room(obj, ch->in_room->vnum);

				save_char_obj(ch, NORMAL_SAVE);

				pop_call();
				return;
			}
			else
			{
				ch_printf(ch, "%s%s tells you 'You don't have any corpses here, go make one!'\n\r",get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(keeper->short_descr));

				pop_call();
				return;
			}
		}

		if (is_number(arg) && argument[0] != '\0')
		{
			amount = UMAX(1, atol(arg));

			strcpy(arg, argument);
		}

		if ((obj = get_obj_carry_keeper(keeper, arg, ch)) == NULL)
		{
			ch_printf(ch, "%s%s tells you 'I cannot sell you a %s.'\n\r",get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(keeper->short_descr), arg);
			pop_call();
			return;
		}

		if (mud->time_info->hour < keeper->pIndexData->pShop->open_hour
		||  mud->time_info->hour > keeper->pIndexData->pShop->close_hour)
		{
			int open  = keeper->pIndexData->pShop->open_hour;
			int close = keeper->pIndexData->pShop->close_hour;

			ch_printf(ch, "%s%s tells you 'Sorry, my shop is closed.'\n\r", get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(keeper->short_descr));
			ch_printf(ch, "%s%s tells you 'I'll be open from %d %s till %d %s.'\n\r", get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(keeper->short_descr),
				open  % 12 != 0 ? open  % 12 : 12, open  >= 12 ? "pm" : "am",
				close % 12 != 0 ? close % 12 : 12, close >= 12 ? "pm" : "am");
			pop_call();
			return;
		}

		if ( !can_see( keeper, ch ) )
		{
			sprintf(buf, "%s%s says 'I don't trade with folks I cannot see.'", get_color_code(ch, COLOR_SPEACH, VT102_DIM), capitalize(keeper->short_descr));
			act(buf, keeper, NULL, NULL, TO_ROOM);

			pop_call();
			return ;
		}

		cost = bargain(ch, get_cost(keeper, obj, TRUE));

		if (ch->gold < cost)
		{
			ch_printf(ch, "%s%s tells you 'You cannot afford to buy %s.'\n\r", get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(keeper->short_descr), obj->short_descr);
			pop_call();
			return;
		}

		if (obj->level > reinc_eq_level(ch))
		{
			ch_printf(ch, "%s%s tells you 'You cannot use %s yet.'\n\r", get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(keeper->short_descr), obj->short_descr);
			pop_call();
			return;
		}

		if (ch->carry_number >= can_carry_n(ch))
		{
			send_to_char("You cannot carry that many items.\n\r", ch);
			pop_call();
			return;
		}

		if (ch->carry_weight + get_obj_weight(obj) > can_carry_w(ch))
		{
			send_to_char("You cannot carry that much weight.\n\r", ch);
			pop_call();
			return;
		}

		found_obj = obj;

		for (total_cost = cnt = 0 ; cnt < amount ; cnt++)
		{
			if ((obj = get_obj_carry_keeper(keeper, arg, ch)) == NULL)
			{
				break;
			}

			if (ch->gold < total_cost + cost)
			{
				break;
			}

			if (ch->carry_number >= can_carry_n(ch))
			{
				break;
			}

			if (ch->carry_weight + get_obj_weight(obj) > can_carry_w(ch))
			{
				break;
			}

			if (obj->reset)
			{
				obj = create_object(obj->pIndexData, 0);
			}
			obj_to_char(obj, ch);
			total_cost += cost;
		}
		if (amount == 1)
		{
			ch_printf(ch, "You buy %s for %d gold.\n\r", obj->short_descr, total_cost);
			act("$n buys $p.", ch, found_obj, NULL, TO_ROOM);
		}
		else
		{
			ch_printf(ch, "You buy %d %s for %d gold.\n\r", cnt, short_to_name(found_obj->short_descr, cnt), total_cost);
			sprintf(buf, "$n buys %d %s.", cnt, short_to_name(found_obj->short_descr, cnt));
			act(buf, ch, found_obj, NULL, TO_ROOM);
		}
		ch->gold -= total_cost;

		check_improve(ch, gsn_bargain);
		pop_call();
		return;
	}
}

void do_list( CHAR_DATA *ch, char *argument )
{
	push_call("do_list(%p,%p)",ch,argument);

	if (IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP))
	{
		ROOM_INDEX_DATA *pRoomIndexNext;
		CHAR_DATA *pet;
		CHAR_DATA *keeper;
		bool found;

		if ((keeper = find_keeper(ch)) == NULL)
		{
			send_to_char( "There is nothing to purchase here.\n\r", ch);
			pop_call();
			return;
		}

		pRoomIndexNext = get_room_index(ch->in_room->vnum + 1);

		if (pRoomIndexNext == NULL)
		{
			bug("do_list: bad pet shop at vnum %u.", ch->in_room->vnum);
			send_to_char("You cannot do that here.\n\r", ch);
			pop_call();
			return;
		}

		found = FALSE;

		for (pet = pRoomIndexNext->first_person ; pet ; pet = pet->next_in_room)
		{
			if (pet->reset != NULL && pet->reset->arg3 == pRoomIndexNext->vnum)
			{
				if (!found )
				{
					found = TRUE;
					ch_printf_color(ch, "{178}[{078}Level {018}Price  {178}] {138}Pet\n\r");
				}
				ch_printf_color(ch, "{178}[{078}%2d {018}%8d  {178}] {138}%s.\n\r",
					pet->level,
					bargain(ch, 100 + pet->level * pet->level),
					capitalize(pet->short_descr));
			}
		}
		if (!found)
		{
			ch_printf(ch, "%s%s tells you 'Sorry, I'm all out of pets right now.\n\r", get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(keeper->short_descr));
		}
		pop_call();
		return;
	}
	else
	{
		char arg1[MAX_INPUT_LENGTH];
		char arg2[MAX_INPUT_LENGTH];
		CHAR_DATA *keeper;
		OBJ_DATA  *obj, *oobj;
		int nrL, nrH, cost;
		bool found;

		cost  = 0;
		found = FALSE;

		argument = one_argument( argument, arg1 );
		argument = one_argument( argument, arg2 );

		if ((keeper = find_keeper(ch)) == NULL)
		{
			send_to_char( "There is nothing to purchase here.\n\r", ch);
			pop_call();
			return;
		}

		if (!strcasecmp(arg1, "all"))
		{
			nrL = 1;
			nrH = 150;
		}
		else
		{
			nrL = 1;
			nrH = reinc_eq_level(ch);
		}

		if (is_number(arg1))
		{
			nrL = URANGE(1, atol(arg1), 95);

			if (is_number(arg2))
			{
				nrH = URANGE(nrL, atol(arg2), 95);
			}
		}


		for (obj = keeper->first_carrying ; obj ; obj = obj->next_content)
		{
			if (obj->wear_loc == WEAR_NONE
			&&  can_see_obj(ch, obj)
			&&  obj->level >= nrL
			&&  obj->level <= nrH
			&& (cost = bargain(ch, get_cost(keeper, obj, TRUE))) > 0)
			{
				if (!found)
				{
					found = TRUE;
					ch_printf_color(ch, "{178}     [{078}Level {018}Price  {178}] {138}Item\n\r");
				}

				for (oobj = keeper->first_carrying ; oobj != obj ; oobj = oobj->next_content)
				{
					if (oobj->wear_loc == WEAR_NONE
					&&  can_see_obj(ch, oobj)
					&&  oobj->level >= nrL
					&&  oobj->level <= nrH
					&&  get_cost(keeper, oobj, TRUE) > 0)
					{
						if (oobj->pIndexData->vnum == obj->pIndexData->vnum && !IS_SET(obj->extra_flags, ITEM_MODIFIED))
						{
							break;
						}
					}
				}
				if (obj == oobj)
				{
					ch_printf_color(ch, "{178}     [{078}%2d {018}%8d  {178}]{138} %s.\n\r",
						obj_eq_level(ch, obj), cost, capitalize(obj->short_descr));
				}
			}
		}

		if (IS_SET(ch->in_room->room_flags, ROOM_MORGUE) && (obj = find_char_corpse(ch, FALSE)) != NULL)
		{
			if (!found)
			{
				ch_printf_color(ch, "{178}     [{078}Lv    {018}Price  {178}] {138}Item\n\r");
			}
			ch_printf_color(ch, "{178}     [{078}%2d {018}%8d  {178}] {138}%s.\n\r", obj->level, bargain(ch, obj->value[3]), capitalize(obj->short_descr));
		}
		else	if (!found)
		{
			ch_printf_color(ch, "%s%s tells you 'Sorry, I have nothing here you would want.'\n\r", get_color_string(ch, COLOR_SPEACH, VT102_BOLD), capitalize(keeper->short_descr));
		}
		pop_call();
		return;
	}
}

void do_sell( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *keeper;
	OBJ_DATA *obj;
	int cost;
	int sell_bargain;

	push_call("do_sell(%p,%p)",ch,argument);

	sell_bargain = 100 + learned(ch, gsn_bargain) / 4;

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Sell what?\n\r", ch );

		pop_call();
		return;
	}

	if ((keeper = find_keeper(ch)) == NULL)
	{
		send_to_char("You cannot do that here.\n\r", ch);
		pop_call();
		return;
	}

	if ((obj = get_obj_carry(ch,arg)) == NULL)
	{
		ch_printf(ch, "%s%s tells you 'You don't have that item.'\n\r", get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(keeper->short_descr));
		pop_call();
		return;
	}

	if (mud->time_info->hour < keeper->pIndexData->pShop->open_hour
	||  mud->time_info->hour > keeper->pIndexData->pShop->close_hour)
	{
		ch_printf(ch, "%s%s tells you 'Sorry, my shop is closed.'\n\r", get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(keeper->short_descr));
		ch_printf(ch, "%s%s tells you 'I'll be open from %d %s till %d %s.'\n\r", get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(keeper->short_descr),
			keeper->pIndexData->pShop->open_hour %12, keeper->pIndexData->pShop->open_hour  < 11 ? "am" : "pm",
			keeper->pIndexData->pShop->close_hour%12, keeper->pIndexData->pShop->close_hour < 11 ? "am" : "pm");
		pop_call();
		return;
	}

	if (!can_see(keeper, ch))
	{
		sprintf(buf, "%s%s says 'I don't trade with folks I cannot see!'", get_color_code(ch, COLOR_SPEACH, VT102_DIM), capitalize(keeper->short_descr));
		act(buf, keeper, NULL, NULL, TO_ROOM);
		pop_call();
		return ;
	}

	if (!can_drop_obj(ch, obj))
	{
		send_to_char( "You cannot let go of it.\n\r", ch );
		pop_call();
		return;
	}

	if (obj->owned_by)
	{
		send_to_char( "You cannot sell personalized equipment.\n\r", ch );
		pop_call();
		return;
	}

	if ((cost = sell_bargain * get_cost(keeper, obj, FALSE) / 100) < 1)
	{
		act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
		pop_call();
		return;
	}

	if (IS_SET(obj->extra_flags, ITEM_MODIFIED))
	{
		act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
		pop_call();
		return;
	}
	sprintf(buf, "You sell $p for %d gold %s.", cost, short_to_name("coin", cost));
	act( buf, ch, obj, NULL, TO_CHAR );
	act( "$n sells $p.", ch, obj, NULL, TO_ROOM );

	give_gold(ch, cost);

	obj_to_char(obj, keeper);

	check_improve(ch, gsn_bargain);
	pop_call();
	return;
}


void do_value( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *keeper;
	OBJ_DATA *obj;
	int cost;
	int sell_bargain;

	push_call("do_value(%p,%p)",ch,argument);

	sell_bargain = 100 + learned(ch, gsn_bargain) / 4;

	one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		send_to_char("Value what?\n\r", ch);
		pop_call();
		return;
	}

	if ((keeper = find_keeper(ch)) == NULL)
	{
		send_to_char("You cannot do that here.\n\r",ch);
		pop_call();
		return;
	}

	if ((obj = get_obj_carry(ch, arg)) == NULL)
	{
		ch_printf(ch, "%s%s tells you 'You don't have that item.'\n\r", get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(keeper->short_descr));
		pop_call();
		return;
	}

	if (!can_drop_obj(ch, obj))
	{
		send_to_char( "You cannot let go of it.\n\r", ch );
		pop_call();
		return;
	}

	if ((cost = sell_bargain * get_cost(keeper, obj, FALSE) / 100) < 1 || IS_SET(obj->extra_flags, ITEM_MODIFIED))
	{
		act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
		pop_call();
		return;
	}
	ch_printf(ch, "%s%s tells you 'I'll give you %d gold coins for %s.'\n\r", get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(keeper->short_descr), cost, obj->short_descr);
	pop_call();
	return;
}

void do_evaluate( CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char msg [MAX_INPUT_LENGTH];
	CHAR_DATA *keeper;
	OBJ_DATA *obj1;
	OBJ_DATA *obj2;
	int value1, value2;


	push_call("do_evaluate(%p,%p)",ch,argument);

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ((keeper = find_keeper(ch)) == NULL)
	{
		send_to_char("You cannot do that here.\n\r", ch);
		pop_call();
		return;
	}

	if ( arg1[0] == '\0' )
	{
		send_to_char( "Evaluate what to what?\n\r", ch );
		pop_call();
		return;
	}

	if ((obj1 = get_obj_carry_keeper(keeper, arg1, ch)) == NULL)
	{
		ch_printf(ch, "%s%s tells you 'I don't have a %s.'\n\r", get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(keeper->short_descr), arg1);
		pop_call();
		return;
	}

	if (arg2[0] == '\0')
	{
		for (obj2 = ch->first_carrying ; obj2 ; obj2 = obj2->next_content)
		{
			if (obj2->wear_loc != WEAR_NONE
			&& can_see_obj(ch, obj2)
			&& obj1->wear_flags == obj2->wear_flags)
			{
				break;
			}
		}

		if (obj2 == NULL)
		{
			ch_printf(ch, "%s%s tells you 'You aren't wearing anything similar.'\n\r", get_color_string(ch, COLOR_SPEACH, VT102_BOLD), capitalize(keeper->short_descr));
			pop_call();
			return;
		}
	}
	else
	{
		if ((obj2 = get_obj_carry(ch, arg2)) == NULL)
		{
			send_to_char("You do not have that item.\n\r", ch);
			pop_call();
			return;
		}
	}

	value1 = obj_level_estimate(obj1->pIndexData);
	value2 = obj_level_estimate(obj2->pIndexData);

	if ( value1 == value2 )
	{
		sprintf(msg, "%s and %s look about the same.", obj1->short_descr, obj2->short_descr);
	}
	else if ( value1  > value2 )
	{
		sprintf(msg, "%s looks better than %s.", obj1->short_descr, obj2->short_descr);
	}
	else
	{
		sprintf(msg, "%s looks worse than %s.", obj1->short_descr, obj2->short_descr);
	}

	ch_printf(ch, "%s%s tells you '%s'\n\r", get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(keeper->short_descr), msg);

	pop_call();
	return;
}

void do_snatch( CHAR_DATA *ch, char *argument )
{
	bool snatched;
	int cnt;

	push_call("do_snatch(%p,%p)",ch,argument);

	if (!check_blind(ch))
	{
		pop_call();
		return;
	}

	cnt = multi( ch, gsn_snatch );

	if ( cnt == -1 )
	{
		send_to_char( "You cannot snatch things.\n\r", ch );
		pop_call();
		return;
	}

	if (argument[0] == '\0')
	{
		send_to_char("Snatch what?\n\r",ch);
		pop_call();
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_char( "You cannot do that here.\n\r", ch);
		pop_call();
		return;
	}
	if (multi(ch,gsn_snatch)==-1 && !IS_NPC(ch))
	{
		pop_call();
		return;
	}
	snatched = number_percent() < learned(ch, gsn_snatch);

	if (snatched)
	{
		SET_BIT(mud->flags, MUD_SKIPOUTPUT);
	}

	do_get(ch, argument);

	if (snatched)
	{
		REMOVE_BIT(mud->flags, MUD_SKIPOUTPUT);
		ch_printf(ch, "Your snatch was unseen by human eyes.\n\r");
		check_improve(ch, gsn_snatch);
	}
	pop_call();
	return;
}

void do_identify( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];

	push_call("do_identify(%p,%p)",ch,argument);

	argument = one_argument( argument, arg );

	{
		CHAR_DATA *keeper;
		OBJ_DATA *obj;
		int cost;

		if ((keeper = find_keeper(ch)) == NULL)
		{
			send_to_char( "You must be in a shop to have items identified.\n\r", ch );
			pop_call();
			return;
		}

		if (arg[0] == '\0')
		{
			send_to_char( "Have what identified?\n\r", ch );
			pop_call();
			return;
		}

		if (ch->in_room->vnum == ROOM_VNUM_IDENTIFY)
		{
			if ((obj = get_obj_here(ch, arg)) == NULL)
			{
				ch_printf(ch, "%s%s tells you 'You don't seem to have a '%s' on you.'\n\r", get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(keeper->short_descr), arg);
				pop_call();
				return;
			}
		}
		else
		{
			if ((obj = get_obj_carry_keeper(keeper, arg, ch)) == NULL)
			{
				ch_printf(ch, "%s%s tells you 'I cannot identify a %s for you.'\n\r", get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(keeper->short_descr), arg);
				pop_call();
				return;
			}
		}

		cost = bargain(ch, get_cost(keeper, obj, TRUE));

		if (cost < 1)
		{
			ch_printf(ch, "%s%s tells you 'I cannot identify a %s for you.'\n\r", get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(keeper->short_descr), arg);
			pop_call();
			return;
		}

		cost = URANGE(10, cost / 20, 100);

		if (ch->gold < cost)
		{
			ch_printf(ch, "%s%s tells you 'It costs you %d gold to identify %s.'\n\r", get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(keeper->short_descr), cost, obj->short_descr);
			pop_call();
			return;
		}

		if (obj->level > reinc_eq_level(ch))
		{
			ch_printf(ch, "%s%s tells you 'You cannot use %s yet.'\n\r", get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(keeper->short_descr), obj->short_descr);
			pop_call();
			return;
		}
		act( "You have $p identified.", ch, obj, NULL, TO_CHAR );
		act( "$n has $p identified.",   ch, obj, NULL, TO_ROOM );

		ch->gold     -= cost;
		keeper->gold += cost;

		spell_identify(0, keeper->level, ch, obj, TAR_OBJ_INV);

		pop_call();
		return;
	}
}

OBJ_DATA * find_char_corpse (CHAR_DATA *ch, bool method)
{
	OBJ_DATA *corpse;

	push_call("find_char_corpse(%p,%p)",ch,method);

	/* find the player's first corpse with objects in it */

	if (method)
	{
		for (corpse = obj_index[OBJ_VNUM_CORPSE_PC]->first_instance ; corpse ; corpse = corpse->next_instance)
		{
			if (corpse->first_content
			&&  corpse->owned_by == ch->pcdata->pvnum
			&& !IS_SET(corpse->extra_flags, ITEM_NOT_VALID))
			{
				break;
			}
		}
	}
	else
	{
		corpse = ch->pcdata->corpse;
	}

	if (corpse)
	{
		OBJ_DATA *obj;

		corpse->cost = ch->level;

		for (obj = corpse->first_content ; obj ; obj = obj->next_content)
		{
			corpse->cost += obj->cost / 10;
		}
	}
	pop_call();
	return corpse;
}

void do_engrave( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	AFFECT_DATA *paf;
	int cost, stat;

	push_call("do_engrave(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (ch->in_room->vnum != ROOM_VNUM_BLACKSMITH)
	{
		send_to_char( "You are not at the blacksmith.\n\r", ch);

		pop_call();
		return;
	}

	if (argument[0] == '\0')
	{
		send_to_char( "You must engrave an object.\n\r", ch);
		pop_call();
		return;
	}

	obj = get_obj_here(ch, argument);

	if (obj == NULL)
	{
		send_to_char( "You do not have that object.\n\r", ch);
		pop_call();
		return;
	}

	if (obj->owned_by)
	{
		send_to_char( "You cannot engrave that object.\n\r", ch);
		pop_call();
		return;
	}

	stat = 1;
	cost = 80;

	if (ch->level > 25)
	{
		stat = 2;
		cost = 400;
	}
	if (ch->level > 50)
	{
		stat = 3;
		cost = 2000;
	}
	if (ch->level > 75)
	{
		stat = 4;
		cost = 10000;
	}

	cost = bargain(ch, cost);

	if (ch->gold < cost)
	{
		ch_printf(ch, "It costs %d gold coins to engrave an object.\n\r", cost);
		pop_call();
		return;
	}

	switch (obj->item_type)
	{
		case ITEM_ARMOR:
			ALLOCMEM(paf, AFFECT_DATA, 1);
			paf->location  = APPLY_AC;
			paf->modifier  = 0 - stat;
			paf->type      = skill_lookup( "enhance object" );
			paf->duration  = -1;
			paf->bitvector = 0;
			LINK( paf, obj->first_affect, obj->last_affect, next, prev );
			break;

		case ITEM_WEAPON:
			ALLOCMEM(paf, AFFECT_DATA, 1);
			paf->location  = APPLY_HITROLL;
			paf->modifier  = stat;
			paf->type      = skill_lookup( "enhance object" );
			paf->duration  = -1;
			paf->bitvector = 0;
			LINK( paf, obj->first_affect, obj->last_affect, next, prev );
			break;

		case ITEM_CONTAINER:
			obj->value[0]	+= stat * 100;
			obj->value[3]	+= obj->value[3] ? stat * 10 : MAX_OBJECTS_IN_CONTAINER + stat * 10;
			break;

		default:
			send_to_char( "You cannot engrave that kind of an object.\n\r", ch);
			pop_call();
			return;
	}
	SET_BIT(obj->extra_flags, ITEM_MODIFIED);
	obj->owned_by = ch->pcdata->pvnum;
	ch->gold -= cost;

	sprintf( arg, "%s of %s", obj->pIndexData->short_descr, capitalize( ch->name ));

	STRFREE(obj->short_descr );
	obj->short_descr = STRALLOC( arg );

	char_reset( ch );

	send_to_char( "It is engraved.\n\r", ch);

	pop_call();
	return;
}

int has_wear_location(OBJ_DATA *obj, int location)
{
	push_call("wear_flags_to_wear_loc(%p,%p)",obj,location);

	if (location == WEAR_LIGHT || location == -1)
	{
		if (obj->item_type == ITEM_LIGHT)
		{
			pop_call();
			return ITEM_WEAR_HOLD;
		}
	}

	if (location == WEAR_FINGER_L || location == WEAR_FINGER_R || location == -1)
	{
		if (HAS_BIT(obj->wear_flags, ITEM_WEAR_FINGER))
		{
			pop_call();
			return ITEM_WEAR_FINGER;
		}
	}

	if (location == WEAR_NECK_1 || location == WEAR_NECK_2 || location == -1)
	{
		if (HAS_BIT(obj->wear_flags, ITEM_WEAR_NECK))
		{
			pop_call();
			return ITEM_WEAR_NECK;
		}
	}

	if (location == WEAR_BODY || location == -1)
	{
		if (HAS_BIT(obj->wear_flags, ITEM_WEAR_BODY))
		{
			pop_call();
			return ITEM_WEAR_BODY;
		}
	}

	if (location == WEAR_HEAD || location == -1)
	{
		if (HAS_BIT(obj->wear_flags, ITEM_WEAR_HEAD))
		{
			pop_call();
			return ITEM_WEAR_HEAD;
		}
	}

	if (location == WEAR_LEGS || location == -1)
	{
		if (HAS_BIT(obj->wear_flags, ITEM_WEAR_LEGS))
		{
			pop_call();
			return ITEM_WEAR_LEGS;
		}
	}

	if (location == WEAR_FEET || location == -1)
	{
		if (HAS_BIT(obj->wear_flags, ITEM_WEAR_FEET))
		{
			pop_call();
			return ITEM_WEAR_FEET;
		}
	}

	if (location == WEAR_HANDS || location == -1)
	{
		if (HAS_BIT(obj->wear_flags, ITEM_WEAR_HANDS))
		{
			pop_call();
			return ITEM_WEAR_HANDS;
		}
	}

	if (location == WEAR_ARMS || location == -1)
	{
		if (HAS_BIT(obj->wear_flags, ITEM_WEAR_ARMS))
		{
			pop_call();
			return ITEM_WEAR_ARMS;
		}
	}

	if (location == WEAR_SHIELD || location == -1)
	{
		if (HAS_BIT(obj->wear_flags, ITEM_WEAR_SHIELD))
		{
			pop_call();
			return ITEM_WEAR_SHIELD;
		}
	}

	if (location == WEAR_ABOUT || location == -1)
	{
		if (HAS_BIT(obj->wear_flags, ITEM_WEAR_ABOUT))
		{
			pop_call();
			return ITEM_WEAR_ABOUT;
		}
	}

	if (location == WEAR_WAIST || location == -1)
	{
		if (HAS_BIT(obj->wear_flags, ITEM_WEAR_WAIST))
		{
			pop_call();
			return ITEM_WEAR_WAIST;
		}
	}

	if (location == WEAR_WRIST_L || location == WEAR_WRIST_R || location == -1)
	{
		if (HAS_BIT(obj->wear_flags, ITEM_WEAR_WRIST))
		{
			pop_call();
			return ITEM_WEAR_WRIST;
		}
	}

	if (location == WEAR_WIELD || location == -1)
	{
		if (HAS_BIT(obj->wear_flags, ITEM_WEAR_WIELD))
		{
			pop_call();
			return ITEM_WEAR_WIELD;
		}
	}

	if (location == WEAR_DUAL_WIELD || location == -1)
	{
		if (HAS_BIT(obj->wear_flags, ITEM_WEAR_WIELD))
		{
			pop_call();
			return ITEM_WEAR_WIELD;
		}
	}

	if (location == WEAR_HOLD || location == -1)
	{
		if (HAS_BIT(obj->wear_flags, ITEM_WEAR_HOLD))
		{
			pop_call();
			return ITEM_WEAR_HOLD;
		}
	}
	pop_call();
	return 0;
}

void do_plant( CHAR_DATA *ch, char *argument )
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA  *obj;

	push_call("do_plant(%p,%p)",ch,argument);

	if (!IS_NPC(ch) && who_fighting(ch) && !IS_NPC(ch->fighting->who))
	{
		send_to_char( "You are too busy to plant anything on someone.\n\r", ch );
		pop_call();
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if (arg1[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char( "Plant what on whom?\n\r", ch );
		pop_call();
		return;
	}

	if (learned(ch, gsn_plant) == 0)
	{
		send_to_char( "They would just get mad at you.\n\r", ch);
		pop_call();
		return;
	}

	if ((obj = get_obj_carry(ch, arg1)) == NULL)
	{
		send_to_char( "You do not have that item.\n\r", ch );
		pop_call();
		return;
	}

	if (obj->wear_loc != WEAR_NONE)
	{
		send_to_char( "You must remove it first.\n\r", ch );
		pop_call();
		return;
	}

	if ((victim = get_char_room(ch, arg2)) == NULL)
	{
		send_to_char( "They aren't here.\n\r", ch );
		pop_call();
		return;
	}

	if (!can_drop_obj(ch, obj))
	{
		send_to_char( "You cannot let go of it.\n\r", ch );
		pop_call();
		return;
	}

	if (find_keeper(ch) == victim)
	{
		send_to_char( "You may only sell items to a shop keeper.\n\r", ch );
		pop_call();
		return;
	}

	if (victim->carry_weight + get_obj_weight(obj) > can_carry_w(victim))
	{
		act( "$N cannot carry that much weight.", ch, NULL, victim, TO_CHAR );
		pop_call();
		return;
	}

	if (!can_attack(ch, victim))
	{
		send_to_char( "You should leave them alone.\n\r", ch);
		pop_call();
		return;
	}

	obj_from_char( obj );
	obj_to_char( obj, victim );

	act( "You attempt to plant $p on $N.", ch, obj, victim, TO_CHAR    );

	if (number_percent() > learned(ch, gsn_plant))
	{
		act( "$n tries to plant $p on $N.", ch, obj, victim, TO_NOTVICT );
		act( "$n plants $p on you!",   ch, obj, victim, TO_VICT    );
		send_to_char("ACK!!!  You were seen!\n\r",ch);
	}
	else
	{
		check_improve(ch, gsn_plant);
	}
	pop_call();
	return;
}

void do_forge( CHAR_DATA *ch, char *argument )
{
	char arg1 [MAX_INPUT_LENGTH];
	OBJ_DATA  *obj;

	push_call("do_forge(%p,%p)",ch,argument);

	if (!IS_NPC(ch) && who_fighting(ch) && !IS_NPC( ch->fighting->who ))
	{
		send_to_char( "You are too busy to forge anything!\n\r", ch );
		pop_call();
		return;
	}

	argument = one_argument( argument, arg1 );

	if (strlen(argument) > 50)
	{
		*(argument+50) = '\0';
	}

	if ( arg1[0] == '\0' || argument[0] == '\0' )
	{
		send_to_char( "Forge what item into what?\n\r", ch );
		pop_call();
		return;
	}

	if ((obj = get_obj_carry(ch, arg1)) == NULL)
	{
		send_to_char( "You do not have that item.\n\r", ch );
		pop_call();
		return;
	}

	wait_state( ch, skill_table[gsn_forge].beats );

	if (obj->owned_by)
	{
		send_to_char("You cannot forge personalized equipment.\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && number_percent() >= learned(ch, gsn_forge))
	{
		if (number_percent() < 50)
		{
			act("Your attempt at forgery has destroyed $p.",ch,obj,NULL,TO_CHAR);
			junk_obj(obj);
		}
		else
		{
			act("Your attempt at forgery has failed.",ch,obj,NULL,TO_CHAR);
		}
		wait_state( ch, skill_table[gsn_forge].beats );
	}
	else if (IS_OBJ_STAT(obj, ITEM_FORGERY))
	{
		send_to_char("That object is already a forgery!\n\r",ch);
	}
	else
	{
		int i;
		smash_tilde( argument );

		if (!strcasecmp(argument, "all"))
		{
			send_to_char( "You cannot forge something into that.\n\r", ch);
			pop_call();
			return;
		}

		if (!str_infix(" all", argument) || !str_infix("all ", argument))
		{
			send_to_char( "You cannot forge something into that.\n\r", ch);
			pop_call();
			return;
		}

		/*
			only allowing alphas now - Scandum
		*/

		for (i = 0 ; argument[i] != '\0' ; i++)
		{
			if (ispunct(argument[i]) || isdigit(argument[i]))
			{
				send_to_char("You cannot forge something into that.\n\r", ch );
				log_printf("Bad forge attempt by %s: %s", ch->name, argument);
				pop_call();
				return;
			}
		}
		wait_state( ch, skill_table[gsn_forge].beats );
		/*
			Place i<vnum> back in the name list - Scandum
		*/
		sprintf(arg1, "%s i%u", argument, obj->pIndexData->vnum);

		RESTRING(obj->name, arg1);

		RESTRING(obj->short_descr, argument);

		sprintf(arg1, "%s is here.", capitalize(argument));
		RESTRING(obj->long_descr, arg1);

		RESTRING(obj->description, "");

		SET_BIT(obj->extra_flags,ITEM_FORGERY);
		send_to_char("You sit down, take your time...and make a decent forgery.\n\r",ch);
		check_improve(ch, gsn_forge);
	}
  	pop_call();
	return;
}

void do_make_poison( CHAR_DATA *ch, char *argument )
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	OBJ_DATA  *obj;
	int idmgL, idmgH, cdmgL, cdmgH, cdurL, cdurH, ptype, level;
	POISON_DATA *pd;

	push_call("do_make_poison(%p,%p)",ch,argument);

	idmgL = idmgH = cdmgL = cdmgH = cdurL = cdurH = 0;

	if (!IS_NPC(ch) && multi(ch, gsn_make_poison) == -1)
	{
		send_to_char( "You never learned to make poisons.\n\r", ch );
		pop_call();
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if (*arg1 != 'm' && *arg1 != 'c')
	{
		send_to_char( "Make what kind of poison?  Clear or murky?\n\r", ch );
		pop_call();
		return;
	}

	if (*arg2 != 'p' && *arg2 != 'b' && *arg2 != 'g')
	{
		send_to_char( "Make what color of poison?  Blue, purple, or green?\n\r", ch );
		pop_call();
		return;
	}

	if ((obj = get_obj_wear(ch, argument)) == NULL)
	{
		if ((obj = get_obj_carry(ch, argument)) == NULL)
		{
			send_to_char( "You do not have that item.\n\r", ch );
			pop_call();
			return;
		}
	}

	if (obj->poison != NULL)
	{
		send_to_char( "That object is already poisoned.\n\r", ch );
		pop_call();
		return;
	}

	level = UMIN(obj->level, multi_skill_level(ch, gsn_make_poison));

	/*
		(Reminder) shoot and eat don't check poison yet - Scandum
	*/

	if (obj->item_type != ITEM_WEAPON && obj->item_type != ITEM_AMMO)
	{
		send_to_char( "That item is not a weapon or arrow.\n\r", ch );
		pop_call();
		return;
	}

	wait_state( ch, skill_table[gsn_make_poison].beats );

	if (*arg2 == 'b')
	{
		ptype ='B';
		idmgL = 0;
		idmgH = 0;
		cdmgL = level/3;
		cdmgH = 2*level/3;
		cdurL = 4;
		cdurH = 8;
	}
	else if (*arg2 == 'p')
	{
		ptype ='P';
		idmgL = 3*level/2;
		idmgH = 3*level;
		cdmgL = 0;
		cdmgH = 0;
		cdurL = 0;
		cdurH = 0;
	}
	else
	{
		ptype ='G';
		idmgL = level;
		idmgH = 3*level/2;
		cdmgL = level/5;
		cdmgH = level/2;
		cdurL = 2;
		cdurH = 4;
	}

	if (number_percent() > learned(ch, gsn_make_poison))
	{
		send_to_char( "You poison yourself!\n\r", ch );
		ALLOCMEM(pd, POISON_DATA, 1);
		pd->next				= ch->poison;
		ch->poison			= pd;
		pd->for_npc			= FALSE;
		pd->poison_type		= ptype;
		pd->instant_damage_low	= idmgL;
		pd->instant_damage_high	= idmgH;
		pd->constant_damage_low	= cdmgL;
		pd->constant_damage_high	= cdmgH;
		pd->constant_duration	= number_range( cdurL, cdurH );
		pd->poisoner			= ch->pcdata->pvnum;
		pd->owner				= ch->pcdata->pvnum;

		pop_call();
		return;
	}

	if (obj->item_type == ITEM_AMMO)
	{
		idmgL /= 2;
		idmgH /= 2;
		cdmgL /= 3;
		cdmgH /= 3;
		cdurL /= 2;
		cdurH /= 2;
	}

	ch_printf(ch, "You make a %s %s poison and place it on %s.\n\r",
		(*arg1 == 'c') ? "clear" : "murky",
		(*arg2 == 'p') ? "purple" : (*arg2 == 'g') ? "green" : "blue",
		obj->short_descr );

	SET_BIT(obj->extra_flags, ITEM_MODIFIED);

	ALLOCMEM(pd, POISON_DATA, 1);
	pd->next				= NULL;
	obj->poison			= pd;

	pd->for_npc			= (*arg1 == 'c') ? FALSE : TRUE;
	pd->poison_type		= ptype;
	pd->instant_damage_low	= idmgL;
	pd->instant_damage_high	= idmgH;
	pd->constant_damage_low	= cdmgL;
	pd->constant_damage_high	= cdmgH;
	pd->constant_duration	= number_range( cdurL, cdurH );
	pd->owner				= IS_NPC(ch) ? 0 : ch->pcdata->pvnum;
	pd->poisoner			= IS_NPC(ch) ? 0 : ch->pcdata->pvnum;

	check_improve(ch, gsn_make_poison);

	pop_call();
	return;
}

void do_make_flash( CHAR_DATA *ch, char *argument )
{
	push_call("do_make_flash(%p,%p)",ch,argument);

	if (!IS_NPC(ch) && multi(ch, gsn_flash_powder) == -1)
	{
		send_to_char( "You cannot make flash powder.\n\r", ch );
		pop_call();
		return;
	}

	if (IS_AFFECTED(ch, AFF2_HAS_FLASH))
	{
		send_to_char( "You already have some flash powder hidden away.\n\r",ch);
		pop_call();
		return;
	}

	if (IS_NPC(ch) || number_percent() < learned(ch, gsn_flash_powder))
	{
		SET_BIT(ch->affected2_by, 0-AFF2_HAS_FLASH);
		send_to_char( "You make some flash powder and hide it about your person.\n\r", ch);
		check_improve(ch, gsn_flash_powder);
	}
	else
	{
		send_to_char( "BOOM! You accidentally put together the wrong components.\n\r", ch);
		damage(ch, ch, number_range(1,80), TYPE_UNDEFINED);
	}
	pop_call();
	return;
}

/*
 * DO_BANK
 *
 * This code manages the character's bank account in pcdata.
 *
 * Presto ?-?-97
 */

void do_bank( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *target;
	char choice[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char amt[MAX_STRING_LENGTH];
	char arg[MAX_STRING_LENGTH];

	int  amount      = 0;
	int  account_max = 0;
	int  targets_max = 0;

	push_call("do_bank(%p,%p)",ch,argument);

	if (!IS_SET(ch->in_room->room_flags, ROOM_BANK))
	{
		send_to_char("You're not in a bank.\n\r", ch);
		pop_call();
		return;
	}

	account_max = (ch->level + 5 + UMIN(ch->pcdata->reincarnation, 10)*5) * 2000000;

	ch->pcdata->account = URANGE(0, ch->pcdata->account, account_max);

	argument = one_argument(argument, choice);
	argument = one_argument(argument, amt);

	if (!strcasecmp(amt, "all"))
	{
		amount = -1;
	}
	else
	{
		amount = atoi(amt);

		if (amount < 0)
		{
			amount = 0;
		}
	}

	if (*choice == '\0' || *choice == ' ')
	{
		*choice = 'b';
	}

	switch (*choice)
	{
		case 'd':
			if (amount == -1)
			{
				amount = ch->gold;
			}
			if (amount > ch->gold)
			{
				send_to_char("You don't have that much gold.\n\r", ch);
				pop_call();
				return;
			}
			else if (amount == 0)
			{
				send_to_char("What good would that do?\n\r", ch);
				pop_call();
				return;
			}
			else
			{
				if (ch->pcdata->account + amount > account_max)
				{
					amount = account_max - ch->pcdata->account;
				}
				ch->gold -= amount;
				ch->pcdata->account += amount;
				sprintf(buf, "You deposit %d gold coins into your account.\n\rYour account balance is now %d gold coins.\n\r", amount, ch->pcdata->account);
				send_to_char(buf, ch);
			}
			break;

		case 'w':
			if (amount == -1)
			{
				amount = ch->pcdata->account;
			}
			if (amount > ch->pcdata->account)
			{
				send_to_char("You don't have that much gold in your account.\n\r", ch);
				pop_call();
				return;
			}
			else if (amount == 0)
			{
				send_to_char("What good would that do?\n\r", ch);
				pop_call();
				return;
			}
			else
			{
				if (ch->gold + amount > ch->level*1000000)
				{
					send_to_char("You cannot carry that much gold.\n\r", ch);
					amount = ch->level*1000000 - ch->gold;
				}
				ch->gold += amount;
				ch->pcdata->account -= amount;
				sprintf(buf, "You withdraw %d gold coins from your account.\n\rYour account balance is now %d gold coins.\n\r", amount, ch->pcdata->account);
				send_to_char(buf, ch);
			}
			break;

		case 'b':
			sprintf(buf, "You have %d gold coins in your account.\n\r",
			ch->pcdata->account);
			send_to_char(buf, ch);
			break;

		case 't': case 'T':
			argument = one_argument(argument, arg);
			if (*arg == '\0')
			{
				send_to_char("Transfer money to whom?\n\r", ch);
				pop_call();
				return;
			}

			if ((target = get_char_room(ch, arg)) == NULL)
			{
				send_to_char("They must be in the bank with you.\n\r", ch);
				pop_call();
				return;
			}

			if (IS_NPC(target))
			{
				send_to_char("Only players have accounts, silly.\n\r", ch);
				pop_call();
				return;
			}
			if (amount == -1)
			{
				amount = ch->pcdata->account;
			}
			if (ch->pcdata->account < amount)
			{
				send_to_char("You don't have enough gold in your account.\n\r", ch);
				pop_call();
				return;
			}
			else if (amount == 0)
			{
				send_to_char("What good would that do?\n\r", ch);
				pop_call();
				return;
			}

			targets_max = (target->level + 5 + UMIN(target->pcdata->reincarnation, 10)*5) * 2000000;

			if (target->pcdata->account + amount > targets_max)
			{
				send_to_char("Their account cannot hold that much.\n\r", ch);
				amount = targets_max - target->pcdata->account;
			}
			ch->pcdata->account     -= amount;
			target->pcdata->account += amount;
			ch_printf(ch,     "You transfer %d coins into %s's account.\n\rYour account balance is now %d gold coins.\n\r", amount, target->name, ch->pcdata->account);
			ch_printf(target, "%s transfers %d coins into your account.\n\rYour account balance is now %d gold coins.\n\r", ch->name, amount, target->pcdata->account);
			break;

		default:
			send_to_char("Usage: bank <deposit/withdraw/balance/transfer> [amount] [target]\n\r", ch);
			break;
	}
	pop_call();
	return;
}


int give_gold(CHAR_DATA* ch, int amount)
{
	int num;

	push_call("give_gold(%p,%p)",ch,amount);

	if (!IS_NPC(ch) && ch->pcdata->clan && ch->pcdata->clan->tax > 0 && ch->pcdata->clan->tax <= 100)
	{
		num = (float) amount * ((float) ch->pcdata->clan->tax / 100);
		if (num > 0 && ch->pcdata->clan->coffers < max_coffer(ch->pcdata->clan))
		{
			if (ch->pcdata->clan->coffers + num > max_coffer(ch->pcdata->clan))
			{
				num = max_coffer(ch->pcdata->clan) - ch->pcdata->clan->coffers;
			}
			ch->pcdata->clan->coffers += num;
			amount -= num;
			ch->gold += amount;

			pop_call();
			return num;
		}
		else
		{
			ch->gold += amount;

			pop_call();
			return 0;
		}
	}
	else
	{
		ch->gold += amount;
	}
	pop_call();
	return 0;
}



void do_enter( CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *portal;
	int destination, old_room;
	CHAR_DATA *fch, *fch_next;
	char buf[MAX_INPUT_LENGTH];

	push_call("do_enter(%p,%p)",ch,argument);

	if (argument[0] == '\0')
	{
		send_to_char("Enter what?\n\r", ch);
		pop_call();
		return;
	}

	if (ch->move < 2)
	{
		send_to_char("You are too exhaused.\n\r", ch);
		pop_call();
		return;
	}

	old_room = ch->in_room->vnum;
	portal   = get_obj_list(ch, argument, ch->in_room->first_content);

	if (portal == NULL)
	{
		send_to_char("You cannot find it.\n\r", ch);
		pop_call();
		return;
	}

	if (portal->item_type != ITEM_PORTAL)
	{
		send_to_char("You cannot seem to find a way in.\n\r", ch);
		pop_call();
		return;
	}

	if (IS_SET(portal->value[2], GATE_RANDOM))
	{
		destination = get_random_room_index(ch, 1, MAX_VNUM-1)->vnum;
	}
	else if (IS_SET(portal->value[2], GATE_RANDOM_AREA))
	{
		destination = get_random_room_index(ch, portal->pIndexData->area->low_r_vnum, portal->pIndexData->area->hi_r_vnum)->vnum;
	}
	else
	{
		destination = portal->value[3];
	}

	if (get_room_index(destination) == NULL || destination == ch->in_room->vnum)
	{
		act("$p doesn't seem to go anywhere.", ch, portal, NULL, TO_CHAR);
		pop_call();
		return;
	}

	if (IS_SET(portal->value[2], GATE_STEP_THROUGH))
	{
		act("You step through $p.", ch, portal, NULL, TO_CHAR);
		sprintf(buf, " steps through %s and vanishes.\n\r", portal->short_descr);
	}
	else if (IS_SET(portal->value[2], GATE_STEP_INTO))
	{
		act("You step into $p.", ch, portal, NULL, TO_CHAR);
		sprintf(buf, " steps into %s and vanishes.\n\r", portal->short_descr);
	}
	else
	{
		act("You enters $p.", ch, portal, NULL, TO_CHAR);
		sprintf(buf, " enters %s and vanishes.\n\r", portal->short_descr);
	}
	show_who_can_see(ch, buf);

	if (IS_SET(portal->value[2], GATE_GOWITH))
	{
		act("$p fades out of existance.", ch, portal, NULL, TO_ROOM);
		obj_from_room(portal);
		obj_to_room(portal, destination);
	}

	char_to_room(ch, destination);

	show_who_can_see(ch, buf);

	ch->move--;

	do_look(ch, "auto");

	if (portal->value[0] > 0)
	{
		portal->value[0]--;
		SET_BIT(portal->extra_flags, ITEM_MODIFIED);
	}

	mprog_greet_trigger(ch);

	if (IS_SET(portal->value[2], GATE_GOWITH))
	{
		obj_to_room(portal, ch->in_room->vnum);
		pop_call();
		return;
	}

	if (ch->in_room->vnum == old_room)
	{
		pop_call();
		return;
	}

	for (fch = room_index[old_room]->first_person ; fch ; fch = fch_next)
	{
		fch_next = fch->next_in_room;

		if (fch->in_room != room_index[old_room])
		{
			fch_next = room_index[old_room]->first_person;
			continue;
		}

		if (fch->master == ch && fch->position == POS_STANDING)
		{
			act("You follow $N.", fch, NULL, ch, TO_CHAR);
			do_enter(fch, argument);
		}
	}

	if (IS_SET(portal->value[2], GATE_GOWITH))
	{
		if ((fch = room_index[old_room]->first_person) != NULL)
		{
			act("$p fades out of existance.", fch, portal, NULL, TO_CHAR);
			act("$p fades out of existence.", fch, portal, NULL, TO_ROOM);
		}
		obj_to_room(portal, ch->in_room->vnum);
	}

	if (portal->value[0] == 0 && (fch = room_index[portal->in_room->vnum]->first_person) != NULL)
	{
		act("$p fades out of existance.", fch, portal, NULL, TO_CHAR);
		act("$p fades out of existence.", fch, portal, NULL, TO_ROOM);
		junk_obj(portal);
	}

	pop_call();
	return;
}

void search_inventory( CHAR_DATA *ch, OBJ_DATA *search_obj, char *argument )
{
	OBJ_DATA *obj;

	push_call("search_inventory(%p,%p,%p)",ch,search_obj,argument);

	for (obj = search_obj ; obj ; obj = obj->next_content)
	{
		if (obj->first_content)
		{
			search_inventory(ch, obj->first_content, argument);
		}

		if (can_see_obj(ch, obj) && is_multi_name_list_short(argument, short_to_name(obj->short_descr, 1)))
		{
			if (obj->in_obj)
			{
				ch_printf(ch, "%s%s is inside %s.\n\r", get_color_string(ch, COLOR_OBJECTS, VT102_BOLD), capitalize(obj->short_descr), obj->in_obj->short_descr);
			}
			else if (obj->wear_loc != WEAR_NONE)
			{
				ch_printf(ch, "%s%s is being worn.\n\r", get_color_string(ch, COLOR_OBJECTS, VT102_BOLD), capitalize(obj->short_descr));
			}
			else
			{
				ch_printf(ch, "%s%s is being carried.\n\r", get_color_string(ch, COLOR_OBJECTS, VT102_BOLD), capitalize(obj->short_descr));
			}
		}
	}
	pop_call();
	return;
}

void do_search( CHAR_DATA *ch, char *argument)
{
	push_call("do_search(%p,%p)",ch,argument);

	if (argument[0] == '\0')
	{
		send_to_char("Search what?\n\r", ch);

		pop_call();
		return;
	}

	act("You rummage through your belongings.", ch, NULL, NULL, TO_CHAR);
/*	act("$n rummages through $s belongings.",   ch, NULL, NULL, TO_ROOM);	*/

	search_inventory(ch, ch->first_carrying, argument);

	pop_call();
	return;
}
