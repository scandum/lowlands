/***************************************************************************
 * Lola  1.0 by Igor van den Hoven.                                        *
 *                                                                         *
 * Emud  3.0 by Igor van den Hoven.                                        *
 ***************************************************************************/

#include "mud.h"


/*
	Scandum 20-05-2002
*/


void send_bounty_message(char *message)
{
	NOTE_DATA *pnote;

	push_call("send_bounty_message(%p)",message);

	ALLOCMEM( pnote, NOTE_DATA, 1);

	pnote->next				= NULL;
	pnote->sender				= STRALLOC("Bounty Office");
	pnote->date				= STRALLOC(get_time_string(mud->current_time));
	pnote->time				= mud->current_time;
	pnote->to_list				= STRALLOC("All");
	pnote->subject				= STRALLOC("New Contract!");
	pnote->room_vnum			= 30961;
	pnote->text				= STRALLOC(message);

	LINK(pnote, mud->f_note, mud->l_note, next, prev);

	pop_call();
	return;
}


void do_bounty( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	int  		amount = 0;
	CHAR_DATA		*victim;
	BOUNTY_DATA	*bounty;
	bool 		loaded = FALSE;

	push_call("do_bounty(%p,%p)",ch,argument);

	argument = one_argument(argument, arg);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (tolower(*arg) == 'p')
	{
		argument = one_argument(argument, arg);

		amount   = atoi(arg);

		if (amount <= 0)
		{
			send_to_char("You can't post a bounty of 0 gold.\n\r", ch);
			pop_call();
			return;
		}

		if (amount < 1000000)
		{
			ch_printf(ch, "The minimum bounty is 1,000,000 gold coins.\n\r");
			pop_call();
			return;
		}

		argument = one_argument(argument, arg);

		if (*arg == '\0' || *arg == ' ')
		{
			send_to_char("Usage: bounty <post/list/info> [amount] [target]\n\r", ch);
			pop_call();
			return;
		}

		if ((victim = lookup_char(arg)) == NULL)
		{
			if ((victim = start_partial_load(ch, arg)) == NULL)
			{
				pop_call();
				return;
			}
			loaded = TRUE;
		}

		if (IS_IMMORTAL(victim))
		{
			send_to_char("You may not post bounties on Immortals.\n\r", ch);
			if (loaded)
			{
				clear_partial_load(victim);
			}
			pop_call();
			return;
		}

		if (!IS_SET(victim->act, PLR_KILLER) && !IS_SET(victim->act, PLR_THIEF))
		{
			send_to_char("You may only bounty killers and thieves.\n\r", ch);
			if (loaded)
			{
				clear_partial_load(victim);
			}
			pop_call();
			return;
		}

		if (which_god(ch) == GOD_NEUTRAL)
		{
			send_to_char("You would not want to risk the wrath of their God.\n\r", ch);
			if (loaded)
			{
				clear_partial_load(victim);
			}
			pop_call();
			return;
		}			

		if (which_god(ch) == which_god(victim))
		{
			send_to_char("You would not want to risk the wrath of your God.\n\r", ch);
			if (loaded)
			{
				clear_partial_load(victim);
			}
			pop_call();
			return;
		}

		if (!gold_transaction(ch, 0 - amount))
		{
			send_to_char("You don't have that much gold.\n\r", ch);
			if (loaded)
			{
				clear_partial_load(victim);
			}
			pop_call();
			return;
		}

		post_bounty(arg, amount);
		save_bounties();

		send_to_char("The bounty has been posted.\n\r", ch);

		if (!IS_IMMORTAL(ch))
		{
			bounty = get_bounty(victim->name);

			strftime(arg, 12, "%2e %b %Y", gmtime( (const time_t *) &bounty->expires));

			sprintf(buf, "\n\r{128}A bounty of %s gold has been posted on %s's head by %s.\n\r", tomoney(bounty->amount), victim->name, ch->name);

			send_bounty_message(buf);
		}

		if (loaded)
		{
			clear_partial_load(victim);
		}
	}
	else if (tolower(*arg) == 'i')
	{
		argument = one_argument(argument, arg);

		if ((bounty = get_bounty(arg)) == NULL)
		{
			ch_printf(ch, "There is no bounty posted on '%s'.\n\r", arg);
			pop_call();
			return;
		}

		if (bounty->expires < mud->current_time)
		{
			send_to_char("That bounty has just expired.\n\r", ch);

			remove_bounty(bounty);
			save_bounties();

			pop_call();
			return;
		}

		ch_printf(ch, "Expiration date for the bounty on %s: %s\n\r", bounty->name, get_time_string(bounty->expires));
	}
	else
	{
		sprintf(buf,  "{138} %12s %13s %11s  %12s %13s %11s\n\r", "Player", "Bounty", "Expiration",  "Player", "Bounty", "Expiration");

		cat_sprintf(buf, "{818}--------------------------------------------------------------------------------\n\r");

		for (amount = 0, bounty = mud->f_bounty ; bounty ; bounty = bounty->next, amount++)
		{
			strftime(arg, 12, "%2e %b %Y", gmtime( (const time_t *) &bounty->expires));

			switch (amount % 2)
			{
				case 0:
					cat_sprintf(buf, " {848}%12s {828}%13s {878}%11s ",   bounty->name, tomoney(bounty->amount), arg);
					break;
				case 1:
					cat_sprintf(buf, " {848}%12s {828}%13s {878}%11s\n\r", bounty->name, tomoney(bounty->amount), arg);
					break;
			}
		}

		if (str_suffix("\n\r", buf))
		{
			strncat(buf, "\n\r", MAX_STRING_LENGTH);
		}
		send_to_char_color(buf, ch);
	}
	pop_call();
	return;
}


void post_bounty(char *name, int amount)
{
	BOUNTY_DATA *bounty;

	push_call("post_bounty(%p,%p)",name,amount);

	/* Increase the value of a current bounty */

	for (bounty = mud->f_bounty ; bounty ; bounty = bounty->next)
	{
		if (!strcasecmp(bounty->name, name))
		{
			bounty->amount  = UMIN(1000000000, bounty->amount + amount);
			bounty->expires = mud->current_time + 3600 * 24 * 28 + bounty->amount / 1000000 * 3600 * 24;

			pop_call();
			return;
		}
	}

	ALLOCMEM(bounty, BOUNTY_DATA, 1);
	bounty->name    = STRALLOC(capitalize(name));
	bounty->amount  = amount;
	bounty->expires = mud->current_time + 3600 * 24 * 28 + bounty->amount / 1000000 * 3600 * 24;
	sort_bounty(bounty);

	pop_call();
	return;
}


void sort_bounty(BOUNTY_DATA *bounty)
{
	BOUNTY_DATA *temp_bounty;

	push_call("sort_bounty(%p)",bounty);

	for (temp_bounty = mud->f_bounty ; temp_bounty ; temp_bounty = temp_bounty->next)
	{
		if (strcmp(bounty->name, temp_bounty->name) < 0)
		{
			INSERT_LEFT(bounty, temp_bounty, mud->f_bounty, next, prev);
			break;
		}
	}
	if (temp_bounty == NULL)
	{
		LINK(bounty, mud->f_bounty, mud->l_bounty, next, prev);
	}
	pop_call();
	return;
}


void remove_bounty(BOUNTY_DATA *bounty)
{
	push_call("remove_bounty(%p)",bounty);

	UNLINK(bounty, mud->f_bounty, mud->l_bounty, next, prev);

	STRFREE(bounty->name);
	FREEMEM(bounty);

	pop_call();
	return;
}


BOUNTY_DATA *get_bounty( char *name )
{
	BOUNTY_DATA *bounty;

	push_call("get_bounty(%p)",name);

	for (bounty = mud->f_bounty ; bounty ; bounty = bounty->next)
	{
		if (!strcasecmp(bounty->name, name))
		{
			pop_call();
			return(bounty);
		}
	}
	pop_call();
	return NULL;
}


int gold_transaction(CHAR_DATA *ch, int amount)
{
	int max_account, max_pocket;

	push_call("gold_transaction(%p)",ch,amount);

	if (IS_NPC(ch))
	{
		pop_call();
		return FALSE;
	}

	if (amount < 0)
	{
		amount = 0 - amount;
		if (ch->gold >= amount)
		{
			ch->gold -= amount;

			pop_call();
			return TRUE;
		}
		else if (ch->gold + ch->pcdata->account >= amount)
		{
			amount -= ch->gold;
			ch->gold = 0;
			ch->pcdata->account -= amount;

			pop_call();
			return TRUE;
		}
		else
		{
			pop_call();
			return FALSE;
		}
	}
	else
	{
		max_pocket = ch->level * 1000000 - ch->gold;

		max_account = (ch->level + 5 + ch->pcdata->reincarnation * 5) * 2000000;

		if (amount <= max_pocket)
		{
			ch->gold += amount;
		}
		else
		{
			amount -= max_pocket;
			ch->gold = ch->level * 1000000;
			ch->pcdata->account += amount;
			if (ch->pcdata->account > max_account)
			{
				ch->pcdata->account = max_account;
			}
		}
		pop_call();
		return TRUE;
	}
}


void collect_bounty( CHAR_DATA *ch, CHAR_DATA *victim )
{
	BOUNTY_DATA 	*bounty;
	OBJ_DATA		*head;
	char			buf[MAX_INPUT_LENGTH];
	AFFECT_DATA	af;

	int stat_bonus[5] = {0};

	int dam_plus, hit_plus, ac_plus, hp_plus, mana_plus, level_plus;

	push_call("collect_bounty(%p,%p)",ch,victim);

	bounty = get_bounty(victim->name);

	if (bounty == NULL)
	{
		pop_call();
		return;
	}

	act("You sever $N's head to collect the bounty on it.", ch, NULL, victim, TO_CHAR);
	act("$n severs $N's head to collect the bounty on it.", ch, NULL, victim, TO_NOTVICT);

	head = create_object(get_obj_index(OBJ_VNUM_BOUNTY_HEAD), 0);

	head->level	= victim->level;
	head->cost	= bounty->amount;
	head->weight	= 1 + race_table[victim->race].weight / 30;
	head->value[0]	= mud->current_time + 60 * 60 * 24 * 30 * 12 + bounty->amount / 1000000 * 86400;

	SET_BIT(head->extra_flags, ITEM_MODIFIED);

	if (!IS_EVIL(victim))
	{
		SET_BIT(head->extra_flags, ITEM_ANTI_EVIL);
	}
	if (!IS_GOOD(victim))
	{
		SET_BIT(head->extra_flags, ITEM_ANTI_GOOD);
	}
	if (!IS_NEUTRAL(victim))
	{
		SET_BIT(head->extra_flags, ITEM_ANTI_NEUTRAL);
	}

	sprintf(buf, head->name, victim->name);
	RESTRING(head->name, buf);
	sprintf(buf, head->short_descr, get_name(victim));
	RESTRING(head->short_descr, buf);
	sprintf(buf, head->long_descr, get_name(victim));
	RESTRING(head->long_descr, buf);
	sprintf(buf, head->description, get_name(victim), bounty->amount);
	RESTRING(head->description, justify(buf, 80));

	stat_bonus[0] = race_table[victim->race].race_mod[0] + victim->pcdata->reincarnation / 2;
	stat_bonus[1] = race_table[victim->race].race_mod[1] + victim->pcdata->reincarnation / 2;
	stat_bonus[2] = race_table[victim->race].race_mod[2] + victim->pcdata->reincarnation / 2;
	stat_bonus[3] = race_table[victim->race].race_mod[3] + victim->pcdata->reincarnation / 2;
	stat_bonus[4] = race_table[victim->race].race_mod[4] + victim->pcdata->reincarnation / 2;

	stat_bonus[class_table[victim->class].attr_prime  -1] += class_table[victim->class].prime_mod;
	stat_bonus[class_table[victim->class].attr_second -1] += class_table[victim->class].sec_mod;

	dam_plus = hit_plus = ac_plus =  hp_plus = mana_plus = 0;

	level_plus = UMAX(1, victim->pcdata->history[HISTORY_KILL_EN] - victim->pcdata->history[HISTORY_KILL_BY_EN]);

	dam_plus = hit_plus = UMIN(level_plus *  1,  victim->level / 6);
	hp_plus = mana_plus = UMIN(level_plus * 12,  victim->level * 2);
	ac_plus			= UMAX(level_plus * -2, -victim->level / 3);

	af.type		= gsn_object_rape;
	af.duration	= -1;
	af.bitvector	= 0;

	if (stat_bonus[0])
	{
		af.modifier = stat_bonus[0];
		af.location = 1;
		affect_to_obj(head, &af);
	}

	if (stat_bonus[1])
	{
		af.modifier = stat_bonus[1];
		af.location = 2;
		affect_to_obj(head, &af);
	}

	if (stat_bonus[2])
	{
		af.modifier = stat_bonus[2];
		af.location = 3;
		affect_to_obj(head, &af);
	}

	if (stat_bonus[3])
	{
		af.modifier = stat_bonus[3];
		af.location = 4;
		affect_to_obj(head, &af);
	}

	if (stat_bonus[4])
	{
		af.modifier = stat_bonus[4];
		af.location = 5;
		affect_to_obj(head, &af);
	}

	if (ac_plus)
	{
		af.modifier = ac_plus;
		af.location = APPLY_AC;
		affect_to_obj(head, &af);
	}

	if (victim->pcdata->mclass[CLASS_RANGER]
	||  victim->pcdata->mclass[CLASS_GLADIATOR]
	||  victim->pcdata->mclass[CLASS_MARAUDER]
	||  victim->pcdata->mclass[CLASS_NINJA])
	{
		if (dam_plus)
		{
			af.modifier = dam_plus;
			af.location = APPLY_DAMROLL;
			affect_to_obj(head, &af);
		}

		if (hit_plus)
		{
			af.modifier = hit_plus;
			af.location = APPLY_HITROLL;
			affect_to_obj(head, &af);
		}
	}
	else
	{
		if (hp_plus)
		{
			af.modifier = hp_plus;
			af.location = APPLY_HIT;
			affect_to_obj(head, &af);
		}

		if (mana_plus)
		{
			af.modifier = mana_plus;
			af.location = APPLY_MANA;
			affect_to_obj(head, &af);
		}
	}

	obj_to_char(head, ch);

	remove_bounty(bounty);
	save_bounties();

	pop_call();
	return;
}

