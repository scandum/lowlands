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
	Global functions.
*/

sh_int	martial_arts_attack;
sh_int	fisticuffs_attack;
sh_int	brawling_attack;
sh_int	body_part_attack;

bool		OGRE_INTIMIDATE;

bool remove_obj args( ( CHAR_DATA *ch, int iWear, bool fReplace, bool fDisplay ) );

/*
	Local functions.
*/

bool        check_add_attack    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void        clear_attack_list   args( ( CHAR_DATA *ch ) );
void        spam_attack_list    args( ( CHAR_DATA *ch ) );
void        add_to_victory_list	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void        show_party_line     args( ( CHAR_DATA *ch ) );
void        raw_kill            args( ( CHAR_DATA *victim ) );
void        arena_death		args( ( CHAR_DATA *victim ) );
void        player_death        args( ( CHAR_DATA *victim ) );
bool        check_dodge         args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool        check_parry         args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool        check_shield_block  args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool        check_tumble        args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void        check_killer        args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void        dam_message         args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt) );
char      * get_dam_nounce      args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt) );
void        one_hit             args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt) );
bool        has_mirror          args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt) );
bool        has_icicle_armor    args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt) );
bool        is_safe             args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int         xp_compute          args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void        group_gain          args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void        disarm              args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void        trip                args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void        backstab            args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void        orc_brawl           args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void        fisticuffs          args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam ) );
void        martial_arts        args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam ) );

struct attack_type
{
	char			*	message;
	int				dam_type;
};

const struct attack_type attack_table[] =
{
	{
		"hit",					DAM_THRUST
	},
	{
		"slice", 					DAM_SLICE
	},
	{
		"stab",					DAM_PIERCE
	},
	{
		"slash",					DAM_SLICE
	},
	{
		"whip",					DAM_REND
	},
	{
		"claw",					DAM_REND
	},
	{
		"blast",					DAM_THRUST
	},
	{
		"pound",					DAM_BASH
	},
	{
		"crush",					DAM_BASH
	},
	{
		"grep",					DAM_THRUST
	},
	{
		"bite",					DAM_PIERCE
	},
	{
		"pierce",					DAM_PIERCE
	},
	{
		"chop",					DAM_CHOP
	}
};


const struct attack_type martial_arts_table[] =
{
	{
		"combination punch",		DAM_THRUST
	},
	{
		"palm punch",				DAM_THRUST
	},
	{
		"thrust kick",				DAM_BASH
	},
	{
		"spinning back-hand",		DAM_THRUST
	},
	{
		"jump kick",				DAM_THRUST
	},
	{
		"round house",				DAM_BASH
	},
	{
		"flip kick",				DAM_THRUST
	},
	{
		"thrusting spear hand",		DAM_PIERCE
	},
	{
		"spinning reverse kick",		DAM_THRUST
	},
	{
		"spinning hook kick",		DAM_THRUST
	},
	{
		"flying scissors kick",		DAM_REND
	},
	{
		"triple round house kick",	DAM_BASH
	}
};


const struct attack_type fisticuffs_table[] =
{
	{
		"combination punch",		DAM_THRUST
	},
	{
		"uppercut",				DAM_THRUST
	},
	{
		"kidney punch",			DAM_THRUST
	},
	{
		"fist blow",				DAM_BASH
	},
	{
		"haymaker",				DAM_BASH
	},
	{
		"sucker punch",			DAM_BASH
	}
};


const struct attack_type brawling_table[] =
{
	{
		"knee",					DAM_THRUST
	},
	{
		"claw",					DAM_REND
	},
	{
		"bite",					DAM_PIERCE
	},
	{
		"head butt",				DAM_BASH
	},
	{
		"elbow slam",				DAM_THRUST
	},
	{
		"uppercut",				DAM_THRUST
	}
};


void check_asn_obj(CHAR_DATA *victim)
{
	push_call("check_asn_obj(%p)",victim);

	if (IS_NPC(victim) || victim->fighting == NULL || IS_NPC(victim->fighting->who))
	{
		pop_call();
		return;
	}

	if (victim->fighting->who->pcdata->asn_obj)
	{
		OBJ_DATA *obj = victim->fighting->who->pcdata->asn_obj;
		victim->fighting->who->pcdata->asn_obj = NULL;
		if (obj->carried_by == victim)
		{
			obj_to_char(obj, victim->fighting->who);
			act("You quickly snatch $p from $N's failing grasp!", victim->fighting->who, obj, victim, TO_CHAR );
		}
	}
	pop_call();
	return;
}

/*
	Control the fights going on.
	Called periodically by update_handler.
*/

void violence_update( void )
{
	CHAR_DATA *ch;
	CHAR_DATA *victim;
	CHAR_DATA *rch;
	OBJ_DATA *obj;
	PLAYER_GAME *gpl;

	push_call("violence_update()");

	for (ch = mud->f_char ; ch ; ch = mud->update_wch)
	{
		mud->update_wch = ch->next;

		if (ch->distracted > 0)
		{
			ch->distracted--;

			if (ch->distracted == 0 && IS_NPC(ch) && ch->position < ch->pIndexData->position)
			{
				do_stand(ch, "");
			}
		}

		if (!IS_NPC(ch))
		{
			if (ch->position >= POS_STUNNED)
			{
				if (ch->hit < ch->max_hit)
				{
					ch->hit += hit_gain(ch, QUIET) / 15;
				}

				if (ch->mana < ch->max_mana)
				{
					ch->mana += mana_gain(ch, QUIET) / 15;
				}

				if (ch->move < ch->max_move)
				{
					ch->move += move_gain(ch, QUIET) / 15;
				}
			}

			if ((obj = get_eq_char(ch, WEAR_WIELD)) != NULL && get_obj_weight(obj) > str_app[get_curr_str(ch)].wield)
			{
				act( "$p slips out of your weakened grasp.", ch, obj, NULL, TO_CHAR );
				act( "$p slips out of $n's weakened grasp.", ch, obj, NULL, TO_ROOM );
				unequip_char(ch, obj);
			}

			if (is_affected(ch, gsn_nightmare))
			{
				if (number_percent() < 5 && (obj = get_eq_char(ch, WEAR_LIGHT)) !=NULL)
				{
					act( "$n screams and flings $p away!", ch, obj, NULL, TO_ROOM);
					switch (number_range(1,3))
					{
						case 1:
							act( "The evil leer of a demon appears within the glaring light of $p!", ch, obj, NULL, TO_CHAR);
							break;
						case 2:
							act( "A soul rending shriek is torn from $p!", ch, obj, NULL, TO_CHAR);
							break;
						case 3:
							act( "Images of your painful demise emanate from $p!", ch, obj, NULL, TO_CHAR);
							break;
					}
					act( "You scream and fling $p away!", ch, obj, NULL, TO_CHAR);
					unequip_char(ch, obj);
				}
				else if (number_percent() < 5 && (obj = get_eq_char(ch, WEAR_HOLD))  !=NULL)
				{
					act( "$n screams and flings $p away!", ch, obj, NULL, TO_ROOM);
					switch (number_range(1,3))
					{
						case 1:
							act( "$p starts to slowly drain away your dwindling vitality!", ch, obj, NULL, TO_CHAR);
							break;
						case 2:
							act( "A whisper hisses from $p 'You shall do my bidding weakling!", ch, obj, NULL, TO_CHAR);
							break;
						case 3:
							act( "A stabbing pain lances through you from $p!", ch, obj, NULL, TO_CHAR);
							break;
					}
					act( "You scream and fling $p away!", ch, obj, NULL, TO_CHAR);
					unequip_char(ch, obj);
				}
				else if ( number_percent() < 5 && (obj = get_eq_char(ch, WEAR_WIELD ))  !=NULL)
				{
					act( "$n screams and flings $p away!", ch, obj, NULL, TO_ROOM);
					switch (number_range(1,3))
					{
						case 1:
							act( "The hilt of $p glows red starting to smolder with heat!", ch, obj, NULL, TO_CHAR);
							break;
						case 2:
							act( "Like a viper, $p turns on you and tries to strike!", ch, obj, NULL, TO_CHAR);
							break;
						case 3:
							act( "$p takes on a life of its own, struggling within your grasp!", ch, obj, NULL, TO_CHAR);
							break;
					}
					act( "You scream and fling $p away!", ch, obj, NULL, TO_CHAR);
					unequip_char(ch, obj);
				}
			}
		}

		if (IS_AFFECTED(ch, AFF2_BLEEDING))
		{
			AFFECT_DATA *paf;
			int dam = 0;

			ch_printf(ch, "%sYour life is bleeding out of many nasty wounds!\n\r", get_color_string(ch, COLOR_YOU_ARE_HIT, VT102_BOLD));
			act("$n's life is bleeding out of a many nasty wounds!", ch, NULL, NULL, TO_ROOM);

			for (paf = ch->first_affect ; paf ; paf = paf->next)
			{
				if (paf->bitvector == AFF2_BLEEDING)
				{
					dam += paf->modifier;
				}
			}
			if (dam < 1)
			{
				dam = 1;
			}
			if ((victim = get_char_pvnum(ch->critical_hit_by)) == NULL)
			{
				victim = ch;
			}
			damage(victim, ch, dam, TYPE_NOFIGHT);
		}

		if (IS_AFFECTED(ch, AFF2_TORRID_BALM) && number_bits(1))
		{
			AFFECT_DATA *paf;
			int dam = 0;

			switch(number_bits(2))
			{
				case 0:
					act("Your skin blisters from the torrid balm enveloping you.",	ch, NULL, NULL, TO_CHAR);
					act("$n's skin blisters from the torrid balm enveloping $m.",	ch, NULL, NULL, TO_ROOM);
					break;
				case 1:
					act("Torment lashes through you as the torrid balm brands your flesh.",	ch, NULL, NULL, TO_CHAR);
					act("Torment lashes through $n as torrid balm brands $s flesh.",			ch, NULL, NULL, TO_ROOM);
					break;
				case 2:
					act("You feel your flesh rendered by the torrid balm.",	ch, NULL, NULL, TO_CHAR);
					act("You watch $n's flesh rendered by torrid balm.",		ch, NULL, NULL, TO_ROOM);
					break;
				case 3:
					act("You scream out in agony as the torrid balm consumes you.",	ch, NULL, NULL, TO_CHAR);
					act("$n screams out in agony as torrid balm consumes $m.",		ch, NULL, NULL, TO_ROOM);
					break;
			}

			for (paf = ch->first_affect ; paf ; paf = paf->next)
			{
				if (paf->bitvector == AFF2_TORRID_BALM)
				{
					dam = number_range(1, 10 + paf->duration * 10) + paf->duration * paf->duration;
					if (number_bits(2) == 0)
					{
						paf->modifier += IS_NPC(ch) ? 5 : -1;
					}
				}
			}
			dam = URANGE(1, dam, UMAX(ch->hit, 1));
			damage(ch, ch, dam, TYPE_NOFIGHT);
		}

		/*
			Add poison continuous damage here
		*/

		if (ch->poison)
		{
			POISON_DATA *pd;
			CHAR_DATA *attacker, *fch;
			int dmg;

			pd = ch->poison;

			attacker = get_char_pvnum(pd->poisoner);

			if (attacker == NULL)
			{
				attacker = ch;
			}

			if (pd->instant_damage_high > 0)
			{
				dmg = number_range(pd->instant_damage_low, pd->instant_damage_high);

				pd->instant_damage_high = 0;
			}
			else
			{
				dmg = number_range(pd->constant_damage_low, pd->constant_damage_high);
			}

			if (pd->for_npc != IS_NPC(ch))
			{
				dmg /= 2;
			}

			for (fch = ch->in_room->first_person ; fch ; fch = fch->next_in_room)
			{
				if (fch == ch && ch == attacker)
				{
					ch_printf(fch, "%sYou are shocked by poison for %dhp.\n\r", get_color_string(fch, COLOR_YOU_HIT, VT102_BOLD), dmg);
				}
				else if (fch == ch)
				{
					ch_printf(ch, "%sYou are shocked by poison.\n\r", get_color_string(ch, COLOR_YOU_ARE_HIT, VT102_BOLD));
				}
				else if (fch == attacker)
				{
					ch_printf(fch, "%s%s is shocked by poison for %dhp.\n\r", get_color_string(fch,COLOR_YOU_HIT,VT102_BOLD), capitalize(get_name(ch)), dmg);
				}
				else
				{
					ch_printf(fch, "%s is shocked by poison.\n\r", capitalize(get_name(ch)));
				}
			}

			damage(attacker, ch, dmg, TYPE_NOFIGHT);

			if (pd->constant_duration <= 0)
			{
				ch->poison = ch->poison->next;
				FREEMEM(pd);
			}
			else
			{
				pd->constant_duration--;
			}
		}

		if (ch->fighting == NULL)
		{
			if (ch->position == POS_FIGHTING)
			{
				stop_fighting(ch, FALSE);
			}
			continue;
		}

		if ((victim = who_fighting(ch)) == NULL)
		{
			stop_fighting(ch, FALSE);
			continue;
		}

		if (ch->in_room != victim->in_room)
		{
			stop_fighting( ch, FALSE );
			continue;
		}

		if (ch->position < POS_SLEEPING)
		{
			continue;
		}

		if (ch->attack != 0)
		{
			ch->attack = 0;
		}

		if (IS_NPC(ch))
		{
			if (!IS_SET(ch->act, ACT_NO_DAMAGE))
			{
				multi_hit( ch, victim, TYPE_UNDEFINED );
			}
			else if (!IS_SET(ch->pIndexData->progtypes, FIGHT_PROG))
			{
				log_build_printf(ch->pIndexData->vnum, "mob has ACT_NO_DAMAGE set but has no fight prog");
			}
		}
		else
		{
			ch->pcdata->last_combat = mud->current_time;

			if (ch->pcdata->auto_flags != AUTO_QUICK)
			{
				check_improve(ch, gsn_first_strike);

				multi_hit( ch, victim, TYPE_UNDEFINED );
			}
		}

		if (!valid_fight(ch, victim))
		{
			continue;
		}

		set_fighting(victim, ch);

		mprog_hitprcnt_trigger(ch, victim);

		if (!valid_fight(ch, victim))
		{
			continue;
		}

		mprog_fight_trigger(ch, victim);

		if (!valid_fight(ch, victim))
		{
			continue;
		}

		/*
			assist rules - Scandum 27-02-2002

			- charmed mobs are not assisted
			- players only assist when victim is a mob
			- grouped mobs assist, no questions asked
			- mobs do not attack their master
			- mobs do not attack their own kind (same vnum)
			- mobs do not attack someone more than 30 levels below
			- mobs of same vnum have 50.0% chance to assist
			- mobs of diff vnum have 12.5% chance to assist
			- mobs attack a random party member of the victim
		*/

		if (IS_AFFECTED(victim, AFF_CHARM) || IS_AFFECTED(ch, AFF_CHARM))
		{
			continue;
		}

		for (rch = ch->in_room->first_person ; rch ; rch = mud->update_rch)
		{
			mud->update_rch = rch->next_in_room;

			if (!IS_AWAKE(rch) || rch->fighting || IS_AFFECTED(rch, AFF_ETHEREAL))
			{
				continue;
			}

			if (!IS_NPC(rch) || rch->desc)
			{
				if (is_same_group(ch, rch) && IS_NPC(victim))
				{
					multi_hit(rch, victim, TYPE_UNDEFINED);
				}
				continue;
			}

			if (IS_AFFECTED(rch, AFF_CHARM))
			{
				if (rch->master == ch)
				{
					multi_hit(rch, victim, TYPE_UNDEFINED);
				}
				continue;
			}

			if (is_same_group(ch, rch))
			{
				multi_hit(rch, victim, TYPE_UNDEFINED);
				continue;
			}

			if (!IS_NPC(ch) || rch->master == who_fighting(ch))
			{
				continue;
			}

			if (rch->pIndexData == victim->pIndexData)
			{
				continue;
			}

			if (rch->pIndexData != ch->pIndexData && number_bits(2))
			{
				continue;
			}

			if (rch->level / 2 <= ch->level && rch->level * 2 >= ch->level && number_bits(1))
			{
				CHAR_DATA *vch;
				CHAR_DATA *target = NULL;
				int number;

				for (number = 0, vch = ch->in_room->first_person ; vch ; vch = vch->next_in_room, number++)
				{
					if (can_see(rch, vch) && is_same_group(vch, victim) && number_range(0, number) == 0)
					{
						target = vch;
					}
				}
				if (target != NULL)
				{
					multi_hit(rch, target, TYPE_UNDEFINED);
				}
 			}
		}
	}
	/*
		Add Spam's party stat line here
	*/
	for (gpl = mud->f_player ; gpl ; gpl = gpl->next)
	{
		if (gpl->ch->fighting != NULL && IS_SET(gpl->ch->pcdata->spam, 1024))
		{
			show_party_line( gpl->ch );
		}
		if (gpl->ch->desc == NULL && gpl->ch->fighting && gpl->ch->wait == 0)
		{
			do_flee(gpl->ch, "");
		}
	}

	pop_call();
	return;
}

/*
	The SPAM stat line
*/

void show_party_line( CHAR_DATA *ch )
{
	CHAR_DATA *fch;
	char buf[MAX_STRING_LENGTH], buf2[200], buf3[200];

	push_call("show_party_line(%p)",ch);
	buf[0]='\0';

	for (fch = ch->in_room->first_person; fch != NULL; fch = fch->next_in_room)
	{
		if (fch != ch && is_same_group(ch, fch))
		{
			strcpy(buf3, !IS_NPC(fch)?capitalize(fch->name): capitalize(fch->short_descr));
			buf3[6]='\0';
			sprintf(buf2, "%s:%d/%d  ", buf3, fch->hit, fch->max_hit);
			strcat(buf, buf2);
		}
	}
	if (buf[0] != '\0')
	{
		ch_printf(ch, "%s%s\n\r", get_color_string(ch,COLOR_PROMPT,VT102_DIM), buf);
	}
	pop_call();
	return;
}

/*
	Do one group of attacks.
*/

void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
	int chance, level, bonus;
	OBJ_DATA *wield;

	push_call("multi_hit(%p,%p,%p)",ch,victim,dt);

	OGRE_INTIMIDATE = FALSE;

	/*
		Disallow fighting in different rooms and shops
	*/

	if (victim->in_room != ch->in_room)
	{
		stop_fighting(ch, FALSE);
		pop_call();
		return;
	}

	if (is_safe(ch, NULL))
	{
		stop_fighting(ch, FALSE);
		pop_call();
		return;
	}

	if (victim->fighting == NULL)
	{
		if (!IS_NPC(victim) || (IS_SET(victim->act, ACT_CLASS) && victim->class == CLASS_GLADIATOR))
		{
			if (number_range(1, 200) < learned(victim, gsn_impale))
			{
				wield = get_eq_char(victim, WEAR_WIELD);

				if (wield && wield->item_type == ITEM_WEAPON && wield->value[0] == WEAPON_TYPE_SPEAR)
				{
					act("You thrust $p into the ground.", victim, wield, NULL, TO_CHAR);
					act("$n thrusts $p into the ground.", victim, wield, NULL, TO_ROOM);

					one_hit(victim, ch, gsn_impale);
				}
			}
		}
	}

	if (ch->in_room != victim->in_room)
	{
		stop_fighting(ch, FALSE);
		pop_call();
		return;
	}

	/*
		Added Ogre Intimidate here  - Chaos 8/20/98
	*/

	if (rspec_req(ch, RSPEC_INTIMIDATE) && !rspec_req(victim, RSPEC_INTIMIDATE))
	{
		if (!IS_UNDEAD(victim) && number_range(1,6) == 1)
		{
			OGRE_INTIMIDATE = TRUE;
			send_to_char( "You intimidate your opponent.\n\r", ch);
			send_to_char( "You are intimidated by your opponent.\n\r", victim);
		}
	}

	one_hit(ch, victim, dt);

	if (!valid_fight(ch, victim) || ch->fighting->who != victim)
	{
		pop_call();
		return;
	}

	if (ch->position != POS_FIGHTING && ch->position > POS_STUNNED)
	{
		set_fighting(ch, victim);
	}

	/*
		Added Orcish brawling  - Chaos 8/20/98
	*/

	if (rspec_req(ch, RSPEC_BRAWLING) && number_bits(3) == 0)
	{
		orc_brawl(ch, victim);

		if (!valid_fight(ch, victim) || ch->fighting->who != victim)
		{
			pop_call();
			return;
		}
	}

	if (get_eq_char(ch, WEAR_DUAL_WIELD) || rspec_req(ch, RSPEC_MULTI_ARMS) || is_affected(ch,AFF2_STEELHAND))
	{
		if (rspec_req(ch, RSPEC_MULTI_ARMS))
		{
			chance = UMAX(50, learned(ch, gsn_dual_wield));
		}
		else
		{
			chance = learned(ch, gsn_dual_wield);
		}
		bonus  = chance / 10;

		if (number_percent() < chance)
		{
			one_hit(ch, victim, dt);
			check_improve(ch, gsn_dual_wield);
		}

		if (!valid_fight(ch, victim) || ch->fighting->who != victim)
		{
			pop_call();
			return;
		}
	}
	else
	{
		bonus = 0;
	}

	if (IS_AFFECTED(ch, AFF_HASTE))
	{
		bonus += 10;
	}

	if (!IS_NPC(ch) && multi(ch, gsn_second_attack) == -1)
	{
		pop_call();
		return;
	}

	level  = multi_skill_level(ch, gsn_second_attack);
	chance = (learned(ch, gsn_second_attack)+bonus)*level/50;

	if (number_percent() < chance)
	{
		one_hit(ch, victim, dt);

		if (!valid_fight(ch, victim) || ch->fighting->who != victim)
		{
			pop_call();
			return;
		}
		check_improve(ch, gsn_second_attack);
	}

	if (!IS_NPC(ch) && multi(ch, gsn_third_attack) == -1)
	{
		pop_call();
		return;
	}
	level  = multi_skill_level(ch, gsn_third_attack);
	chance = (learned(ch, gsn_third_attack)+bonus)*level/150;

	if (number_percent() < chance)
	{
		one_hit(ch, victim, dt);

		if (!valid_fight(ch, victim) || ch->fighting->who != victim)
		{
			pop_call();
			return;
		}
		check_improve(ch, gsn_third_attack);
	}

	if (!IS_NPC(ch) && multi(ch, gsn_fourth_attack) == -1)
	{
		pop_call();
		return;
	}

	level  = multi_skill_level(ch, gsn_fourth_attack);
	chance = (learned(ch, gsn_fourth_attack)+bonus)*level/300;

	if (number_percent() < chance)
	{
		one_hit(ch, victim, dt);

		if (!valid_fight(ch, victim) || ch->fighting->who != victim)
		{
			pop_call();
			return;
		}
		check_improve(ch, gsn_fourth_attack);
	}

	if (!IS_NPC(ch) && multi(ch, gsn_fifth_attack) == -1)
	{
		pop_call();
		return;
	}

	level  = multi_skill_level(ch, gsn_fifth_attack);
	chance = (learned(ch, gsn_fifth_attack)+bonus)*level/450;

	if (number_percent() < chance)
	{
		one_hit(ch,victim,dt);

		if (!valid_fight(ch, victim) || ch->fighting->who != victim)
		{
			pop_call();
			return;
		}
		check_improve(ch, gsn_fifth_attack);
	}
	pop_call();
	return;
}

bool check_hit( CHAR_DATA *ch, CHAR_DATA *victim, int hit_roll, int dam_type )
{
	int thac0, diceroll, level, scale;
	OBJ_DATA *wield;

	push_call("check_hit(%p,%p,%p,%p)",ch,victim,hit_roll,dam_type);

	/*
		Calculate to-hit-armor-class-0 versus armor - Chaos 10/5/93
	*/

	level = multi_skill_level(ch, dam_type);

	if (dam_type == gsn_kick)				scale = 15;
	else if (dam_type == gsn_shoot)			scale = 14;
	else if (dam_type == gsn_backstab)			scale = 14;
	else if (dam_type == gsn_head_butt)		scale = 14;
	else if (dam_type == gsn_knife)			scale = 13;
	else if (dam_type == gsn_trip)			scale = 13;
	else if (dam_type == gsn_circle)			scale = 13;
	else if (dam_type == gsn_throw)			scale = 13;
	else if (dam_type == gsn_vampiric_touch)	scale = 13;
	else if (dam_type == gsn_whirl)			scale = 13;
	else if (dam_type == gsn_impale)			scale = 13;
	else if (dam_type == gsn_bash)			scale = 12;
	else if (dam_type == gsn_fisticuffs)		scale = 12;
	else if (dam_type == gsn_gouge)			scale = 12;
	else if (dam_type == gsn_martial_arts)		scale = 11;
	else if (dam_type == gsn_acupunch)			scale = 11;
	else if (dam_type == gsn_cripple)			scale = 11;
	else
	{
		scale = 10;
		level = ch->level;
	}

	if (scale != 10 && number_percent() > learned(ch, dam_type))
	{
		if (!IS_NPC(ch) && IS_SET(ch->act, PLR_WIZTIME))
		{
			ch_printf_color(ch, "{078}(check_hit(false) scale: %d, level: %d, learned: %d)\n\r", scale, level, learned(ch, dam_type));
		}
		pop_call();
		return FALSE;
	}


	if (IS_NPC(victim))
	{
		thac0 = mob_armor(victim) + victim->level*4;
	}
	else
	{
		thac0 = 0 - (GET_AC(victim) - 100);
	}

	if (IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch))
	{
		thac0 += 20;
	}

	if (IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch) )
	{
		thac0 += 20;
	}

	if (OGRE_INTIMIDATE)
	{
		thac0 -= ch->level*2;
	}

	if (victim->position == POS_SLEEPING)
	{
		thac0 = 0;
	}

	if (victim->position == POS_RESTING || victim->position == POS_SITTING)
	{
		thac0 -= ch->level;
	}

	if (!IS_NPC(ch) && ch->in_room->sector_type == SECT_ETHEREAL)
	{
		thac0 += (200 - ((get_curr_wis(ch) + get_curr_int(ch)) * 4));
	}
	if (!IS_NPC(ch) && ch->in_room->sector_type == SECT_ASTRAL)
	{
		thac0 += (100 - (get_curr_int(ch) * 4));
	}

	if (ch->distracted > 0)
	{
		thac0 += ch->level/3;
		ch->distracted--;
	}
	if (victim->distracted > 0)
	{
		thac0 -= ch->level/3;
		victim->distracted--;
	}

	if (IS_NPC(ch))
	{
		diceroll = number_range(0, 34 + scale*level + hit_roll) + level*2;
	}
	else
	{
		if (learned(ch, gsn_precision))
		{
			hit_roll += hit_roll * URANGE(25, multi_skill_level(ch, gsn_precision)/2, 50) * learned(ch, gsn_precision) / 10000;
		}
		diceroll = number_range(0, 39 + scale*level*12/10) + (hit_roll > 0 ? number_range(1, hit_roll * 2) : hit_roll) + level*2;
	}

	if (dam_type >= TYPE_HIT)
	{
		if ((wield = get_eq_char(ch, WEAR_DUAL_WIELD)) != NULL)
		{
			if (!IS_SET(mud->flags, MUD_DUALFLIP))
			{
				wield = get_eq_char( ch, WEAR_WIELD );
			}
		}
		else
		{
			wield = get_eq_char(ch, WEAR_WIELD);
		}

		if (wield != NULL && wield->item_type == ITEM_WEAPON)
		{
			diceroll = (100 + weapon_skill(ch, wield->value[0])) * diceroll / 200;
		}
	}

	if (ch->move <= 0)
	{
		diceroll -= 10;
	}

	if (!can_see(ch, victim))
	{
		diceroll -= 40;
	}

	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_WIZTIME))
	{
		ch_printf_color(ch, "{078}(check_hit() diceroll: %d, thac: %d)\n\r", diceroll, thac0);
	}

	pop_call();
	return(diceroll >= thac0);
}

/*
	Hit one guy once.
*/

void one_hit(CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
	OBJ_DATA *wield;
	AFFECT_DATA *paf;
	int dam;

	push_call("one_hit(%p,%p,%p)",ch,victim,dt);

	if (is_safe(ch, NULL))
	{
		pop_call();
		return;
	}

	if (ch->in_room != victim->in_room)
	{
		pop_call();
		return;
	}

	if ((wield = get_eq_char(ch, WEAR_DUAL_WIELD)) != NULL)
	{
		TOGGLE_BIT(mud->flags, MUD_DUALFLIP);

		if (!IS_SET(mud->flags, MUD_DUALFLIP))
		{
			wield = get_eq_char(ch, WEAR_WIELD);
		}
	}
	else
	{
		wield = get_eq_char(ch, WEAR_WIELD);
	}

	/*
		Make them a bit tired
	*/

	if (!IS_NPC(ch) && ch->move > 0)
	{
		ch->move--;
	}

	/*
		Figure out the type of damage message.
	*/

	if (dt == TYPE_UNDEFINED)
	{
		if (IS_SET(ch->attack, ATTACK_MIRROR))
		{
			pop_call();
			return;
		}

		dt = TYPE_HIT;

		if (wield != NULL && wield->item_type == ITEM_WEAPON)
		{
			dt += wield->value[3];
		}
	}

	/*
		Calc damage and check for specials
	*/

	dam = 0;

	if (IS_NPC(ch))
	{
		dam = dice(get_damnodice(ch), get_damsizedice(ch)) + ch->npcdata->damplus;

		if (wield != NULL && wield->item_type == ITEM_WEAPON)
		{
			dam += weapon_skill(ch, wield->value[0]) * dice(wield->value[1], wield->value[2]) / 200;
		}
		else if (IS_SET(ch->act, ACT_CLASS))
		{
			switch (ch->class)
			{
				case CLASS_MONK:
					dt = gsn_fisticuffs;
					break;
				case CLASS_NINJA:
					dt = gsn_martial_arts;
					break;
			}
		}
	}
	else
	{
		if (wield != NULL && wield->item_type == ITEM_WEAPON)
		{
			dam = (100 + weapon_skill(ch, wield->value[0])) * dice(wield->value[1], wield->value[2]) / 200;
		}
		else
		{
			dam = dice(get_damnodice(ch), get_damsizedice(ch));

			if(is_affected(ch,AFF2_STEELHAND)) /* only counts if no weapons are worn */
			{
				for(paf=ch->first_affect;paf != NULL || paf->bitvector==AFF2_STEELHAND; paf=paf->next)
				{
					/* temporary check */
					if(paf->bitvector != AFF2_STEELHAND)
					{
						log_printf("(%s [%d]): have to fix right affect grabbing.\n",__FILE__,__LINE__);
					}
					/* end of temp check */
				}
				dam = dice(ch->level,paf->modifier);
			}
			else if (learned(ch, gsn_fisticuffs) || learned(ch, gsn_martial_arts))
			{
				if (multi(ch, gsn_fisticuffs) != -1 && !wield && number_percent() < multi_skill_level(ch, gsn_fisticuffs))
				{
					dt = gsn_fisticuffs;
				}
				else if (multi(ch, gsn_martial_arts) != -1 && !wield && number_percent() < multi_skill_level(ch, gsn_martial_arts))
				{
					dt = gsn_martial_arts;
				}
			}
		}
	}

	/*
		Bonuses.
	*/

	dam += GET_DAMROLL(ch);

	if (victim->position == POS_SLEEPING)
	{
		dam = 4*dam/3;
	}

	if (victim->position == POS_RESTING)
	{
		dam = 6*dam/5;
	}

	if (victim->position == POS_SITTING)
	{
		dam = 7*dam/6;
	}

	if (!IS_NPC(ch) && ch->in_room->sector_type == SECT_ETHEREAL)
	{
		dam = 2 * dam / 3;
	}
	else if (!IS_NPC(ch) && ch->in_room->sector_type == SECT_ASTRAL)
	{
		dam = 2 * dam / 3;
	}

	if (dt == gsn_backstab)
	{
		if (!IS_NPC(ch))
		{
			if (number_percent() > learned(ch, gsn_greater_backstab))
			{
				dam = dam*(12 + multi_skill_level(ch, gsn_backstab))/10;
			}
			else
			{
				dam = dam*(12 + multi_skill_level(ch, gsn_backstab))/8;
			}
		}
		else
		{
			dam = dam*(12 + ch->level/2)/10;
		}
	}
	else if (dt == gsn_circle)
	{
		if (!IS_NPC(ch))
		{
			dam = dam*(12 + multi_skill_level(ch, gsn_circle))/20;
		}
		else
		{
			dam = dam*(12 + ch->level/2)/20;
		}
	}
	else if (dt == gsn_knife)
	{
		if (!IS_NPC(ch))
		{
			dam = dam * (12 + multi_skill_level(ch, gsn_knife))/24;
		}
		else
		{
			dam = dam * (12 + ch->level/2)/24;
		}
	}
	else if (dt == gsn_impale)
	{
		if (!IS_NPC(ch))
		{
			dam = dam * (10 + multi_skill_level(ch, gsn_impale))/25;
		}
		else
		{
			dam = dam * (10 + ch->level/2)/25;
		}
	}
	else if (dt == gsn_martial_arts)
	{
		martial_arts(ch, victim, dam);
		pop_call();
		return;
	}
	else if (dt == gsn_fisticuffs)
	{
		fisticuffs(ch, victim, dam);
		pop_call();
		return;
	}
	else	if (gsn_anatomy && !check_hit(ch, victim, GET_HITROLL(ch), dt))
	{
		damage(ch, victim, 0, dt);
		check_improve(victim, gsn_armor_usage);
		pop_call();
		return;
	}

	if (dam <= 0)
	{
		dam = 1;
	}

	damage(ch, victim, dam, dt);

	if (dt == gsn_backstab)
	{
		check_improve(ch, gsn_backstab);
		check_improve(ch, gsn_greater_backstab);
	}
	else if (dt == gsn_knife)
	{
		check_improve(ch, gsn_knife);
	}
	else if (dt == gsn_impale)
	{
		check_improve(ch, gsn_impale);
	}
	pop_call();
	return;
}

void damage_hurt( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
	bool auto_sac, auto_loot;
	bool confused=FALSE;
	char buf[MAX_INPUT_LENGTH];

	/*
		Hurt the victim.
		Inform the victim of his new state.
	*/

	push_call("damage_hurt(%p,%p,%p,%p)",ch,victim,dam,dt);

	victim->hit -= dam;

	if (!IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL && victim->hit < 1)
	{
		victim->hit = 1;
	}

	update_pos(victim);


	if (victim->position != POS_DEAD && dam > 0)
	{
		show_char_to_char_1( victim, NULL);
	}

	if (dam > 0 && dt != TYPE_NOFIGHT && !is_spell(dt))
	{
		if (IS_AFFECTED(victim, AFF2_FIRESHIELD))
		{
			if (number_percent() <= 25)
			{
				damage(victim, ch, number_fuzzy(dam), gsn_fireshield);
			}

			if (ch->in_room != victim->in_room)
			{
				pop_call();
				return;
			}
		}

		if (IS_AFFECTED(victim, AFF2_ICICLE_ARMOR))
		{
			if (number_percent() <= 10)
			{
				damage(victim, ch, number_fuzzy(ch->level), gsn_icicle_armor);
			}

			if (ch->in_room != victim->in_room)
			{
				pop_call();
				return;
			}
		}
	}

	switch (victim->position)
	{
		case POS_MORTAL:
			act("$n is mortally wounded, and will die soon, if not aided.", victim, NULL, NULL, TO_ROOM);
			send_to_char( "You are mortally wounded, and will die soon, if not aided.\n\r", victim);
			break;
		case POS_INCAP:
			act( "$n is incapacitated and will slowly die, if not aided.", victim, NULL, NULL, TO_ROOM );
			send_to_char("You are incapacitated and will slowly die, if not aided.\n\r",victim );
			break;
		case POS_STUNNED:
			act( "$n is stunned, but will probably recover.",victim, NULL, NULL, TO_ROOM );
			send_to_char("You are stunned, but will probably recover.\n\r", victim );
			break;
		case POS_DEAD:
			if (victim->fighting)
			{
				if (IS_NPC(ch) && ch->master)
				{
					victim->fighting->who = ch->master;
				}
				else
				{
					victim->fighting->who = ch;
				}
			}
			mprog_death_trigger(victim);
			mprog_kill_trigger(ch);
			act( "$n is DEAD!!", victim, NULL, NULL, TO_ROOM );
			send_to_char( "You have been KILLED!!\n\r\n\r", victim );
			break;
		default:
			if (dam > victim->max_hit / 4 && (IS_NPC(victim) || victim->level < LEVEL_IMMORTAL))
			{
				send_to_char( "*** That really did HURT!\n\r", victim);
			}
			if (victim->hit < victim->max_hit / 4 && (IS_NPC(victim) || victim->level < LEVEL_IMMORTAL))
			{
				send_to_char( "*** You sure are BLEEDING!\n\r", victim);
			}
			break;
	}

	/*
		Sleep spells and extremely wounded folks.
	*/
	if (!IS_AWAKE(victim) || ch == victim)
	{
		if (dt != TYPE_NOFIGHT)
		{
			if (victim->fighting)
			{
				stop_hate_fear(victim->fighting->who);
			}
		}
	}
	/*
		Intercept object programs for damaging others
	*/
	if (!IS_NPC(ch) && dam > 0 && victim->position != POS_DEAD)
	{
		oprog_hit_trigger(victim, ch);

		if (dt != TYPE_NOFIGHT && ch->desc != NULL)
		{
			OBJ_DATA *obj, *obj_next;
			OBJ_PROG *prg;

			for (obj = ch->first_carrying ; obj ; obj = obj_next)
			{
				obj_next = obj->next_content;

				if (obj->carried_by && IS_SET(obj->pIndexData->oprogtypes, TRIG_DAMAGE))
				{
					for (prg = obj->pIndexData->first_oprog ; prg ; prg = prg->next)
					{
						if (prg->trigger == TRIG_DAMAGE && number_percent() <= prg->percentage)
						{
							start_object_program(ch, obj, prg, "");
						}
					}
				}
			}
		}
	}

	/*
		Intercept object programs for getting damaged
	*/

	if (!IS_NPC(victim) && dam > 0 && victim->position != POS_DEAD)
	{
		OBJ_DATA *obj, *obj_next;
		OBJ_PROG *prg;

		if (dt != TYPE_NOFIGHT && victim->desc != NULL)
		{
			for (obj = victim->first_carrying ; obj ; obj = obj_next)
			{
				obj_next = obj->next_content;

				if (obj->carried_by && IS_SET(obj->pIndexData->oprogtypes, TRIG_HIT))
				{
					for (prg = obj->pIndexData->first_oprog ; prg != NULL ; prg = prg->next)
					{
						if (prg->trigger == TRIG_HIT && number_percent() <= prg->percentage)
						{
							start_object_program( victim, obj, prg, "");
						}
					}
				}
			}
		}
	}

	/*
		Payoff for killing things.
	*/

	if (victim->position == POS_DEAD)
	{
		OBJ_DATA *obj;

		if (victim->in_room->area->low_r_vnum != ROOM_VNUM_ARENA && !IS_NPC(victim))
		{
			if (!IS_NPC(ch))
			{
				if (ch != victim)
				{
					add_to_victory_list(ch, victim);
					spam_attack_list(victim);
				}

				if (get_bounty(victim->name) != NULL)
				{
					collect_bounty(ch, victim);
				}


				if (ch->pcdata->clan != NULL && victim->pcdata->clan != NULL)
				{
					if (which_god(ch) == GOD_NEUTRAL || which_god(victim) == GOD_NEUTRAL)
					{
						ch->pcdata->clan->pkills[CLAN_NEUTRAL_KILL]++;
						victim->pcdata->clan->pdeaths[CLAN_NEUTRAL_DEATH]++;

						victim->pcdata->history[HISTORY_KILL_BY_PC]++;
						ch->pcdata->history[HISTORY_KILL_PC]++;
					}
					else
					{
						switch (which_god(ch))
						{
							case GOD_CHAOS:
								ch->pcdata->clan->pkills[CLAN_CHAOS_KILL]++;
								break;
							case GOD_ORDER:
								ch->pcdata->clan->pkills[CLAN_ORDER_KILL]++;
								break;
						}

						switch (which_god(victim))
						{
							case GOD_CHAOS:
								victim->pcdata->clan->pdeaths[CLAN_CHAOS_DEATH]++;
								break;
							case GOD_ORDER:
								victim->pcdata->clan->pdeaths[CLAN_ORDER_DEATH]++;
								break;
						}
						victim->pcdata->history[HISTORY_KILL_BY_EN]++;
						ch->pcdata->history[HISTORY_KILL_EN]++;
					}
					save_clan(ch->pcdata->clan);
					save_clan(victim->pcdata->clan);

					if (which_god(victim) != GOD_NEUTRAL)
					{
						ch->hit  = ch->max_hit;
						ch->mana = ch->max_mana;
						ch->move = ch->max_move;

						act(god_table[ch->pcdata->god].pk_char_msg, ch, NULL, victim, TO_CHAR);
						act(god_table[ch->pcdata->god].pk_room_msg, ch, NULL, victim, TO_ROOM);

						sprintf(buf, "%s", get_name(ch));
					}
				}
			}
			else
			{
				victim->pcdata->history[HISTORY_KILL_BY_NPC]++;

				if (victim->pcdata->clan != NULL)
				{
					victim->pcdata->clan->mdeaths++;
					save_clan(victim->pcdata->clan);
				}
			}
		}

		if (IS_NPC(victim) && !IS_NPC(ch))
		{
			if (ch->pcdata->clan != NULL)
			{
				ch->pcdata->clan->mkills++;
			}
			ch->pcdata->history[HISTORY_KILL_NPC]++;
		}

		if (!IS_NPC(victim))
		{
			log_printf("%s killed by %s at %u", get_name(victim), get_name(ch), victim->in_room->vnum);

			save_char_obj(victim, BACKUP_SAVE);

			int exp_loss = victim->pcdata->exp - exp_level(victim->class, victim->level - 1);

			if (exp_loss > 0)
			{
				gain_exp(victim, 0 - exp_loss / 2);
			}

			/*
				Knights Death     - Chaos 9/14/95
			*/

			if (victim->level > 91 && victim->level < 96 && victim->in_room->area->low_r_vnum != ROOM_VNUM_ARENA)
			{
				demote_level(victim, TRUE);

				knight_adjust_hpmnmv(victim);
			}

			if (victim->in_room->area->low_r_vnum != ROOM_VNUM_ARENA)
			{
				if (IS_SET(victim->act, PLR_KILLER) || IS_SET(victim->act, PLR_THIEF))
				{
					if (victim->pcdata->exp > exp_level(victim->class, victim->level - 2)+1)
					{
						gain_exp( victim, exp_level(victim->class, (victim->level)-2 )+1- victim->pcdata->exp);
					}
					if (IS_SET(victim->act, PLR_KILLER))
					{
						REMOVE_BIT(victim->act, PLR_KILLER);
					}
					if (IS_SET(victim->act, PLR_THIEF))
					{
						REMOVE_BIT(victim->act, PLR_THIEF);
					}
				}
			}
		}
		else
		{
			group_gain(ch, victim);
		}

		if (ch == victim)
		{
			if (ch->position == POS_DEAD)
			{
				if (IS_NPC(ch))
				{
					log_printf("[%u] %s just killed itself using damage type %d.", ch->pIndexData->vnum, ch->short_descr, dt);
				}
				else
				{
					log_printf("%s just killed itself using damage type %d.", get_name(ch), dt);
				}
				raw_kill(ch);
			}
			pop_call();
			return;
		}

		if (!confused)
		{
			set_fighting(victim, ch);
		}

		auto_loot = FALSE;
		auto_sac = FALSE;
		if (!IS_NPC(ch) && IS_NPC(victim))
		{
			if (IS_SET(ch->act, PLR_AUTOSAC))
			{
				auto_sac = TRUE;
			}
			if (IS_SET(ch->act, PLR_AUTOLOOT) || IS_SET(ch->act, PLR_AUTO_SPLIT))
			{
				auto_loot = TRUE;
			}
		}

		if (!IS_NPC(ch) && !IS_NPC(victim) && ch->in_room->area->low_r_vnum == ROOM_VNUM_ARENA)
		{
			PLAYER_GAME *gpl;
			sprintf( buf, "%s has killed %s!\n\r", ch->name, victim->name );
			for (gpl = mud->f_player ; gpl ; gpl = gpl->next )
			{
				if (gpl->ch != ch && gpl->ch != victim &&  gpl->ch->in_room->area->low_r_vnum == ROOM_VNUM_ARENA )
				{
					send_to_char( buf, gpl->ch );
				}
			}
		}

		raw_kill( victim );

		if ((obj = get_obj_list(ch, "npc_corpse", ch->in_room->first_content)) != NULL && can_see_obj(ch, obj))
		{
			if ( auto_loot )
			{
				FIGHT_DATA *fptr;

				fptr = ch->fighting;
				ch->fighting = NULL;
				do_get( ch, NULL );
				ch->fighting = fptr;
			}
			if (auto_sac && rspec_req(ch, RSPEC_VAMPIRIC) && ch->pcdata->condition[COND_THIRST] < 48)
			{
				do_drink(ch, "npc_corpse");
			}
			if ( auto_sac )
			{
				do_sacrifice(ch, "npc_corpse");
			}
		}

		if (ch->desc && HAS_BIT(CH(ch->desc)->pcdata->vt100_flags, VT100_INTERFACE))
		{
			vt100prompt(ch);
		}
		pop_call();
		return;
	}

	if (!IS_NPC(victim) && victim->hit > 0 && victim->hit <= victim->pcdata->wimpy && victim->wait == 0 && !IS_AFFECTED(ch, AFF2_BERSERK))
	{
		if (victim->desc != NULL && victim->desc->incomm[0] == '\0')
		{
			SET_BIT(victim->pcdata->interp, INTERP_AUTO);
			str_cpy_max(victim->desc->incomm, "flee", MAX_INPUT_LENGTH);
		}
	}

	if (ch->desc)
	{
		if (HAS_BIT(CH(ch->desc)->pcdata->vt100_flags, VT100_INTERFACE))
		{
			vt100prompt(ch);
		}
	}

	if (victim->desc)
	{
		if (HAS_BIT(CH(victim->desc)->pcdata->vt100_flags, VT100_INTERFACE))
		{
			vt100prompt(victim);
		}
	}
	pop_call();
	return;
}

void damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
	OBJ_DATA *wield;
	bool confused;

	push_call("damage(%p,%p,%p,%p)",ch,victim,dam,dt);

	if (victim->position == POS_DEAD)
	{
		pop_call();
		return;
	}

	if (dt != TYPE_NOFIGHT)
	{
		if (is_safe(ch, NULL))
		{
			/*
				Disallow damage in shops and safe place.
			*/
			stop_fighting(ch, FALSE);
			pop_call();
			return;
		}

		ch->furniture = NULL;

		if (ch->fighting == NULL && ch != victim)
		{
			set_fighting(ch,victim);
		}

		if (victim->fighting == NULL && ch != victim)
		{
			set_fighting(victim,ch);
		}

		if (ch->position == POS_STANDING)
		{
			ch->position = POS_FIGHTING;
		}

		if (victim != ch)
		{
			if (IS_AFFECTED(ch, AFF_HIDE))
			{
				affect_strip(ch, gsn_hide);
				REMOVE_BIT(ch->affected_by, AFF_HIDE);
				act( "$n steps out from $s hiding spot.",   ch, NULL, NULL, TO_ROOM );
				act( "You step out from your hiding spot.", ch, NULL, NULL, TO_CHAR );
			}

			if (IS_AFFECTED(ch, AFF_STEALTH))
			{
				affect_strip(ch, gsn_stealth);
				affect_strip(ch, gsn_greater_stealth);
				REMOVE_BIT(ch->affected_by, AFF_STEALTH);
				act( "$n steps out into the room.", ch, NULL, NULL, TO_ROOM );
				act( "You step out into the room.", ch, NULL, NULL, TO_CHAR );
			}

			if (IS_AFFECTED(ch, AFF_INVISIBLE))
			{
				affect_strip(ch, gsn_invis);
				REMOVE_BIT(ch->affected_by, AFF_INVISIBLE);
				if (!IS_AFFECTED(ch, AFF_IMP_INVISIBLE))
				{
					act( "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
				}
			}

			if (has_mirror(ch, victim, dt) || has_icicle_armor(ch, victim, dt))
			{
				if (ch->position < POS_FIGHTING)
				{
					ch->position++;
				}
				pop_call();
				return;
			}
		}
	}


	if (IS_SET(ch->in_room->room_flags, ROOM_ICE) && !CAN_FLY(ch))
	{
		if (number_percent() > 80 && number_percent() > get_curr_dex(ch))
		{
			switch (number_bits(1))
			{
				case 0:
					act("You slip on the ice, falling flat on your back.",		ch, NULL, NULL, TO_CHAR);
					act("$n slips on the ice, falling flat on $s back.",		ch, NULL, NULL, TO_ROOM);
					break;
				case 1:
					act("You lose your footing on the slippery ice and fall.",	ch, NULL, NULL, TO_CHAR);
					act("$n loses $s footing on the slippery ice and falls.",	ch, NULL, NULL, TO_ROOM);
					break;
			}
			wait_state(ch, PULSE_VIOLENCE / 4);

			ch->position = POS_SITTING;

			pop_call();
			return;
		}
	}

	if (dt != TYPE_NOFIGHT)
	{
		switch (ch->position)
		{
			case POS_SLEEPING:
				if (!IS_AFFECTED(ch, AFF_SLEEP))
				{
					ch->position = POS_RESTING;
				}
				pop_call();
				return;

			case POS_RESTING:
			case POS_SITTING:
				do_stand(ch,"");
				pop_call();
				return;
		}
	}


	/*
     	Confusion spell   - Chaos  7/26/96
	*/

	confused = FALSE;

	if (IS_AFFECTED(ch, AFF2_CONFUSION) && number_range(0,4) == 0 && dt != TYPE_NOFIGHT)
	{
		CHAR_DATA *fch;
		int tch, pch;

		for (tch = 0, fch = ch->in_room->first_person ; fch != NULL ; fch = fch->next_in_room)
		{
			if (!can_see(ch, fch))
			{
				continue;
			}

			if (!can_attack(ch, fch) && ch != fch)
			{
				continue;
			}
			tch++;
		}
		if (tch > 1)
		{
			pch = number_range(1, tch);
			for (tch = 0, fch = ch->in_room->first_person ; fch != NULL ; fch = fch->next_in_room )
			{
				if (!can_see(ch, fch))
				{
					continue;
				}

				if (!can_attack(ch, fch) && ch != fch)
				{
					continue;
				}
				tch++;
				if (tch == pch)
				{
					victim = fch;
					confused = TRUE;
					break;
				}
			}
		}
	}

	/*
		Set recently fought info.
	*/

	if (IS_NPC(victim) && !IS_NPC(ch) && !confused && dam > 0 && dt != TYPE_NOFIGHT)
	{
		if (ch->leader && !IS_NPC(ch->leader))
		{
			victim->npcdata->pvnum_last_hit = ch->leader->pcdata->pvnum;
		}
		else
		{
			victim->npcdata->pvnum_last_hit = ch->pcdata->pvnum;
		}
	}

	if (dam && IS_NPC(victim) && ch != victim && dt != TYPE_NOFIGHT)
	{
		if (ch->level > 25 && victim->npcdata->hate_fear != ch->name)
		{
			RESTRING(victim->npcdata->hate_fear, ch->name);
		}
	}

	/*
		Damage enhancers
	*/

	if (!IS_NPC(ch) && !is_spell(dt) && dt != TYPE_NOFIGHT)
	{
		if (get_curr_str(ch) > 29)
		{
			dam = dam * get_curr_str(ch) / 29;
		}
		if (multi(ch, gsn_enhanced_damage) != -1)
		{
			dam += dam * URANGE(15, multi_skill_level(ch, gsn_enhanced_damage)/2, 35) * learned(ch, gsn_enhanced_damage) / 10000;
			check_improve(ch, gsn_enhanced_damage);
		}
	}

	/*
		Stop up any residual loopholes.
		Modified moving dam cap  -  Chaos  4/27/99
	*/

	if (dt == gsn_backstab)
	{
		int dam_cap;
		int used_reincs;

		used_reincs = IS_NPC(ch) ? 0 : URANGE(0, ch->pcdata->reincarnation, 100);

		if (number_percent() < learned(ch, gsn_greater_backstab))
		{
			dam_cap = 800+ch->level*10 + (250*used_reincs);
		}
		else
		{
			dam_cap = 800+ch->level*6 + (250*used_reincs);
		}

		dam_cap = number_fuzzy(dam_cap);

		if (dam > dam_cap)
		{
			dam = dam_cap;
		}
	}

	if (victim != ch)
	{
		/*
			Certain attacks are forbidden.
			Most other attacks are Returned.
		*/

		if (victim->position > POS_STUNNED && dt != TYPE_NOFIGHT)
		{
			if (victim->fighting == NULL && !confused )
			{
				set_fighting(victim, ch);
			}
		}

		if (ch->position > POS_STUNNED && ch != victim && dt != TYPE_NOFIGHT)
		{
			if (ch->fighting == NULL && !confused)
			{
				set_fighting( ch, victim );
			}
			/*
				If victim is charmed, ch might attack victim's master.
			*/
			if (IS_NPC(ch) && IS_NPC(victim) && IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL && victim->master->in_room == ch->in_room && number_bits(4) == 0 && dt != TYPE_NOFIGHT)
			{
				stop_fighting(ch, FALSE);
				multi_hit(ch, victim->master, TYPE_UNDEFINED);
				pop_call();
				return;
			}
		}


		/*
			Add poison off of item
		*/

		if ((wield = get_eq_char(ch, WEAR_WIELD)) != NULL && wield->poison != NULL && dam > 0)
		{
			POISON_DATA *pd;

			if (IS_NPC(ch) || ch->pcdata->pvnum != wield->poison->owner)
			{
				while ((pd = wield->poison) != NULL)
				{
					wield->poison = wield->poison->next;
					FREEMEM(pd);
				}
			}
			else
			{
				if (ch->in_room->area->low_r_vnum == ROOM_VNUM_ARENA)
				{
					wield->poison->poisoner = -1;
				}
				else
				{
					wield->poison->poisoner = ch->pcdata->pvnum;
				}

				wield->poison->next = victim->poison;
				victim->poison		= wield->poison;
			}
			wield->poison = NULL;
		}

		if (dt == gsn_knife && !IS_AFFECTED(victim, AFF2_BLEEDING) && dam > 0)
		{
			if (multi(ch, gsn_critical_hit) != -1 && number_percent() < learned(ch, gsn_critical_hit) / 4)
			{
				AFFECT_DATA af;

				af.type      = gsn_critical_hit;
				af.duration  = 0;
				af.modifier  = 20 + number_range(1, multi_skill_level(ch, gsn_critical_hit)/2);
				af.location  = APPLY_NONE;
				af.bittype   = AFFECT_TO_CHAR;
				af.bitvector = AFF2_BLEEDING;
				if (!IS_NPC(ch))
				{
					victim->critical_hit_by = ch->pcdata->pvnum;
				}
				else
				{
					victim->critical_hit_by = 0;
				}
				affect_join( victim, &af );

				SET_BIT(victim->affected2_by, 0-AFF2_BLEEDING);
				ch_printf(ch, "%sYou feel particularly satisfied as...\n\r", get_color_string(ch,COLOR_YOU_HIT,VT102_BOLD));
				check_improve(ch, gsn_critical_hit);
			}
		}

		/*
			Check for disarm, trip, parry, and dodge.
		*/

 		if (dt >= TYPE_HIT)
		{
			if (IS_NPC(ch) && ch->desc == NULL)
			{
				if (number_percent() < ch->level / 30 + 1)
				{
					trip(ch, victim);
				}
				if (number_percent() < ch->level / 30 + 1)
				{
					disarm(ch, victim);
				}
			}
			if (victim->position > POS_RESTING)
			{
				if (check_parry(ch, victim))
				{
					pop_call();
					return;
				}
				if (check_dodge(ch, victim))
				{
					pop_call();
					return;
				}
				if (check_shield_block(ch, victim))
				{
					pop_call();
					return;
				}
			}
		}
		else if (victim->position > POS_RESTING && !is_spell(dt))
		{
			if (check_tumble(ch, victim))
			{
				pop_call();
				return;
			}
		}
	}

	if (dt >= TYPE_HIT)
	{
		OBJ_DATA *obj;
		if (IS_SET(mud->flags, MUD_DUALFLIP) && get_eq_char(ch, WEAR_DUAL_WIELD) != NULL)
		{
			obj = get_eq_char(ch, WEAR_DUAL_WIELD);
		}
		else
		{
			obj = get_eq_char(ch, WEAR_WIELD);
		}
		if (obj && IS_SET(obj->extra_flags, ITEM_BURNING))
		{
			dt = gsn_flame_blade;

			if (dam > 0)
			{
				dam += dice(1, 1 + obj->level / 10);
			}
		}
	}

	dam = damage_modify(ch, victim, dt, dam);

	if (dam > 0 && is_affected(victim, gsn_mana_shield))
	{
		int ms_red, ms_dam;

		ms_red = URANGE(25, 100 * victim->mana / UMAX(1, victim->max_mana), 75);
		ms_dam = UMIN(dam*ms_red/100, victim->mana);

		dam -= ms_dam;
		victim->mana -= ms_dam;
	}

	if (dt != gsn_anatomy)
	{
		dam_message(ch, victim, dam, dt);
	}
	else
	{
		dam = victim->hit;
	}

	if (dam > 0 && is_affected(victim, gsn_mana_shield))
	{
		if (100*victim->mana/victim->max_mana < 25)
		{
			act("$n flares brightly as $s mana shield shatters!", victim, NULL, NULL, TO_ROOM);
			send_to_char("You flare brightly as your mana shield shatters!\n\r", victim);
			affect_strip(victim, gsn_mana_shield);
		}
	}

	damage_hurt(ch, victim, dam, dt);

	pop_call();
	return;
}


bool has_mirror(CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
	AFFECT_DATA *paf;
	char message[MAX_INPUT_LENGTH];
	const char *attack;
	bool all;

	push_call("has_mirror(%p,%p)",ch,victim);

	attack = NULL;
	all    = FALSE;

	if (IS_AFFECTED(ch, AFF_TRUESIGHT))
	{
		pop_call();
		return FALSE;
	}

	if (!IS_AFFECTED(victim, AFF2_MIRROR_IMAGE))
	{
		pop_call();
		return FALSE;
	}

	if (dt == TYPE_NOFIGHT)
	{
		pop_call();
		return FALSE;
	}

	SET_BIT(ch->attack, ATTACK_MIRROR);

	for (paf = victim->first_affect ; paf ; paf = paf->next)
	{
		if (paf->bitvector == AFF2_MIRROR_IMAGE)
		{
			if (number_range(0, paf->modifier))
			{
				paf->modifier -= 1;

				if (paf->modifier <= 0)
				{
					all = TRUE;
					affect_strip(victim, paf->type);
				}
			}
			else
			{
				act( "You discover the real $T.", ch, NULL, short_to_name(get_name(victim), 1), TO_CHAR);
				act( "Your images disappear.",    ch, NULL, victim, TO_VICT    );
				act( "$N's images disappear.",    ch, NULL, victim, TO_NOTVICT );
				affect_strip(victim, paf->type);

				pop_call();
				return FALSE;
			}
			break;
		}
	}
	body_part_attack = get_body_part(ch, victim, dt, 0);
	attack = get_dam_nounce(ch, victim, dt);

	if (attack[0] != '\0')
	{
		sprintf(message, "Your %s destroys $N in one blow!", attack);
		act(message, ch, NULL, victim, TO_CHAR);
		sprintf(message, "$n's %s destroys $N in one blow!", attack);
		act(message, ch, NULL, victim, TO_ROOM);
	}
	else
	{
		act("You destroy $N in one blow!", ch, NULL, victim, TO_CHAR);
		act("$n destroys $N in one blow!", ch, NULL, victim, TO_ROOM);
	}

	if (all)
	{
		act( "You discover the real $T.", ch, NULL, short_to_name(get_name(victim), 1), TO_CHAR    );
		act( "Your images disappear.",    ch, NULL, victim, TO_VICT    );
		act( "$N's images disappear.",    ch, NULL, victim, TO_NOTVICT );
	}
	pop_call();
	return TRUE;
}


bool has_icicle_armor(CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
	AFFECT_DATA *paf;
	char message[MAX_INPUT_LENGTH];
	const char *attack = NULL;

	push_call("has_icicle_armor(%p,%p)",ch,victim);

	if (!IS_AFFECTED(victim, AFF2_ICICLE_ARMOR))
	{
		pop_call();
		return FALSE;
	}

	if (dt == TYPE_NOFIGHT || is_spell(dt))
	{
		pop_call();
		return FALSE;
	}

	for (paf = victim->first_affect ; paf ; paf = paf->next)
	{
		if (paf->bitvector == AFF2_ICICLE_ARMOR)
		{
			if (number_range(0, 200) < paf->modifier * -1)
			{
				body_part_attack = get_body_part(ch, victim, dt, 0);
				attack = get_dam_nounce(ch, victim, dt);

				if (attack[0] == '\0')
				{
					attack = "hit";
				}

				affect_modify(victim, paf, FALSE);

				paf->modifier += 5;

				affect_modify(victim, paf, TRUE);

				if (paf->modifier < 0)
				{
					sprintf(message, "Your %s shatters a spike of $N's icicle armor into a hundred pieces!", attack);
					act(message, ch, NULL, victim, TO_CHAR);
					sprintf(message, "$n's %s shatters a spike of your icicle armor into a hundred pieces!", attack);
					act(message, ch, NULL, victim, TO_VICT);
					sprintf(message, "$n's %s shatters a spike of $N's icicle armor into a hundred pieces!", attack);
					act(message, ch, NULL, victim, TO_NOTVICT);
				}
				else
				{
					sprintf(message, "Your %s splinters $N's icicle armor into a thousand pieces!", attack);
					act(message, ch, NULL, victim, TO_CHAR);
					sprintf(message, "$n's %s splinters your icicle armor into a thousand pieces!", attack);
					act(message, ch, NULL, victim, TO_VICT);
					sprintf(message, "$n's %s splinters $N's icicle armor into a thousand pieces!", attack);
					act(message, ch, NULL, victim, TO_NOTVICT);

					affect_strip(victim, paf->type);
				}
				pop_call();
				return TRUE;
			}
			break;
		}
	}
	pop_call();
	return FALSE;
}

/*
	true if victim is a legal target for a mass attack - Scandum 27/05/2002
*/

bool can_mass_attack( CHAR_DATA *ch, CHAR_DATA *victim )
{
	push_call("can_mass_attack(%p,%p)",ch,victim);

	if (victim == ch)
	{
		pop_call();
		return FALSE;
	}

	if (victim->fighting && victim->fighting->who == ch)
	{
		pop_call();
		return TRUE;
	}

	if (victim->fighting && !is_same_group(ch, victim->fighting->who))
	{
		pop_call();
		return FALSE;
	}

	if (IS_NPC(victim) && IS_AFFECTED(victim, AFF_ETHEREAL))
	{
		pop_call();
		return FALSE;
	}

	if (is_same_group(ch, victim))
	{
		pop_call();
		return FALSE;
	}

	if (ch->in_room->area->low_r_vnum == ROOM_VNUM_ARENA)
	{
		pop_call();
		return TRUE;
	}

	if (IS_NPC(victim) && IS_AFFECTED(victim, AFF_CHARM))
	{
		pop_call();
		return FALSE;
	}

	if (!IS_NPC(ch) && !IS_NPC(victim))
	{
		pop_call();
		return FALSE;
	}

	if (check_recently_fought(ch, victim))
	{
		pop_call();
		return FALSE;
	}
	pop_call();
	return TRUE;
}

/*
	Rewrote this function to put all the no fighting crap in one
	neat function, charmed and possessed mobs attack like players.

	Scandum 18/02/02
*/

bool is_safe( CHAR_DATA *ch, CHAR_DATA *victim)
{
	push_call("is_safe(%p,%p)",ch,victim);

	if (find_keeper(ch) != NULL)
	{
		ch_printf(ch, "%s%s tells you 'No fighting in here! You thug!'\n\r", get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(find_keeper(ch)->short_descr));
		pop_call();
		return TRUE;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_char( "You cannot do that here.\n\r", ch);
		pop_call();
		return TRUE;
	}

	if (victim == NULL)
	{
		pop_call();
		return FALSE;
	}

	if (who_fighting(ch) == victim)
	{
		pop_call();
		return FALSE;
	}

	if (who_fighting(victim) == ch)
	{
		pop_call();
		return FALSE;
	}

	if (ch == victim)
	{
		send_to_char( "You may not do that.\n\r", ch );
		pop_call();
		return TRUE;
	}

	if (IS_NPC(victim) && IS_AFFECTED(victim, AFF_CHARM) && victim->master)
	{
		if (victim->master == ch)
		{
			raw_kill(victim);
			pop_call();
			return TRUE;
		}
		send_to_char( "You cannot kill a slave.\n\r", ch );
		pop_call();
		return TRUE;
	}

	if (ch->in_room->area->low_r_vnum == ROOM_VNUM_ARENA)
	{
		pop_call();
		return FALSE;
	}

	if (IS_NPC(ch) && ch->fighting == NULL && ch->hit < ch->max_hit / 2)
	{
		if (!IS_AFFECTED(ch, AFF_CHARM) && ch->desc == NULL)
		{
			pop_call();
			return TRUE;
		}
	}

	if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc != NULL)
	{
		if (!IS_NPC(victim))
		{
			send_to_char( "You must MURDER a player.\n\r", ch);
			pop_call();
			return TRUE;
		}
	}

	if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
	{
		act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR);
		pop_call();
		return TRUE;
	}

	if (check_recently_fought(ch, victim))
	{
		ch_printf(ch, "%s was recently fought.  Try later.\n\r", capitalize(victim->short_descr));
		pop_call();
		return TRUE;
	}

	if (IS_NPC(victim) && victim->fighting != NULL)
	{
		if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || ch->desc != NULL)
		{
			if (!is_same_group(ch, victim->fighting->who))
			{
				ch_printf(ch, "%s seems to be in combat already!\n\r", capitalize(victim->short_descr));
				pop_call();
				return TRUE;
			}
		}
	}

	pop_call();
	return FALSE;
}

bool check_recently_fought( CHAR_DATA *ch, CHAR_DATA *victim )
{
	CHAR_DATA *ch_ld;
	int pvnum_ld;

	push_call("check_recently_fought(%p,%p)",ch,victim);

	if (!IS_NPC(victim))
	{
		pop_call();
		return FALSE;
	}

	if (victim->npcdata->pvnum_last_hit == 0)
	{
		pop_call();
		return FALSE;
	}

	if (MP_VALID_MOB(ch))
	{
		pop_call();
		return FALSE;
	}

	ch_ld = ch;

	if (IS_NPC(ch) && ch->master)
	{
		ch_ld = ch_ld->master;
	}

	if (ch_ld->leader != NULL)
	{
		ch_ld = ch_ld->leader;
	}

	if (!IS_NPC(ch_ld))
	{
		pvnum_ld = ch_ld->pcdata->pvnum;
	}
	else
	{
		pvnum_ld = 0;
	}

	if (ch_ld == NULL || IS_NPC(ch_ld))
	{
		pop_call();
		return TRUE;
	}

	if (pvnum_ld != victim->npcdata->pvnum_last_hit)
	{
		pop_call();
		return TRUE;
	}
	pop_call();
	return FALSE;
}

/*
	See if an attack justifies a KILLER flag.
*/

void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{
	push_call("check_killer(%p,%p)",ch,victim);

	if (IS_NPC(ch) || IS_NPC(victim))
	{
		pop_call();
		return;
	}

	if (IS_SET(victim->act, PLR_KILLER))
	{
		pop_call();
		return;
	}

	if (!IS_SET(ch->act, PLR_KILLER))
	{
		send_to_char("*** You are now a KILLER!! ***\n\r", ch);
	}
	SET_BIT(ch->act, PLR_KILLER);
	ch->pcdata->killer_played = ch->pcdata->played;

	pop_call();
	return;
}

int check_flash(CHAR_DATA *ch, CHAR_DATA *victim)
{
	push_call("check_flash(%p,%p)",ch,victim);

	if (IS_AFFECTED(victim, AFF2_HAS_FLASH))
	{
		if (number_percent() < learned(victim, gsn_flash_powder) / 2)
		{
			AFFECT_DATA af;

			REMOVE_BIT(victim->affected2_by, 0-AFF2_HAS_FLASH);
			act( "$n throws some powder onto the ground and there is a bright flash!", victim, NULL, NULL, TO_ROOM );
			act( "You throw some powder onto the ground and there is a bright flash!", victim, NULL, ch, TO_CHAR );

			if (!IS_AFFECTED(ch, AFF_TRUESIGHT))
			{
				af.type      = gsn_flash_powder;
				af.bittype   = AFFECT_TO_CHAR;
				af.bitvector = AFF_BLIND;
				af.location  = APPLY_HITROLL;
				af.duration  = 0;
				af.modifier  = -6;
				affect_to_char(ch, &af);

				pop_call();
				return TRUE;
			}
			else
			{
				act( "$N is unaffected by the bright flash!", ch, NULL, victim, TO_VICT );
				act( "You ignore the effects of the flash.", ch, NULL, victim, TO_CHAR );
			}
		}
		else
		{
			send_to_char( "You fumble the flash powder and it spills everywhere!\n\r", victim);
			REMOVE_BIT(victim->affected2_by, 0-AFF2_HAS_FLASH);
		}
	}
	pop_call();
	return FALSE;
}

bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim )
{
	int chance;
	bool cross_parry, riposte;

	push_call("check_parry(%p,%p)",ch,victim);

	if (IS_SET(victim->attack, ATTACK_PARRY))
	{
		pop_call();
		return FALSE;
	}

	if (get_eq_char(victim, WEAR_WIELD) == NULL)
	{
		pop_call();
		return FALSE;
	}

	if (!IS_NPC(victim) && multi(victim, gsn_parry) == -1)
	{
		pop_call();
		return( FALSE );
	}

	riposte     = (!IS_NPC(victim) && number_percent() < learned(victim, gsn_riposte));
	cross_parry = (get_eq_char(victim, WEAR_DUAL_WIELD) != NULL);

	chance = learned(victim, gsn_parry) / (5 - cross_parry - riposte);

	riposte     =     riposte ? number_bits(1) : FALSE;
	cross_parry = cross_parry ? number_bits(1) : FALSE;

	if (OGRE_INTIMIDATE)
	{
		chance     -= 15;
		riposte     = FALSE;
	}

	if (number_percent() > chance)
	{
		pop_call();
		return FALSE;
	}
	else
	{
		SET_BIT(victim->attack, ATTACK_PARRY);
	}

	if (!IS_NPC(ch) && number_percent() < learned(ch, gsn_precision)/3)
	{
		act( "You try to parry $n's attack, but $s aim is too precise.",		ch, NULL, victim, TO_VICT);
		act( "$N tries to parry your attack, but your aim is too precise.",	ch, NULL, victim, TO_CHAR);

		check_improve(ch, gsn_precision);

		pop_call();
		return FALSE;
	}

	if (victim->desc && !IS_SET(CH(victim->desc)->pcdata->spam, 8))
	{
		if (riposte)
		{
			act( "You parry and riposte $n's attack.", ch, NULL, victim, TO_VICT);
		}
		else if (cross_parry)
		{
			act( "You cross-parry $n's attack.", ch, NULL, victim, TO_VICT);
		}
		else
		{
			act( "You parry $n's attack.", ch, NULL, victim, TO_VICT);
		}
	}

	if (ch->desc && !IS_SET(CH(ch->desc)->pcdata->spam, 2))
	{
		if (riposte)
		{
			act( "$N parries and ripostes your attack.", ch, NULL, victim, TO_CHAR);
		}
		else if (cross_parry)
		{
			act( "$N cross-parries your attack.", ch, NULL, victim, TO_CHAR);
		}
		else
		{
			act( "$N parries your attack.", ch, NULL, victim, TO_CHAR);
		}
	}

	if (riposte)
	{
		one_hit(victim, ch, TYPE_UNDEFINED);
		check_improve(victim, gsn_riposte);
	}
	else
	{
		check_improve(victim, gsn_parry);
	}
	pop_call();
	return TRUE;
}

bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim )
{
	int chance;

	push_call("check_dodge(%p,%p)",ch,victim);

	if (IS_SET(victim->attack, ATTACK_DODGE))
	{
		pop_call();
		return FALSE;
	}

	if (!learned(victim, gsn_dodge))
	{
		pop_call();
		return( FALSE );
	}

	chance = UMIN(multi_skill_level(victim, gsn_dodge)/2+5, learned(victim, gsn_dodge) / 3);

	if (OGRE_INTIMIDATE)
	{
		chance -= 15;
	}

	if (number_percent() > chance)
	{
		pop_call();
		return FALSE;
	}
	else
	{
		SET_BIT(victim->attack, ATTACK_DODGE);
	}

	if (!IS_NPC(ch) && number_percent() < learned(ch, gsn_precision)/3)
	{
		act( "You try to dodge $n's attack, but $s aim is too precise.",		ch, NULL, victim, TO_VICT);
		act( "$N tries to dodge your attack, but your aim is too precise.",	ch, NULL, victim, TO_CHAR);

		check_improve(ch, gsn_precision);

		pop_call();
		return FALSE;
	}

	if (victim->desc && !IS_SET(CH(victim->desc)->pcdata->spam, 8))
	{
		act( "You dodge $n's attack.", ch, NULL, victim, TO_VICT);
	}

	if (ch->desc && !IS_SET(CH(ch->desc)->pcdata->spam, 2))
	{
		act( "$N dodges your attack.", ch, NULL, victim, TO_CHAR);
	}
	check_improve(victim, gsn_dodge);
	pop_call();
    	return TRUE;
}

bool check_shield_block( CHAR_DATA *ch, CHAR_DATA *victim )
{
	int chance = 0;

	push_call("check_shield_block(%p,%p)",ch,victim);

	if (IS_SET(victim->attack, ATTACK_BLOCK))
	{
		pop_call();
		return FALSE;
	}

	if (get_eq_char(victim, WEAR_SHIELD) == NULL)
	{
		pop_call();
		return FALSE;
	}

	if (!learned(victim, gsn_shield_block))
	{
		pop_call();
		return( FALSE );
	}
	else
	{
		chance = UMIN(multi_skill_level(victim, gsn_shield_block)/2+5, learned(victim, gsn_shield_block) / 3);
	}

	if (OGRE_INTIMIDATE)
	{
		chance -= 15;
	}

	if (number_percent() > chance)
	{
		pop_call();
		return FALSE;
	}
	else
	{
		SET_BIT(victim->attack, ATTACK_BLOCK);
	}

	if (!IS_NPC(ch) && number_percent() < learned(ch, gsn_precision)/3)
	{
		act( "You try to dodge $n's attack, but $s aim is too precise.",		ch, NULL, victim, TO_VICT);
		act( "$N tries to dodge your attack, but your aim is too precise.",	ch, NULL, victim, TO_CHAR);

		check_improve(ch, gsn_precision);

		pop_call();
		return FALSE;
	}

	if (victim->desc && !IS_SET(CH(victim->desc)->pcdata->spam, 8))
	{
		act( "You block $n's attack.", ch, NULL, victim, TO_VICT);
	}

	if (ch->desc && !IS_SET(CH(ch->desc)->pcdata->spam, 2))
	{
		act( "$N blocks your attack.", ch, NULL, victim, TO_CHAR);
	}
	check_improve(victim, gsn_shield_block);
	pop_call();
	return TRUE;
}


bool check_tumble( CHAR_DATA *ch, CHAR_DATA *victim)
{
	int chance;

	push_call("check_tumble(%p,%p)",ch,victim);

	if (IS_SET(victim->attack, ATTACK_TUMBLE))
	{
		pop_call();
		return FALSE;
	}

	if (IS_NPC(victim) && (!IS_SET(victim->act, ACT_CLASS) || victim->class != CLASS_NINJA))
	{
		pop_call();
		return FALSE;
	}

	if (!learned(victim, gsn_tumble))
	{
		pop_call();
		return( FALSE );
	}

	chance = UMIN(multi_skill_level(victim, gsn_tumble)/2+5, learned(victim, gsn_tumble) / 3);

	if (OGRE_INTIMIDATE)
	{
		chance -= 15;
	}

	if (number_percent() > chance)
	{
		pop_call();
		return FALSE;
	}
	else
	{
		SET_BIT(victim->attack, ATTACK_TUMBLE);
	}

	if (ch == NULL)
	{
		act( "You flip back on your feet.", victim, NULL, NULL, TO_CHAR);
		act( "$n flips back on his feet.",  victim, NULL, NULL, TO_ROOM);
	}
	else
	{
		if (victim->desc && !IS_SET(CH(victim->desc)->pcdata->spam, 8))
		{
			act( "You tumble away from $n's attack.", ch, NULL, victim, TO_VICT);
		}

		if (ch->desc && !IS_SET(CH(ch->desc)->pcdata->spam, 2))
		{
			act( "$N tumbles away from your attack.", ch, NULL, victim, TO_CHAR);
		}
	}
	check_improve(victim, gsn_tumble);
	pop_call();
    	return TRUE;
}

/*
	Set position of a victim.
*/

void update_pos( CHAR_DATA *victim )
{
	push_call("update_pos(%p)",victim);

	if (victim->fighting && in_camp(victim))
	{
		CHAR_DATA *gch;
		for (gch = victim->in_room->first_person ; gch ; gch = gch->next_in_room)
		{
			if (is_same_group(victim, gch) && IS_AFFECTED(gch, AFF2_CAMPING))
			{
				send_to_char( "Your camp is destroyed!\n\r", victim );
				REMOVE_BIT(gch->affected2_by, 0-AFF2_CAMPING);
				break;
			}
		}
	}

	if (victim->hit > 0)
	{
		if (victim->position <= POS_STUNNED)
		{
			victim->position = POS_STANDING;
		}
		pop_call();
		return;
	}

	if (IS_NPC(victim) || victim->hit <= -11)
	{
		victim->position = POS_DEAD;
		pop_call();
		return;
	}

	if (victim->hit <= -6)
	{
		victim->position = POS_MORTAL;
	}
	else if (victim->hit <= -3)
	{
		victim->position = POS_INCAP;
	}
	else
	{
		victim->position = POS_STUNNED;
	}
	pop_call();
	return;
}

void free_fight( CHAR_DATA *ch )
{
	push_call("free_fight(%p)",ch);

	if (ch->fighting != NULL)
	{
		if ((ch->fighting->prev == NULL && ch->fighting != mud->f_fight)
		||  (ch->fighting->next == NULL && ch->fighting != mud->l_fight))
		{
			log_printf("UNLINK ERROR free_fight: fight data not found on %s", ch->name);
			dump_stack();
			pop_call();
			return;
		}

		UNLINK(ch->fighting, mud->f_fight, mud->l_fight, next, prev);
		FREEMEM(ch->fighting);
	}

	ch->position = POS_STANDING;

	if (!IS_NPC(ch))
	{
		ch->pcdata->asn_obj = NULL;
	}
	update_pos(ch);

	pop_call();
	return;
}

/*
	Start fights.
*/

void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
	FIGHT_DATA *fight;

	push_call("set_fighting(%p,%p)",ch,victim);

	if (ch == victim)
	{
		pop_call();
		return;
	}

	if (ch->fighting)
	{
		pop_call();
		return;
	}

	if (ch->in_room != victim->in_room)
	{
		pop_call();
		return;
	}

	if (IS_AFFECTED(ch, AFF_SLEEP))
	{
		affect_strip(ch, gsn_sleep);
	}

	ALLOCMEM(fight, FIGHT_DATA, 1);

	fight->who	= victim;
	fight->ch		= ch;

	LINK(fight, mud->f_fight, mud->l_fight, next, prev);

	ch->fighting	= fight;

	if (ch->position == POS_STANDING)
	{
		ch->position = POS_FIGHTING;
	}

	if (!IS_NPC(ch))
	{
		ch->pcdata->last_combat = mud->current_time;
	}

	if (!IS_NPC(victim))
	{
		victim->pcdata->last_combat = mud->current_time;
	}

	update_pos(ch);

	pop_call();
	return;
}

/*
	Make a corpse out of a character.
*/

void make_corpse( CHAR_DATA *ch )
{
	char buf[MAX_INPUT_LENGTH];
	OBJ_DATA *corpse;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	push_call("make_corpse(%p)",ch);

	if (IS_NPC(ch) && IS_SET(ch->act, ACT_UNDEAD))
	{
		for (obj = ch->first_carrying ; obj ; obj = obj_next)
		{
			obj_next = obj->next_content;

			if (IS_SET(obj->extra_flags, ITEM_INVENTORY))
			{
				junk_obj( obj );
			}
			else
			{
				obj_to_room( obj, ch->in_room->vnum );
			}
		}
		pop_call();
		return;
	}

	if (IS_NPC(ch))
	{
		corpse             = create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
		corpse->timer      = number_range( 2, 4 );
		corpse->level      = ch->level;
		corpse->owned_by   = 0;

		if (ch->fighting != NULL)
		{
			if (IS_NPC(ch->fighting->who) && IS_AFFECTED(ch->fighting->who, AFF_CHARM))
			{
				if (!IS_NPC(ch->fighting->who->master))
				{
					corpse->owned_by = ch->fighting->who->master->pcdata->pvnum;
				}
			}
			if (!IS_NPC(ch->fighting->who))
			{
				corpse->owned_by = ch->fighting->who->pcdata->pvnum;
			}
			if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL && ch->master->pcdata!=NULL )
			{
				corpse->owned_by = ch->master->pcdata->pvnum;
			}
		}
		if (ch->gold > 0)
		{
			obj_to_obj(create_money(ch->gold), corpse);
			ch->gold = 0;
		}
		if (IS_SET(ch->act, ACT_MOUNT))
		{
			SET_BIT(corpse->extra_flags, ITEM_MOUNT);
		}
	}
	else
	{
		corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
		corpse->timer	= 60;

		SET_BIT(corpse->extra_flags, ITEM_MODIFIED);

		if (ch->pcdata->corpse == NULL)
		{
			ch->pcdata->corpse      = corpse;
			ch->pcdata->corpse_room = ch->in_room->vnum;
		}
		corpse->owned_by = ch->pcdata->pvnum;
	}

	sprintf(buf, "corpse %s %s", IS_NPC(ch) ? "npc_corpse" : "pc_corpse", short_to_name(get_name(ch), 1));
	RESTRING(corpse->name, buf);

	sprintf(buf, "the corpse of %s", get_name(ch));
	RESTRING(corpse->short_descr, buf);

	sprintf(buf, "The corpse of %s is lying here.", get_name(ch));
	RESTRING(corpse->long_descr, buf);

	for (obj = ch->first_carrying ; obj ; obj = obj_next)
	{
		obj_next = obj->next_content;

		if (!IS_SET(obj->extra_flags, ITEM_INVENTORY) && !IS_SET(obj->extra_flags, ITEM_NODROP))
		{
			obj_to_obj( obj, corpse );
		}
	}

	obj_to_room(corpse, ch->in_room->vnum);

	if (!IS_NPC(ch))
	{
		ch->pcdata->time_of_death		= mud->current_time;
		ch->pcdata->just_died_ctr		= 10;
		ch->pcdata->condition[COND_FULL]	= 48;
		ch->pcdata->condition[COND_THIRST]	= 48;
		ch->pcdata->condition[COND_DRUNK]	=  0;

		ch_printf(ch, "%s\n\r", god_table[ch->pcdata->god].death_msg);
	}
	pop_call();
	return;
}

/*
	Improved Death_cry contributed by Diavolo.
*/

void death_cry( CHAR_DATA *ch )
{
	EXIT_DATA *pexit;
	char *msg;
	int door, vnum, ran, temp_vnum;
	bool IS_BODY;

	push_call("death_cry(%p)",ch);

	ran     = number_bits(4);
	vnum    = 0;
	IS_BODY = (IS_NPC(ch) && (IS_SET(ch->act, ACT_BODY) || IS_SET(ch->act, ACT_RACE)));

	if (IS_BODY && number_bits(2) == 0 && !IS_SET(ch->act, ACT_UNDEAD))
	{
		ran = get_body_part(ch, NULL, TYPE_HIT, 1);

		if (ran == -1)
		{
			msg = "$n splatters blood on your armor.";
		}
		else
		{
			vnum = OBJ_VNUM_SLICED_LEG;
			msg  = body_table[ran].sliced;
		}
	}
	else
	{
		msg  = "You hear $n's death cry.";
	}

	if (IS_SET( ch->act, ACT_UNDEAD))
	{
		vnum = 0;
		ran = 7;
	}

	if (vnum == 0)
	{
		switch (ran)
		{
			default:
				break;
			case  0:
				msg  = "$n hits the ground ... DEAD.";
				break;
			case  1:
				msg  = "$n splatters blood on your armor.";
				break;
			case  2:
				if (IS_BODY)
				{
					break;
				}
				msg  = "You smell $n's sphincter releasing in death.";
				vnum = OBJ_VNUM_FINAL_TURD;
				break;
			case  3:
				if (IS_BODY)
				{
					break;
				}
				msg  = "$n's severed head plops on the ground.";
				vnum = OBJ_VNUM_SEVERED_HEAD;
				break;
			case  4:
				if (IS_BODY)
				{
					break;
				}
				msg  = "$n's heart is torn from $s chest.";
				vnum = OBJ_VNUM_TORN_HEART;
				break;
			case  5:
				if (IS_BODY)
				{
					break;
				}
				msg  = "$n's arm is sliced from $s dead body.";
				vnum = OBJ_VNUM_SLICED_ARM;
				break;
			case  6:
				if (IS_BODY)
				{
					break;
				}
				msg  = "$n's leg is sliced from $s dead body.";
				vnum = OBJ_VNUM_SLICED_LEG;
				break;
			case 7:
				msg = "$n turns to dust.";
				break;
		}
	}
	if (!IS_SET(ch->act, ACT_ELEMENTAL))
	{
		act( msg, ch, NULL, NULL, TO_ROOM );
	}

	if (vnum != 0)
	{
		char buf[MAX_INPUT_LENGTH];
		OBJ_DATA *obj;
		char *name;

		name            = IS_NPC(ch) ? ch->short_descr : ch->name;
		obj             = create_object( get_obj_index( vnum ), 0 );
		obj->timer      = number_range( 4, 7 );

		SET_BIT(obj->extra_flags, ITEM_MODIFIED);

		if (IS_BODY)
		{
			sprintf(buf, body_table[ran].short_descr, name);
		}
		else
		{
			sprintf(buf, obj->short_descr, name);
		}
		RESTRING(obj->short_descr, buf);

		if (IS_BODY)
		{
			sprintf(buf, body_table[ran].long_descr, capitalize(name));
		}
		else
		{
			sprintf(buf, obj->long_descr, name);
		}
		RESTRING(obj->long_descr, buf);

		if (IS_BODY)
		{
			sprintf(buf, body_table[ran].name, name);
			RESTRING(obj->name, buf);

			sprintf(buf, body_table[ran].description, capitalize(name));
			RESTRING(obj->description, buf);
		}
		obj_to_room(obj, ch->in_room->vnum);
	}

	if (IS_NPC(ch))
	{
		msg = "You hear something's death cry.";
	}
	else
	{
		msg = "You hear someone's death cry.";
	}

	temp_vnum = ch->in_room->vnum;

	for (door = 0 ; door <= 5 ; door++)
	{
		if ((pexit = get_exit(temp_vnum, door)) != NULL)
		{
			ch->in_room = room_index[pexit->to_room];
			act( msg, ch, NULL, NULL, TO_ROOM );
		}
	}
	ch->in_room = room_index[temp_vnum];

	pop_call();
	return;
}

void raw_kill( CHAR_DATA *victim )
{
	FIGHT_DATA	*fight,	*fight_next;
	PET_DATA		*pet,	*pet_next;

	push_call("raw_kill(%p)",victim);

	if (IS_NPC(victim))
	{
		if (IS_SET(victim->act, ACT_WILL_DIE))
		{
			pop_call();
			return;
		}
		SET_BIT(victim->act, ACT_WILL_DIE);
	}

	for (fight = mud->f_fight ; fight ; fight = fight_next)
	{
		fight_next = fight->next;

		if (IS_NPC(fight->ch) && fight->ch != victim)
		{
			if (who_fighting(fight->ch) == victim && IS_SET(fight->ch->act, ACT_ONE_FIGHT) && !SET_BIT(victim->act, ACT_WILL_DIE))
			{
				raw_kill(fight->ch);
				fight_next = mud->f_fight;
			}
		}
	}

	for (pet = mud->f_pet ; pet ; pet = pet_next)
	{
		pet_next = pet->next;

		if (IS_NPC(pet->ch) && pet->ch != victim)
		{
			if (pet->ch->master == victim && IS_AFFECTED(pet->ch, AFF_CHARM) && !IS_SET(pet->ch->act, ACT_WILL_DIE))
			{
				raw_kill(pet->ch);
				pet_next = mud->f_pet;
			}
		}
	}

	if (victim->poison)
	{
		POISON_DATA *pd;

		while ((pd = victim->poison) != NULL)
		{
			victim->poison = victim->poison->next;
			FREEMEM( pd );
		}
	}

	while (victim->first_affect)
	{
		affect_from_char(victim, victim->first_affect);
	}
	victim->affected_by		= 0;
	victim->affected2_by	= 0;

	if (!IS_NPC(victim))
	{
		if (victim->pcdata->shadowing)
		{
			stop_shadow(victim);
		}

		if (victim->pcdata->shadowed_by)
		{
			stop_shadow(victim->pcdata->shadowed_by);
		}

		if (victim->in_room->area->low_r_vnum == ROOM_VNUM_ARENA)
		{
			arena_death(victim);
		}
		else
		{
			player_death(victim);
		}
		pop_call();
		return;
	}

	make_corpse( victim );

	stop_fighting(victim, TRUE);

	if (IS_AFFECTED(victim, AFF_CHARM) && victim->master && !IS_SET(victim->act, ACT_ELEMENTAL))
	{
		send_to_char("Your pet dies a horrible death.\n\r", victim->master);
	}

	junk_mob(victim);

	if (victim->desc && victim->desc->original)
	{
		victim->desc->original->hit -= 50;

		send_to_char("You suffer from the death of your imp!\n\r", victim);

		do_return(victim, NULL);
	}
	pop_call();
	return;
}


void arena_death( CHAR_DATA *victim )
{
	push_call("arena_death(%p)",victim);

	victim->position 		= POS_RESTING;
	victim->hit      		= UMAX( 1, victim->hit  );
	victim->mana     		= UMAX( 1, victim->mana );
	victim->move     		= UMAX( 1, victim->move );

	stop_fighting(victim, TRUE);

	char_from_room(victim);
	char_to_room(victim, ROOM_VNUM_TEMPLE);

	char_reset(victim);

	pop_call();
	return;
}

void player_death( CHAR_DATA *victim )
{
	OBJ_DATA *obj, *obj_next;

	push_call("player_death(%p)",victim);

	if (victim->pcdata->death_room == ROOM_VNUM_SCHOOL)
	{
		if (victim->in_room->area->low_r_vnum != ROOM_VNUM_SCHOOL)
		{
			victim->pcdata->death_room = ROOM_VNUM_TEMPLE;
		}
	}

	check_asn_obj(victim);

	stop_fighting(victim, TRUE);

	make_corpse(victim);

	for (obj = victim->first_carrying ; obj ; obj = obj_next)
	{
		obj_next = obj->next_content;
		if (!IS_SET(obj->extra_flags, ITEM_INVENTORY))
		{
			junk_obj( obj );
		}
	}

	char_from_room(victim);

	if (victim->level < 2)
	{
		char_to_room(victim, ROOM_VNUM_SCHOOL);
	}
	else
	{
		char_to_room(victim, victim->pcdata->death_room);
	}
	victim->pcdata->recall = victim->in_room->vnum;

	victim->position 		= POS_RESTING;
	victim->hit      		= UMAX( 1, victim->hit  );
	victim->mana     		= UMAX( 1, victim->mana );
	victim->move     		= UMAX( 1, victim->move );

	char_reset(victim);

	save_char_obj(victim, NORMAL_SAVE);

	pop_call();
	return;
}

void group_gain(CHAR_DATA *ch, CHAR_DATA *victim)
{
	CHAR_DATA *gch;
	int xp, cnt, group_level, group_count;
	char ip_list[4][20];

	push_call("group_gain(%p,%p)",ch,victim);

	if (IS_AFFECTED(victim, AFF_CHARM))
	{
		pop_call();
		return;
	}

	if (IS_NPC(victim) && victim->pIndexData->vnum == 9901)
	{
		pop_call();
		return;
	}

	morph_update(ch, victim);

	group_level = group_count = 1;

	for (cnt = 0 ; cnt < 4 ; cnt++)
	{
		ip_list[cnt][0] = 0;
	}

	for (gch = ch->in_room->first_person ; gch ; gch = gch->next_in_room)
	{
		if (is_same_group(gch, ch))
		{
			if (group_level < gch->level)
			{
				group_level = gch->level;
			}

			if (!IS_NPC(gch))
			{
				for (cnt = 0 ; cnt < 4 ; cnt++)
				{
					if (ip_list[cnt][0] == 0)
					{
						strcpy(ip_list[cnt], gch->pcdata->host);
						break;
					}

					if (!strcmp(ip_list[cnt], gch->pcdata->host))
					{
						group_count++;
						break;
					}
				}

				if (cnt == 4)
				{
					group_count++;
				}
			}

			if (victim->alignment / 50)
			{
				gch->alignment -= victim->alignment / 50;

				gch->alignment = URANGE(-1000, gch->alignment, 1000);

				check_zap(gch, TRUE);
			}
		}
	}

	for (gch = ch->in_room->first_person ; gch ; gch = gch->next_in_room)
	{
		if (!is_same_group(gch, ch) || IS_NPC(gch))
		{
			continue;
		}

		xp = xp_compute(gch, victim) * reinc_eq_level(gch) / UMAX(1, group_level);

		xp = xp * 100 / (80 + group_count * 20);

		if (gch->pcdata->reincarnation > 0)
		{
			for (cnt = 0 ; cnt < gch->pcdata->reincarnation ; cnt++)
			{
				xp /= 2;
			}
		}

		if (gch->level == 90)
		{
			xp /= 2;
		}

		if (gch->level > 90)
		{
			xp = xp * 2 / 3;
		}

		ch_printf(gch, god_table[which_god(gch)].exp_msg, xp);
		ch_printf(gch, "\n\r");

		gain_exp(gch, xp);


	}
	pop_call();
	return;
}


/*
	Calculate roughly how much experience a character is worth
*/

int get_exp_worth( CHAR_DATA *ch )
{
	int exp = 0;

	push_call("get_exp_worth(%p)",ch);

	exp += ch->max_hit;

	exp += get_damnodice(ch) * get_damsizedice(ch) + GET_DAMROLL(ch);

	pop_call();
	return exp;
}


/*
	Compute xp for a kill.
	Edit this function to change xp computations.
*/

int xp_compute( CHAR_DATA *ch, CHAR_DATA *victim )
{
	int xp;
	int spread;

	push_call("xp_compute(%p,%p)",ch,victim);

	if (ch->in_room == NULL)
	{
		pop_call();
		return(0);
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_RIP))
	{
		pop_call();
		return(0);
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_IS_CASTLE))
	{
		pop_call();
		return(0);
	}

	xp = exp_level(CLASS_MONSTER, victim->level) * (number_range(0, 50) / 100.0 + 0.75);

	if (!IS_NPC(victim))
	{
		xp = 0;
	}

	if (xp < 75)
	{
		xp = 75;
	}

	if (which_god(ch) != GOD_NEUTRAL)
	{
		xp = xp * (100 + ch->pcdata->reincarnation) / 100;
	}

	spread = 10 + reinc_eq_level(ch) / 10;

	if (reinc_eq_level(ch) > victim->level)
	{
		xp = xp * spread / (reinc_eq_level(ch) - victim->level + spread);
	}

	if (ch->in_room != NULL && ch->in_room->sector_type == SECT_ASTRAL)
	{
		xp += (xp * 35 / 100);
	}

	if (ch->in_room != NULL && ch->in_room->sector_type == SECT_ETHEREAL)
	{
		xp += (xp * 30 / 100);
	}

	xp = number_range(xp * 0.75, xp * 1.25);

	pop_call();
	return xp;
}


void dam_message( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
	CHAR_DATA *fch;
	OBJ_DATA	*wield;
	char buf1[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH], buf3[MAX_INPUT_LENGTH], ch_name[MAX_INPUT_LENGTH];
	char youHit[41],hitYou[41];
	const char *vs;
	const char *vp;
	const char *attack;
	char punct;
	int percent;

	push_call("dam_message(%p,%p,%d,%d)",ch,victim,dam,dt);

	if (dt == TYPE_NOFIGHT)
	{
		pop_call();
		return;
	}

	percent = 100 * dam / UMAX(1, victim->max_hit);

	attack = NULL;

	if ( dam == 0 )
	{
		vs = "miss";
		vp = "misses";
	}
	else if ( percent == 0 )
	{
		vs = "tickle";
		vp = "tickles";
	}
	else if ( percent <= 2 )
	{
		vs = "scratch";
		vp = "scratches";
	}
	else if ( percent <= 4 )
	{
		vs = "graze";
		vp = "grazes";
	}
	else if ( percent <= 6 )
	{
		vs = "hit";
		vp = "hits";
	}
	else if ( percent <= 9 )
	{
		vs = "injure";
		vp = "injures";
	}
	else if ( percent <= 12 )
	{
		vs = "wound";
		vp = "wounds";
	}
	else if ( percent <= 15 )
	{
		vs = "maul";
		vp = "mauls";
	}
	else if ( percent <= 18 )
	{
		vs = "decimate";
		vp = "decimates";
	}
	else if ( percent <= 21 )
	{
		vs = "devastate";
		vp = "devastates";
	}
	else if ( percent <= 25 )
	{
		vs = "maim";
		vp = "maims";
	}
	else if ( percent <= 29 )
	{
		vs = "MUTILATE";
		vp = "MUTILATES";
	}
	else if ( percent <= 33 )
	{
		vs = "* DISEMBOWEL *";
		vp = "* DISEMBOWELS *";
	}
	else if ( percent <= 37 )
	{
		vs = "! EVISCERATE !";
		vp = "! EVISCERATES !";
	}
	else if ( percent <= 41 )
	{
		vs = "** MASSACRE **";
		vp = "** MASSACRES **";
	}
	else if ( percent <= 50 )
	{
		vs = "!! DEMOLISH !!";
		vp = "!! DEMOLISHES !!";
	}
	else if ( percent <= 60 )
	{
		vs = "*** DISMEMBER ***";
		vp = "*** DISMEMBERS ***";
	}
	else if ( percent <= 75 )
	{
		vs = "!!! ANNIHILATE !!!";
		vp = "!!! ANNIHILATES !!!";
	}
	else if (percent <= 90 )
	{
		vs = "**** SLAUGHTER ****";
		vp = "**** SLAUGHTERS ****";
	}
	else
	{
		vs = "!!!! OBLITERATE !!!!";
		vp = "!!!! OBLITERATES !!!!";
	}

	punct = (percent <= 18) ? '.' : '!';

	strcpy(hitYou, (dam <= 0) ? "" : get_color_code(victim, COLOR_YOU_ARE_HIT, VT102_BOLD));
	strcpy(youHit, (dam <= 0) ? "" : get_color_code(ch, COLOR_YOU_HIT, VT102_BOLD));

	str_cpy_max(ch_name, get_name(ch), 64);

	if (dt == TYPE_HIT)
	{
		attack = get_dam_nounce(ch, victim, dt);

		if (attack[0] != '\0')
		{
			sprintf(buf1, "%s's %s %s %s%c", capitalize(ch_name), attack, vp, get_name(victim), punct);

			if (dam > 0)
			{
				sprintf(buf2, "%sYour %s %s %s for %dhp%c",  youHit, attack, vp, get_name(victim), dam, punct);
			}
			else
			{
				sprintf(buf2, "%sYour %s %s %s%c",  youHit, attack, vp, get_name(victim), punct);
			}
			if (dam > 0)
			{
				sprintf(buf3, "%s%s's %s %s you for %dhp%c", hitYou, capitalize(get_name(ch)), attack, vp, dam, punct);
			}
			else
			{
				sprintf(buf3, "%s%s's %s %s you%c", hitYou, capitalize(get_name(ch)), attack, vp, punct);
			}
		}
		else
		{
			sprintf(buf1, "%s %s %s%c", capitalize(ch_name), vp, get_name(victim), punct);

			if (dam > 0)
			{
				sprintf(buf2, "%sYou %s $N for %dhp%c", youHit, vs, dam, punct );
			}
			else
			{
				sprintf(buf2, "%sYou %s $N%c", youHit, vs, punct);
			}
			if (dam > 0)
			{
				sprintf(buf3, "%s%s %s you for %dhp%c", hitYou, capitalize(get_name(ch)), vp, dam, punct);
			}
			else
			{
				sprintf(buf3, "%s%s %s you%c", hitYou, capitalize(get_name(ch)), vp, punct);
			}
		}
	}
	else
	{
		attack = get_dam_nounce(ch, victim, dt);

		sprintf(buf1, "%s's %s %s %s%c", capitalize(ch_name), attack, vp, get_name(victim), punct);

		if (dam > 0)
		{
			sprintf( buf2, "%sYour %s %s $N for %dhp%c", youHit,attack,vp,dam,punct );
		}
		else
		{
			sprintf( buf2, "%sYour %s %s $N%c",	youHit,	 attack, vp, punct );
		}
		if (dam > 0)
		{
			sprintf(buf3, "%s%s's %s %s you for %dhp%c", hitYou, capitalize(get_name(ch)), attack, vp, dam, punct);
		}
		else
		{
			sprintf(buf3,"%s%s's %s %s you%c", hitYou, capitalize(get_name(ch)), attack, vp, punct);
		}
	}

	/*
		Spam code for combat			-	Chaos 12/20/94
	*/

	for (fch = ch->in_room->first_person ; fch ; fch = fch->next_in_room)
	{
		if (fch->desc && fch != ch && fch != victim)
		{
			if (fch->position > POS_SLEEPING)
			{
				bool	gch, gvic;
				gch = is_same_group( ch, fch );
				gvic = is_same_group( victim, fch );
				if (gch && !gvic)
				{
					if (dam <= 0 && !IS_SET(CH(fch->desc)->pcdata->spam, 32))
					{
						ch_printf(fch, "%s\n\r", buf1);
					}
					if (dam > 0  && !IS_SET(CH(fch->desc)->pcdata->spam, 16))
					{
						ch_printf(fch, "%s\n\r", buf1);
					}
				}
				else	if (!gch && gvic)
				{
					if (dam <= 0 && !IS_SET(CH(fch->desc)->pcdata->spam, 128))
					{
						ch_printf(fch, "%s\n\r", buf1);
					}
					else if (dam > 0 && !IS_SET(CH(fch->desc)->pcdata->spam, 64))
					{
						ch_printf(fch, "%s%s\n\r", get_color_string(fch,COLOR_PARTY_HIT,VT102_BOLD), buf1);
					}
				}
				else	if (!gch && !gvic)
				{
					if (dam <= 0 && !IS_SET(CH(fch->desc)->pcdata->spam, 512))
					{
						ch_printf(fch, "%s\n\r", buf1);
					}
					if (dam > 0 && !IS_SET(CH(fch->desc)->pcdata->spam, 256))
					{
						ch_printf(fch, "%s\n\r", buf1);
					}
				}
				else
				{
					ch_printf(fch, "%s\n\r", buf1);
				}
			}
		}
	}
	if (dam == 0)
	{
		if (ch->desc && !IS_SET(CH(ch->desc)->pcdata->spam, 2))
		{
			if (ch->position > POS_SLEEPING)
			{
				act(buf2, ch, NULL, victim, TO_CHAR);
			}
		}
		if (victim->desc && !IS_SET(CH(victim->desc)->pcdata->spam, 8))
		{
			if (victim->position > POS_SLEEPING)
			{
				act(buf3, ch, NULL, victim, TO_VICT);
			}
		}
	}
	else
	{
		if (ch->desc && !IS_SET(CH(ch->desc)->pcdata->spam, 1))
		{
			if (ch->position > POS_SLEEPING)
			{
				act(buf2, ch, NULL, victim, TO_CHAR);
			}
		}
		if (victim->desc && !IS_SET(CH(victim->desc)->pcdata->spam, 4))
		{
			if (victim->position > POS_SLEEPING)
			{
				act(buf3, ch, NULL, victim, TO_VICT);
			}
		}
	}

	if (dt > TYPE_HIT)
	{
		if ((wield = get_eq_char(ch, WEAR_DUAL_WIELD)) != NULL)
		{
			if (!IS_SET(mud->flags, MUD_DUALFLIP))
			{
				wield = get_eq_char( ch, WEAR_WIELD );
			}
		}
		else
		{
			wield = get_eq_char(ch, WEAR_WIELD);
		}

		if (wield == NULL)
		{
			pop_call();
			return;
		}

		if (wield->item_type == ITEM_WEAPON)
		{
			switch (wield->value[0])
			{
				case WEAPON_TYPE_SWORD:   check_improve(ch, gsn_sword_fighting);	break;
				case WEAPON_TYPE_DAGGER:  check_improve(ch, gsn_dagger_fighting);	break;
				case WEAPON_TYPE_AXE:     check_improve(ch, gsn_axe_fighting);    break;
				case WEAPON_TYPE_MACE:    check_improve(ch, gsn_mace_fighting);	break;
				case WEAPON_TYPE_STAFF:   check_improve(ch, gsn_staff_fighting);	break;
				case WEAPON_TYPE_WHIP:    check_improve(ch, gsn_whip_fighting);	break;
				case WEAPON_TYPE_FLAIL:   check_improve(ch, gsn_flail_fighting);	break;
				case WEAPON_TYPE_SPEAR:   check_improve(ch, gsn_spear_fighting);	break;
				case WEAPON_TYPE_CLAW:    check_improve(ch, gsn_claw_fighting);	break;
			}
		}
		oprog_damage_trigger(ch, wield, victim);
	}
	pop_call();
	return;
}


int get_body_part( CHAR_DATA *ch, CHAR_DATA *victim, int dt, int type )
{
	int part, pick, cnt, body;

	push_call("get_body_part(%p,%p,%p,%p)",ch,victim,dt,type);

	if (dt != TYPE_HIT)
	{
		pop_call();
		return -1;
	}

	if (IS_NPC(ch) && IS_SET(ch->act, ACT_BODY) && ch->pIndexData->attack_parts && type == 0)
	{
		body = ch->pIndexData->attack_parts;
	}
	else if (IS_NPC(ch) && IS_SET(ch->act, ACT_BODY) && ch->pIndexData->body_parts && type == 1)
	{
		body = ch->pIndexData->body_parts;
	}
	else if (!IS_NPC(ch) || IS_SET(ch->act, ACT_RACE))
	{
		body = race_table[ch->race].attack_parts;
	}
	else
	{
		pop_call();
		return -1;
	}

	for (cnt = part = 0 ; part < MAX_BODY ; part++)
	{
		if (IS_SET(body, 1 << part))
		{
			cnt++;
		}
	}
	pick = number_range(1, cnt);

	for (cnt = part = 0 ; part < MAX_BODY ; part++)
	{
		if (IS_SET(body, 1 << part) && ++cnt == pick)
		{
			pop_call();
			return part;
		}
	}
	pop_call();
	return -1;
}


/*
	Scandum 4-6-2003
*/

int damage_modify( CHAR_DATA *ch, CHAR_DATA *victim, int dt, int dam )
{
	int dam_type = DAM_THRUST;

	push_call("damage_modify(%p,%p,%p,%p)",ch,victim,dt,dam);

	if (dt == TYPE_HIT)
	{
		body_part_attack = get_body_part(ch, victim, dt, 0);
	}

	if (dam <= 0)
	{
		pop_call();
		return 0;
	}

	if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
	{
		if (ch->in_room->sector_type == SECT_ETHEREAL || ch->in_room->sector_type == SECT_ASTRAL)
		{
			if (dt != TYPE_HIT && dt != TYPE_UNDEFINED)
			{
				dam = 2 * dam / 3;
			}
		}
	}

	if (!IS_NPC(victim) && dam > 1 && number_percent() < learned(victim, gsn_resilience))
	{
		dam = UMAX(1, UMAX(dam - victim->level / 9, dam / 2));
		if (number_bits(2) == 0)
		{
			check_improve(victim, gsn_resilience);
		}
	}

	if (IS_AFFECTED(victim, AFF_SANCTUARY))
	{
		if (dam > 1)
		{
			dam = dam * 2/3;
		}
	}

	if (is_affected(victim, gsn_black_aura))
	{
		dam = dam * 5/4;
	}

	/*
		Gith Damage +25% astral, +25% ethereall
	*/

	if (ch->in_room)
	{
		if (ch->in_room->sector_type == SECT_ASTRAL && rspec_req(ch, RSPEC_ASTRAL))
		{
			dam = dam * 5/4;
		}
		if (ch->in_room->sector_type == SECT_ETHEREAL && rspec_req(ch, RSPEC_ETHEREAL))
		{
			dam = dam * 5/4;
		}
	}

	if (IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch))
	{
		dam = dam * 10/11;
	}

	if (IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch))
	{
		dam = dam * 10/11;
	}

	if (dt == TYPE_NOFIGHT)
	{
		pop_call();
		return dam;
	}

	if (dt == TYPE_HIT)
	{
		if (body_part_attack != -1)
		{
			dam_type = body_table[body_part_attack].dam_type;
		}
	}
	else
	{
		if (dt == gsn_martial_arts)
		{
			dam_type = martial_arts_table[martial_arts_attack].dam_type;
		}
		else if (dt == gsn_fisticuffs)
		{
			dam_type = fisticuffs_table[fisticuffs_attack].dam_type;
		}
		else if (dt == gsn_brawling)
		{
			dam_type = brawling_table[brawling_attack].dam_type;
		}
		else if (dt >= 0 && dt < MAX_SKILL)
		{
			dam_type = skill_table[dt].dam_type;
		}
		else if (dt >= TYPE_HIT)
		{
			if (dt < TYPE_HIT+sizeof(attack_table)/sizeof(attack_table[0]))
			{
				dam_type = attack_table[dt - TYPE_HIT].dam_type;
			}
		}
		else
		{
			if (dt != TYPE_UNDEFINED)
			{
				bug("damage_modify: bad dt %d.", dt);
			}
		}
	}

	if (dam <= 0)
	{
		dam = 1;
	}

	if (dam_type == DAM_NONE)
	{
		pop_call();
		return dam;
	}

	if (dam_type == god_table[which_god(victim)].resistance)
	{
		dam = dam * 75 / 100;
	}

	if (IS_UNDEAD(victim))
	{
		if (IS_SET(DAM_LIFE|DAM_POISON|DAM_EVIL, dam_type))
		{
			dam = dam * 75 / 100;
		}
	}

	if (IS_SET(race_table[victim->race].res_low, dam_type))
	{
		dam = dam * 90 / 100;
	}

	if (IS_SET(race_table[victim->race].res_high, dam_type))
	{
		dam = dam * 80 / 100;
	}

	if (IS_SET(race_table[victim->race].vul_low, dam_type))
	{
		dam = dam * 110 / 100;
	}

	if (IS_SET(race_table[victim->race].vul_high, dam_type))
	{
		dam = dam * 120 / 100;
	}

	if (dam <= 0)
	{
		dam = 1;
	}

	pop_call();
	return dam;
}


char *get_dam_nounce( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
	static char *attack;

	push_call("get_dam_nounce(%p,%p,%p)",ch,victim,dt);

	attack = "";

	if (dt == TYPE_HIT)
	{
		if (body_part_attack != -1)
		{
			attack = body_table[body_part_attack].attack;
		}
	}
	else
	{
		if (dt == gsn_martial_arts)
		{
			attack = martial_arts_table[martial_arts_attack].message;
		}
		else if (dt == gsn_fisticuffs)
		{
			attack = fisticuffs_table[fisticuffs_attack].message;
		}
		else if (dt == gsn_brawling)
		{
			attack = brawling_table[brawling_attack].message;
		}
		else if (dt == gsn_object_rape)
		{
			attack = &mpdamstring[0];
		}
		else if (dt >= 0 && dt < MAX_SKILL)
		{
			attack = skill_table[dt].noun_damage;
		}
		else if (dt >= TYPE_HIT)
		{
			OBJ_DATA *obj;
			if (IS_SET(mud->flags, MUD_DUALFLIP) && get_eq_char(ch, WEAR_DUAL_WIELD) != NULL)
			{
				obj = get_eq_char(ch, WEAR_DUAL_WIELD);
			}
			else
			{
				obj = get_eq_char(ch, WEAR_WIELD);
			}
			if (obj->pIndexData->attack_string[0] != '\0')
			{
				attack = obj->pIndexData->attack_string;
			}
			else if (dt < TYPE_HIT+sizeof(attack_table)/sizeof(attack_table[0]))
			{
				attack = attack_table[dt - TYPE_HIT].message;
			}
		}
		else
		{
			if (dt != TYPE_UNDEFINED)
			{
				bug("get_dam_nounce: bad dt %d.", dt);
			}
			attack	= attack_table[0].message;
		}
	}
	pop_call();
	return attack;
}

int get_damnodice( CHAR_DATA *ch)
{
	int dam = 0;

	push_call("get_damnodice(%p)",ch);

	if (IS_NPC(ch))
	{
		dam = ch->npcdata->damnodice;
	}
	else
	{
		if (learned(ch, gsn_fisticuffs) || learned(ch, gsn_martial_arts))
		{
			if (multi_skill_level(ch, gsn_fisticuffs) >= multi_skill_level(ch, gsn_martial_arts))
			{
				dam = 3 + multi_skill_level(ch, gsn_fisticuffs) * learned(ch, gsn_fisticuffs) / 150;
			}
			else
			{
				dam = 3 + multi_skill_level(ch, gsn_martial_arts) * learned(ch, gsn_martial_arts) / 150;
			}
			if (is_affected(ch, gsn_stone_fist))
			{
				dam += 3 + multi_skill_level(ch, gsn_stone_fist) / 20;
			}
		}
		else
		{
			dam = 1;
		}

	}
	pop_call();
	return dam;
}

int get_damsizedice( CHAR_DATA *ch)
{
	int dam = 0;

	push_call("get_damsizedice(%p)",ch);

	if (IS_NPC(ch))
	{
		dam = ch->npcdata->damsizedice;
	}
	else
	{
		if (learned(ch, gsn_fisticuffs) || learned(ch, gsn_martial_arts))
		{
			dam = 2;
		}
		else
		{
			dam = 4 + ch->level * 2 / 3;

			if (is_affected(ch, gsn_stone_fist))
			{
				dam += 10 + multi_skill_level(ch, gsn_stone_fist) / 10;
			}
		}
	}
	pop_call();
	return dam;
}

/*
	Disarm a creature.
	Caller must check for successful attack.
*/

void disarm( CHAR_DATA *ch, CHAR_DATA *victim )
{
	OBJ_DATA *obj;

	push_call("disarm(%p,%p)",ch,victim);

	if ((obj = get_eq_char(victim, WEAR_WIELD)) == NULL)
	{
		pop_call();
		return;
	}

	if (get_eq_char(victim, WEAR_DUAL_WIELD) != NULL && number_bits(1) == 0)
	{
		obj = get_eq_char(victim, WEAR_DUAL_WIELD);
	}

	if (IS_SET(obj->extra_flags,ITEM_INVENTORY))
	{
		send_to_char( "You failed.\n\r", ch );
		pop_call();
		return;
	}

	if (!can_see_obj(ch, obj))
	{
		if (number_percent() > 33)
		{
			send_to_char( "You failed.\n\r", ch);
			pop_call();
			return;
		}
	}

	if (get_eq_char(ch, WEAR_WIELD) == NULL && number_bits(1) == 0)
	{
		send_to_char( "You failed.\n\r", ch);
		pop_call();
		return;
	}

	if (IS_SET(ch->attack, ATTACK_DISARM))
	{
		pop_call();
		return;
	}
	else
	{
		SET_BIT(ch->attack, ATTACK_DISARM);
	}

	if (number_bits(1) == 0 && is_affected(victim, gsn_stone_fist))
	{
		act( "$n tries to disarm you, but your stone fist's grasp is unbreakable.", ch, NULL, victim, TO_VICT);
		act( "You try to disarm $N, but $S stone fist's grasp is unbreakable.",     ch, NULL, victim, TO_CHAR);
		pop_call();
		return;
	}

	act( "$n disarms you!", ch, NULL, victim, TO_VICT    );
	act( "You disarm $N!",  ch, NULL, victim, TO_CHAR    );
	act( "$n disarms $N!",  ch, NULL, victim, TO_NOTVICT );

	if (!IS_NPC(victim) && learned(victim, gsn_rearm) > number_percent())
	{
		if (can_see_in_room(victim, victim->in_room) && !IS_AFFECTED(victim, AFF_BLIND))
		{
			act( "You rearm yourself!!!", ch, NULL, victim, TO_VICT    );
			act( "$N rearms $Mself!!!",   ch, NULL, victim, TO_CHAR    );
			act( "$N rearms $Mself!!!",   ch, NULL, victim, TO_NOTVICT );
			check_improve(victim, gsn_rearm);
			pop_call();
			return;
		}
	}

	unequip_char(victim, obj);

	pop_call();
	return;
}

void do_cripple( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	AFFECT_DATA af;

	push_call("do_cripple(%p,%p)",ch,argument);

	if (!IS_NPC(ch) && multi(ch, gsn_cripple) == -1)
	{
		send_to_char( "You better leave the street fighting to the marauders.\n\r", ch);
		pop_call();
		return;
	}

	if (is_safe(ch, NULL))
	{
		pop_call();
		return;
	}

	if ((victim = who_fighting(ch)) == NULL)
	{
		send_to_char( "You aren't fighting anyone.\n\r", ch );
		pop_call();
		return;
	}

	wait_state(ch, skill_table[gsn_trip].beats);

	if (!IS_AFFECTED(ch, AFF_TRUESIGHT) && IS_AFFECTED(victim, AFF2_MIRROR_IMAGE))
	{
		damage(ch, victim, 0, gsn_cripple);
	}

	if (!check_hit(ch, victim, GET_HITROLL(ch), gsn_cripple))
	{
		ch_printf(ch, "%s evades your vicious kick.\n\r", capitalize(get_name(victim)));
		pop_call();
		return;
	}

	act( "$n cripples you with a well aimed kick!", ch, NULL, victim, TO_VICT    );
	act( "You cripple $N with a well aimed kick!", ch, NULL, victim, TO_CHAR    );
	act( "$n cripples $N with a well aimed kick!", ch, NULL, victim, TO_NOTVICT );

	check_improve(ch, gsn_cripple);

	af.type      = gsn_cripple;
	af.duration  = number_range(1,2);
	af.bitvector = 0;

	if (IS_NPC(victim))
	{
		af.modifier  = number_range(5, ch->level/5);
		af.location  = APPLY_AC;
	}
	else
	{
		af.modifier  = -1 * number_range(1, ch->level/20);
		af.location  = APPLY_DEX;
	}
	affect_join( victim, &af );

	victim->speed = UMIN(victim->speed, get_max_speed(victim));

	pop_call();
	return;
}

/*
	Trip, player version
*/

void do_trip( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char text2[MAX_STRING_LENGTH];

	push_call("do_trip(%p,%p)",ch,argument);

	if (!IS_NPC(ch) && multi(ch, gsn_trip) == -1)
	{
		send_to_char( "You better leave the tripping to the street fighters.\n\r", ch);
		pop_call();
		return;
	}

	if (is_safe(ch, NULL))
	{
		pop_call();
		return;
	}

	if ((victim = who_fighting(ch)) == NULL)
	{
		send_to_char( "You aren't fighting anyone.\n\r", ch );
		pop_call();
		return;
	}

	wait_state(ch, skill_table[gsn_trip].beats);

	if (!IS_AFFECTED(ch, AFF_TRUESIGHT) && IS_AFFECTED(victim, AFF2_MIRROR_IMAGE))
	{
		damage(ch, victim, 0, gsn_trip);
	}

	if (CAN_FLY(victim))
	{
		sprintf(text2,"%s flies over your outstretched leg.\n\r", capitalize(get_name(victim)));
		send_to_char(text2, ch);
		pop_call();
		return;
	}

	if (!check_hit(ch, victim, GET_HITROLL(ch), gsn_trip))
	{
		sprintf(text2,"%s jumps over your outstretched leg.\n\r", capitalize(get_name(victim)));
		send_to_char(text2, ch);
		pop_call();
		return;
	}

	act( "$n trips you and you go down!", ch, NULL, victim, TO_VICT    );
	act( "You trip $N and $E goes down!", ch, NULL, victim, TO_CHAR    );
	act( "$n trips $N and $E goes down!", ch, NULL, victim, TO_NOTVICT );
	check_improve(ch, gsn_trip);

	if (!check_tumble(NULL, victim))
	{
		victim->position = POS_RESTING;
		wait_state(victim, skill_table[gsn_trip].beats / 2);
	}
	pop_call();
	return;
}

/*
	Trip, mob version
*/

void trip( CHAR_DATA *ch, CHAR_DATA *victim )
{
	push_call("trip(%p,%p)",ch,victim);

	if (IS_SET(ch->attack, ATTACK_TRIP))
	{
		pop_call();
		return;
	}
	else
	{
		SET_BIT(ch->attack, ATTACK_TRIP);
	}

	if (victim->position <= POS_RESTING)
	{
		pop_call();
		return;
	}

	if (CAN_FLY(victim))
	{
		ch_printf(ch, "%s flies over your outstretched leg.\n\r", capitalize(get_name(victim)));
		pop_call();
		return;
	}

	act( "$n trips you and you go down!", ch, NULL, victim, TO_VICT    );
	act( "You trip $N and $E goes down!", ch, NULL, victim, TO_CHAR    );
	act( "$n trips $N and $E goes down!", ch, NULL, victim, TO_NOTVICT );

	if (!check_tumble(NULL, victim))
	{
		victim->position = POS_RESTING;
	}
	pop_call();
	return;
}

void do_kill( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	bool found;

	push_call("do_kill(%p,%p)",ch,argument);

	if (!IS_NPC(ch))
	{
		ch->pcdata->just_died_ctr = 0;
	}

	found  = FALSE;
	victim = NULL;

	one_argument(argument, arg);

	if (argument[0] == '\0')
	{
		if (is_safe(ch, NULL))
		{
			pop_call();
			return;
		}
		for (victim = ch->in_room->first_person ; victim != NULL ; victim = victim->next_in_room)
		{
			if (IS_NPC(victim) && ch != victim && ch->level <= victim->level-(ch->level*2+10))
			{
				if (can_see(ch,victim))
				{
					sprintf(buf, "%s doesn't think you should fight in here.\n\r", capitalize(victim->short_descr));
					send_to_char( buf, ch);
					pop_call();
					return;
				}
			}
		}
		for (victim = ch->in_room->first_person ; found == FALSE && victim != NULL ; victim = victim->next_in_room)
		{
			if (can_see(ch, victim) && !IS_AFFECTED(victim, AFF_CHARM)
			&& IS_NPC(victim) && ch != victim && !is_same_group(ch,victim))
			{
				found = TRUE;
				break;
			}
		}
		if (!found)
		{
			send_to_char( "You cannot see anyone you can attack.\n\r", ch);
			pop_call();
			return;
		}
	}

	if (!found)
	{
		if ((victim = get_char_room(ch, arg)) == NULL)
		{
			send_to_char( "They aren't here.\n\r", ch );
			pop_call();
			return;
		}
	}

	if (victim == ch)
	{
		send_to_char( "You hit yourself.  Ouch!\n\r", ch );
		pop_call();
		return;
	}

	if (is_safe(ch, victim))
	{
		pop_call();
		return;
	}

	if ( ch->position == POS_FIGHTING )
	{
		if (who_fighting(ch) == victim
		||  IS_NPC(ch)
		||  number_percent() > learned(ch, gsn_attack))
		{
			send_to_char( "You do the best you can!\n\r", ch);
			pop_call();
			return;
		}
		else
		{
			stop_fighting(ch, FALSE);
			check_improve(ch, gsn_attack);
		}
	}

	if (check_flash(ch, victim))
	{
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && ch->pcdata->auto_flags == AUTO_QUICK && (ch->hit > ch->pcdata->wimpy || IS_AFFECTED(ch, AFF2_BERSERK)) && ch->wait == 0)
	{
		int sn;
		bool ms	= FALSE;

		if (!str_prefix("mass ", ch->pcdata->auto_command))
		{
			ms = TRUE;
			sn = skill_lookup(&ch->pcdata->auto_command[5]);
		}
		else
		{
			sn = skill_lookup(ch->pcdata->auto_command);
		}

		if (sn != -1 && is_spell(sn) && ch->mana >= 3 * get_mana(ch, sn))
		{
			sprintf(buf, "'%s'", skill_table[sn].name);

			set_fighting(ch,victim);
			set_fighting(victim,ch);

			if (ms)
			{
				do_mass(ch, buf);
			}
			else
			{
				do_cast(ch, buf);
			}
			pop_call();
			return;
		}
	}

	wait_state(ch, PULSE_VIOLENCE/2);
	multi_hit(ch, victim, TYPE_UNDEFINED);

	pop_call();
	return;
}

void do_murde( CHAR_DATA *ch, char *argument )
{
	push_call("do_murde(%p,%p)",ch,argument);

	send_to_char( "If you want to MURDER, spell it out.\n\r", ch );

	pop_call();
	return;
}

void add_to_victory_list( CHAR_DATA *ch, CHAR_DATA *victim)
{
	int i;
	char buf[MAX_STRING_LENGTH];

	push_call("add_to_victory_list(%p,%p)",ch,victim);

	for (i = 1 ; i < VICTORY_LIST_SIZE ; i++)
	{
		STRFREE(victory_list[i-1]);
		victory_list[i-1] = STRALLOC(victory_list[i]);
	}
	sprintf(buf, "Z%12s (%2dx Level %2d) %12s (%2dx Level %2d) %s", ch->name, ch->pcdata->reincarnation, ch->level, victim->name, victim->pcdata->reincarnation, victim->level, get_time_string(mud->current_time));

	RESTRING(victory_list[i-1], buf);

	save_victors();
	pop_call();
	return;
}

void do_murder( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_INPUT_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	push_call("do_murder(%p,%p)",ch,argument);

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Murder whom?\n\r", ch );
		pop_call();
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		pop_call();
		return;
	}

	if (victim == ch)
	{
		send_to_char( "You hit yourself.  Ouch!\n\r", ch );
		pop_call();
		return;
	}

	if (is_safe(ch, NULL))
	{
		pop_call();
		return;
	}

	if (IS_NPC(ch) || IS_NPC(victim))
	{
		send_to_char( "You may not murder them.\n\r", ch );
		pop_call();
		return;
	}

	if (ch->in_room->area->low_r_vnum == ROOM_VNUM_ARENA)
	{
		send_to_char("Use kill instead.\n\r", ch);
		pop_call();
		return;
	}

	if (ch->position == POS_FIGHTING)
	{
		send_to_char( "You do the best you can!\n\r", ch );
		pop_call();
		return;
	}

	if (!can_attack (ch, victim) )
	{
		send_to_char( "You may not murder that person.\n\r", ch);
		pop_call();
		return;
	}

	if (victim->pcdata->just_died_ctr > 0)
	{
		send_to_char("That person is currently protected by their god.\n\r", ch);
		pop_call();
		return;
	}

	if (victim->fighting != NULL)
	{
		send_to_char( "You may not murder that person, at the moment.\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_SET(victim->act, PLR_KILLER) && !IS_SET(victim->act, PLR_THIEF))
	{
		if (victim->hit < victim->max_hit / 5)
		{
			send_to_char( "It would be dishonorable to murder someone so badly injured.\n\r", ch);
			pop_call();
			return;
		}
/*
		if (ch->pcdata->last_connect + 120 > mud->current_time)
		{
			ch_printf(ch, "You must wait %d seconds before you can murder anyone.\n\r", ch->pcdata->last_connect + 120 - mud->current_time);
			pop_call();
			return;
		}
*/
	}

	if (!IS_NPC(victim ) && victim->pcdata->corpse != NULL)
	{
		ch_printf(ch, "%s has been killed quite recently.\n\r", get_name(victim));
		pop_call();
		return;
	}

	/*
		Code for total and max pvnum attacks  -  Chaos 5/6/99
	*/

	if (!check_add_attack(ch, victim))
	{
		ch_printf(ch, "%s has been attacked too many times.\n\r", get_name(victim));
		pop_call();
		return;
	}


	ch->pcdata->just_died_ctr = 0;

	clear_attack_list(ch);

	check_killer( ch, victim );

	if (check_flash(ch, victim))
	{
		pop_call();
		return;
	}

	log_printf("ROOM [%u] MURDER %s attacking %s", ch->in_room->vnum, ch->name, victim->name);

	if (ch->pcdata->auto_flags == AUTO_QUICK && (ch->hit > ch->pcdata->wimpy || IS_AFFECTED(ch, AFF2_BERSERK)) && ch->wait == 0)
	{
		int sn;
		bool ms	= FALSE;

		if (!str_prefix("mass ", ch->pcdata->auto_command))
		{
			ms = TRUE;
			sn = skill_lookup(&ch->pcdata->auto_command[5]);
		}
		else
		{
			sn = skill_lookup(ch->pcdata->auto_command);
		}

		if (sn != -1 && is_spell(sn) && ch->mana >= 3 * get_mana(ch, sn))
		{
			sprintf(buf, "'%s'", skill_table[sn].name);

			set_fighting(ch, victim);
			set_fighting(victim, ch);

			if (ms)
			{
				do_mass(ch, buf);
			}
			else
			{
				do_cast(ch, buf);
			}
			pop_call();
			return;
		}
	}

	/*
		bs chance based on multi_skill_level - Scandum 12/02/02
	*/

	if (number_range(50, 99) < 100 * victim->hit / UMAX(1, victim->max_hit)
	&& check_hit(ch, victim, GET_HITROLL(ch), gsn_backstab)
	&& number_percent() < 100 * multi_skill_level(ch, gsn_backstab) / ch->level)
	{
		OBJ_DATA * obj;

		if ((obj = get_eq_char(ch,WEAR_WIELD)) != NULL && (obj->value[3] == WEAPON_STAB || obj->value[3] == WEAPON_PIERCE))
		{
			one_hit(ch, victim, gsn_backstab);
			wait_state(ch, skill_table[gsn_backstab].beats);
		}
		pop_call();
		return;
	}
	wait_state(ch, PULSE_VIOLENCE / 2);
	multi_hit(ch, victim, TYPE_UNDEFINED);

	pop_call();
	return;
}


void backstab( CHAR_DATA *ch, CHAR_DATA *victim )
{
	push_call("backstab(%p,%p)",ch,victim);

	/*
		Anatomy skill - Martin 4/8/98
	*/

	if (IS_NPC(victim) && !IS_NPC(ch) && victim->level < (ch->level-10)
	&& number_percent() < learned(ch, gsn_anatomy)/12)
	{
		if (number_bits(1) == 0)
		{
			act( "$n thrusts $p into $N's heart.", ch, get_eq_char( ch, WEAR_WIELD), victim, TO_ROOM);
			act( "You thrust $p into $N's heart.", ch, get_eq_char( ch, WEAR_WIELD), victim, TO_CHAR);
		}
		else
		{
			act( "$n slits $N's throat with $p.", ch, get_eq_char( ch, WEAR_WIELD), victim, TO_ROOM);
			act( "You slit $N's throat with $p.", ch, get_eq_char( ch, WEAR_WIELD), victim, TO_CHAR);
		}
		check_improve(ch, gsn_anatomy);
		one_hit( ch, victim, gsn_anatomy );
		pop_call();
		return;
	}
	one_hit( ch, victim, gsn_backstab );
	pop_call();
	return;
}

void do_backstab( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	push_call("do_backstab(%p,%p)",ch,argument);

	one_argument( argument, arg );

	if (!IS_NPC(ch) && multi(ch, gsn_backstab) == -1)
	{
		send_to_char("You are not that practiced a cut-throat.\n\r", ch );
		pop_call();
		return;
	}

	if (argument[0] == '\0')
	{
		for (victim = ch->in_room->first_person ; victim ; victim = victim->next_in_room)
		{
			if (IS_NPC(victim) && ch != victim && ch->level <= victim->level-(ch->level*2+10))
			{
				if (can_see(ch, victim))
				{
					ch_printf(ch, "%s doesn't think you should fight in here.\n\r", capitalize(victim->short_descr));
					pop_call();
					return;
				}
			}
		}

		for (victim = ch->in_room->first_person ; victim ; victim = victim->next_in_room)
		{
			if (can_see(ch, victim) && !IS_AFFECTED(victim, AFF_CHARM)
			&& IS_NPC(victim) && ch != victim && !is_same_group(ch,victim))
			{
				break;
			}
		}
		if (victim == NULL)
		{
			send_to_char( "You cannot see anyone you can backstab.\n\r", ch);
			pop_call();
			return;
		}
	}
	else
	{
		if ((victim = get_char_room(ch, arg)) == NULL)
		{
			send_to_char( "They aren't here.\n\r", ch );
			pop_call();
			return;
		}
	}

	if (victim == ch)
	{
		send_to_char( "How can you sneak up on yourself?\n\r", ch);
		pop_call();
		return;
	}

	if (is_safe(ch, victim))
	{
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && !IS_NPC(victim))
	{
		if (ch->in_room->area->low_r_vnum != ROOM_VNUM_ARENA)
		{
			send_to_char( "You may not backstab another player.\n\r", ch);
			pop_call();
			return;
		}
	}

	if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL || (obj->value[3] != 11 && obj->value[3] != 2))
	{
		send_to_char( "You need to wield a piercing weapon.\n\r", ch );
		pop_call();
		return;
	}

	if (victim->fighting != NULL)
	{
		send_to_char( "You can't backstab a fighting person.\n\r", ch );
		pop_call();
		return;
	}

	if (victim->hit < victim->max_hit/2)
	{
		act( "$N is hurt and suspicious ... you can't sneak up.", ch, NULL, victim, TO_CHAR );
		pop_call();
		return;
	}

	wait_state(ch, skill_table[gsn_backstab].beats);

	if (!check_hit(ch, victim, GET_HITROLL(ch), gsn_backstab)
	||  number_range(50, 100) > 100 * victim->hit / UMAX(1, victim->max_hit))
	{
		damage( ch, victim, 0, gsn_backstab);
		pop_call();
		return;
	}
	backstab(ch,victim);
	pop_call();
	return;
}

void do_flee( CHAR_DATA *ch, char *argument )
{
	ROOM_INDEX_DATA *was_in = NULL;
	ROOM_INDEX_DATA *now_in = NULL;
	CHAR_DATA *victim       = NULL;
	CHAR_DATA *fch          = NULL;
	EXIT_DATA *pexit        = NULL;
	int dir, attempt, exp, hunt_door, hunt_chance;

	int door, door_count;
	bool valid_exits[6]={FALSE,FALSE,FALSE,FALSE,FALSE,FALSE};
	char buf[MAX_INPUT_LENGTH];

	push_call("do_flee(%p,%p)",ch,argument);

	/*
		Check berserk and position for link lost players - Scandum 21-08-2002
	*/

	if (IS_AFFECTED(ch, AFF2_BERSERK))
	{
		send_to_char( "You are berserk!  Kill!  Kill!  Kill!\n\r", ch );
		pop_call();
		return;
	}

	if (ch->position < POS_FIGHTING)
	{
		send_to_char("Get up first!.\n\r", ch);
		pop_call();
		return;
	}

	if ((victim = who_fighting(ch)) == NULL)
	{
		send_to_char("You aren't fighting anyone.\n\r", ch);
		pop_call();
		return;
	}

	was_in = ch->in_room;
	now_in = ch->in_room;

	dir = -1;

	if (argument[0] && number_percent() < learned(ch, gsn_withdraw))
	{
		dir = direction_door(argument);
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_SMOKE)
	&& number_bits(2)
	&& !IS_AFFECTED(ch, AFF_TRUESIGHT))
	{
		send_to_char( "You blunder around and get lost in the smoke!\n\r", ch);
		if (!IS_NPC(ch) && !rspec_req(ch, RSPEC_EASY_FLEE))
		{
			wait_state(ch, PULSE_VIOLENCE / 4);
		}
		else
		{
			wait_state(ch, PULSE_VIOLENCE / 8);
		}
		pop_call();
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_ICE) && !CAN_FLY(ch))
	{
		if (number_bits(2) == 0 && number_percent() > get_curr_dex(ch))
		{
			switch (number_bits(1))
			{
				case 0:
					act("You try to flee but your feet slip on the icy surface.",		ch, NULL, NULL, TO_CHAR);
					act("$n tries to flee but $s feet slip on the icy surface.",		ch, NULL, NULL, TO_ROOM);
					break;
				case 1:
					act("Your feet slip as you try to flee and you crash to the ice.",	ch, NULL, NULL, TO_CHAR);
					act("$n's feet slip as $e tries to flee and $e crashes to the ice.",	ch, NULL, NULL, TO_ROOM);
					break;
			}
			wait_state(ch, PULSE_VIOLENCE / 4);

			ch->position = POS_SITTING;

			pop_call();
			return;
		}
	}

	if (is_affected(ch, gsn_cripple) && number_bits(1) == 0)
	{
		act("You try to limp away from combat, but $N does not let you get very far.", ch, NULL, victim, TO_CHAR);
		act("$n tries to limp away from combat, but you do not let $m get very far.",  ch, NULL, victim, TO_VICT);
		act("$n tries to limp away from combat, but $N does not let $m get very far.", ch, NULL, victim, TO_NOTVICT);
		if (!IS_NPC(ch) && !rspec_req(ch, RSPEC_EASY_FLEE))
		{
			wait_state( ch, PULSE_VIOLENCE/2 );
		}
		else
		{
			wait_state( ch, PULSE_VIOLENCE/4 );
		}
		pop_call();
		return;
	}

	for (door = 0, door_count = 0 ; door < 6 ; door++)
	{
		if (cflag(ch, CFLAG_QUICK_FLEE) || dir == door)
		{
			if (!can_move_char(ch, door))
			{
				continue;
			}
		}
		else
		{
			if ((pexit = get_exit(was_in->vnum, door)) == NULL)
			{
				continue;
			}
			if (!can_use_exit(ch, pexit))
			{
				continue;
			}
			if (IS_SET(pexit->flags, EX_CLOSED))
			{
				continue;
			}
			if (IS_NPC(ch))
			{
				if (IS_SET(room_index[pexit->to_room]->room_flags, ROOM_NO_MOB))
				{
					continue;
				}
				if (room_index[pexit->to_room]->area != ch->in_room->area)
				{
					continue;
				}
				if (IS_SET(ch->act, ACT_STAY_SECTOR) && ch->reset && room_index[pexit->to_room]->sector_type != room_index[ch->reset->arg3]->sector_type)
				{
					continue;
				}
			}
		}
		valid_exits[door] = TRUE;
		door_count++;
	}

	if (door_count == 0)
	{
		send_to_char("Ah!!!  You can't find an exit!\n\r",ch);
		if (!IS_NPC(ch) && !rspec_req(ch, RSPEC_EASY_FLEE))
		{
			wait_state( ch, PULSE_VIOLENCE/4 );
		}
		else
		{
			wait_state( ch, PULSE_VIOLENCE/8 );
		}
		pop_call();
		return;
	}

	if (!can_see_in_room(ch, now_in) || IS_AFFECTED(ch, AFF_BLIND) || IS_NPC(ch)
	|| (dir == -1 && !cflag(ch, CFLAG_QUICK_FLEE) && !rspec_req(ch,RSPEC_EASY_FLEE) && !IS_OUTSIDE(ch)))
	{
		for (attempt = 0 ; attempt < 6; attempt++)
		{
			door = number_door();
			if (valid_exits[door] != TRUE)
			{
				continue;
			}
			move_char(ch, door, TRUE);
			now_in = ch->in_room;
			break;
		}
	}
	else if (was_in == now_in)
	{
		if (dir == -1)
		{
			door = number_door();
		}
		else
		{
			door = dir;
		}
		while(valid_exits[door] != TRUE)
		{
			door++;
			if (door > 5)
			{
				door = 0;
			}
		}
		move_char(ch, door, TRUE);
		now_in = ch->in_room;
	}

	if (was_in != now_in)
	{
		char_from_room( ch );
		char_to_room( ch, was_in->vnum );
		act( "$n flees head over heels!", ch, NULL, NULL, TO_ROOM );
		char_from_room( ch );
		char_to_room( ch, now_in->vnum );
		act( "$n glances around for signs of pursuit.", ch, NULL, NULL, TO_ROOM);
	}
	if (was_in != now_in)
	{
		if (IS_NPC(ch) || ch->level >= 95)
		{
			send_to_char( "You flee head over heels from combat.\n\r", ch);
		}
		else
		{
			if (cflag(ch, CFLAG_QUICK_FLEE))
			{
				exp = 25+ch->level*ch->level*1.5;
			}
			else
			{
				exp = 50+ch->level*ch->level*2.5;
			}
			sprintf(buf, "You flee head over heels from combat, losing %d experience.\n\r", exp);
			send_to_char(buf, ch);
			gain_exp(ch, 0-exp);
		}
		check_improve(ch, gsn_withdraw);
	}
	/*
		Hunt skill - Chaos 11/19/93
	*/

	if (was_in != now_in)
	{
		for (fch = was_in->first_person ; fch ; fch = fch->next_in_room)
		{
			if (IS_NPC(fch))
			{
				if (was_in->area != now_in->area)
				{
					continue;
				}
				if (IS_SET(ch->act, ACT_STAY_SECTOR) && ch->reset && now_in->sector_type != room_index[ch->reset->arg3]->sector_type)
				{
					continue;
				}
			}

			if (fch->fighting
			&&  fch->fighting->who == ch
			&&  IS_AFFECTED(fch, AFF_HUNT)
			&& !IS_AFFECTED(fch, AFF_CHARM))
			{
				hunt_chance  = learned(fch, gsn_hunt)           / 4;
				hunt_chance += learned(fch, gsn_greater_hunt)   / 5;
				hunt_chance += 33 * multi_skill_level(fch, gsn_hunt) / fch->level;
				hunt_chance -= can_hear(fch, ch) ? 0 : 10;
				hunt_chance -= can_see(fch, ch)  ? 0 : 10;
				hunt_chance -= rspec_req(ch, RSPEC_EASY_FLEE) ? 10 : 0;
				hunt_chance += fch->speed * 10;
				hunt_chance -=  ch->speed * 10;
				hunt_chance -= learned(ch, gsn_withdraw) / 10;

				if (!IS_NPC(fch) && IS_SET(fch->act, PLR_WIZTIME))
				{
					ch_printf_color(fch, "{078}(do_flee() hunt_chance: %d)\n\r", hunt_chance);
				}
				if (number_percent() < hunt_chance)
				{
					if (IS_AFFECTED(fch, AFF_BLIND)) /* added a harder hunt for blinded hunters, Manwe */
					{
						hunt_door = number_door();

						if (fch->in_room->exit[hunt_door] != 0)
						{
							pexit = fch->in_room->exit[hunt_door];

							if (room_index[pexit->to_room] == NULL)
							{
								send_to_char("You try to hunt them down, but you blunder into a wall.\n\r",fch);
								act("$n starts hunting, but blunders into a wall!",fch, NULL, NULL, TO_ROOM);
							}
							else if (IS_SET(pexit->flags, EX_CLOSED) && !IS_AFFECTED(fch, AFF_PASS_DOOR) && !rspec_req(ch,RSPEC_PASS_DOOR))
							{
								send_to_char("You try to hunt them down, but you smash into a closed door.\n\r",fch);
								act("$n starts hunting, but smashes into a closed door!",fch, NULL, NULL, TO_ROOM);
							}
							else if (IS_SET(pexit->flags, EX_CLOSED) && IS_SET(pexit->flags, EX_PASSPROOF))
							{
								send_to_char("You try to hunt them down, but you smash into a closed door.\n\r",fch);
								act("$n starts hunting, but smashes into a closed door!",fch, NULL, NULL, TO_ROOM);
							}
							else
							{
								char_from_room(fch);
								char_to_room(fch, pexit->to_room);

								if (fch->in_room == ch->in_room)
								{
									send_to_char("You try to hunt them down, and now you SMELL them...\n\r",fch);
									send_to_char("You have been hunted down.\n\r",ch);
									stop_fighting(ch,TRUE);
									stop_fighting(fch,TRUE);
									set_fighting(ch,fch);
									set_fighting(fch,ch);
									pop_call();
									return;
								}
								else
								{
									send_to_char("You tried to hunt them down, but now you're lost.\n\r",fch);
									act("$n looks as if they're hunting, but looks somehow lost.",fch, NULL, NULL, TO_ROOM);
								}
							}
						}
						else
						{
							send_to_char("You try to hunt them down, but you blunder into a wall.\n\r",fch);
							act("$n starts hunting, but blunders into a wall!",fch, NULL, NULL, TO_ROOM);
						}
					}
					else
					{
						send_to_char("You hunt them down!\n\r", fch);
						check_improve(fch, gsn_hunt);
						check_improve(fch, gsn_greater_hunt);
						char_from_room( fch );
						char_to_room( fch, ch->in_room->vnum );
						send_to_char("You have been hunted down.\n\r", ch);

						stop_fighting( ch, TRUE);
						stop_fighting( fch, TRUE);
						set_fighting( ch, fch);
						set_fighting( fch, ch);
						pop_call();
						return;
					}
				}
			}
		}
	}

	if (was_in == now_in && !IS_SET(now_in->room_flags, ROOM_SAFE))
	{
		if (IS_NPC(ch) || ch->level >= 95)
		{
			send_to_char("You failed!\n\r", ch);
		}
		else
		{
			exp = 20+ch->level*ch->level*1;
			sprintf( buf, "You failed!  You lose %d exp.\n\r", exp);
			send_to_char( buf, ch);
			gain_exp( ch, -exp);
		}
		if (!IS_NPC(ch) && !rspec_req(ch, RSPEC_EASY_FLEE))
		{
			wait_state( ch, PULSE_VIOLENCE/4 );
		}
		else
		{
			wait_state( ch, PULSE_VIOLENCE/8 );
		}
		pop_call();
		return;
	}

	if (which_god(ch) != GOD_NEUTRAL && which_god(victim) != GOD_NEUTRAL)
	{
		if (which_god(ch) != which_god(victim) && ch->in_room->area->low_r_vnum != ROOM_VNUM_ARENA)
		{
			sprintf(buf, "%s", get_name(ch));
			if (ch->desc == NULL)
			{
				do_battle("%s's arcane connection to the realm has been severed like a coward while in battle with %s!", buf, get_name(victim));
			}
			else
			{
				do_battle("%s has fled like a coward from the wrath of %s!", buf, get_name(victim));
			}
		}
		stop_fighting(ch, TRUE);
		pop_call();
		return;
	}
	pop_call();
	return;
}


void do_rescue( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *fch;

	push_call("do_rescue(%p,%p)",ch,argument);

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Rescue whom?\n\r", ch );
		pop_call();
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char( "They aren't here.\n\r", ch );
		pop_call();
		return;
	}

	if (victim == ch)
	{
		send_to_char( "What about fleeing instead?\n\r", ch );
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && IS_NPC(victim) && victim->master!=ch)
	{
		send_to_char( "They do not need your help.\n\r", ch );
		pop_call();
		return;
	}

	if ( !ch->fighting )
	{
		send_to_char( "Too late.\n\r", ch );
		pop_call();
		return;
	}

	for (fch = ch->in_room->first_person ; fch != NULL ; fch = fch->next_in_room)
	{
		if (fch->fighting && fch->fighting->who == victim)
		{
			break;
		}
	}

	if (fch == NULL)
	{
		send_to_char( "That person isn't under attack.\n\r", ch );
		pop_call();
		return;
	}

	if (multi(ch, gsn_rescue)==-1 && !IS_NPC(ch))
	{
		send_to_char( "You cannot rescue.\n\r", ch );
		pop_call();
		return;
	}

	wait_state( ch, skill_table[gsn_rescue].beats );

	if (number_percent() > learned(ch, gsn_rescue)
	|| !is_same_group(ch, victim) || !IS_NPC(fch))
	{
		send_to_char( "You failed to rescue.\n\r", ch );
		ch_printf(victim, "%s tried to rescue you, but failed!\n\r", get_name(ch));
		pop_call();
		return;
	}

	act( "You rescue $N!",  ch, NULL, victim, TO_CHAR    );
	act( "$n rescues you!", ch, NULL, victim, TO_VICT    );
	act( "$n rescues $N!",  ch, NULL, victim, TO_NOTVICT );

	check_improve(ch, gsn_rescue);

	for (fch = ch->in_room->first_person ; fch != NULL ; fch = fch->next_in_room)
	{
		if (fch->fighting && fch->fighting->who == victim)
		{
			fch->fighting->who = ch;
			break;
		}
	}
	pop_call();
	return;
}


void do_head_butt( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	push_call("do_headbutt(%p,%p)",ch,argument);

	if (!IS_NPC(ch) && multi(ch, gsn_head_butt) == -1)
	{
		send_to_char( "You better leave the fisticuffs to the shamans.\n\r", ch);
		pop_call();
		return;
	}

	if (is_safe(ch, NULL))
	{
		pop_call();
		return;
	}

	if ((victim = who_fighting(ch)) == NULL)
	{
		send_to_char( "You aren't fighting anyone.\n\r", ch);
		pop_call();
		return;
	}

	wait_state( ch, skill_table[gsn_head_butt].beats );

	if (!check_hit(ch, victim, GET_HITROLL(ch), gsn_head_butt))
	{
		damage( ch, victim, 0, gsn_head_butt );
		pop_call();
		return;
	}
	else
	{
		int dam = 5 + GET_DAMROLL(ch);

		if (IS_NPC(ch))
		{
			dam += number_range(1, ch->level*2);
		}
		else
		{
			dam += number_range(multi_skill_level(ch, gsn_head_butt)/2, multi_skill_level(ch, gsn_head_butt)*2);
		}
		damage(ch, victim, dam, gsn_head_butt);
		check_improve(ch, gsn_head_butt);
	}
	pop_call();
	return;
}

void do_kick( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	push_call("do_kick(%p,%p)",ch,argument);

	if (!IS_NPC(ch) && multi(ch, gsn_kick) == -1)
	{
		send_to_char( "You better leave the martial arts to fighters.\n\r", ch);
		pop_call();
		return;
	}

	if (is_safe(ch, NULL))
	{
		pop_call();
		return;
	}

	if ((victim = who_fighting(ch)) == NULL)
	{
		send_to_char( "You aren't fighting anyone.\n\r", ch);
		pop_call();
		return;
	}

	wait_state( ch, skill_table[gsn_kick].beats );

	if (!check_hit(ch, victim, GET_HITROLL(ch), gsn_kick))
	{
		damage( ch, victim, 0, gsn_kick );
		pop_call();
		return;
	}
	else
	{
		int dam = 5 + GET_DAMROLL(ch);

		if (IS_NPC(ch))
		{
			dam += number_range(1, ch->level*2);
		}
		else
		{
			dam += number_range(multi_skill_level(ch, gsn_kick), multi_skill_level(ch, gsn_kick)*4/3);
		}
		damage(ch, victim, dam, gsn_kick);
		check_improve(ch, gsn_kick);
	}
	pop_call();
	return;
}

void do_bash( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	push_call("do_bash(%p,%p)",ch,argument);

	if (get_eq_char(ch, WEAR_SHIELD) == NULL)
	{
		send_to_char("You cannot bash without a shield.\n\r", ch );
		pop_call();
		return;
	}

	if (learned(ch, gsn_bash) == 0)
	{
		send_to_char( "You better leave the martial arts to fighters.\n\r", ch);
		pop_call();
		return;
	}

	if (is_safe(ch, NULL))
	{
		pop_call();
		return;
	}

	if ((victim = who_fighting(ch)) == NULL)
	{
		send_to_char( "You aren't fighting anyone.\n\r", ch );
		pop_call();
		return;
	}

	wait_state(ch, skill_table[gsn_bash].beats);

	if (!check_hit(ch, victim, GET_HITROLL(ch), gsn_bash))
	{
		damage( ch, victim, 0, gsn_bash );
		pop_call();
		return;
	}
	else
	{
		int dam = 10 + GET_DAMROLL(ch);

		if (IS_NPC(ch))
		{
			dam += number_range(1, ch->level*3);
		}
		else
		{
			dam += number_range(multi_skill_level(ch, gsn_bash), multi_skill_level(ch, gsn_bash)*2);
		}

		if (IS_AFFECTED(ch, AFF_TRUESIGHT) || !IS_AFFECTED(victim, AFF2_MIRROR_IMAGE))
		{
			if (number_bits(2) == 0)
			{
				act( "$n sends you sprawling with $s powerful bash!",  ch, NULL, victim, TO_VICT   );
				act( "You send $N sprawling with your powerful bash!", ch, NULL, victim, TO_CHAR   );
				act( "$n sends $N sprawling with $s powerful bash!",   ch, NULL, victim, TO_NOTVICT);

				wait_state(victim, skill_table[gsn_bash].beats/2);
				victim->position = POS_RESTING;
			}
		}
		damage(ch, victim, dam, gsn_bash);
		check_improve(ch, gsn_bash);
	}
	pop_call();
	return;
}

void do_whirl( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim, *victim_next;

	push_call("do_whirl(%p,%p)",ch,argument);

	if (!IS_NPC(ch) && multi(ch, gsn_whirl) == -1)
	{
		send_to_char( "You better leave the martial arts to the fighters.\n\r", ch);
		pop_call();
		return;
	}

	if (is_safe(ch, NULL))
	{
		pop_call();
		return;
	}

	wait_state(ch, skill_table[gsn_whirl].beats);

	act("You spin around your pole in a whirlwind of movement!", ch, NULL, NULL, TO_CHAR);
	act("$n spins around $s pole in a whirlwind of movement!",   ch, NULL, NULL, TO_ROOM);

	for (victim = ch->in_room->first_person ; victim ; victim = victim_next)
	{
		victim_next = victim->next_in_room;

		if (can_mass_attack(ch, victim))
		{
			if (!check_hit(ch, victim, GET_HITROLL(ch), gsn_whirl))
			{
				damage( ch, victim, 0, gsn_whirl);
				pop_call();
				return;
			}
			else
			{
				int dam = GET_DAMROLL(ch);

				if (IS_NPC(ch))
				{
					dam += number_range(1, ch->level*2);
				}
				else
				{
					dam += number_range(1, multi_skill_level(ch, gsn_whirl)*7/3);
				}
				damage(ch, victim, dam, gsn_whirl);
				check_improve(ch, gsn_whirl);
			}
		}
	}
	pop_call();
	return;
}

void do_acupunch(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	int level, stun, dmgL, dmgH;

	push_call("do_acupunch(%p,%p)",ch,argument);

	if (is_safe(ch, NULL))
	{
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && multi(ch,gsn_acupunch) == -1)
	{
		send_to_char("Don't try this at home!!!\n\r",ch);
		pop_call();
		return;
	}

	if (get_eq_char(ch, WEAR_WIELD) != NULL)
	{
		send_to_char("You cannot punch while wielding a weapon.\n\r", ch);
		pop_call();
		return;
	}

	if (IS_NPC(ch))
	{
		level = ch->level;
	}
	else
	{
		level = multi_skill_level(ch, gsn_acupunch);
	}

	if ((victim = who_fighting(ch)) == NULL)
	{
		send_to_char("You aren't fighting anyone.\n\r",ch);
		pop_call();
		return;
	}

	wait_state(ch, skill_table[gsn_acupunch].beats);

	if (!check_hit(ch, victim, GET_HITROLL(ch), gsn_acupunch))
	{
 		damage(ch,victim,0,gsn_acupunch);
 		pop_call();
 		return;
	}
	dmgL = level / 2;

	switch (number_range(1, 9))
	{
		default: dmgH = level * 2; stun = 0; break;
		case 1:  dmgH = level * 2; stun = 1; break;
		case 2:  dmgH = level * 3; stun = 2; break;
		case 3:  dmgH = level * 4; stun = 3; break;
	}

	damage(ch, victim, GET_DAMROLL(ch)+number_range(dmgL,dmgH), gsn_acupunch);

	if (ch->in_room != victim->in_room)
	{
		pop_call();
		return;
	}

	if (stun)
	{
		act("$n stuns you and you go down!", ch, NULL, victim, TO_VICT);
		act("$n stuns $N and $E goes down!", ch, NULL, victim, TO_NOTVICT);
		act("You stun $N and $E goes down!", ch, NULL, victim, TO_CHAR);
	}

	switch (stun)
	{
		case 1:
			victim->position = POS_SITTING;
			break;
		case 2:
			victim->position = POS_RESTING;
			wait_state(victim, skill_table[gsn_acupunch].beats / 3);
			break;
		case 3:
			victim->position = POS_SLEEPING;
			wait_state(victim,(skill_table[gsn_acupunch].beats) / 2);
			break;
	}
	check_improve(ch, gsn_acupunch);
	pop_call();
	return;
}


void fisticuffs( CHAR_DATA *ch, CHAR_DATA *victim, int dam )
{
	int level;

	push_call("fisticuffs(%p,%p,%p)",ch,victim,dam);

	level = multi_skill_level(ch, gsn_fisticuffs);

	if (number_bits(1) == 0)
	{
		fisticuffs_attack = number_range(level/30, level/17);
	}
	else
	{
		fisticuffs_attack = number_range(0,5);
	}

	fisticuffs_attack = URANGE(0, fisticuffs_attack, 5);

	switch (fisticuffs_attack)
	{
		case 0: dam = (int) dam * 1.1; break;
		case 1: dam = (int) dam * 1.2; break;
		case 2: dam = (int) dam * 1.3; break;
		case 3: dam = (int) dam * 1.4; break;
		case 4: dam = (int) dam * 1.5; break;
		case 5: dam = (int) dam * 1.6; break;
	}

	if (!check_hit(ch, victim, GET_HITROLL(ch), gsn_fisticuffs))
	{
		damage(ch, victim, 0, gsn_fisticuffs);
	}
	else
	{
		damage(ch, victim, dam, gsn_fisticuffs);
		check_improve(ch, gsn_fisticuffs);
	}
	pop_call();
	return;
}


void martial_arts( CHAR_DATA *ch, CHAR_DATA *victim, int dam)
{
	int level;

	push_call("martial_arts_attack(%p,%p,%p)",ch,victim,dam);

	level = multi_skill_level(ch, gsn_martial_arts);

	if (number_bits(1) == 0)
	{
		martial_arts_attack = number_range(level/16, level/8);
	}
	else
	{
		martial_arts_attack = number_range(0,11);
	}

	martial_arts_attack = URANGE(0, martial_arts_attack, 11);

	switch (martial_arts_attack)
	{
		case  0: dam = (int) dam * 1.00; break;
		case  1: dam = (int) dam * 1.05; break;
		case  2: dam = (int) dam * 1.10; break;
		case  3: dam = (int) dam * 1.15; break;
		case  4: dam = (int) dam * 1.20; break;
		case  5: dam = (int) dam * 1.25; break;
		case  6: dam = (int) dam * 1.30; break;
		case  7: dam = (int) dam * 1.35; break;
		case  8: dam = (int) dam * 1.40; break;
		case  9: dam = (int) dam * 1.45; break;
		case 10: dam = (int) dam * 1.50; break;
		case 11: dam = (int) dam * 1.55; break;
	}

	if (!check_hit(ch, victim, GET_HITROLL(ch), gsn_martial_arts))
	{
		damage(ch, victim, 0, gsn_martial_arts);
	}
	else
	{
		damage(ch, victim, dam, gsn_martial_arts);
		check_improve(ch, gsn_martial_arts);
	}
	pop_call();
	return;
}


void do_disarm( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	push_call("do_disarm(%p,%p)",ch,argument);

	if (!IS_NPC(ch) && multi(ch, gsn_disarm) == -1)
	{
		send_to_char( "You don't know how to disarm opponents.\n\r", ch );
		pop_call();
		return;
	}

	if ((victim = who_fighting(ch)) == NULL)
	{
		send_to_char( "You aren't fighting anyone.\n\r", ch );
		pop_call();
		return;
	}

	if (get_eq_char(victim, WEAR_WIELD) == NULL)
	{
		send_to_char( "Your opponent is not wielding a weapon.\n\r", ch );
		pop_call();
		return;
	}

	wait_state( ch, skill_table[gsn_disarm].beats );

	if (IS_NPC(ch) || number_range(1, 100+get_curr_str(victim)) < learned(ch, gsn_disarm))
	{
		disarm( ch, victim );
		check_improve(ch, gsn_disarm);
	}
	else
	{
		send_to_char( "You failed.\n\r", ch );
	}
	pop_call();
	return;
}


void do_sla( CHAR_DATA *ch, char *argument )
{
	push_call("do_sla(%p,%p)",ch,argument);

	send_to_char( "If you want to SLAY, spell it out.\n\r", ch );

	pop_call();
	return;
}


void do_slay( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];

	push_call("do_slay(%p,%p)",ch,argument);

	one_argument( argument, arg );
	if ( arg[0] == '\0' )
	{
		send_to_char( "Slay whom?\n\r", ch );
		pop_call();
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char( "They aren't here.\n\r", ch );
		pop_call();
		return;
	}

	if (ch == victim)
	{
		send_to_char( "Suicide is a mortal sin.\n\r", ch );
		pop_call();
		return;
	}

	if (!IS_NPC(victim) && victim->level == MAX_LEVEL)
	{
		act( "$N laughs at your pathetic attempt to slay $M.",  ch, NULL, victim, TO_CHAR   );
		act( "You laugh at $n's pathetic attempt to slay you.", ch, NULL, victim, TO_VICT   );
		act( "$N laughs at $n's pathetic attempt to slay $M.",  ch, NULL, victim, TO_NOTVICT);
		pop_call();
		return;
	}

	act( "You slay $M in cold blood!",  ch, NULL, victim, TO_CHAR   );
	act( "$n slays you in cold blood!", ch, NULL, victim, TO_VICT   );
	act( "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT);

	/*
		to make progs trigger :)
	*/

	set_fighting(victim, ch);
	mprog_death_trigger(victim);
	raw_kill(victim);

	stop_hate_fear(ch);

	pop_call();
	return;
}


void do_slaughter( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];

	push_call("do_slaughter(%p,%p)",ch,argument);

	one_argument( argument, arg );
	if ( arg[0] == '\0' )
	{
		send_to_char( "Slaughter whom?\n\r", ch );
		pop_call();
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char( "They aren't here.\n\r", ch );
		pop_call();
		return;
	}

	if ( ch == victim )
	{
		send_to_char( "Suicide is a mortal sin.\n\r", ch );
		pop_call();
		return;
	}

	if (!IS_NPC(victim) && victim->level == MAX_LEVEL)
	{
		act( "$N laughs at your pathetic attempt to slaughter $M.",  ch, NULL, victim, TO_CHAR   );
		act( "You laugh at $n's pathetic attempt to slaughter you.", ch, NULL, victim, TO_VICT   );
		act( "$N laughs at $n's pathetic attempt to slaughter $M.",  ch, NULL, victim, TO_NOTVICT);
		pop_call();
		return;
	}

	act( "You slaughter $M in cold blood!",  ch, NULL, victim, TO_CHAR   );
	act( "$n slaughters you in cold blood!", ch, NULL, victim, TO_VICT   );
	act( "$n slaughters $N in cold blood!",  ch, NULL, victim, TO_NOTVICT);

	if (!IS_NPC(victim))
	{
		log_printf("%s slaughtered by %s at %u", victim->name, get_name(ch), victim->in_room->vnum);

		if (victim->pcdata->exp > exp_level(victim->class, (victim->level)-1)+1)
		{
			gain_exp( victim, exp_level(victim->class, (victim->level)-1)+1 - victim->pcdata->exp);
		}
		if (IS_SET(victim->act, PLR_KILLER) || IS_SET(victim->act, PLR_THIEF))
		{
			if (victim->pcdata->exp > exp_level(victim->class, (victim->level)-2)+1)
			{
				gain_exp( victim, exp_level(victim->class, (victim->level)-2)+1 - victim->pcdata->exp);
			}
			if (IS_SET(victim->act, PLR_KILLER))
			{
				REMOVE_BIT(victim->act, PLR_KILLER);
			}
			if (IS_SET(victim->act, PLR_THIEF))
			{
				REMOVE_BIT(victim->act, PLR_THIEF);
			}
		}
	}
	set_fighting(victim,ch);	/* to make progs trigger :) */
	mprog_death_trigger(victim);
	raw_kill( victim );

	pop_call();
	return;
}


void do_suicide( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	bool killer, thief;
	killer=FALSE;
	thief=FALSE;

	push_call("do_suicide(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		raw_kill(ch);
		pop_call();
		return;
	}

	if (IS_SET(ch->act, PLR_KILLER))
	{
		killer = TRUE;
	}

	if (IS_SET(ch->act, PLR_THIEF))
	{
		thief = TRUE;
	}

	if (ch->fighting != NULL)
	{
		ch_printf(ch, "%s is trying to kill you, and suicide is not an option.\n\r", ch->fighting->who->name );
		pop_call();
		return;
	}

	one_argument_nolower( argument, arg );

	if (arg[0] == '\0')
	{
		send_to_char( "!!!  WARNING  !!!   This command is used to kill yourself.\n\rTo use you must enter in your password after the command.\n\r", ch );
		pop_call();
		return;
	}

	if (encrypt64(arg) == ch->pcdata->password)
	{
		send_to_char( "You commited suicide!\n\r", ch);
		raw_kill(ch);
		if (killer)
		{
			SET_BIT( ch->act, PLR_KILLER);
		}
		if (thief)
		{
			SET_BIT( ch->act, PLR_THIEF);
		}
	}
	else
	{
		send_to_char( "That was not your password.  Are you sure you want to commit suicide?\n\r", ch);
	}
	pop_call();
	return;
}


void do_shoot(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	OBJ_DATA  *ammo, *cont, *weapon;
	int dir, dam;
	char *name;
	char buf1[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];

	push_call("do_shoot(%p,%p)",ch,argument);

	if (!IS_NPC(ch) && multi(ch, gsn_shoot) == -1)
	{
		send_to_char("You better leave the shooting to ninjas.\n\r", ch);
		pop_call();
		return;
	}

	if (ch->fighting != NULL)
	{
		send_to_char( "You are too distracted to shoot!\n\r", ch );
		pop_call();
		return;
	}

	if ((weapon = get_eq_char(ch,WEAR_WIELD)) == NULL)
	{
		send_to_char("You must wield a weapon to shoot!\n\r", ch);
		pop_call();
		return;
	}

	if (weapon->item_type != ITEM_WEAPON)
	{
		send_to_char("You are not wielding a weapon.\n\r", ch);
		pop_call();
		return;
	}

	switch(weapon->value[0])
	{
		case WEAPON_TYPE_SHORTBOW:
		case WEAPON_TYPE_LONGBOW:
		case WEAPON_TYPE_CROSSBOW:
		case WEAPON_TYPE_BLOWPIPE:
			break;
		default:
			send_to_char("You are not wielding a weapon that can shoot.\n\r", ch);
			break;
	}

	for (ammo = ch->first_carrying ; ammo ; ammo = ammo->next_content)
	{
		if (ammo->item_type == ITEM_AMMO && ammo->value[0] == weapon->value[0] && ammo->level <= weapon->level)
		{
			break;
		}
		if (ammo->item_type == ITEM_CONTAINER && !IS_SET(ammo->value[1], CONT_CLOSED))
		{
			cont = ammo;

			for (ammo = ammo->first_content ; ammo ; ammo = ammo->next_content)
			{
				if (ammo->item_type == ITEM_AMMO && ammo->value[0] == weapon->value[0] && ammo->level <= weapon->level)
				{
					break;
				}
			}
			if (ammo)
			{
				break;
			}
			else
			{
				ammo = cont;
			}
		}
	}
	if (ammo == NULL)
	{
		send_to_char("Ooops...forgot to stock up on ammunition, eh?\n\r", ch);
		pop_call();
		return;
	}

	if (number_percent() > learned(ch, gsn_shoot))
	{
		send_to_char("You fumble around and don't get any shots off!\n\r", ch);
		pop_call();
		return;
	}

	argument = one_argument(argument, buf1);
	argument = one_argument(argument, buf2);

	if ((dir = direction_door(buf1)) == -1)
	{
		if ((dir = direction_door(buf2)) == -1)
		{
			send_to_char( "Shoot which direction?\n\r", ch );
			pop_call();
			return;
		}
		name = buf1;
	}
	else
	{
		name = buf2;
	}

	if (*name == '\0')
	{
		send_to_char( "Shoot at whom?\n\r", ch );
		pop_call();
		return;
	}

	if ((victim = find_char_dir(ch, dir, name)) == NULL)
	{
		send_to_char("Your victim must have slipped out of sight!\n\r", ch);
		pop_call();
		return;
	}

	if (is_safe(ch, victim))
	{
		pop_call();
		return;
	}

	if (find_keeper(victim) != NULL || IS_SET(victim->in_room->room_flags, ROOM_SAFE))
	{
		send_to_char("You can't shoot into that room.\n\r", ch);
		pop_call();
		return;
	}

	if (ch->in_room->area != victim->in_room->area)
	{
		send_to_char("They are too far to shoot at.\n\r", ch );
		pop_call();
		return;
	}

	if (IS_NPC(victim) && victim->hit < victim->max_hit / 2)
	{
		send_to_char("Your target is too wary to be shot at!\n\r", ch);
		pop_call();
		return;
	}

	/*
		set wait time partial based on ammo speed..
	*/

	wait_state(ch, skill_table[gsn_shoot].beats - ammo->value[2] * 2);

	if (ammo->in_obj)
	{
		act( "$n pulls $p from $P.", ch, ammo, ammo->in_obj, TO_ROOM );
		act( "You pull $p from $P.", ch, ammo, ammo->in_obj, TO_CHAR );
	}

	if (!check_hit(ch, victim, GET_HITROLL(ch), gsn_shoot))
	{
		act( "$p whistles past your face.", ch, ammo, victim, TO_VICT );
		act( "$p whistles past $N's face.", ch, ammo, victim, TO_CHAR );

		if (number_percent() < 50)
		{
			junk_obj(ammo);
		}
		else
		{
			ammo->sac_timer = OBJ_SAC_TIME;
			obj_to_room(ammo, victim->in_room->vnum);
		}
	}
	else
	{
		dam = number_range(1, ammo->value[1] * ammo->value[3]);
		obj_to_char(ammo, victim);
		damage(ch, victim, dam, gsn_shoot);

		if (MP_VALID_MOB(victim) && dam > 0)
		{
			mprog_range_trigger(victim, ch, rev_dir[dir]);
		}
	}
	pop_call();
	return;
}

void do_throw( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	OBJ_DATA  *weapon;
	int dir, dam, wielded;
	char buf1[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH], buf3[MAX_INPUT_LENGTH];
	char *name, *item;

	push_call("do_throw(%p,%p)",ch,argument);

	if (IS_NPC(ch) && ch->fighting == NULL && ch->hit < ch->max_hit / 2)
	{
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && multi(ch, gsn_throw) == -1)
	{
		send_to_char("You decide not to throw...might hurt your back.\n\r", ch);
		pop_call();
		return;
	}

	if (ch->fighting != NULL)
	{
		send_to_char( "You are too distracted to throw!\n\r", ch );
		pop_call();
		return;
	}

	argument = one_argument(argument, buf1);
	argument = one_argument(argument, buf2);
	argument = one_argument(argument, buf3);

	if ((dir = direction_door(buf1)) == -1)
	{
		if ((dir = direction_door(buf2)) == -1)
		{
			if ((dir = direction_door(buf3)) == -1)
			{
				send_to_char( "Throw which direction?\n\r", ch );
				pop_call();
				return;
			}
			else
			{
				name = buf2;
				item = buf1;
			}
		}
		else
		{
			name = buf3;
			item = buf1;
		}
	}
	else
	{
		name = buf3;
		item = buf2;
	}

	if (name[0] == '\0' || item[0] == '\0')
	{
		send_to_char( "Throw what at whom?\n\r", ch );
		pop_call();
		return;
	}

	if ((victim = find_char_dir(ch, dir, name)) == NULL)
	{
		send_to_char("Your victim must have slipped out of sight.\n\r", ch);
		pop_call();
		return;
	}

	if (is_safe(ch, victim))
	{
		pop_call();
		return;
	}

	if (find_keeper(victim) != NULL || IS_SET(victim->in_room->room_flags, ROOM_SAFE))
	{
		send_to_char("You can't throw into that room.\n\r", ch);
		pop_call();
		return;
	}

	if (ch->in_room->area != victim->in_room->area)
	{
		send_to_char("They are too far to throw things at.\n\r", ch);
		pop_call();
		return;
	}

	if (IS_NPC(victim) && victim->hit < victim->max_hit/2)
	{
		send_to_char("You can't seem to get a good sight of your target.\n\r", ch);
		pop_call();
		return;
	}


	wielded = FALSE;

	if ((weapon = get_eq_char(ch, WEAR_WIELD)) != NULL)
	{
		if (is_name(item, weapon->name) || is_name_short(item, weapon->name))
		{
			wielded = TRUE;
		}
	}

	if (wielded == FALSE)
	{
		if (!remove_obj(ch, WEAR_WIELD, TRUE, FALSE))
		{
			send_to_char("You can't throw with another weapon wielded!\n\r", ch);
			pop_call();
			return;
		}
		weapon = get_obj_carry(ch, item);
	}

	if (weapon == NULL)
	{
		send_to_char("Hmmm...you don't seem to have that weapon.\n\r", ch);
		pop_call();
		return;
	}

	if (IS_SET(weapon->extra_flags, ITEM_NODROP))
	{
		send_to_char("You can't let go of it.\n\r", ch);
		pop_call();
		return;
	}

	switch (weapon->value[0])
	{
		case WEAPON_TYPE_DAGGER:
		case WEAPON_TYPE_AXE:
		case WEAPON_TYPE_SPEAR:
			break;
		default:
			send_to_char("You can't throw that type of weapon.\n\r", ch);
			pop_call();
			return;
	}

	if (!wielded)
	{
		wait_state( ch, skill_table[gsn_throw].beats );
	}
	else
	{
		wait_state( ch, skill_table[gsn_throw].beats / 2 );
	}

	if (number_percent() > learned(ch, gsn_throw))
	{
		send_to_char("You fumble around and don't throw a thing!\n\r", ch);
		pop_call();
		return;
	}

	if (!check_hit(ch, victim, GET_HITROLL(ch), gsn_throw))
	{
		act("$p whistles past your face.", ch, weapon, victim, TO_VICT );
		act("$p whistles past $N's face.", ch, weapon, victim, TO_CHAR );

		if (number_percent() < 50)
		{
			junk_obj(weapon);
		}
		else
		{
			weapon->sac_timer = OBJ_SAC_TIME;
			obj_to_room(weapon, victim->in_room->vnum);
		}
	}
	else
	{
		dam = 3 * number_range(weapon->value[1], weapon->value[1]*weapon->value[2]);
		obj_to_char(weapon, victim);

		damage(ch, victim, dam, gsn_throw);

		if (MP_VALID_MOB(victim) && dam > 0)
		{
			mprog_range_trigger(victim, ch, rev_dir[dir]);
		}
		stop_fighting(ch, FALSE);

		pop_call();
		return;
	}
}

void do_divert( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *fch;

	push_call("do_divert(%p,%p)",ch,argument);

	if (!IS_NPC(ch) && multi(ch,gsn_divert) == -1)
	{
		send_to_char( "What about fleeing instead?\n\r", ch );
		pop_call();
		return;
	}

	if (ch->fighting == NULL)
	{
		send_to_char( "You aren't fighting!\n\r", ch );
		pop_call();
		return;
	}

	for (fch = ch->in_room->first_person ; fch ; fch = fch->next_in_room)
	{
		if (fch->fighting && fch->fighting->who == ch)
		{
			break;
		}
	}
	if (fch == NULL)
	{
		send_to_char( "You aren't under attack.\n\r", ch );
		pop_call();
		return;
	}

	one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		send_to_char( "Divert to whom?\n\r", ch );
		pop_call();
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char( "They aren't here.\n\r", ch );
		pop_call();
		return;
	}

	if (victim == ch)
	{
		send_to_char( "What about fleeing instead?\n\r", ch );
		pop_call();
		return;
	}

	if (ch->fighting->who == victim)
	{
		act( "Trying to make $m fight $mself?",  ch, NULL, victim, TO_CHAR);
		pop_call();
		return;
	}

	if ((fch = who_fighting(victim)) == NULL)
	{
		send_to_char( "That person is not fighting right now.\n\r", ch );
		pop_call();
		return;
	}

	if (!is_same_group(ch,victim))
	{
		act( "$N isn't a member of your group.", ch, NULL, victim, TO_CHAR);
		pop_call();
		return;
	}

	wait_state( ch, skill_table[gsn_divert].beats );

	if (!IS_NPC(ch->fighting->who))
	{
		send_to_char( "You fail the divert.\n\r", ch );
		pop_call();
		return;
	}

	if (number_percent() > learned(ch, gsn_divert) || !IS_NPC(fch))
	{
		send_to_char( "You fail the divert.\n\r", ch );
		pop_call();
		return;
	}

	act( "You divert the attack to $N!",  ch, NULL, victim, TO_CHAR);
	act( "$n diverts the attack to you!", ch, NULL, victim, TO_VICT);
	act( "$n diverts the attack to $N!",  ch, NULL, victim, TO_NOTVICT);

	check_improve(ch, gsn_divert);

	for (fch = ch->in_room->first_person ; fch != NULL ; fch = fch->next_in_room)
	{
		if (fch->fighting && fch->fighting->who == ch)
		{
			fch->fighting->who = victim;
			break;
		}
	}
	pop_call();
	return;
}

void do_knife( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	push_call("do_knife(%p,%p)",ch,argument);

	if (!IS_NPC(ch) && multi(ch, gsn_knife) == -1)
	{
		send_to_char( "You'd better leave the nasty stuff to ninjas.\n\r", ch );
		pop_call();
		return;
	}

	if ((obj = get_eq_char(ch,WEAR_WIELD)) == NULL || (obj->value[3] != 11 && obj->value[3] != 2))
	{
		send_to_char( "You need to wield a piercing weapon.\n\r", ch );
		pop_call();
		return;
	}

	one_argument( argument, arg );

	if (arg[0] == '\0')
	{
		if ((victim = who_fighting(ch)) == NULL)
		{
			send_to_char("You are not fighting anyone.\n\r",ch);
			pop_call();
			return;
		}
	}
	else if ((victim = get_char_room(ch,arg)) == NULL)
	{
		send_to_char("Knife whom?\n\r",ch);
		pop_call();
		return;
	}

	if (is_safe(ch, victim))
	{
		pop_call();
		return;
	}

	wait_state( ch, skill_table[gsn_knife].beats );

	if (!check_hit(ch, victim, GET_HITROLL(ch), gsn_knife))
	{
		damage( ch, victim, 0, gsn_knife);
	}
	else
	{
		one_hit( ch, victim, gsn_knife );
	}

	pop_call();
	return;
}

void do_distract( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	push_call("do_distract(%p,%p)",ch,argument);

	if (!IS_NPC(ch) && multi(ch, gsn_distract) == -1)
	{
		send_to_char( "They'd just get mad at you if you tried.\n\r", ch );
		pop_call();
		return;
	}

	one_argument(argument,arg);

	if ((victim = get_char_room(ch,arg)) == NULL)
	{
		send_to_char("Distract whom?\n\r",ch);
		pop_call();
		return;
	}

	if (is_safe(ch, victim))
	{
		pop_call();
		return;
	}

	wait_state( ch, skill_table[gsn_distract].beats );

	if (IS_NPC(ch) || number_range(1, 150) < learned(ch, gsn_distract))
	{
		switch (number_bits(1))
		{
			case 0:
				act( "$n informs you that your shoes are untied.", ch, NULL, victim, TO_VICT    );
				act( "You inform $N that $S shoes are untied.",    ch, NULL, victim, TO_CHAR    );
				act( "$n inform $N that $S shoes are untied.",     ch, NULL, victim, TO_NOTVICT );
				break;
			case 1:
				act( "$n points at the sky above you and gasps. ", ch, NULL, victim, TO_VICT    );
				act( "You point at the sky above $M and gasp.",    ch, NULL, victim, TO_CHAR    );
				act( "$n points at the sky above $N and gasps.",   ch, NULL, victim, TO_NOTVICT );
				break;
		}
		check_improve(ch, gsn_distract);
		victim->distracted += 10;
		wait_state( victim, skill_table[gsn_distract].beats );
	}
	else
	{
		send_to_char( "They suddenly look very suspicious.\n\r", ch );
		act( "$n is trying to distract you!", ch, NULL, victim, TO_VICT);
		victim->distracted = 0;
		if (IS_NPC(victim) && (number_bits(1) == 0))
		{
			act( "$n suddenly looks VERY angry!",victim,NULL, NULL,TO_ROOM);
			multi_hit(victim,ch,TYPE_UNDEFINED);
		}
	}
	pop_call();
	return;
}

bool	valid_fight( CHAR_DATA *ch, CHAR_DATA *victim )
{
	push_call("valid_fight(%p,%p)",ch,victim);

	if (who_fighting(ch) == NULL)
	{
		pop_call();
		return FALSE;
	}

	if (ch->in_room != victim->in_room)
	{
		pop_call();
		return FALSE;
	}

	pop_call();
	return TRUE;
}


CHAR_DATA *who_fighting( CHAR_DATA *ch )
{
	push_call("who_fighting(%p)",ch);

	if (ch->fighting == NULL)
	{
		pop_call();
		return NULL;
	}

	if (ch->fighting->who == NULL)
	{
		pop_call();
		return NULL;
	}

	if (ch->fighting->who->name == NULL)
	{
		log_printf("who_fighting: who->name == NULL for %s", ch->name);
		dump_stack();
		pop_call();
		return NULL;
	}

	if (ch->fighting->who->in_room != ch->in_room)
	{
		pop_call();
		return NULL;
	}

	pop_call();
	return ch->fighting->who;
}


void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
	FIGHT_DATA *fight, *fight_next;

	push_call("stop_fighting(%p,%p)",ch,fBoth);

	free_fight( ch );

	if (!fBoth)
	{
		pop_call();
		return;
	}

	for (fight = mud->f_fight ; fight ; fight = fight_next)
	{
		fight_next = fight->next;

		if (fight->who == ch)
		{
			free_fight(fight->ch);
		}
	}
	pop_call();
	return;
}

void do_auto( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_INPUT_LENGTH];
	int sn;
	bool ms;

	push_call("do_auto(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (argument[0] == '\0')
	{
		send_to_char( "Syntax:  AUTO [AUTO|QUICK] [SKILL|SPELL]\n\r", ch);

		if (ch->pcdata->auto_command != NULL && ch->pcdata->auto_command[0] != '\0')
		{
			ch_printf( ch, "Current command: %s\n\r", ch->pcdata->auto_command);
		}
		else
		{
			ch_printf( ch, "Current command: Not defined\n\r" );
		}

		switch (ch->pcdata->auto_flags)
		{
			case AUTO_OFF:
				send_to_char( "Current mode:    Off\n\r", ch);
				break;
			case AUTO_AUTO:
				send_to_char( "Current mode:    Auto\n\r", ch);
				break;
			case AUTO_QUICK:
				send_to_char( "Current mode:    Quick\n\r", ch);
				break;
		}
		pop_call();
		return;
	}

	if (!strcasecmp( argument, "auto"))
	{
		if (ch->pcdata->auto_command[0] != '\0')
		{
			send_to_char( "Turning AUTO mode on.\n\r", ch);
			ch->pcdata->auto_flags = AUTO_AUTO;
			pop_call();
			return;
		}
		else
		{
			send_to_char( "You haven't defined a combat skill/spell.\n\r", ch);
			pop_call();
			return;
		}
	}

	if (!strcasecmp(argument, "quick"))
	{
		if (ch->pcdata->auto_command[0] != '\0')
		{
			send_to_char( "Turning QUICK mode on.\n\r", ch);
			ch->pcdata->auto_flags=AUTO_QUICK;
			pop_call();
			return;
		}
		else
		{
			send_to_char( "You haven't defined a combat skill/spell.\n\r", ch);
			pop_call();
			return;
		}
	}

	if (!strcasecmp(argument, "off"))
	{
		send_to_char( "Turning auto modes off.\n\r", ch);
		ch->pcdata->auto_flags = AUTO_OFF;
		pop_call();
		return;
	}

	/*
		Adding mass to auto command - Scandum 18-04-2002
	*/

	if (!str_prefix("mass ", argument))
	{
		if (learned(ch, gsn_mass) <= 0)
		{
			send_to_char( "You can't do that.\n\r", ch );
			pop_call();
			return;
		}
		sn = skill_lookup( &argument[5] );
		ms = TRUE;

		if (!is_spell(sn))
		{
			send_to_char( "That is not a spell.\n\r", ch );
			pop_call();
			return;
		}
	}
	else
	{
		sn = skill_lookup( argument );
		ms = FALSE;
	}

	if (sn < 0 && ms == FALSE)
	{
		send_to_char( "That is not a skill or spell.\n\r", ch );
		pop_call();
		return;
	}

	/*
		Disallow ppl to do auto short range etc - Martin 7/8/98
	*/

	if (skill_table[sn].position != POS_FIGHTING)
	{
		send_to_char( "You can only put combat skills/spells on auto.\n\r", ch );
		pop_call();
		return;
	}

	if (!IS_SET(skill_table[sn].race, 1 << ch->race))
	{
		if (multi(ch, sn) == -1 || learned(ch, sn) <= 0)
		{
			send_to_char( "You can't do that.\n\r", ch );
			pop_call();
			return;
		}
	}

	if (ms == TRUE)
	{
		int sha_mlv = ch->pcdata->mclass[CLASS_MONK];
		int sor_mlv = ch->pcdata->mclass[CLASS_ILLUSIONIST];

		if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
		&& sor_mlv < skill_table[gsn_mass].skill_level[CLASS_ILLUSIONIST])
		|| (skill_table[sn].target == TAR_CHAR_DEFENSIVE
		&& sha_mlv < skill_table[gsn_mass].skill_level[CLASS_MONK]))
		{
			send_to_char( "You don't have enough skill in the proper class.\n\r", ch );
			pop_call();
			return;
		}
		if (skill_table[sn].target != TAR_CHAR_OFFENSIVE
		&&  skill_table[sn].target != TAR_CHAR_DEFENSIVE)
		{
			send_to_char( "You cannot put mass spells on auto mass.\n\r", ch );
			pop_call();
			return;
		}
		sprintf(argument, "mass %s", skill_table[sn].name);
	}
	else
	{
		strcpy(argument, skill_table[sn].name);
	}

	STRFREE (ch->pcdata->auto_command);
	ch->pcdata->auto_command = STRALLOC( argument );
	sprintf( buf, "Auto command set to: %s\n\r", argument );
	send_to_char(buf, ch);

	pop_call();
	return;
}

void do_spam( CHAR_DATA *ch, char *arg)
{
	int val;

	push_call("do_spam(%p,%p)",ch,arg);

	if (arg == NULL || arg[0]=='\0')
	{
		send_to_char( "SPAM control settings:\n\r", ch);
		send_to_char( "Toggle  Status  Description\n\r", ch);

		if (!IS_SET(CH(ch->desc)->pcdata->spam, 1))
		{
			send_to_char( "  A       On    You hit\n\r" , ch);
		}
		else
		{
			send_to_char( "  A       Off   You hit\n\r" , ch);
		}

		if (!IS_SET(CH(ch->desc)->pcdata->spam, 2))
		{
			send_to_char( "  B       On    You miss\n\r" , ch);
		}
		else
		{
			send_to_char( "  B       Off   You miss\n\r" , ch);
		}
		if (!IS_SET(CH(ch->desc)->pcdata->spam, 4))
		{
			send_to_char( "  C       On    They hit you\n\r" , ch);
		}
		else
		{
			send_to_char( "  C       Off   They hit you\n\r" , ch);
		}
		if( !IS_SET( CH(ch->desc)->pcdata->spam, 8))
		{
			send_to_char( "  D       On    They miss you\n\r" , ch);
		}
		else
		{
			send_to_char( "  D       Off   They miss you\n\r" , ch);
		}
		if( !IS_SET( CH(ch->desc)->pcdata->spam, 16))
		{
			send_to_char( "  E       On    Party hits\n\r" , ch);
		}
		else
		{
			send_to_char( "  E       Off   Party hits\n\r" , ch);
		}

		if( !IS_SET( CH(ch->desc)->pcdata->spam, 32))
		{
			send_to_char( "  F       On    Party misses\n\r" , ch);
		}
		else
		{
			send_to_char( "  F       Off   Party misses\n\r" , ch);
		}
		if( !IS_SET( CH(ch->desc)->pcdata->spam, 64))
		{
			send_to_char( "  G       On    They hit party\n\r" , ch);
		}
		else
		{
			send_to_char( "  G       Off   They hit party\n\r" , ch);
		}
		if( !IS_SET( CH(ch->desc)->pcdata->spam, 128))
		{
			send_to_char( "  H       On    They miss party\n\r" , ch);
		}
		else
		{
			send_to_char( "  H       Off   They miss party\n\r" , ch);
		}
		if( !IS_SET( CH(ch->desc)->pcdata->spam, 256))
		{
			send_to_char( "  I       On    Other hit\n\r" , ch);
		}
		else
		{
			send_to_char( "  I       Off   Other hit\n\r" , ch);
		}
		if( !IS_SET( CH(ch->desc)->pcdata->spam, 512))
		{
			send_to_char( "  J       On    Other miss\n\r" , ch);
		}
		else
		{
			send_to_char( "  J       Off   Other miss\n\r" , ch);
		}
		if( IS_SET( CH(ch->desc)->pcdata->spam, 1024))
		{
			send_to_char( "  K       On    Show party status line\n\r" , ch);
		}
		else
		{
			send_to_char( "  K       Off   Show party status line\n\r" , ch);
		}
		if (!IS_SET( CH(ch->desc)->pcdata->spam, 2048))
		{
			send_to_char( "  L       ON    You hear party members moving\n\r", ch);
		}
		else
		{
			send_to_char( "  L       off   You don't hear party members moving\n\r", ch);
		}
		if (!IS_SET(CH(ch->desc)->pcdata->spam, 4096))
		{
			send_to_char( "  M       ON    You hear the uttering of spells\n\r",ch);
		}
		else
		{
			send_to_char( "  M       off   You don't hear the uttering of spells\n\r",ch);
		}
		pop_call();
		return;
	}

	val = -1;

	switch (tolower(arg[0]))
	{
		case 'a':	val = 1;		break;
		case 'b':	val = 2;		break;
		case 'c':	val = 4;		break;
		case 'd':	val = 8;		break;
		case 'e': val = 16;		break;
		case 'f':	val = 32;		break;
		case 'g':	val = 64;		break;
		case 'h':	val = 128;	break;
		case 'i':	val = 256;	break;
		case 'j':	val = 512;	break;
		case 'k':	val = 1024;	break;
		case 'l':	val = 2048;	break;
		case 'm':	val = 4096;	break;
	}

	if (val == -1)
	{
		send_to_char( "That is not an option.\n\r", ch );
		pop_call();
		return;
	}

	TOGGLE_BIT(CH(ch->desc)->pcdata->spam, val);

	do_spam( ch, "" );   /* show stat change */

	pop_call();
	return;
}

void do_assassin( CHAR_DATA *ch, char *argument )
{
	push_call("do_assassin(%p,%p)",ch,argument);

	send_to_char( "If you want to ASSASSINATE, spell it out.\n\r", ch );

	pop_call();
	return;
}

void do_assassinate( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_INPUT_LENGTH];
	char arg[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	push_call("do_assassinate(%p,%p)",ch,argument);

	if (IS_NPC(ch) || multi(ch, gsn_assassinate) == -1)
	{
		send_to_char( "Only ninjas can assassinate.\n\r", ch );
		pop_call();
		return;
	}

	argument = one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char( "Assassinate whom?\n\r", ch );
		pop_call();
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char( "They aren't here.\n\r", ch );
		pop_call();
		return;
	}

	if ( victim == ch )
	{
		send_to_char( "Suicide is a mortal sin.\n\r", ch );
		pop_call();
		return;
	}

	if (is_safe(ch, NULL))
	{
		pop_call();
		return;
	}

	if (IS_NPC(victim))
	{
		send_to_char( "You may not assassinate them.\n\r", ch );
		pop_call();
		return;
	}

	if (ch->in_room->area->low_r_vnum == ROOM_VNUM_ARENA)
	{
		send_to_char("Use kill instead.\n\r", ch);
		pop_call();
		return;
	}

	if (ch->position == POS_FIGHTING)
	{
		send_to_char( "You do the best you can!\n\r", ch );
		pop_call();
		return;
	}

	if (!can_attack (ch, victim) )
	{
		send_to_char( "You may not assassinate that person.\n\r", ch);
		pop_call();
		return;
	}

	if (victim->pcdata->just_died_ctr > 0)
	{
		send_to_char("That person is currently protected by their god.\n\r", ch);
		pop_call();
		return;
	}

	if (victim->fighting != NULL)
	{
		send_to_char( "You may not assassinate that person, at the moment.\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_NPC(victim ) && victim->pcdata->corpse != NULL)
	{
		ch_printf(ch, "%s has been killed quite recently.\n\r", get_name(victim));
		pop_call();
		return;
	}

	/*
		Code for total and max pvnum attacks  -  Chaos 5/6/99
	*/

	if (!check_add_attack(ch, victim))
	{
		ch_printf(ch, "%s has been attacked too many times.\n\rYou should leave them alone.\n\r", get_name(victim));
		pop_call();
		return;
	}

	/*
		ok...check and see if the victim has the specified item
	*/

	argument=one_argument( argument, arg2 );

	if (arg2[0] != '\0' && number_percent() < learned(ch, gsn_assassinate))
	{
		OBJ_DATA *obj;

		if ((obj = get_obj_list(victim, arg2, victim->first_carrying)) == NULL)
		{
			send_to_char( "They don't seem to have it!\n\r", ch );
			pop_call();
			return;
		}
		if (IS_SET(obj->extra_flags, ITEM_INVENTORY) || obj->level > ch->level)
		{
			send_to_char( "You wouldn't be able to pry it away.\n\r", ch );
			pop_call();
			return;
		}
		if (ch->carry_number >= can_carry_n(ch))
		{
			send_to_char( "You have your hands full.\n\r", ch );
			pop_call();
			return;
		}
		if (ch->carry_weight >= can_carry_w(ch))
		{
			send_to_char( "You can't carry that much weight.\n\r", ch );
			pop_call();
			return;
		}
		ch->pcdata->asn_obj = obj;
	}
	else
	{
		ch->pcdata->asn_obj = NULL;
	}

	check_killer( ch, victim );

	if (number_percent() > learned(ch, gsn_muffle)/2)
	{
		sprintf(buf, "%s", get_name(victim));
		do_battle("%s battles 'Help!  I am being assassinated by %s!'", buf, get_name(ch));
	}
	else
	{
		act( "You quickly silence $N before they can call for help!", ch, NULL, victim, TO_CHAR);
		act( "You are silenced by $n before you can call for help!",  ch, NULL, victim, TO_VICT);
		check_improve(ch, gsn_muffle);
	}

	ch->pcdata->just_died_ctr = 0;

	clear_attack_list(ch);

	log_printf("ROOM [%u] ASSASSINATE %s attacking %s", ch->in_room->vnum, ch->name, victim->name);

	if (number_range(33, 100) < 100 * victim->hit / UMAX(1, victim->max_hit)
	&&  number_percent() < learned(ch, gsn_knife)
	&&  number_percent() < learned(ch, gsn_quick_draw)
	&&  number_percent() < 100*multi_skill_level(ch, gsn_quick_draw)/ch->level)
	{
		OBJ_DATA * obj;
		if ((obj = get_eq_char(ch,WEAR_WIELD)) != NULL && (obj->value[3]==11 || obj->value[3]==2))
		{
			one_hit(ch, victim, gsn_knife);
			check_improve(ch, gsn_quick_draw);
			wait_state(ch, skill_table[gsn_knife].beats);

			pop_call();
			return;
		}
	}

	wait_state(ch, PULSE_PER_SECOND);

	multi_hit( ch, victim, TYPE_UNDEFINED );

	pop_call();
	return;
}

void do_gouge( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	AFFECT_DATA af;

	push_call("do_gouge(%p,%p)",ch,argument);

	if (!IS_NPC(ch) && multi(ch, gsn_gouge) == -1)
	{
		ch_printf(ch, "You haven't been taught this dirty trick!\n\r");
		pop_call();
		return;
	}

	if ((victim = who_fighting(ch)) == NULL)
	{
		send_to_char( "You aren't fighting anyone.\n\r", ch );
		pop_call();
		return;
	}

	wait_state( ch, skill_table[gsn_gouge].beats );

	if (!check_hit(ch, victim, GET_HITROLL(ch), gsn_gouge))
	{
		damage(ch, victim, 0, gsn_gouge);
		pop_call();
		return;
	}

	damage(ch, victim, number_range(5, ch->level), gsn_gouge);

	check_improve(ch, gsn_gouge);

	if (valid_fight(ch, victim))
	{
		if (!IS_AFFECTED(victim, AFF_BLIND))
		{
			if (!IS_AFFECTED(ch, AFF_TRUESIGHT))
			{
				af.type      = gsn_gouge;
				af.location  = APPLY_HITROLL;
				af.modifier  = -6;
				af.duration  = multi_skill_level(ch,gsn_gouge) / 2 / get_curr_con(victim);
				af.bittype   = AFFECT_TO_CHAR;
				af.bitvector = AFF_BLIND;
				affect_to_char( victim, &af );
				act( "You can't see a thing!", victim, NULL, NULL, TO_CHAR );
			}
			else
			{
				act( "$N seems to be able to see despite your attack!", ch, NULL, victim, TO_CHAR );
				act( "You are unaffected by $n's attack.", ch, NULL, victim, TO_VICT );
			}
		}
		wait_state( ch,     PULSE_VIOLENCE );
	}
	else
	{
		act( "Your fingers plunge into your victim's brain, causing immediate death!", ch, NULL, NULL, TO_CHAR );
	}
	pop_call();
	return;
}

void do_circle( CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	push_call("do_circle(%p,%p)",ch,argument);

	if ((victim = who_fighting(ch)) == NULL)
	{
		send_to_char( "You can't circle when you aren't fighting.\n\r", ch);
		pop_call();
		return;
	}

	if (is_safe(ch, NULL))
	{
		pop_call();
		return;
	}

	if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL
	||  (obj->value[3] != 11 && obj->value[3] != 2))
	{
		send_to_char( "You need to wield a piercing weapon.\n\r", ch );
		pop_call();
		return;
	}

	if (!ch->fighting)
	{
		send_to_char( "You can't circle when you aren't fighting.\n\r", ch);
		pop_call();
		return;
	}

	if (!victim->fighting)
	{
		send_to_char( "You can't circle around a person who is not fighting.\n\r", ch );
		pop_call();
		return;
	}

	if (who_fighting(victim) == ch)
	{
		act( "You can't circle around them without a distraction.", ch, NULL, victim, TO_CHAR );
		pop_call();
		return;
	}

	wait_state( ch, skill_table[gsn_circle].beats );

	if (!check_hit(ch, victim, GET_HITROLL(ch), gsn_circle))
	{
		damage( ch, victim, 0, gsn_circle );
		pop_call();
		return;
	}

	one_hit( ch, victim, gsn_circle );

	pop_call();
	return;
}

void do_berserk( CHAR_DATA *ch, char *argument )
{
	push_call("do_berserk(%p,%p)",ch,argument);

	if (!IS_NPC(ch) && ch->pcdata->mclass[multi(ch, gsn_berserk)] != ch->level)
	{
		send_to_char("You spit foam and roar but nobody seems to notice.\n\r", ch);
		pop_call();
		return;
	}

	if (IS_NPC(ch) || number_percent() < learned(ch, gsn_berserk))
	{
		AFFECT_DATA af;


		af.type      = gsn_berserk;
		af.duration  = 5;
		af.bittype   = AFFECT_TO_CHAR;
		af.bitvector = AFF2_BERSERK;

		af.modifier  = 0 - ch->level / 3;
		af.location  = APPLY_SAVING_SPELL;
		affect_to_char( ch, &af );

		af.modifier  = ch->level / 4;
		af.location  = APPLY_HITROLL;
		affect_to_char( ch, &af );

		af.modifier  = ch->level / 2;
		af.location  = APPLY_DAMROLL;
		affect_to_char( ch, &af );

		send_to_char( "Now you REALLY wanna KILL!\n\r",ch);
		check_improve(ch, gsn_berserk);
	}
	else
	{
		wait_state( ch, skill_table[gsn_berserk].beats );
		send_to_char("You failed to go berserk!\n\r",ch);
	}
	pop_call();
	return;
}

void do_blood_frenzy(CHAR_DATA *ch, char *arg)
{
	push_call("do_blood_frenzy(%p,%p)",ch,arg);

	if (number_percent() < learned(ch, gsn_blood_frenzy))
	{
		AFFECT_DATA af;

		if (is_affected(ch, gsn_blood_frenzy))
		{
			send_to_char( "Blood already maddened you!\n\r", ch );
			pop_call();
			return;
		}

		af.type      = gsn_blood_frenzy;
		af.duration  = 12;
		af.bitvector = 0;

		af.modifier  = (1 + ch->level / 18 + ch->pcdata->reincarnation);
		af.location  = APPLY_DAMROLL;
		affect_to_char( ch, &af );

		af.modifier  = (1 + ch->level / 18);
		af.location  = APPLY_STR;
		affect_to_char( ch, &af );

		send_to_char("You feel rage spreading through your body.\n\r",ch);
		act( "$n gets a crazy look in $s eyes.", ch, NULL, NULL, TO_ROOM );
		check_improve(ch, gsn_blood_frenzy);
	}
	else
	{
		wait_state(ch, skill_table[gsn_blood_frenzy].beats);
		send_to_char("You failed to get mad at the sight of blood.\n\r",ch);
	}
	pop_call();
	return;
}

void do_drain( CHAR_DATA *ch, char *arg )
{
	CHAR_DATA *victim;
	char arg1[MAX_INPUT_LENGTH];

	push_call("do_drain(%p,%p)",ch,arg);

	if (learned(ch, gsn_drain) == 0)
	{
		send_to_char("The undead curse is not upon you.\n\r",ch);
		pop_call();
		return;
	}

	one_argument( arg, arg1 );

	if (arg1[0] == '\0')
	{
		if ((victim = who_fighting(ch)) == NULL)
		{
			send_to_char("You are not fighting anyone.\n\r",ch);
			pop_call();
			return;
		}
	}
	else if ((victim = get_char_room(ch, arg1)) == NULL)
	{
		send_to_char("Drain whom?\n\r",ch);
		pop_call();
		return;
	}

	if (is_safe(ch, victim))
	{
		pop_call();
		return;
	}

	wait_state(ch, skill_table[gsn_drain].beats);


	if (!check_hit(ch, victim, GET_HITROLL(ch), TYPE_HIT))
	{
		damage(ch, victim, 0, gsn_drain);
	}
	else
	{
		int dam = GET_DAMROLL(ch);

		dam += number_range(ch->level*1, ch->level*2);

		ch->hit = UMIN(ch->max_hit, ch->hit+dam/10);

		gain_condition(ch, COND_FULL,   1);
		gain_condition(ch, COND_THIRST, 1);

		damage(ch, victim, dam, gsn_drain);
	}
	pop_call();
	return;
}


void do_strangle( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	AFFECT_DATA af;

	push_call("do_strangle(%p,%p)",ch,argument);

	if (learned(ch, gsn_strangle) == 0)
	{
		send_to_char( "They would just get mad at you.\n\r", ch );
		pop_call();
		return;
	}

	if ((victim = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("Strangle whom?\n\r",ch);
		pop_call();
		return;
	}

	if (IS_NPC(victim) && is_safe(ch, victim))
	{
		pop_call();
		return;
	}

	if (!IS_NPC(victim))
	{
		if (victim == ch)
		{
			send_to_char( "You strangle yourself.  Uch!\n\r", ch );
			pop_call();
			return;
		}

		if (is_safe(ch, NULL))
		{
			pop_call();
			return;
		}

		if (ch->position == POS_FIGHTING)
		{
			send_to_char( "You do the best you can!\n\r", ch );
			pop_call();
			return;
		}

		if (!can_attack (ch, victim) )
		{
			send_to_char( "You may not strangle that person.\n\r", ch);
			pop_call();
			return;
		}

		if (victim->pcdata->just_died_ctr > 0)
		{
			send_to_char("That person is currently protected by their god.\n\r", ch);
			pop_call();
			return;
		}

		if (victim->fighting != NULL)
		{
			send_to_char( "You may not strangle that person, at the moment.\n\r", ch);
			pop_call();
			return;
		}

/*
		if (!IS_SET(victim->act, PLR_KILLER) && !IS_SET(victim->act, PLR_THIEF))
		{
			if (ch->pcdata->last_connect + 120 > mud->current_time)
			{
				ch_printf(ch, "You must wait %d seconds before you can strangle anyone.\n\r", ch->pcdata->last_connect + 120 - mud->current_time);
				pop_call();
				return;
			}
		}
*/
		if (victim->pcdata->corpse != NULL)
		{
			ch_printf(ch, "%s has been killed quite recently.\n\r", get_name(victim));
			pop_call();
			return;
		}
	}

	wait_state( ch, skill_table[gsn_strangle].beats );

	if (IS_AFFECTED(victim, AFF_SLEEP))
	{
		send_to_char( "They are already unconscious.\n\r", ch );
		pop_call();
		return;
	}

	if (number_range(1, victim->level * 3 / 2) > multi_skill_level(ch, gsn_strangle) || number_percent() > learned(ch, gsn_strangle))
	{
		act("$n is trying to strangle you!", ch, NULL, victim, TO_VICT);
		act("You failed to strangle $N!",    ch, NULL, victim, TO_CHAR);

		if (IS_NPC(victim))
		{
			act( "$n suddenly looks VERY angry!", victim, NULL, NULL, TO_ROOM);
			multi_hit(victim, ch, TYPE_UNDEFINED);
		}
		pop_call();
		return;
	}

	af.type      = gsn_sleep;
	af.duration  = 1;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF_SLEEP;
	affect_to_char( victim, &af );

	wait_state(victim, skill_table[gsn_strangle].beats * 3 / 2);

	if ( IS_AWAKE(victim) )
	{
		send_to_char( "The world blurs as your consciousness fades.\n\r", victim );
		act( "$n collapses onto the ground.", victim, NULL, NULL, TO_ROOM );
		victim->position = POS_SLEEPING;
	}

	pop_call();
	return;
}


void knight_adjust_hpmnmv( CHAR_DATA *ch )
{
	push_call("knight_adjust_hpmnmv(%p)",ch);

	if (ch->pcdata->actual_max_hit < 100)
	{
		ch->pcdata->actual_max_hit  = ch->max_hit  = 100;
	}

	if (ch->pcdata->actual_max_mana < 100)
	{
		ch->pcdata->actual_max_mana = ch->max_mana = 100;
	}

	if (ch->pcdata->actual_max_move < 100)
	{
		ch->pcdata->actual_max_move = ch->max_move = 100;
	}

	pop_call();
	return;
}

bool can_reincarnate_attack( CHAR_DATA *ch, CHAR_DATA *victim )
{
	push_call("can_reincarnate_attack(%p,%p)",ch,victim);

	if (IS_NPC(ch) || IS_NPC(victim))
	{
		pop_call();
		return TRUE;
	}

	if (ch->pcdata->reincarnation*2/3 > victim->pcdata->reincarnation)
	{
		pop_call();
		return FALSE;
	}

	if (ch->pcdata->reincarnation*3/2 < victim->pcdata->reincarnation)
	{
		pop_call();
		return FALSE;
	}
	pop_call();
	return TRUE;
}

/*
	  0 1 2 3 4 5 6 7 8 9 A	reincarnate attack chart:
	0 *
	1   *
	2     * *
	3     * * *
	4       * * * *
	5         * * * *
	6         * * * * * *
	7           * * * * * *
	8             * * * * *
	9             * * * * *
	A	           * * * *
*/

bool can_attack( CHAR_DATA *ch, CHAR_DATA *victim)
{
	push_call("can_attack(%p,%p)",ch,victim);

	if (IS_NPC(ch) || IS_NPC(victim))
	{
		pop_call();
		return TRUE;
	}

	if (ch->in_room->area->low_r_vnum == ROOM_VNUM_ARENA && victim->in_room->area->low_r_vnum == ROOM_VNUM_ARENA)
	{
		pop_call();
		return TRUE;
	}

	if (victim->level >= LEVEL_IMMORTAL || ch->level >= LEVEL_IMMORTAL)
	{
		pop_call();
		return FALSE;
	}

	if (ch->pcdata->reincarnation < victim->pcdata->reincarnation)
	{
		if (ch->pcdata->reincarnation*3/2 < victim->pcdata->reincarnation)
		{
			pop_call();
			return FALSE;
		}
		if (victim->pcdata->reincarnation*2/3 > ch->pcdata->reincarnation)
		{
			pop_call();
			return FALSE;
		}
	}
	if (ch->pcdata->reincarnation > victim->pcdata->reincarnation)
	{
		if (ch->pcdata->reincarnation*2/3 > victim->pcdata->reincarnation)
		{
			pop_call();
			return FALSE;
		}
		if (victim->pcdata->reincarnation*3/2 < ch->pcdata->reincarnation)
		{
			pop_call();
			return FALSE;
		}
	}

	if (which_god(ch) == GOD_NEUTRAL || which_god(victim) == GOD_NEUTRAL)
	{
		pop_call();
		return FALSE;
	}

	if (ch->pcdata->clan && ch->pcdata->clan == victim->pcdata->clan)
	{
		pop_call();
		return FALSE;
	}

	if (which_god(victim) == which_god(ch))
	{
		pop_call();
		return FALSE;
	}

	if (ch->level <= 25 || victim->level <= 25)
	{
		pop_call();
		return FALSE;
	}

	if (victim->level < ch->level-10 || victim->level > ch->level+10)
	{
		pop_call();
		return FALSE;
	}

	pop_call();
	return TRUE;
}

/*
	Orcish Brawling  - Chaos 8/20/98
*/

void orc_brawl( CHAR_DATA *ch, CHAR_DATA *victim)
{
	int level;
	int dmg, dmgL, dmgH;
	int tshares, pshare;

	push_call("orc_brawl(%p,%p)",ch,victim);

	level = ch->level;

	/*
		Find total shares
	*/

	if (level < 15)
	{
		tshares = 8;
	}
	else if (level < 30)
	{
		tshares = 15;
	}
	else if (level < 45)
	{
		tshares = 21;
	}
	else if (level < 60)
	{
		tshares = 26;
	}
	else if (level < 75)
	{
		tshares = 30;
	}
	else
	{
		tshares = 33;
	}
	pshare = number_range(1, tshares);

	if (pshare <= 8)
	{
		brawling_attack = 0;
		dmgL = (int) level * 0.25;
		dmgH = (int) level * 0.50;
	}
	else if( pshare <= 15 )
	{
		brawling_attack = 1;
		dmgL = (int) level * 0.50;
		dmgH = (int) level * 0.75;
	}
	else if( pshare <= 21 )
	{
		brawling_attack = 2;
		dmgL = (int) level * 0.75;
		dmgH = (int) level * 1.00;
	}
	else if( pshare <= 26 )
	{
		brawling_attack = 3;
		dmgL = (int) level * 1.00;
		dmgH = (int) level * 1.25;
	}
	else if( pshare <= 30 )
	{
		brawling_attack = 4;
		dmgL = (int) level * 1.25;
		dmgH = (int) level * 1.50;
	}
	else
	{
		brawling_attack = 5;
		dmgL = (int) level * 1.50;
		dmgH = (int) level * 1.75;
	}

	dmg = number_range(dmgL, dmgH);

	if (check_hit(ch, victim, GET_HITROLL(ch), TYPE_HIT))
	{
		damage(ch, victim, GET_DAMROLL(ch) + dmg, gsn_brawling);
	}
	else
	{
		damage(ch, victim, 0, gsn_brawling);
	}
	pop_call();
	return;
}

void stop_hate_fear( CHAR_DATA *ch )
{
	push_call("stop_hate_fear(%p)",ch);

	if (IS_NPC(ch) && *ch->npcdata->hate_fear)
	{
		STRFREE(ch->npcdata->hate_fear);
		ch->npcdata->hate_fear = STRDUPE(str_empty);
	}
	pop_call();
	return;
}

/*
	Could use some messages
*/

void found_hating( CHAR_DATA *ch, CHAR_DATA *victim )
{
	char buf[MAX_INPUT_LENGTH];

	push_call("found_hating(%p,%p)",ch,victim);

	if (!can_see(ch, victim))
	{
		if (number_bits(2) != 0)
		{
			pop_call();
			return;
		}

		if (IS_SET(ch->act, ACT_SMART))
		{
			switch (number_bits(2))
			{
				case 0:
					sprintf(buf, "I can smell your blood %s!", short_to_name(get_name(victim), 1));
					break;
				case 1:
					sprintf(buf, "Where are you %s? Come here and die!", short_to_name(get_name(victim), 1));
					break;
				case 2:
					sprintf(buf, "Just wait until I find you %s - then you'll pay!", short_to_name(get_name(victim), 1));
					break;
				case 3:
					sprintf(buf, "You're dead %s! Just as soon as I get my hands on you!", short_to_name(get_name(victim), 1));
					break;
			}
			do_say(ch, buf);
		}

		switch (number_bits(2))
		{
			case 0:
				act("$n staggers around blindly, attacking the air!", ch, NULL, NULL, TO_ROOM);
				break;
			case 1:
				act("$n lurches around the area trying to find something to attack!", ch, NULL, NULL, TO_ROOM);
				break;
			case 2:
				act("$n growls in frustration as $e tries to attack unseen enemies!", ch, NULL, NULL, TO_ROOM);
				break;
			case 3:
				act("$n snarls quietly as $e attacks thin air!", ch, NULL, NULL, TO_ROOM);
				break;
		}
		pop_call();
		return;
	}

	if (is_safe(ch, NULL))
	{
		if (number_bits(2) != 0)
		{
			pop_call();
			return;
		}
		if (IS_SET(ch->act, ACT_SMART))
		{
			switch (number_bits(2))
			{
				case 0:
					sprintf(buf, "You're a coward %s! Fight me!", short_to_name(get_name(victim), 1));
					break;
				case 1:
					sprintf(buf, "That's right %s, hide from me you coward!", short_to_name(get_name(victim), 1));
					break;
				case 2:
					sprintf(buf, "C'mon %s, fight you craven dog!", short_to_name(get_name(victim), 1));
					break;
				case 3:
					sprintf(buf, "Only a coward would hide here %s!", short_to_name(get_name(victim), 1));
					break;
			}
			do_say(ch, buf);
		}
		pop_call();
		return;
	}

	if (IS_SET(ch->act, ACT_SMART))
	{
		switch (number_bits(2))
		{
			case 0:
				sprintf(buf, "I'll get you now %s! Die!", short_to_name(get_name(victim), 1));
				break;
			case 1:
				sprintf(buf, "We were not done yet %s!", short_to_name(get_name(victim), 1));
				break;
			case 2:
				sprintf(buf, "Defend yourself %s!", short_to_name(get_name(victim), 1));
				break;
			case 3:
				sprintf(buf, "Prepare for your death %s!", short_to_name(get_name(victim), 1));
				break;
		}
		do_say(ch, buf);
	}
	else
	{
		switch (number_bits(2))
		{
			case 0:
				act("$n contorts $s face and surges to the attack!", ch, NULL, NULL, TO_ROOM);
				break;
			case 1:
				act("$n snarls and leaps to the attack!", ch, NULL, NULL, TO_ROOM);
				break;
			case 2:
				act("$n growls and lunges forwards!", ch, NULL, NULL, TO_ROOM);
				break;
			case 3:
				act("$n grinds $s teeth and leaps forwards!", ch, NULL, NULL, TO_ROOM);
				break;
		}
	}
	multi_hit(ch, victim, TYPE_UNDEFINED);

	pop_call();
	return;
}

void do_attack( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim = NULL;
	char buf[MAX_STRING_LENGTH];

	push_call("do_attack(%p,%p)",ch,argument);

	one_argument(argument, buf);

	if (buf[0] == '\0')
	{
		victim = ch;
	}
	else	if ((victim = get_player_world(ch, buf)) == NULL)
	{
		send_to_char("Syntax: attack <player>\n\r", ch);
		pop_call();
		return;
	}

	get_attack_string( victim, ch, buf );
	send_to_char( ansi_translate_text( ch, buf), ch );

	pop_call();
	return;
}

void get_attack_string( CHAR_DATA *ch, CHAR_DATA *viewer, char *buf )
{
	int cnt, leng;
	int attacks;
	char buf2[MAX_INPUT_LENGTH];

	push_call("get_attack_string(%p,%p,%p)",ch,viewer,buf);

	if (ch == NULL)
	{
		pop_call();
		return;
	}

	leng = str_cpy_max( buf, "{138}  Total Player Killer Attacks:\n\r", MAX_STRING_LENGTH);

	attacks=0;
	for (cnt = 0; cnt < MAX_PK_ATTACKS ; cnt++)
	{
		if (ch->pcdata->last_pk_attack_time[cnt] > 0)
		{
			attacks++;
			if( ch->pcdata->last_pk_attack_time[cnt] > mud->current_time )
			{
				sprintf( buf2, "{178}%12s till {128}%s\n\r",
					ch->pcdata->last_pk_attack_name[cnt],
					get_time_string(ch->pcdata->last_pk_attack_time[cnt]));
			}
			else
			{
				sprintf( buf2, "{078}%12s till {028}%s\n\r",
					ch->pcdata->last_pk_attack_name[cnt],
					get_time_string(ch->pcdata->last_pk_attack_time[cnt]));
			}
			leng = str_apd_max( buf, buf2, leng, MAX_STRING_LENGTH );
		}
	}
	if (attacks == 0 )
	{
		if ( viewer == ch )
		{
			sprintf( buf, "{138}  You have never been attacked.\n\r" );
		}
		else
		{
			sprintf( buf, "{138}  %s has never been attacked.\n\r", ch->name );
		}
	}
	pop_call();
	return;
}

/*
	Code for total and max pvnum attacks  -  Chaos 4/20/99
*/

bool check_add_attack( CHAR_DATA *ch, CHAR_DATA *victim )
{
	push_call("check_add_attack(%p,%p)",ch,victim);

	/*
		Killers and Thieves can always be attacked - Scandum 07/02/02
	*/

	if (!IS_NPC(ch) && !IS_NPC(victim))
	{
		int oldest_attack, total_pvnum_attacks, attacks_today, cnt;

		attacks_today = 0;
		oldest_attack = 0;
		total_pvnum_attacks = 0;

		for (cnt = 0 ; cnt < MAX_PK_ATTACKS ; cnt++)
		{
			if (victim->pcdata->last_pk_attack_time[cnt] == 0)
			{
				oldest_attack = cnt;
			}
			else if (victim->pcdata->last_pk_attack_time[oldest_attack] > victim->pcdata->last_pk_attack_time[cnt])
			{
				oldest_attack = cnt;
			}

			if (victim->pcdata->last_pk_attack_time[cnt] > mud->current_time)
			{
				attacks_today++;
			}

			if (ch->pcdata->pvnum == victim->pcdata->last_pk_attack_pvnum[cnt]
			&& victim->pcdata->last_pk_attack_time[cnt] > mud->current_time)
			{
				total_pvnum_attacks++;
			}
		}
		if (!IS_SET(victim->act, PLR_KILLER) && !IS_SET(victim->act, PLR_THIEF))
		{
			if (attacks_today >= MAX_PK_ATTACKS)
			{
				pop_call();
				return( FALSE );
			}

			if (total_pvnum_attacks >= 5)
			{
				pop_call();
				return( FALSE );
			}
			victim->pcdata->last_pk_attack_time[oldest_attack] = mud->current_time + 60 * 60 * 6;
			victim->pcdata->last_pk_attack_pvnum[oldest_attack] = ch->pcdata->pvnum;

			STRFREE(victim->pcdata->last_pk_attack_name[oldest_attack]);
			victim->pcdata->last_pk_attack_name[oldest_attack] = STRALLOC(ch->name);
		}
	}
	pop_call();
	return( TRUE );
}

/*
	Scandum 14-01-2003
*/

void clear_attack_list( CHAR_DATA *ch )
{
	int cnt;

	push_call("clear_attack_list(%p)",ch);

	for (cnt = 0 ; cnt < MAX_PK_ATTACKS ; cnt++)
	{
		ch->pcdata->last_pk_attack_time[cnt] = 0;

		STRFREE(ch->pcdata->last_pk_attack_name[cnt]);
		ch->pcdata->last_pk_attack_name[cnt] = STRDUPE(str_empty);
	}
	pop_call();
	return;
}

void spam_attack_list( CHAR_DATA *ch )
{
	int cnt;

	push_call("spam_attack_list(%p)",ch);

	for (cnt = 0 ; cnt < MAX_PK_ATTACKS ; cnt++)
	{
		ch->pcdata->last_pk_attack_time[cnt]  = mud->current_time + 60 * 60 * 24;
		ch->pcdata->last_pk_attack_pvnum[cnt] = 0;

		STRFREE(ch->pcdata->last_pk_attack_name[cnt]);
		ch->pcdata->last_pk_attack_name[cnt] = STRDUPE(str_empty);
	}
	pop_call();
	return;
}
