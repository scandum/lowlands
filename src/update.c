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

void msdp_update                args ((void));
void mobile_update              args ((void));
void shop_update                args ((void));
void weather_update             args ((void));
void time_update                args ((void));
void char_update                args ((void));
void obj_update                 args ((void));
void aggr_update                args ((void));
void bounty_update              args ((void));
void auto_area_save             args ((void));
void auto_char_save             args ((void));
void mob_program_update         args ((void));
void obj_program_update         args ((void));
void purger_update              args ((void));

/*
	Advancement stuff.
*/

void advance_level (CHAR_DATA * ch, bool fSave)
{
	int add_hp;
	int add_mana;
	int add_move;
	int add_prac;

	push_call("advance_level(%p,%p)",ch,fSave);

	send_to_char("You raise a level!!", ch);

	ch->pcdata->exp = exp_level(ch->class, ch->level) + 1;

	ch->level += 1;
	ch->trust += 1;

	ch->pcdata->mclass[ch->class] += 1;

	number_seed(ch->pcdata->pvnum + ch->pcdata->reincarnation * MAX_LEVEL + ch->level);

	add_hp   = god_table[ch->pcdata->god].bonus_hp   + con_app[get_curr_con(ch)].hitp  + race_table[ch->race].hp_mod   + number_range(class_table[ch->class].hp_min,   class_table[ch->class].hp_max);
	add_mana = god_table[ch->pcdata->god].bonus_mana + int_app[get_curr_int(ch)].manap + race_table[ch->race].mana_mod + number_range(class_table[ch->class].mana_min, class_table[ch->class].mana_max);
	add_move = god_table[ch->pcdata->god].bonus_move + dex_app[get_curr_dex(ch)].movep + race_table[ch->race].move_mod;
	add_prac = wis_app[get_curr_wis(ch)].practice;

	add_hp   = UMAX (1, add_hp);
	add_mana = UMAX (1, add_mana);
	add_move = UMAX (1, add_move);

	add_hp   += ch->pcdata->reincarnation * 2;
	add_mana += ch->pcdata->reincarnation * 3;
	add_move += ch->pcdata->reincarnation * 3;
	add_prac += ch->pcdata->reincarnation * 2;

	/* Knight stuff get double   - Chaos     9/14/95 */

	if (ch->level > 90)
	{
		add_hp   *= 2;
		add_mana *= 2;
		add_move *= 2;
		add_prac *= 2;
	}

	ch->pcdata->actual_max_hit  += add_hp;
	ch->pcdata->actual_max_mana += add_mana;
	ch->pcdata->actual_max_move += add_move;

	ch->max_hit  += add_hp;
	ch->max_mana += add_mana;
	ch->max_move += add_move;

	ch->pcdata->practice += add_prac;

	if (ch->level % 15 == 0)
	{
		add_language(ch);
	}

	if (fSave)
	{
		sub_player (ch);
		add_player (ch);
		save_char_obj (ch, NORMAL_SAVE);
		save_char_obj (ch, BACKUP_SAVE);
	}

	ch_printf(ch, "Your gain is: %d/%d hp, %d/%d m, %d/%d mv %d/%d prac.\n\r",
		add_hp,   ch->max_hit,
		add_mana, ch->max_mana,
		add_move, ch->max_move,
		add_prac, ch->pcdata->practice );

	vt100prompt(ch);

	pop_call();
	return;
}

void demote_level(CHAR_DATA *ch, bool fSave)
{
	int add_hp;
	int add_mana;
	int add_move;
	int add_prac;

	push_call("demote_level(%p,%p)",ch,fSave);

	send_to_char("You lost a level!!", ch);

	number_seed(ch->pcdata->pvnum + ch->pcdata->reincarnation * MAX_LEVEL + ch->level);

	ch->level -= 1;
	ch->trust -= 1;

	ch->pcdata->mclass[ch->class] -= 1;

	if (ch->pcdata->exp > exp_level(ch->class, ch->level-1))
	{
		ch->pcdata->exp = exp_level(ch->class, ch->level-1) + 1;
	}

	add_hp   = god_table[ch->pcdata->god].bonus_hp   + con_app[get_max_con(ch)].hitp  + race_table[ch->race].hp_mod   + number_range(class_table[ch->class].hp_min,   class_table[ch->class].hp_max);
	add_mana = god_table[ch->pcdata->god].bonus_mana + int_app[get_max_int(ch)].manap + race_table[ch->race].mana_mod + number_range(class_table[ch->class].mana_min, class_table[ch->class].mana_max);
	add_move = god_table[ch->pcdata->god].bonus_move + dex_app[get_max_dex(ch)].movep + race_table[ch->race].move_mod;
	add_prac = wis_app[get_max_wis(ch)].practice;

	add_hp   = UMAX (1, add_hp);
	add_mana = UMAX (1, add_mana);
	add_move = UMAX (1, add_move);

	add_hp   += ch->pcdata->reincarnation * 2;
	add_mana += ch->pcdata->reincarnation * 3;
	add_move += ch->pcdata->reincarnation * 4;
	add_prac += ch->pcdata->reincarnation * 2;

	/* Knight stuff get double   - Chaos     9/14/95 */

	if (ch->level >= 90)
	{
		add_hp   *= 2;
		add_mana *= 2;
		add_move *= 2;
		add_prac *= 2;
	}

	add_hp += add_prac * 2;

	ch->pcdata->actual_max_hit  -= add_hp;
	ch->pcdata->actual_max_mana -= add_mana;
	ch->pcdata->actual_max_move -= add_move;

	ch->max_hit  -= add_hp;
	ch->max_mana -= add_mana;
	ch->max_move -= add_move;

	if (fSave)
	{
		sub_player (ch);
		add_player (ch);
		save_char_obj (ch, NORMAL_SAVE);
		save_char_obj (ch, BACKUP_SAVE);
	}

	ch_printf(ch, "Your loss is: %d hp, %d m, %d mv.\n\r", add_hp, add_mana, add_move);

	vt100prompt(ch);

	pop_call();
	return;
}


void gain_exp (CHAR_DATA * ch, int gain)
{
	push_call("gain_exp(%p,%p)",ch,gain);

	if (IS_NPC (ch) || ch->level >= LEVEL_HERO - 1 || ch->in_room->area->low_r_vnum == ROOM_VNUM_ARENA)
	{
		pop_call();
		return;
	}

	ch->pcdata->exp = UMAX(0, ch->pcdata->exp + gain);

	if (ch->pcdata->exp >= exp_level(ch->class, ch->level))
	{
		ch->pcdata->exp = exp_level(ch->class, ch->level) + 1;
		advance_level (ch, TRUE);
	}
	pop_call();
	return;
}

/*
	Regeneration stuff.
*/

int hit_gain (CHAR_DATA * ch, int message)
{
	int gain;

	push_call("hit_gain(%p)",ch);

	if (IS_NPC(ch))
	{
		gain = ch->level;
	}
	else
	{
		gain = con_app[get_curr_con(ch)].regen;

		if (ch->pcdata->condition[COND_FULL] == 0)
		{
			gain = gain * 0.5;
		}

		if (ch->pcdata->condition[COND_THIRST] == 0)
		{
			gain = gain * 0.5;
		}
	}

	if (ch->in_room && IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		gain = gain * 1.5;
	}

	if (ch->furniture && ch->furniture->item_type == ITEM_FURNITURE)
	{
		gain = gain * ch->furniture->value[3] / 100;
	}

	if (IS_AFFECTED(ch, AFF2_BERSERK))
	{
		gain = gain * 2;
	}

	if (IS_AFFECTED(ch, AFF_POISON))
	{
		gain = gain * 0.5;
	}

	if (IS_AFFECTED(ch, AFF2_TORRID_BALM))
	{
		gain = gain * 0.5;
	}

	if (IS_AFFECTED(ch, AFF2_BLEEDING))
	{
		gain = gain * 0.5;
	}

	if (in_camp(ch))
	{
		gain = gain * 2;
	}

	if (rspec_req(ch, RSPEC_FASTHEAL))
	{
		gain = gain * 1.5;
	}

	if (IS_AFFECTED(ch, AFF2_ENHANCED_HEAL))
	{
		if (message == VERBOSE)
		{
			if (!IS_NPC(ch) && rspec_req(ch,RSPEC_FASTHEAL))
			{
				send_to_char ("You heal faster.\n\r", ch);
			}
			else
			{
				send_to_char ("You heal easier.\n\r", ch);
			}
		}
		gain = gain * 1.5;
	}

	if (ch->fighting)
	{
		gain = gain * 0.25;
	}
	else
	{
		switch (ch->position)
		{
			case POS_SLEEPING:
				gain = gain * 2;
				break;
			case POS_RESTING:
				gain = gain * 1.75;
				break;
			case POS_SITTING:
				gain = gain * 1.5;
				break;
			case POS_STANDING:
				gain = gain * 0.5;
				break;
			default:
				gain = gain * 0.25;
				break;
		}
	}

	pop_call();
	return UMIN(gain, ch->max_hit - ch->hit);
}


int mana_gain (CHAR_DATA * ch, int message)
{
	int gain;

	push_call("mana_gain(%p)",ch);

	if (IS_NPC(ch))
	{
		gain = ch->level * 2;
	}
	else
	{
		gain = wis_app[get_curr_wis(ch)].regen;

		switch (ch->position)
		{
			case POS_SLEEPING:
				gain = gain * 2;
				break;
			case POS_RESTING:
				gain = gain * 1.75;
				break;
			case POS_SITTING:
				gain = gain * 1.5;
				break;
		}

		if (ch->pcdata->condition[COND_FULL] == 0)
		{
			gain /= 2;
		}

		if (ch->pcdata->condition[COND_THIRST] == 0)
		{
			gain /= 2;
		}
	}

	if (ch->in_room && IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		gain *= 1.5;
	}

	if (ch->furniture && ch->furniture->item_type == ITEM_FURNITURE)
	{
		gain = gain * ch->furniture->value[3] / 100;
	}

	if (IS_AFFECTED (ch, AFF2_BERSERK))
	{
		gain /= 4;
	}

	if (IS_AFFECTED (ch, AFF_POISON))
	{
		gain /= 4;
	}

	if (IS_AFFECTED (ch, AFF2_QUICKEN))
	{
		gain /= 2;
	}

	if (in_camp (ch))
	{
		gain *= 2;
	}

	if (rspec_req(ch, RSPEC_FASTREVIVE))
	{
		gain *= 1.5;
	}

	if (!IS_NPC(ch) && learned(ch, gsn_warmth))
	{
		gain += gain * multi_skill_level(ch, gsn_warmth) / 100;
	}

	if (IS_AFFECTED (ch, AFF2_ENHANCED_REVIVE))
	{
		if (message == VERBOSE)
		{
			if (!IS_NPC(ch) && learned(ch, gsn_warmth))
			{
				send_to_char("You feel warmth spreading through your body.\n\r", ch);
				check_improve(ch, gsn_warmth);
			}
			else if (!IS_NPC(ch) && rspec_req(ch, RSPEC_FASTREVIVE))
			{
				send_to_char ("You revive quicker.\n\r", ch);
			}
			else
			{
				send_to_char ("You revive easier.\n\r", ch);
			}
		}
		gain *= 1.5;
	}
	pop_call();
	return UMIN (gain, ch->max_mana - ch->mana);
}

int move_gain (CHAR_DATA * ch, int message)
{
	int gain;

	push_call("move_gain(%p)",ch);

	if (IS_NPC (ch))
	{
		gain = ch->level;
	}
	else
	{
		gain = dex_app[get_curr_dex(ch)].regen;

		switch (ch->position)
		{
			case POS_SLEEPING:
				gain *= 2;
				break;
			case POS_RESTING:
				gain *= 1.75;
				break;
			case POS_SITTING:
				gain *= 1.5;
				break;
		}

		if (ch->pcdata->condition[COND_FULL] == 0)
		{
			gain /= 2;
		}
		if (ch->pcdata->condition[COND_THIRST] == 0)
		{
			gain /= 2;
		}
	}

	if (ch->in_room && IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		gain *= 1.5;
	}

	if (ch->furniture && ch->furniture->item_type == ITEM_FURNITURE)
	{
		gain = gain * ch->furniture->value[3] / 100;
	}

	if (IS_AFFECTED (ch, AFF2_BERSERK))
	{
		gain *= 2;
	}

	if (IS_AFFECTED(ch, AFF2_BLEEDING))
	{
		gain /= 2;
	}

	if (IS_AFFECTED (ch, AFF_POISON))
	{
		gain /= 4;
	}

	if (in_camp (ch))
	{
		gain *= 2;
	}

	if (IS_AFFECTED (ch, AFF2_ENHANCED_REST))
	{
		if (message == VERBOSE)
		{
			send_to_char ("You rest easier.\n\r", ch);
		}
		gain *= 1.5;
	}

	pop_call();
	return UMIN (gain, ch->max_move - ch->move);
}

void gain_condition (CHAR_DATA * ch, int iCond, int value)
{
	int condition;

	push_call("gain_condition(%p,%p,%p)",ch,iCond,value);

	if ( value == 0 || IS_NPC (ch))
	{
		pop_call();
		return;
	}
	condition	= ch->pcdata->condition[iCond];
	ch->pcdata->condition[iCond]	= URANGE( 0, condition + value, 72 );

	if (rspec_req(ch,RSPEC_VAMPIRIC))
	{
		if (iCond == COND_THIRST && ch->pcdata->condition[COND_THIRST] <= 3)
		{
			send_to_char("Your body is craving for blood.\n\r", ch);
			act("$n looks pale and weak.", ch, NULL, NULL, TO_ROOM);

			damage(ch, ch, 1+ch->level/7, TYPE_NOFIGHT);
		}

		pop_call();
		return;
	}

	if ( ch->pcdata->condition[iCond] == 0 )
	{
		switch ( iCond )
		{
			case COND_FULL:
				if (ch->level > 25)
				{
					send_to_char( "You are STARVING!\n\r",  ch );
					act( "$n is starved half to death!", ch, NULL, NULL, TO_ROOM);
					damage(ch, ch, 1+ch->level/7, TYPE_NOFIGHT);
				}
				else
				{
					send_to_char( "You are really hungry.\n\r",  ch );
				}
				pop_call();
				return;
				break;

			case COND_THIRST:
				if( ch->level > 25 )
				{
					send_to_char( "You are DYING of THIRST!\n\r", ch );
					act( "$n is dying of thirst!", ch, NULL, NULL, TO_ROOM);
					damage(ch, ch, 1+ch->level/5, TYPE_NOFIGHT);
				}
				else
				{
					send_to_char( "You are really thirsty.\n\r", ch );
				}
				pop_call();
				return;
				break;

			case COND_DRUNK:
				if ( condition != 0 )
				{
					send_to_char( "You are sober.\n\r", ch );
					pop_call();
					return;

				}
				break;

			default:
				bug( "Gain_condition: invalid condition type %d", iCond );
				break;
		}
	}

	if ( ch->pcdata->condition[iCond] == 1 )
	{
		switch ( iCond )
		{
			case COND_FULL:
				send_to_char( "You are really hungry.\n\r",  ch );
				act( "You can hear $n's stomach growling.", ch, NULL, NULL, TO_ROOM);
				break;

			case COND_THIRST:
				send_to_char( "You are really thirsty.\n\r", ch );
				act( "$n looks a little parched.", ch, NULL, NULL, TO_ROOM);
				break;

			case COND_DRUNK:
				if ( condition != 0 )
				{
					send_to_char( "You are feeling a little less light headed.\n\r", ch );
				}
				break;
		}
	}

	if ( ch->pcdata->condition[iCond] == 2 )
	{
		switch ( iCond )
		{
			case COND_FULL:
				send_to_char( "You are hungry.\n\r",  ch );
				break;

			case COND_THIRST:
				send_to_char( "You are thirsty.\n\r", ch );
				break;
		}
	}

	if ( ch->pcdata->condition[iCond] == 3 )
	{
		switch ( iCond )
		{
			case COND_FULL:
				send_to_char( "You are a mite peckish.\n\r",  ch );
				break;

			case COND_THIRST:
				send_to_char( "You could use a sip of something refreshing.\n\r", ch );
				break;
		}
	}
	pop_call();
	return;
}

void msdp_update(void)
{
	DESCRIPTOR_DATA *d;

	for (d = mud->f_desc ; d ; d = d->next)
	{
		if (d->mth->msdp_data == NULL || CH(d) == NULL)
		{
			continue;
		}

		msdp_update_var(d, "ALIGNMENT", "%d", CH(d)->alignment);
		msdp_update_var(d, "EXPERIENCE", "%d", CH(d)->pcdata->exp);
		msdp_update_var(d, "EXPERIENCE_MAX", "%d", exp_level(CH(d)->class, CH(d)->level) - exp_level(CH(d)->class, CH(d)->level-1));
		msdp_update_var(d, "HEALTH", "%d", CH(d)->hit);
		msdp_update_var(d, "HEALTH_MAX", "%d", CH(d)->max_hit);
		msdp_update_var(d, "LEVEL", "%d", CH(d)->level);
		msdp_update_var(d, "MANA", "%d", CH(d)->mana);
		msdp_update_var(d, "MANA_MAX", "%d", CH(d)->max_mana);
		msdp_update_var(d, "MONEY", "%d", CH(d)->gold);
		msdp_update_var(d, "MOVEMENT", "%d", CH(d)->move);
		msdp_update_var(d, "MOVEMENT_MAX", "%d", CH(d)->max_move);

		if (HAS_BIT(d->mth->comm_flags, COMM_FLAG_MSDPUPDATE))
		{
			msdp_send_update(d);
		}
	}
}

void obj_program_update (void)
{
	PLAYER_GAME *pch, *pch_next;
	OBJ_DATA *obj, *obj_next;
	OBJ_PROG *prg;

	push_call("obj_program_update()");

	for (pch = mud->f_player ; pch ; pch = pch_next)
	{
		pch_next = pch->next;

		if (pch->ch->desc)
		{
			oprog_rand_trigger(pch->ch);

			for (obj = pch->ch->first_carrying ; obj ; obj = obj_next)
			{
				obj_next = obj->next_content;

				if (obj->carried_by && IS_SET(obj->pIndexData->oprogtypes, TRIG_TICK))
				{
					for (prg = obj->pIndexData->first_oprog ; prg ; prg = prg->next)
					{
						if (prg->trigger == TRIG_TICK)
						{
							if (number_range(1, 200) <= prg->percentage)
							{
								start_object_program( pch->ch, obj, prg, "");
							}
						}
					}
				}
			}
		}
	}
	pop_call();
	return;
}

void mob_program_update (void)
{
	CHAR_DATA *ich;
	AREA_DATA *area;

	int vnum;

	push_call("mob_program_update()");

	for (area = mud->f_area ; area ; area = area->next)
	{
		for (vnum = area->low_m_vnum ; vnum <= area->hi_m_vnum ; vnum++)
		{
			if (mob_index[vnum] == NULL || mob_index[vnum]->first_instance == NULL)
			{
				continue;
			}

			if (IS_SET(mob_index[vnum]->progtypes, RAND_PROG))
			{
				push_call("mob_program_update(RAND_PROG,%d)", vnum);

				for (ich = mob_index[vnum]->first_instance ; ich ; ich = mud->update_ich)
				{
					mud->update_ich = ich->next_instance;

					if (!MP_VALID_MOB(ich))
					{
						continue;
					}

					if (ich->position <= POS_SLEEPING || ich->position == POS_FIGHTING)
					{
						continue;
					}
					mudprog_percent_check(ich->pIndexData->first_prog, ich, NULL, NULL, NULL, RAND_PROG, ich->pIndexData->vnum);
				}
				pop_call();
				continue;
			}

			if (mob_index[vnum]->spec_fun && area->nplayer)
			{
				push_call("mob_program_update(SPEC_FUN,%d)",vnum);

				for (ich = mob_index[vnum]->first_instance ; ich ; ich = mud->update_ich)
				{
					mud->update_ich = ich->next_instance;

					if (!MP_VALID_MOB(ich))
					{
						continue;
					}

					(*ich->pIndexData->spec_fun) (ich);
				}
				pop_call();
			}
		}
	}
	/*
		Let's see if some trivia can be played - Scandum 01-11-2002
	*/

	if (mud->f_trivia)
	{
		update_trivia();
	}
	pop_call();
	return;
}

void auto_char_save (void)
{
	PLAYER_GAME *npl;
	CHAR_DATA *oldest;
	CLAN_DATA *oldest_clan, *clan;
	int oldest_time, oldest_clan_time;

	push_call("auto_char_save()");

	oldest		= NULL;
	oldest_time	= mud->current_time;

	for (npl = mud->f_player ; npl ; npl = npl->next)
	{
		if (npl->ch->pcdata->last_saved < oldest_time)
		{
			oldest_time	= npl->ch->pcdata->last_saved;
			oldest		= npl->ch;
		}
	}

	if (oldest)
	{
		save_char_obj(oldest, NORMAL_SAVE);
	}

	oldest_clan_time	= mud->current_time;
	oldest_clan		= NULL;

	for (clan = mud->f_clan ; clan ; clan = clan->next)
	{
		if (clan->last_saved < oldest_clan_time)
		{
			oldest_clan_time	= clan->last_saved;
			oldest_clan		= clan;
		}
	}

	if (oldest_clan)
	{
		save_clan(oldest_clan);
		oldest_clan->last_saved = mud->current_time;
	}

	pop_call();
	return;
}

void auto_area_save (void)
{
	push_call("auto_area_save(void)");

	do_savearea(NULL, "forreal");

	pop_call();
	return;
}


void shop_update (void)
{
	OBJ_DATA *obj;
	CHAR_DATA *ich;
	AREA_DATA *area;

	int vnum;

	push_call("shop_update()");

	for (area = mud->f_area ; area ; area = area->next)
	{
		for (vnum = area->low_m_vnum ; vnum <= area->hi_m_vnum ; vnum++)
		{
			if (mob_index[vnum] == NULL || mob_index[vnum]->pShop == NULL)
			{
				continue;
			}

			for (ich = mob_index[vnum]->first_instance ; ich ; ich = ich->next_instance)
			{
				for (obj = ich->last_carrying ; obj ; obj = obj->prev_content)
				{
					if (obj->reset == NULL)
					{
						if (number_bits(3) == 0)
						{
							act ("$n discards $p.", ich, obj, NULL, TO_ROOM);
							junk_obj(obj);
						}
					}
					else
					{
						break;
					}
				}
			}
		}
	}
	pop_call();
	return;
}


void mobile_update (void)
{
	CHAR_DATA *ich;
	CHAR_DATA *rch;
	AREA_DATA *area;
	PLAYER_GAME *gch;
	EXIT_DATA *pexit;
	OBJ_DATA *obj, *obj_best;
	int vnum, door, max;

	push_call("mobile_update()");

	for (area = mud->f_area ; area ; area = area->next)
	{
		for (vnum = area->low_m_vnum ; vnum <= area->hi_m_vnum ; vnum++)
		{
			if (mob_index[vnum] == NULL || mob_index[vnum]->first_instance == NULL)
			{
				continue;
			}

			if (mob_index[vnum]->area->nplayer == 0 && number_bits(6))
			{
				continue;
			}

			if (!IS_SET(mob_index[vnum]->act, ACT_PET))
			{
				for (ich = mob_index[vnum]->first_instance ; ich ; ich = mud->update_ich)
				{
					mud->update_ich = ich->next_instance;

					if (!MP_VALID_MOB(ich))
					{
						continue;
					}

					if (HAS_BIT(ich->act, ACT_SENTINEL))
					{
						if (*ich->npcdata->hate_fear == 0)
						{
							continue;
						}

						if (ich->reset == NULL || ich->reset->arg3 == ich->in_room->vnum)
						{
							continue;
						}
					}

					if (ich->position != POS_STANDING)
					{
						continue;
					}

					if (*ich->npcdata->hate_fear == 0 && number_bits(4) != 0)
					{
						continue;
					}

					door = number_door();

					if ((pexit = ich->in_room->exit[door]) == NULL)
					{
						continue;
					}

					if (room_index[pexit->to_room] == NULL)
					{
						continue;
					}

					if (IS_SET(room_index[pexit->to_room]->room_flags, ROOM_NO_MOB))
					{
						continue;
					}
					if (IS_SET(ich->act, ACT_STAY_SECTOR) && ich->reset && room_index[pexit->to_room]->sector_type != room_index[ich->reset->arg3]->sector_type)
					{
						continue;
					}
					if (room_index[pexit->to_room]->area == ich->in_room->area)
					{
						move_char(ich, door, TRUE);
					}
				}
			}

			if (IS_SET(mob_index[vnum]->act, ACT_SCAVENGER))
			{
				for (ich = mob_index[vnum]->first_instance ; ich ; ich = mud->update_ich)
				{
					mud->update_ich = ich->next_instance;

					if (!MP_VALID_MOB(ich))
					{
						continue;
					}

					if (ich->position != POS_STANDING)
					{
						continue;
					}

					if (ich->in_room->first_content == NULL)
					{
						continue;
					}

					if (ich->carry_number >= 10)
					{
						continue;
					}

					if (number_bits(4))
					{
						continue;
					}

					max = 1;
					obj_best = NULL;

					for (obj = ich->in_room->first_content ; obj ; obj = obj->next_content)
					{
						if (IS_SET(obj->wear_flags, ITEM_TAKE) && obj->cost > max)
						{
							obj_best = obj;
							max = obj->cost;
						}
					}
					if (obj_best)
					{
						char objName[MAX_INPUT_LENGTH];
						sprintf (objName, "i%u", obj_best->pIndexData->vnum);
						do_get (ich, objName);
						do_wear(ich, objName);
					}
				}
			}

			if (mob_index[vnum]->area->nplayer == 0)
			{
				continue;
			}

			for (ich = mob_index[vnum]->first_instance ; ich ; ich = mud->update_ich)
			{
				mud->update_ich = ich->next_instance;

				if (*ich->npcdata->hate_fear == '\0')
				{
					continue;
				}

				if (!MP_VALID_MOB(ich))
				{
					continue;
				}

				if (ich->position < POS_STANDING)
				{
					continue;
				}

				if (!IS_SET(ich->act, ACT_WIMPY))
				{
					if (ich->fighting == NULL)
					{
						for (rch = ich->in_room->first_person ; rch ; rch = rch->next_in_room)
						{
							if (!strcmp(ich->npcdata->hate_fear, rch->name))
							{
								found_hating(ich, rch);
								break;
							}
						}
					}
					continue;
				}
				else
				{
					if (ich->hit > ich->max_hit / 2)
					{
						continue;
					}
					if (ich->fighting)
					{
						do_flee(ich, "");
						continue;
					}
					door = number_door();

					if ((pexit = get_exit(ich->in_room->vnum, door)) == NULL)
					{
						continue;
					}

					if (IS_SET(pexit->flags, EX_CLOSED))
					{
						continue;
					}

					if (IS_SET(room_index[pexit->to_room]->room_flags, ROOM_NO_MOB))
					{
						continue;
					}

					if (IS_SET(ich->act, ACT_STAY_SECTOR) && ich->reset && room_index[pexit->to_room]->sector_type != room_index[ich->reset->arg3]->sector_type)
					{
						continue;
					}
					for (rch = ich->in_room->first_person ; rch ; rch = rch->next_in_room)
					{
						if (ich->npcdata->hate_fear == rch->name && IS_SET(ich->act, ACT_SMART))
						{
							char buf[MAX_INPUT_LENGTH];

							if (IS_SET(ich->act, ACT_SMART))
							{
								switch (number_bits (2))
								{
									case 0:
										sprintf (buf, "Argh! %s is trying to kill me!", capitalize(get_name(rch)));
										break;
									case 1:
										sprintf (buf, "Help! I'm being attacked by %s!", get_name(rch));
										break;
									case 2:
										sprintf (buf, "Stay away from me %s! Help!", short_to_name(get_name(rch), 1));
										break;
									case 3:
										sprintf (buf, "Someone help me! %s is attacking me!", get_name(rch));
										break;
								}
								do_shout (ich, buf);
							}
							else
							{
								switch (number_bits(2))
								{
									case 0:
										act("$n looks around frantically as it searches for an escape!", ich, NULL, NULL, TO_ROOM);
										break;
									case 1:
										act("$n darts around mindlessly in an attempt to get away!", ich, NULL, NULL, TO_ROOM);
										break;
									case 2:
										act("$n lets out a whimper and tries to flee!", ich, NULL, NULL, TO_ROOM);
										break;
									case 3:
										act("$n howls in fear as it attempts to flee!", ich, NULL, NULL, TO_ROOM);
										break;
								}
							}
							break;
						}
					}
					if (rch)
					{
						move_char(ich, door, TRUE);
					}
				}
			}
		}
	}

	for (gch = mud->f_player ; gch ; gch = gch->next)
	{
		switch (gch->ch->in_room->sector_type)
		{
			case SECT_ASTRAL:
				if (!CAN_ASTRAL_WALK(gch->ch))
				{
					if (gch->ch->position == POS_FIGHTING)
					{
						stop_fighting (gch->ch, FALSE);
					}
					char_from_room (gch->ch);
					if (room_index[gch->ch->pcdata->last_real_room]->sector_type == SECT_ASTRAL)
					{
						char_to_room (gch->ch, ROOM_VNUM_TEMPLE);
					}
					else
					{
						char_to_room (gch->ch, gch->ch->pcdata->last_real_room);
					}
				}
				break;

			case SECT_UNDER_WATER:
				if (!CAN_BREATH_WATER(gch->ch))
				{
					if (!vnum_in_group (gch->ch, MOB_VNUM_WATER_ELEMENTAL))
					{
						if (gch->ch->pcdata->condition[COND_AIR] <= 0)
						{
							send_to_char ("You cannot breath!\n\r", gch->ch);
							damage (gch->ch, gch->ch, 50, TYPE_NOFIGHT);
						}
						else
						{
							gch->ch->pcdata->condition[COND_AIR]--;
							if (gch->ch->pcdata->condition[COND_AIR] == 5)
							{
								send_to_char("You cannot hold in your breath much longer.\n\r", gch->ch);
							}
						}
					}
				}
				break;

			case SECT_LAVA:
				if (!CAN_FIREWALK(gch->ch))
				{
					if (!vnum_in_group (gch->ch, MOB_VNUM_FIRE_ELEMENTAL))
					{
						send_to_char ("That lava is REALLY hot!\n\r", gch->ch);

						if (CAN_FLY(gch->ch))
						{
							damage(gch->ch, gch->ch, 15, TYPE_NOFIGHT);
						}
						else
						{
							damage(gch->ch, gch->ch, 20, TYPE_NOFIGHT);
						}
					}
				}
				break;

			case SECT_OCEAN:
				if (!CAN_FLY(gch->ch))
				{
					if (!CAN_BREATH_WATER(gch->ch))
					{
						if (!CAN_SWIM(gch->ch))
						{
							OBJ_DATA *obj;
							for (obj = gch->ch->first_carrying ; obj ; obj = obj->next_content)
							{
								if (obj->item_type == ITEM_BOAT)
								{
									break;
								}
							}
							if (obj == NULL)
							{
								send_to_char ("You are sinking FAST!\n\r", gch->ch);
								damage (gch->ch, gch->ch, 10, TYPE_NOFIGHT);
								continue;
							}
						}
					}
				}
				break;

			default:
				if (gch->ch->pcdata->condition[COND_AIR] < 10)
				{
					send_to_char("You take a deep breath of fresh air.\n\r", gch->ch);
					gch->ch->pcdata->condition[COND_AIR] = 10;
				}
				break;
		}
	}

	for (gch = mud->f_player ; gch ; gch = gch->next)
	{
		if (gch->ch->timer > 20)
		{
			if (is_desc_valid(gch->ch) && gch->ch->timer < 40)
			{
				continue;
			}

			if (gch->ch->pcdata->switched || IS_IMMORTAL(gch->ch))
			{
				continue;
			}

			if (gch->ch->in_room->vnum == ROOM_VNUM_LIMBO)
			{
				char_from_room(gch->ch);
				char_to_room(gch->ch, gch->ch->pcdata->was_in_room);
			}
			do_quit(gch->ch, NULL);
			break;
		}
	}


	pop_call();
	return;
}


/*
	Update the weather - Scandum 22-06-2003
*/

void weather_area_update( AREA_DATA *area )
{
	char buf[MAX_STRING_LENGTH];

	PLAYER_GAME *gch;

	int temp, season, daily;

	buf[0] = '\0';

	/*
		Calculate temperature in degrees Celcius
	*/

	season = area->weather_info->temp_summer - area->weather_info->temp_winter;
	daily  = area->weather_info->temp_daily;

	temp   = area->weather_info->temp_winter;

	temp  += season * ( 8 - abs(mud->time_info->month -  8)) /  8;
	temp  += daily  * (12 - abs(mud->time_info->hour  - 12)) / 12;

	/*
		Calculate the wind speed based on current wind speed 0 - 10
	*/

	if (area->weather_info->wind_speed > area->weather_info->wind_scale + 1)
	{
		area->weather_info->wind_speed += number_range(0, 3) - 2;
	}
	else if (area->weather_info->wind_speed < area->weather_info->wind_scale - 1)
	{
		area->weather_info->wind_speed += number_range(0, 3) - 1;
	}
	else
	{
		area->weather_info->wind_speed += number_range(0, 2) - 1;
	}

	area->weather_info->wind_speed = URANGE(-10, area->weather_info->wind_speed, 20);

	/*
		Calculate the wind direction, based on current wind direction
	*/

	area->weather_info->wind_dir   = abs(area->weather_info->wind_dir + number_range(0, 2) - 1) % 8;

	/*
		Calculate the weather, based on current weather
	*/

	if (area->weather_info->change < area->weather_info->wet_scale - 1)
	{
		area->weather_info->change += number_range(0, 3) - 1;
	}
	else if (area->weather_info->change > area->weather_info->wet_scale + 1)
	{
		area->weather_info->change += number_range(0, 3) - 2;
	}
	else
	{
		area->weather_info->change += number_range(0, 4) - 2;
	}

	area->weather_info->change = URANGE(-100, area->weather_info->change, 110);

	/*
		Modify temperature based on weather
	*/

	temp += 5 - URANGE(0, area->weather_info->change, 10);

	area->weather_info->temperature = temp;

	switch (area->weather_info->sky)
	{
		default:
			bug ("Weather_update: bad sky %d.", area->weather_info->sky);
			area->weather_info->sky = SKY_CLOUDLESS;
			break;

		case SKY_CLOUDLESS:
			if (area->weather_info->change > 3)
			{
				switch (number_bits(2))
				{
					case 0:
						cat_sprintf(buf, "Threatening clouds gather on the %s horizon, blocking the sky from view.\n\r", wind_dir_name[area->weather_info->wind_dir]);
						break;
					case 1:
						cat_sprintf(buf, "Low clouds form on the %s horizon and slowly drift towards you.\n\r", wind_dir_name[area->weather_info->wind_dir]);
						break;
					case 2:
						cat_sprintf(buf, "Clouds sweep quickly across the %s horizon, darkening the skies above.\n\r", wind_dir_name[area->weather_info->wind_dir]);
						break;
					case 3:
						cat_sprintf(buf, "Bright white clouds drift lazily towards you from the %s horizon.\n\r", wind_dir_name[area->weather_info->wind_dir]);
						break;
				}
				area->weather_info->sky = SKY_CLOUDY;
			}
			break;

		case SKY_CLOUDY:
			if (area->weather_info->change > 6)
			{
				if (area->weather_info->temperature < 0)
				{
					switch (number_bits(2))
					{
						case 0:
							strcat(buf, "Snowflakes flutter down from the leaden skies above.\n\r");
							break;
						case 1:
							strcat(buf, "Lacy flakes of snow silently fall to the ground from the clouds above.\n\r");
							break;
						case 2:
							strcat(buf, "The clouds above give way to softly falling snow.\n\r");
							break;
						case 3:
							strcat(buf, "Snowflakes whisper down from the skies in a soft white dance.\n\r");
							break;
					}
				}
				else if (area->weather_info->temperature < 4)
				{
					switch (number_bits(2))
					{
						case 0:
							strcat(buf, "Snow begins to meld with icy rain as sleet falls to the ground.\n\r");
							break;
						case 1:
							strcat(buf, "Sharp sleet stings as it falls from the clouds above.\n\r");
							break;
						case 2:
							strcat(buf, "Icy sleet rains down from the heavens.\n\r");
							break;
						case 3:
							strcat(buf, "Sheets of stinging sleet fall from the dark clouds.\n\r");
							break;
					}
				}
				else
				{
					switch (number_bits(2))
					{
						case 0:
							strcat(buf, "Dark clouds break open, pouring rain down upon the lands.\n\r");
							break;
						case 1:
							strcat(buf, "Silvery drops of rain fall from angry looking clouds above.\n\r");
							break;
						case 2:
							strcat(buf, "Soft ribbons of rain slip unheeded from the dark skies overhead.\n\r");
							break;
						case 3:
							strcat(buf, "Fat raindrops merrily pepper the lands from the clouds looming above.\n\r");
							break;
					}
				}
				area->weather_info->change += 2;
				area->weather_info->sky     = SKY_RAINING;
			}
			else if (area->weather_info->change < 4)
			{
				switch (number_bits(2))
				{
					case 0:
						strcat(buf, "The clouds recede, revealing a crystal clear sky.\n\r");
						break;
					case 1:
						strcat(buf, "Dark clouds dip below the horizon as the skies clear.\n\r");
						break;
					case 2:
						strcat(buf, "The clouds swiftly move across the skies as it grows clear.\n\r");
						break;
					case 3:
						strcat(buf, "Clouds sigh softly as they disperse, leaving the skies clear.\n\r");
						break;
				}
				area->weather_info->change -= 2;
				area->weather_info->sky     = SKY_CLOUDLESS;
			}
			break;

		case SKY_RAINING:
			if (area->weather_info->change < 7)
			{
				if (area->weather_info->temperature < 0)
				{
					switch (number_bits(2))
					{
						case 0:
							strcat(buf, "A few last snowflakes swirl over the ground as the snow stops.\n\r");
							break;
						case 1:
							strcat(buf, "The world is left in silence as the snows stop falling.\n\r");
							break;
						case 2:
							strcat(buf, "The lands are blanketed in white as the last snowflakes fall.\n\r");
							break;
						case 3:
							strcat(buf, "With a final whisper, the snow stops falling.\n\r");
							break;
					}
				}
				else if (area->weather_info->temperature < 4)
				{
					switch (number_bits(2))
					{
						case 0:
							strcat(buf, "The final slivers of sleet fall from the dark clouds as it stops.\n\r");
							break;
						case 1:
							strcat(buf, "The miserable sleet falling from the clouds above stops.\n\r");
							break;
						case 2:
							strcat(buf, "The slippery razor-sharp sleet ends its assault on the lands.\n\r");
							break;
						case 3:
							strcat(buf, "The stinging sleet suddenly ceases as quickly as it began.\n\r");
							break;
					}
				}
				else
				{
					switch (number_bits(2))
					{
						case 0:
							strcat(buf, "The air smells fresh as the rain stops falling.\n\r");
							break;
						case 1:
							strcat(buf, "The clouds above dry their tears as the rain ceases.\n\r");
							break;
						case 2:
							strcat(buf, "The greyness above seems to ease as the rains halt.\n\r");
							break;
						case 3:
							strcat(buf, "A sweet scent fills the air as the rain suddenly stops.\n\r");
							break;
					}
				}
				area->weather_info->change -= 1;
				area->weather_info->sky     = SKY_CLOUDY;
			}
			else if (area->weather_info->wind_speed > 6)
			{
				if (area->weather_info->temperature < 0)
				{
					switch (number_bits(2))
					{
						case 0:
							strcat(buf, "A ferocious blizzard sweeps through the lands, burying them in white.\n\r");
							break;
						case 1:
							strcat(buf, "Heavy snows swirl in the air as the blizzard rages.\n\r");
							break;
						case 2:
							strcat(buf, "The snows fall heavily upon the ground as a blizzard takes hold.\n\r");
							break;
						case 3:
							strcat(buf, "Snowdrifts form quickly as the blizzard increases its fierceness.\n\r");
							break;
					}
				}
				else if (area->weather_info->temperature < 4)
				{
					switch (number_bits(2))
					{
						case 0:
							strcat(buf, "The clouds darken and thicken as they fill with hail.\n\r");
							break;
						case 1:
							strcat(buf, "Huge chunks of hail fall to the grounds with a sizzle.\n\r");
							break;
						case 2:
							strcat(buf, "Hail bombards the ground as creatures run for cover.\n\r");
							break;
						case 3:
							strcat(buf, "A shower of hail falls from the sky to pelts the lands below.\n\r");
							break;
					}
				}
				else
				{
					switch (number_bits(2))
					{
						case 0:
							strcat(buf, "The lands glow blue-white as lightning cracks across the sky.\n\r");
							break;
						case 1:
							strcat(buf, "Electricity fills the air as forks of lightning dance over the sky.\n\r");
							break;
						case 2:
							strcat(buf, "The horizon shines brightly as sheet lightning flashes in a quick stutter.\n\r");
							break;
						case 3:
							strcat(buf, "The winds gather speed as lightning paints the skies with broad strokes.\n\r");
							break;
					}
				}
				area->weather_info->wind_speed += 2;
				area->weather_info->sky         = SKY_LIGHTNING;
			}
			break;

		case SKY_LIGHTNING:
			if (area->weather_info->change < 7 || area->weather_info->wind_speed < 7)
			{
				if (area->weather_info->temperature < 0)
				{
					switch (number_bits(2))
					{
						case 0:
							strcat(buf, "The thick, swirling snow peters out as the blizzard ends.\n\r");
							break;
						case 1:
							strcat(buf, "The ferocity of the blizzard gives way as the snows diminish.\n\r");
							break;
						case 2:
							strcat(buf, "With a last gasp, the blizzard dwindles away to nothing.\n\r");
							break;
						case 3:
							strcat(buf, "With a final stroke of white, the blizzard dies out.\n\r");
							break;
					}
				}
				else if (area->weather_info->temperature < 4)
				{
					switch (number_bits(2))
					{
						case 0:
							strcat(buf, "The cannonade of hail comes to a sudden halt.\n\r");
							break;
						case 1:
							strcat(buf, "The burning hail ends its furious salvo of the lands.\n\r");
							break;
						case 2:
							strcat(buf, "The icy hot hail diminishes, retreating to the clouds above.\n\r");
							break;
						case 3:
							strcat(buf, "As swiftly as it started, the hailstorm recedes.\n\r");
							break;
					}
				}
				else
				{
					switch (number_bits(2))
					{
						case 0:
							strcat(buf, "With a few last flashes, the lightning moves down the horizon.\n\r");
							break;
						case 1:
							strcat(buf, "The skies turn ice blue and brilliant white with a final flash of lightning.\n\r");
							break;
						case 2:
							strcat(buf, "The heavens calm as the lightning storm ends.\n\r");
							break;
						case 3:
							strcat(buf, "The air is alive with electricity as the last fork of lightning appears.\n\r");
							break;
					}
				}
				area->weather_info->wind_speed -= 2;
				area->weather_info->sky         = SKY_RAINING;
			}
			break;
	}

	if (area->nplayer <= 0 || buf[0] == '\0')
	{
		return;
	}

	for (gch = mud->f_player ; gch ; gch = gch->next)
	{
		if (gch->ch->in_room->area != area)
		{
			continue;
		}
		if (!IS_OUTSIDE(gch->ch) || NO_WEATHER_SECT(gch->ch->in_room->sector_type))
		{
			continue;
		}
		if (!IS_AWAKE(gch->ch))
		{
			continue;
		}
		send_to_char(justify(buf, get_page_width(gch->ch)), gch->ch);
	}
}

void weather_update (void)
{
	AREA_DATA *area;

	push_call("weather_update()");

	for (area = mud->f_area ; area ; area = area->next)
	{
		weather_area_update(area);
	}

	pop_call();
	return;
}

void strip_greater (char *str)
{
	char *pt;

	push_call("strip_greater(%p)",str);

	for (pt = str; *pt != '\0'; pt++)
	{
		if (*pt == '<')
		{
			*pt = '(';
		}
		else if (*pt == '>')
		{
			*pt = ')';
		}
	}
	pop_call();
	return;
}


/* Make a html web page - Chaos  3/28/96 */

void save_html_who (void)
{
	FILE *fp;
	char buf[MAX_STRING_LENGTH], buf_race[20];
	char buf2[MAX_STRING_LENGTH];
	int leng;
//	CHAR_DATA *fch;
//	DESCRIPTOR_DATA *d;
	int nMatch;
	int nTotal;
	CHAR_DATA *wch;
	char const *class;
	char god;
	char killer_thief;
	PLAYER_GAME *fpl;
	char *pt;

	push_call("save_html_who()");

	/*
		Disabled since we're not using this atm - Scandum
	*/

	pop_call();
	return;

	close_reserve();

	fp = my_fopen ("../public_html/who.html", "w",TRUE);

	if (fp == NULL)
	{
		open_reserve();
		pop_call();
		return;
	}

	fprintf (fp, "<!DOCTYPE html PUBLIC \"-//IETF//DTD// HTML 2.0//EN\">\n");
	fprintf (fp, "<BODY BACKGROUND=\"bumps1.jpg\" text=#ffff30 alink=#80FF30 vlink=#90FF30 link=#FFFF30 >\n");

	fprintf (fp, "<HTML><HEAD><TITLE>E-Mud Who List</TITLE></HEAD>\n");
	fprintf (fp, "<BODY><FONT SIZE=+2><CENTER>\n");
	fprintf (fp, "E-Mud WHO Page<p>\n");

	/* Set default arguments. */

//	fch = NULL;

	/* Now show matching chars. */

	nMatch = 0;
	nTotal = 0;
	buf[0] = '\0';
	leng = 0;
	for (fpl = mud->f_player ; fpl ; fpl = fpl->next)
	{
		wch = fpl->ch;
/*		d = NULL;
		if (is_desc_valid (wch))
			d = wch->desc;*/
		/* Check for match against restrictions.
		 * Don't use trust as that exposes trusted mortals.
		 * Chaos set to see all chars, invis or not.
		 */

		if (IS_SET (wch->act, PLR_WIZINVIS)|| IS_SET(wch->act,PLR_WIZCLOAK))
		{
			continue;
		}
		nTotal++;
		nMatch++;

	      /*
     	  * Figure out what to print for class.
	       */

		class = class_table[wch->class].who_name;
		switch (wch->level)
		{
			default:
				break;
			case MAX_LEVEL - 0:
				class = "GOD";
				break;
			case MAX_LEVEL - 1:
				class = "ARC";
				break;
			case MAX_LEVEL - 2:
				class = "ANG"; break;
				break;
		}

		strcpy (buf_race, race_table[wch->race].race_name);
		buf_race[3] = '\0';


		if (wch->level > MAX_LEVEL - 4)
			strcpy (buf_race, "---");

		god = god_table[wch->pcdata->god].who_letter[0];

		if (IS_SET (wch->act, PLR_KILLER))
		{
			killer_thief = 'K';
		}
		else if (IS_SET (wch->act, PLR_THIEF))
		{
			killer_thief = 'T';
		}
		else
		{
			killer_thief = ' ';
		}
		/*
		* Format it up.
		*/

		sprintf (buf2, "[%2d %s %s]%c%c%s%s",
			wch->level, class, buf_race, god, killer_thief, wch->name,
			IS_NPC (wch) ? "the monster" : wch->pcdata->title);
		buf2[71] = '\0';
		strip_greater (buf2);
		while (strlen (buf2) < 71)
		{
			str_cat_max (buf2, " ", MAX_STRING_LENGTH);\
		}
		leng = str_apd_max (buf, buf2, leng, MAX_STRING_LENGTH);

		if ((wch->pcdata->switched || wch->desc != NULL)
		&&  !IS_AFFECTED (wch, AFF_STEALTH))
		{
			strcpy (buf2, wch->in_room->area->name);
			buf2[8] = '\0';
			while (strlen (buf2) < 8)
			{
				str_cat_max (buf2, " ", MAX_STRING_LENGTH);
			}
		}
		else if (wch->desc == NULL)
		{
			strcpy (buf2, "LinkLost");
		}
		else
		{
			strcpy (buf2, "Unknown ");	/* Stealth Mode */
		}

		leng = str_apd_max (buf, " {", leng, MAX_STRING_LENGTH);
		leng = str_apd_max (buf, buf2, leng, MAX_STRING_LENGTH);
		leng = str_apd_max (buf, "} ", leng, MAX_STRING_LENGTH);

		/*  They don't need to see this either -  Chaos  4/30/99

		if (wch->desc != NULL && wch->desc->host != NULL)
		{
			leng = str_apd_max (buf, " (", leng, MAX_STRING_LENGTH);
			leng = str_apd_max (buf, wch->desc->host, leng, MAX_STRING_LENGTH);
			leng = str_apd_max (buf, ")", leng, MAX_STRING_LENGTH);
		}  */

		/*  Let's be a bit discreet here.   -  Chaos   4/25/99
		if (wch->pcdata->mail_address != NULL &&
			*wch->pcdata->mail_address != '\0')
		{
			leng = str_apd_max (buf, " Email: <a href=\"mailto:", leng, MAX_STRING_LENGTH);
			leng = str_apd_max (buf, wch->pcdata->mail_address, leng, MAX_STRING_LENGTH);
			leng = str_apd_max (buf, "\">", leng, MAX_STRING_LENGTH);
			leng = str_apd_max (buf, wch->pcdata->mail_address, leng, MAX_STRING_LENGTH);
			leng = str_apd_max (buf, "</a>", leng, MAX_STRING_LENGTH);
		}   */

		if (wch->pcdata->html_address != NULL && *wch->pcdata->html_address != '\0')
		{
			strcpy (buf2, wch->pcdata->html_address);
			for (pt = buf2; *pt != '\0'; pt++)
			{
				if (*pt == '*')
				{
					*pt = '~';
				}
			}
			leng = str_apd_max (buf, " Home Page: <a href=\"http://", leng, MAX_STRING_LENGTH);
			leng = str_apd_max (buf, buf2, leng, MAX_STRING_LENGTH);
			leng = str_apd_max (buf, "\">", leng, MAX_STRING_LENGTH);
			leng = str_apd_max (buf, buf2, leng, MAX_STRING_LENGTH);
			leng = str_apd_max (buf, "</a>", leng, MAX_STRING_LENGTH);
		}

		leng = str_apd_max (buf, "<p>\n", leng, MAX_STRING_LENGTH);
	}

	sprintf (buf2, "Players: %d</CENTER></font><p><font size=-1>\n", nTotal);


	fprintf (fp, "%s", buf2);
	fprintf (fp, "%s", buf);

	fprintf (fp, "</font></BODY></HTML>\n");

	if(fp) /* this prevents the possibility to my_fclose(NULL) - Manwe, 15-10-2000 */
	{
		my_fclose (fp);
	}

	open_reserve();

	pop_call();
	return;
}


void time_update (void)
{
	char buf[MAX_INPUT_LENGTH];
	AREA_DATA 		*area;
	PLAYER_GAME		*gpl;

	push_call("time_update()");

	mud->time_info->hour++;

	if (mud->time_info->hour >= 24)
	{
		mud->time_info->hour = 0;
		mud->time_info->day++;
	}

	if (mud->time_info->day >= 35)
	{
		mud->time_info->day = 0;
		mud->time_info->month++;
	}

	if (mud->time_info->month >= 16)
	{
		mud->time_info->month = 0;
		mud->time_info->year++;
	}

	switch (mud->time_info->hour)
	{
		case 5:
			mud->sunlight = SUN_RISE;
			break;
		case 6:
			mud->sunlight = SUN_LIGHT;
			break;

		case 19:
			mud->sunlight = SUN_SET;
			break;

		case 20:
			mud->sunlight = SUN_DARK;
			break;
	}

	for (area = mud->f_area ; area ; area = area->next)
	{


		if (area->nplayer == 0)
		{
			continue;
		}

		buf[0] = '\0';

		switch (mud->time_info->hour)
		{
			case 5:
				sprintf(buf, "The day has begun.\n\r");
				break;

			case 6:
				sprintf(buf, "The sun rises in the east.\n\r");
				break;

			case 19:
				sprintf(buf, "The sun slowly disappears in the west.\n\r");
				break;

			case 20:
				sprintf(buf, "The night has begun.\n\r");
				break;
		}

		if (buf[0] == '\0')
		{
			continue;
		}

		for (gpl = mud->f_player ; gpl ; gpl = gpl->next)
		{
			if (gpl->ch->in_room->area == area
			&&  IS_OUTSIDE(gpl->ch)
			&& !NO_WEATHER_SECT(gpl->ch->in_room->sector_type)
			&& IS_AWAKE(gpl->ch))
			{
				send_to_char(buf, gpl->ch);
			}
		}
	}
	pop_call();
	return;
}


void char_update (void)
{
	CHAR_DATA *ch;
	ROOM_TIMER_DATA *rtd, *rtd_next;
	AFFECT_DATA *paf, *paf_next;

	push_call("char_update()");

	for (rtd = mud->f_room_timer ; rtd ; rtd = rtd_next)
	{
		rtd_next = rtd->next;

		if (--rtd->timer <= 0)
		{
			switch (rtd->type)
			{
				case ROOM_TIMER_SANCTIFY:
					REMOVE_BIT(room_index[rtd->vnum]->room_flags, ROOM_SAFE);
					room_index[rtd->vnum]->sanctify_char = NULL;
					send_to_room("The area does not look safe now..\n\r", room_index[rtd->vnum]);
					break;

				case ROOM_TIMER_SMOKE:
					REMOVE_BIT(room_index[rtd->vnum]->room_flags, ROOM_SMOKE);
					send_to_room("The smoke dissipates.\n\r", room_index[rtd->vnum]);
					break;

				case ROOM_TIMER_GLOBE:
					REMOVE_BIT(room_index[rtd->vnum]->room_flags, ROOM_GLOBE);
					send_to_room("The shroud of darkness dissipates.\n\r", room_index[rtd->vnum]);
					break;

				case ROOM_TIMER_BLOCK:
					REMOVE_BIT(room_index[rtd->vnum]->room_flags, ROOM_BLOCK);
					send_to_room("The magical plant life withers and dies.\n\r", room_index[rtd->vnum]);
					break;

				case ROOM_TIMER_ICE:
					REMOVE_BIT(room_index[rtd->vnum]->room_flags, ROOM_ICE);
					send_to_room("The air becomes warmer as the ice sheets melt away.\n\r", room_index[rtd->vnum]);
					break;

				case ROOM_TIMER_UNBLOCK:
					SET_BIT(room_index[rtd->vnum]->room_flags, ROOM_BLOCK);
					send_to_room("All sorts of plant growth sprout up everywhere!\n\r", room_index[rtd->vnum]);
					break;
			}
			del_room_timer(rtd->vnum, rtd->type);
		}
	}

	for (ch = mud->f_char ; ch ; ch = mud->update_wch)
	{
		mud->update_wch = ch->next;

		if (!IS_NPC(ch))
		{
			if (ch->gold > 1000000 * ch->level)
			{
				ch->gold = 1000000 * ch->level;
			}

			if (ch->gold < 0)
			{
				ch->gold = 0;
			}

			if (ch->level < LEVEL_IMMORTAL)
			{
				if (ch->level < ch->in_room->area->low_hard_range || ch->level > ch->in_room->area->hi_hard_range)
				{
					char_to_room(ch, ROOM_VNUM_TEMPLE);
					ch->pcdata->death_room = ch->pcdata->recall = ROOM_VNUM_TEMPLE;
				}
			}

			if (IS_SET(ch->act, PLR_KILLER) && (ch->pcdata->played - ch->pcdata->killer_played) > 60 * 60 * 24)
			{
				REMOVE_BIT (ch->act, PLR_KILLER);
			}

			if (IS_SET(ch->act, PLR_OUTCAST) && (ch->pcdata->played - ch->pcdata->outcast_played) > 60 * 60 * 24)
			{
				REMOVE_BIT (ch->act, PLR_OUTCAST);
			}

			if (ch->pcdata->just_died_ctr > 0)
			{
				if (--ch->pcdata->just_died_ctr == 0)
				{
					send_to_char ("The Gods are no longer protecting you.\n\r", ch); break;
				}
			}

			ch->timer++;

			if (ch->timer > 10)
			{
				if (is_desc_valid(ch) && ch->timer < 20)
				{
					continue;
				}

				if (ch->in_room && ch->in_room->vnum != ROOM_VNUM_LIMBO)
				{
					if (!ch->pcdata->switched && !IS_IMMORTAL(ch))
					{
						ch->pcdata->was_in_room = ch->in_room->vnum;
						if (ch->fighting)
						{
							stop_fighting(ch, FALSE);
						}
						act("$n disappears into the void.", ch, NULL, NULL, TO_ROOM);
						act("You disappear into the void.", ch, NULL, NULL, TO_CHAR);
						save_char_obj(ch, NORMAL_SAVE);
						char_from_room(ch);
						char_to_room(ch, ROOM_VNUM_LIMBO);
					}
				}
			}

			ch->pcdata->idle++;

			gain_condition (ch, COND_DRUNK, -1);

			if (ch->level > 5 && ch->level < 95 && (!IS_UNDEAD(ch) || rspec_req(ch, RSPEC_VAMPIRIC)))
			{
				gain_condition (ch, COND_FULL,   -1);
				gain_condition (ch, COND_THIRST, -1);
			}
		}

		if (ch->position >= POS_STUNNED)
		{
			if (ch->hit < ch->max_hit)
			{
				ch->hit += hit_gain(ch, VERBOSE);
			}

			if (ch->mana < ch->max_mana)
			{
				ch->mana += mana_gain(ch, VERBOSE);
			}

			if (ch->move < ch->max_move)
			{
				ch->move += move_gain(ch, VERBOSE);
			}
		}

		if (IS_NPC(ch))
		{
			if (ch->npcdata->pvnum_last_hit != 0)
			{
				if (ch->hit == ch->max_hit)
				{
					ch->npcdata->pvnum_last_hit = 0;

					RESTRING(ch->npcdata->hate_fear, "");
				}
			}

			if (ch->fighting == NULL && ch->position != POS_FIGHTING)
			{
				if (IS_SET(ch->pIndexData->affected_by, AFF_INVISIBLE) && !IS_SET(ch->affected_by, AFF_INVISIBLE))
				{
					SET_BIT(ch->affected_by, AFF_INVISIBLE );
				}
				if (IS_SET(ch->pIndexData->affected_by, AFF_HIDE) && !IS_SET(ch->affected_by, AFF_HIDE))
				{
					SET_BIT(ch->affected_by, AFF_HIDE);
				}
				if (IS_SET(ch->pIndexData->affected_by,  AFF_STEALTH) && !IS_SET(ch->affected_by, AFF_STEALTH))
				{
					SET_BIT(ch->affected_by, AFF_STEALTH);
				}
			}
		}

		update_pos(ch);

		if (ch->position == POS_DEAD)
		{
			raw_kill(ch);
			continue;
		}

		for (paf = ch->first_affect ; paf ; paf = paf_next)
		{
			paf_next = paf->next;

			if (paf->duration >= 0 && --paf->duration <= 0)
			{
				if (paf->type > 0 && skill_table[paf->type].msg_off)
				{
					if (paf->next == NULL || paf->type != paf->next->type)
					{
						ch_printf(ch, "%s\n\r", skill_table[paf->type].msg_off);
					}
				}

				switch (paf->bitvector)
				{
					case AFF_CHARM:
						raw_kill(ch);
						continue;

					case AFF2_POSSESS:
						raw_kill(ch);
						continue;

					case AFF_SLEEP:
						if (IS_NPC(ch))
						{
							ch->position = ch->pIndexData->position;
						}
						break;
				}

				if (paf->type == gsn_polymorph)
				{
					do_polymorph(ch, NULL);
					continue;
				}

				affect_from_char(ch, paf);
			}
		}

		if (IS_AFFECTED(ch, AFF_POISON))
		{
			act ("$n shivers and suffers.", ch, NULL, NULL, TO_ROOM);
			send_to_char ("You shiver and suffer.\n\r", ch);
			damage (ch, ch, 2 + ch->level / 3, TYPE_NOFIGHT);
		}
		if (ch->position < POS_STUNNED)
		{
			damage(ch, ch, ch->position, TYPE_UNDEFINED);
		}

		if (!MP_VALID_MOB(ch))
		{
			if (ch->desc && HAS_BIT(CH(ch->desc)->pcdata->vt100_flags, VT100_INTERFACE))
			{
				vt100prompt(ch);
			}
			continue;
		}

		if (ch->timer > 0)
		{
			ch->timer--;
			if (ch->timer == 0)
			{
				mprog_delay_trigger(ch, ch->npcdata->delay_index);
			}
		}

		if (IS_SET(ch->pIndexData->progtypes, TIME_PROG))
		{
			if (!MP_VALID_MOB(ch) || ch->position < POS_RESTING || ch->position == POS_FIGHTING)
			{
				continue;
			}
			mprog_time_check(ch, NULL, NULL, NULL, TIME_PROG);
		}
	}

	{
		static int lastHour;

		mud->usage->players[mud->time.tm_hour][mud->time.tm_wday] = mud->total_plr;

		if (lastHour != mud->time.tm_hour)
		{
			save_usage();
			save_hiscores();
			bounty_update();
			save_timeinfo();

			/*
				Check for Clan Rent, Purger, Backup - Scandum 03-09-2002
			*/

			if (mud->time.tm_hour == 0)
			{
				log_printf("Backing up player files.");

				system("cd ..;./backup&");
			}

			if (mud->time.tm_wday == 0 && mud->time.tm_hour == 0)
			{
				if (IS_SET(mud->flags, MUD_CLANRENT))
				{
					if (mud->f_clan == NULL)
					{
						log_printf("There are no clans.");
					}
					else
					{
						do_forcerent(NULL, NULL);
					}
					REMOVE_BIT(mud->flags, MUD_CLANRENT);

					start_purger();
				}
			}
			else if (mud->time.tm_hour != 0)
			{
				SET_BIT(mud->flags, MUD_CLANRENT);
			}
		}
		lastHour = mud->time.tm_hour;
	}

	/*
		Minutely update on the WHO HTML Page    -   Chaos 3/26/96
		We're not using this atm - Scandum
		if (IS_SET(mud->flags, MUD_EMUD_REALGAME))
		{
			save_html_who();
		}
	*/

	pop_call();
	return;
}

/*
	Update all objs.
*/

void obj_update (void)
{
	OBJ_DATA *obj;
	CHAR_DATA *owner;
	AFFECT_DATA *paf, *paf_next;
	char *message;

	push_call("obj_update()");

	for (obj = mud->f_obj ; obj ; obj = mud->update_obj)
	{
		mud->update_obj = obj->next;

		/*
			Look for enhanced objects
		*/

		if (obj->item_type != obj->pIndexData->item_type)
		{
			obj->item_type = obj->pIndexData->item_type;

			obj->value[0] = obj->pIndexData->value[0];
			obj->value[1] = obj->pIndexData->value[1];
			obj->value[2] = obj->pIndexData->value[2];
			obj->value[3] = obj->pIndexData->value[3];

			log_printf("obj_update: bad item type: %d", obj->pIndexData->vnum);
		}

		switch (obj->item_type)
		{
			case ITEM_ARMOR:
			case ITEM_WEAPON:
				if (IS_SET(obj->extra_flags, ITEM_MODIFIED))
				{
					if (obj->value[0] != obj->pIndexData->value[0]
					||  obj->value[1] != obj->pIndexData->value[1]
					||  obj->value[2] != obj->pIndexData->value[2]
					||  obj->value[3] != obj->pIndexData->value[3])
					{
						obj->value[0] = obj->pIndexData->value[0];
						obj->value[1] = obj->pIndexData->value[1];
						obj->value[2] = obj->pIndexData->value[2];
						obj->value[3] = obj->pIndexData->value[3];
					}
				}
				break;

			case ITEM_CONTAINER:
				if (obj->carried_by == NULL && IS_SET(obj->value[1], CONT_CLOSEABLE) && !IS_SET(obj->value[1], CONT_CLOSED))
				{
					SET_BIT(obj->value[1], CONT_CLOSED);

					if (obj->value[2] > 0)
					{
						SET_BIT(obj->value[1], CONT_LOCKED);
					}
				}
				break;

			case ITEM_LIGHT:
				if (obj->value[0] != obj->pIndexData->value[0])
				{
					obj->value[0] = obj->pIndexData->value[0];
				}
				if (obj->wear_loc == WEAR_LIGHT && obj->carried_by && obj->carried_by->desc && obj->carried_by->desc->connected >= CON_PLAYING)
				{
					if (obj->value[2] > 1 && obj->value[2] < 5)
					{
						act ("$p flickers.", obj->carried_by, obj, NULL, TO_CHAR);
					}
					if (obj->value[2] > 0)
					{
						SET_BIT(obj->extra_flags, ITEM_MODIFIED);

						if (--obj->value[2] == 0)
						{
							act ("$p goes out.", obj->carried_by, obj, NULL, TO_ROOM);
							act ("$p goes out.", obj->carried_by, obj, NULL, TO_CHAR);
							junk_obj(obj);
							continue;
						}
					}
				}
				break;

			case ITEM_ARTIFACT:
				if (obj->value[0] && obj->value[0] < mud->current_time)
				{
					if (obj->carried_by)
					{
						act("$p has started to decay.", obj->carried_by, obj, NULL, TO_CHAR);
					}
					obj->value[0]	= 0;
					obj->timer	= OBJ_SAC_TIME;
				}
				break;
			case ITEM_TOTEM:
				totem_cast_spell(obj);
				break;
		}

		for (paf = obj->first_affect ; paf ; paf = paf_next)
		{
			paf_next = paf->next;

			if (paf->duration >= 0 && --paf->duration <= 0)
			{
				if (obj->carried_by)
				{
					act(skill_table[paf->type].msg_obj_off, obj->carried_by, obj, NULL, TO_CHAR);
				}
				if (obj->in_room && obj->in_room->first_person)
				{
					act(skill_table[paf->type].msg_obj_off, obj->in_room->first_person, obj, NULL, TO_CHAR);
					act(skill_table[paf->type].msg_obj_off, obj->in_room->first_person, obj, NULL, TO_ROOM);
				}
				affect_from_obj(obj, paf);
			}
		}

		if (!IS_SET(obj->extra_flags, ITEM_NOT_VALID)
		&&  !IS_SET(obj->pIndexData->extra_flags, ITEM_NOT_VALID))
		{
			if (obj->timer == 0 && obj->sac_timer == 0)
			{
				continue;
			}

			if (obj->in_room == NULL)
			{
				if (obj->timer == 0 || (obj->timer > 0 && --obj->timer > 0))
				{
					continue;
				}
			}
			if (obj->in_room != NULL)
			{
				if (IS_SET(obj->in_room->room_flags, ROOM_CLAN_DONATION))
				{
					if (obj->timer == 0 || (obj->timer > 0 && --obj->timer > 0))
					{
						continue;
					}
				}
				else
				{
					if ((obj->timer == 0     || (obj->timer > 0     && --obj->timer     > 0))
					&&  (obj->sac_timer == 0 || (obj->sac_timer > 0 && --obj->sac_timer > 0)))
					{
						continue;
					}
				}
			}
		}

		switch (obj->item_type)
		{
			default:
				message = "$p vanishes.";
				break;
			case ITEM_FOUNTAIN:
				message = "$p dries up.";
				break;
			case ITEM_CORPSE_NPC:
			case ITEM_CORPSE_PC:
				message = "$p decays into dust.";
				break;
			case ITEM_FOOD:
				message = "$p decomposes.";
				break;
			case ITEM_ARTIFACT:
				message = "$p cackles in delight as the last vestiges of its power dies away.";
				break;
		}

		if (obj->carried_by != NULL)
		{
			act (message, obj->carried_by, obj, NULL, TO_CHAR);
		}
		else if (obj->in_room != NULL && obj->in_room->first_person != NULL)
		{
			act (message, obj->in_room->first_person, obj, NULL, TO_ROOM);
			act (message, obj->in_room->first_person, obj, NULL, TO_CHAR);
		}

		while (obj->first_content)
		{
			if (obj->carried_by)
			{
				obj_to_char(obj->first_content, obj->carried_by);
			}
			else if (obj->in_room)
			{
				obj->first_content->sac_timer = OBJ_SAC_TIME;
				obj_to_room(obj->first_content, obj->in_room->vnum);
			}
			else
			{
				junk_obj(obj->first_content);
			}
		}

		if (obj->owned_by
		&& (owner = get_char_pvnum(obj->owned_by)) != NULL
		&&  owner->pcdata->corpse == obj)
		{
			send_to_char("Your corpse decays.\n\r", owner);
			owner->pcdata->corpse = find_char_corpse (owner, TRUE);
			save_char_obj(owner, NORMAL_SAVE);
		}
		junk_obj(obj);
	}
	pop_call();
	return;
}

void aggr_update (void)
{
	CHAR_DATA *ich;
	CHAR_DATA *ch;
	AREA_DATA *area;
	PLAYER_GAME *npl, *next_npl;
	int vnum;

	push_call("aggr_update()");

	for (area = mud->f_area ; area ; area = area->next)
	{
		for (vnum = area->low_m_vnum ; vnum <= area->hi_m_vnum ; vnum++)
		{
			if (mob_index[vnum] == NULL)
			{
				continue;
			}

			if (!IS_SET(mob_index[vnum]->progtypes, DELAY_PROG))
			{
				continue;
			}

			for (ich = mob_index[vnum]->first_instance ; ich ; ich = mud->update_ich)
			{
				mud->update_ich = ich->next_instance;
	
				if (ich->wait > 0 && !ich->desc)
				{
					ich->wait--;

					if (ich->wait == 0 && MP_VALID_MOB(ich))
					{
						mprog_delay_trigger(ich, ich->npcdata->delay_index);
					}
				}
			}
		}
	}

	for (npl = mud->f_player ; npl ; npl = next_npl)
	{
		next_npl = npl->next;

		if (npl->ch->level >= LEVEL_IMMORTAL)
		{
			continue;
		}

		if (npl->ch->desc == NULL && npl->ch->wait > 0)
		{
			npl->ch->wait = UMAX(0, npl->ch->wait - PULSE_AGGRESSIVE);
		}

		for (ch = npl->ch->in_room->first_person ; ch ; ch = mud->update_rch)
		{
			mud->update_rch = ch->next_in_room;

			if (!IS_NPC(ch) || !IS_SET(ch->act, ACT_AGGRESSIVE))
			{
				continue;
			}

			if (!MP_VALID_MOB(ch))
			{
				continue;
			}

			if (!IS_AWAKE(ch) || ch->fighting || ch->hit < ch->max_hit/2)
			{
				continue;
			}

			if (IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(npl->ch))
			{
				continue;
			}

			if (!can_see(ch, npl->ch) || number_bits(1) == 0)
			{
				continue;
			}

			if (abs(npl->ch->alignment - ch->alignment) <= 500)
			{
				if (npl->ch->class == CLASS_MONK || number_bits(1) == 0)
				{
					continue;
				}
			}
			multi_hit(ch, npl->ch, TYPE_UNDEFINED);
		}
	}
	pop_call();
	return;
}


void purger_update (void)
{
	JUNK_DATA *junk;

	push_call("purger_update(void)");

	while (mud->f_junk)
	{
		junk = mud->f_junk;

		if (junk->mob)
		{
			extract_char(junk->mob);
		}
		else
		{
			extract_obj(junk->obj);
		}
		UNLINK(junk, mud->f_junk, mud->l_junk, next, prev);
		FREEMEM(junk);
	}
	pop_call();
	return;
}

/*
	Called once per pulse from game loop.
	Random times to defeat tick-timing clients and players.
	Update routines spread out over 10 pulses
	Scandum - 14-05-2003
*/

void update_handler (void)
{
	static sh_int pulse_areasave		= 9 + PULSE_AREASAVE;
	static sh_int pulse_shops		= 9 + PULSE_SHOPS;
	static sh_int pulse_area			= 9 + PULSE_TICK;
	static sh_int pulse_weather		= 9 + PULSE_TICK;
	static sh_int pulse_obj			= 8 + PULSE_TICK;
	static sh_int pulse_tick			= 7 + PULSE_TICK;
	static sh_int pulse_charsave		= 6 + PULSE_CHARSAVE;
	static sh_int pulse_msdp           = 5 + PULSE_MSDP;
	static sh_int pulse_mobprog		= 4 + PULSE_PROGRAM;
	static sh_int pulse_objprog		= 3 + PULSE_PROGRAM;
	static sh_int pulse_violence		= 2 + PULSE_VIOLENCE;
	static sh_int pulse_mobile		= 1 + PULSE_MOBILE;
	static sh_int pulse_aggressive	= 0 + PULSE_AGGRESSIVE;

	push_call("update_handler()");

	if (--pulse_areasave <= 0)
	{
		pulse_areasave = PULSE_AREASAVE;
		start_timer(TIMER_AREA_SAVE);
		auto_area_save();
		close_timer(TIMER_AREA_SAVE);
	}

	if (--pulse_shops <= 0)
	{
		pulse_shops = PULSE_SHOPS;
		start_timer (TIMER_SHOP_UPD);
		shop_update ();
		close_timer (TIMER_SHOP_UPD);
	}

	if (--pulse_tick <= 0)
	{
		pulse_tick = PULSE_TICK / 2 + 10 * number_range(0, PULSE_TICK / 10);
		start_timer (TIMER_CHAR_UPD);
		time_update();
		char_update();
		close_timer(TIMER_CHAR_UPD);
	}

	if (--pulse_area <= 0)
	{
		pulse_area = PULSE_AREA / 2 + 10 * number_range(0, PULSE_AREA / 10);
		start_timer (TIMER_AREA_UPD);
		area_update ();
		close_timer (TIMER_AREA_UPD);
	}

	if (--pulse_weather <= 0)
	{
		pulse_weather = PULSE_TICK / 2 + 10 * number_range(0, PULSE_TICK / 10);
		start_timer (TIMER_WEATHER_UPD);
		weather_update ();
		close_timer (TIMER_WEATHER_UPD);
	}

	if (--pulse_obj <= 0)
	{
		pulse_obj = PULSE_TICK / 2 + 10 * number_range(0, PULSE_TICK / 10);
		start_timer (TIMER_OBJ_UPD);
		obj_update ();
		close_timer (TIMER_OBJ_UPD);
	}

	if (--pulse_charsave <= 0)
	{
		pulse_charsave = PULSE_CHARSAVE;
		start_timer( TIMER_CHAR_SAVE );
		auto_char_save();
		close_timer( TIMER_CHAR_SAVE );
	}

	if (--pulse_violence <= 0)
	{
		pulse_violence = PULSE_VIOLENCE;
		start_timer (TIMER_VIOL_UPD);
		violence_update ();
		close_timer (TIMER_VIOL_UPD);
	}

	if (--pulse_msdp <= 0)
	{
		pulse_msdp = PULSE_MSDP;
		start_timer(TIMER_MSDP);
		msdp_update();
		close_timer(TIMER_MSDP);
	}

	if (--pulse_mobprog <= 0)
	{
		pulse_mobprog = PULSE_PROGRAM;
		start_timer(TIMER_MOB_PROG);
		mob_program_update();
		close_timer(TIMER_MOB_PROG);
	}

	if (--pulse_objprog <= 0)
	{
		pulse_objprog = PULSE_PROGRAM;
		start_timer(TIMER_OBJ_PROG);
		obj_program_update();
		close_timer(TIMER_OBJ_PROG);
	}

	if (--pulse_mobile <= 0)
	{
		pulse_mobile = PULSE_MOBILE;
		start_timer (TIMER_MOB_UPD);
		mobile_update();
		close_timer (TIMER_MOB_UPD);
	}

	if (--pulse_aggressive <= 0)
	{
		pulse_aggressive = PULSE_AGGRESSIVE;
		start_timer (TIMER_AGGR_UPD);
		aggr_update();
		close_timer (TIMER_AGGR_UPD);
	}

	{
		start_timer (TIMER_PURGE);
		purger_update();
		if (IS_SET(mud->flags, MUD_PURGER))
		{
			update_purger();
		}
		close_timer (TIMER_PURGE);
	}

	pop_call();
	return;
}


int exp_level(int class, int level)
{
	int num, cnt;
	float low, mid, top;

	push_call("exp_level(%p,%p)",class,level);

	switch (class)
	{
		case CLASS_RANGER:
			low = 1700;
			mid =  574;
			top =   16;
			break;

		case CLASS_GLADIATOR:
			low = 1750;
			mid =  680;
			top =   17;
			break;

		case CLASS_MARAUDER:
			low = 1500;
			mid =  430;
			top =   15;
			break;

		case CLASS_NINJA:
			low = 1650;
			mid =  574;
			top =   17;
			break;

		case CLASS_ELEMENTALIST:
			low = 1600;
			mid =  501;
			top =   16;
			break;

		case CLASS_ILLUSIONIST:
			low = 1650;
			mid =  610;
			top =   17;
			break;

		case CLASS_MONK:
			low = 1550;
			mid =  501;
			top =   15;
			break;

		case CLASS_NECROMANCER:
			low = 1700;
			mid =  644;
			top =   17;
			break;

		case CLASS_MONSTER:
			low =  75.00;
			mid = 100.00;
			top = URANGE(1, level, 100);
			break;

		default:
			pop_call();
			return (0);
	}

	if (class != CLASS_MONSTER)
	{
		num = level * low;

		for (cnt = 0 ; cnt < level ; cnt++)
		{
			num += cnt * cnt * mid;

			num += cnt * cnt * cnt * top;
		}
	}
	else
	{
		num = low;

		for (cnt = 0 ; cnt < level ; cnt++)
		{
			num += top * mid / 2;

			num += top * top / 4;
		}
	}
	pop_call();
	return num;
}


void bounty_update( void )
{
	BOUNTY_DATA *bounty, *bounty_next;

	push_call("bounty_update()");

	for (bounty = mud->f_bounty ; bounty ; bounty = bounty_next)
	{
		bounty_next = bounty->next;

		if (bounty->expires < mud->current_time)
		{
			remove_bounty( bounty );
			save_bounties();
		}
	}
	pop_call();
	return;
}


void set_room_timer(int vnum, int type, int timer)
{
	ROOM_TIMER_DATA *rtd;

	push_call("set_room_timer(%d,%d,%d)",vnum,type,timer);

	ALLOCMEM(rtd, ROOM_TIMER_DATA, 1);

	rtd->vnum		= vnum;
	rtd->type		= type;
	rtd->timer	= timer;

	LINK(rtd, mud->f_room_timer, mud->l_room_timer, next, prev);

	pop_call();
	return;
}


bool del_room_timer(int vnum, int type)
{
	ROOM_TIMER_DATA *rtd;

	push_call("del_room_timer(%d,%d)",vnum,type);

	for (rtd = mud->f_room_timer ; rtd ; rtd = rtd->next)
	{
		if (rtd->vnum == vnum && type == -1)
		{
			rtd->vnum = ROOM_VNUM_JUNK;
			continue;
		}

		if (rtd->vnum == vnum && rtd->type == type)
		{
			break;
		}
	}

	if (rtd)
	{
		UNLINK(rtd, mud->f_room_timer, mud->l_room_timer, next, prev);

		FREEMEM(rtd);

		pop_call();
		return TRUE;
	}

	pop_call();
	return FALSE;
}
