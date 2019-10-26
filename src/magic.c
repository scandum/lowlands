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

int		multi_spell_level	args( ( CHAR_DATA *ch, int sn ) );
int		can_cast_spell		args( ( CHAR_DATA *ch, int sn ) );

int		caster_levels		args( ( CHAR_DATA *ch ) );
bool		is_safe_magic		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int sn ) );
int		magic_modify		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int sn, int level ) );
bool		can_mass_cast		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

#define DO_SPELL(spell) void spell( int sn, int level, CHAR_DATA *ch, void *vo, int target )

int value;	/* This is the value after the target */

void make_char_fight_char( CHAR_DATA *ch, CHAR_DATA *victim )
{
	push_call("make_char_fight_char(%p,%p)",ch,victim);

	if (ch == NULL || victim == NULL || victim->position == POS_DEAD)
	{
		pop_call();
		return;
	}

	if (ch == victim)
	{
		pop_call();
		return;
	}

	/*
		set recently fought
	*/

	if (!IS_NPC(ch) && IS_NPC(victim))
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
	pop_call();
	return;
}

/*
	Compute a saving throw.
	Negative apply's make saving throw better.
*/

bool saves_spell( int level, CHAR_DATA *ch, CHAR_DATA *victim )
{
	CHAR_DATA *fch;
	int range, point;

	push_call("saves_spell(%p,%p,%p)",level,ch,victim);

	point = 50 + level * 3 / 4;

	if (IS_NPC(victim))
	{
		range = 100 + victim->level / 2 - GET_SAVING_THROW(victim) * 2;
	}
	else
	{
		range = 100 - GET_SAVING_THROW(victim) * 4;
	}

	/* Add a bit of distraction to tanking spell casters - Chaos 7/11/96  */

	for (fch = ch->in_room->first_person ; fch != NULL ; fch = fch->next_in_room)
	{
		if (fch != ch && fch->fighting != NULL && fch->fighting->who == ch)
		{
			range += fch->level/5;
		}
	}

	range -= get_curr_int(ch);

	if (IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch))
	{
		range += 15;
	}

	if (IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch))
	{
		range += 15;
	}

	if (range > 1 && number_range(1, range) >= point)
	{
		pop_call();
		return(TRUE);    /* Spell Fails */
	}
	else
	{
		pop_call();
		return( FALSE );   /* Spell Works */
	}
	/*  Chaos changing to universal scaling system  12/16/94 */
}

/*
	The kludgy global is for spells who want more stuff from command line.
*/

char *target_name;

void do_cast( CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	void *vo;
	int mana, sn, level, percentage, target;

	push_call("do_cast(%p,%p)",ch,argument);

	if (strlen(argument) > 100)
	{
		*(argument+80)='\0';
	}

	percentage = 100;

	argument = one_argument(argument, arg1);

	/*
		Grab reduced spell strength percentage value - Chaos 7/26/96
	*/

	if (*arg1 != '\0' && is_number(arg1))
	{
		percentage = atol(arg1);

		argument = one_argument(argument, arg1);

		percentage = URANGE(1, percentage, 100);
	}

	target_name = argument;
	argument    = one_argument( argument, arg2 );
	argument    = one_argument( argument, arg3 );

	if (strlen(target_name) > 50)
	{
		target_name[50] = '\0';
	}

	if (arg1[0] == '\0')
	{
		send_to_char( "Cast which what where?\n\r", ch );
		pop_call();
		return;
	}

	value = -1;   /* Default value */

	if (is_number(arg2))   /*  Special numbers only spell */
	{
		value = atol(arg2);
		strcpy( arg2, "");
		if (value < 1)
		{
			value = -1;   /* Default value */
		}
	}
	else if (arg3[0] != '\0')     /* Read the extra value  - Chaos  2/8/95  */
	{
		value = atol( arg3 );
		if ( value < 1 )
		{
			value = -1;   /* Default value */
		}
	}

	sn = skill_lookup( arg1 );

	if (sn < 0 || !is_spell(sn))
	{
		if (!IS_NPC(ch) && ch->desc == NULL)
		{
			log_printf("[%u] casting unknown spell: %s", ch->pIndexData->vnum, arg1);
		}
		send_to_char( "That is not a spell.\n\r", ch );
		pop_call();
		return;
	}

	if ((level = can_cast_spell(ch, sn)) == -1)
	{
		pop_call();
		return;
	}

	if (is_safe_magic(ch, NULL, sn))
	{
		pop_call();
		return;
	}

	mana  = get_mana(ch, sn);

	level = level * percentage / 100 ;

	if (level < 1)
	{
		level = 1;
	}

	/*
		Locate targets.
	*/

	victim	= NULL;
	obj		= NULL;
	vo		= NULL;
	target	= skill_table[sn].target;

	switch (target)
	{
		default:
			bug( "do_cast: bad target for sn %d.", sn );
			pop_call();
			return;

		case TAR_IGNORE:
			break;

		case TAR_CHAR_OFFENSIVE:
			if (arg2[0] == '\0')
			{
				if (ch->fighting == NULL || ch->fighting->who == NULL)
				{
					send_to_char( "Cast the spell on whom?\n\r", ch );
					pop_call();
					return;
				}
				victim = ch->fighting->who;
			}
			else
			{
				if ((victim = get_char_room(ch, arg2)) == NULL)
				{
					send_to_char( "They aren't here.\n\r", ch );
					pop_call();
					return;
				}
			}
			if (is_safe_magic(ch, victim, sn))
			{
				pop_call();
				return;
			}
			vo = (void *) victim;
			break;

		case TAR_CHAR_DEFENSIVE:

			if (arg2[0] == '\0')
			{
				victim = ch;
			}
			else
			{
				if ((victim = get_char_room(ch, arg2)) == NULL)
				{
					send_to_char( "They aren't here.\n\r", ch );
					pop_call();
					return;
				}
			}

			if (is_safe_magic(ch, victim, sn))
			{
				pop_call();
				return;
			}

			vo = (void *) victim;
			break;

		case TAR_CHAR_SELF:

			if (arg2[0] != '\0' && !is_name_short(arg2, ch->name))
			{
				send_to_char( "You cannot cast this spell on another.\n\r", ch );
				pop_call();
				return;
			}
			vo = (void *) ch;
			break;

		case TAR_OBJ_INV:

			if (arg2[0] == '\0')
			{
				send_to_char( "What should the spell be cast upon?\n\r", ch );
				pop_call();
				return;
			}

			if ((obj = get_obj_carry(ch, arg2)) == NULL)
			{
				send_to_char( "You are not carrying that.\n\r", ch );
				pop_call();
				return;
			}
			vo = (void *) obj;
			break;

		case TAR_OBJ_CHAR_DEF:

			if (arg2[0] == '\0')
			{
				victim = ch;
			}
			else	if ((victim = get_char_room(ch, arg2)) == NULL)
			{
				if ((obj = get_obj_carry(ch, arg2)) == NULL)
				{
					send_to_char( "Who or what should the spell be cast upon?\n\r", ch );
					pop_call();
					return;
				}
			}

			if (victim)
			{
				if (is_safe_magic(ch, victim, sn))
				{
					pop_call();
					return;
				}
				target = TAR_CHAR_DEFENSIVE;
				vo     = (void *) victim;
			}
			else
			{
				target = TAR_OBJ_INV;
				vo     = (void *) obj;
			}
			break;

		case TAR_OBJ_CHAR_OFF:

			if (arg2[0] == '\0')
			{
				if (who_fighting(ch) == NULL)
				{
					send_to_char("Cast the spell on whom?\n\r", ch);
					pop_call();
					return;
				}
				victim = ch->fighting->who;
			}
			else	if ((victim = get_char_room(ch, arg2)) == NULL)
			{
				if ((obj = get_obj_carry(ch, arg2)) == NULL)
				{
					send_to_char( "Who or what should the spell be cast upon?\n\r", ch );
					pop_call();
					return;
				}
			}

			if (victim)
			{
				if (is_safe_magic(ch, victim, sn))
				{
					pop_call();
					return;
				}
				target = TAR_CHAR_OFFENSIVE;
				vo     = (void *) victim;
			}
			else
			{
				target = TAR_OBJ_INV;
				vo     = (void *) obj;
			}
			break;
	}

	if (!IS_IMMORTAL(ch) && ch->mana < mana)
	{
		send_to_char( "You don't have enough mana.\n\r", ch );
		pop_call();
		return;
	}

	if (strcasecmp( skill_table[sn].name, "ventriloquate" ) )
	{
		say_spell( ch, sn );
	}

	if (IS_AFFECTED(ch, AFF2_QUICKEN))
	{
		wait_state( ch, skill_table[sn].beats * 4 / 5 );
	}
	else
	{
		wait_state( ch, skill_table[sn].beats );
	}

	if (!IS_IMMORTAL(ch) && number_percent() > learned(ch, sn))
	{
		if (ch->fighting != NULL)
		{
			switch (number_bits(3))
			{
				case 0 : send_to_char( "This round of battle is too hectic to concentrate properly.\n\r",     ch ); break;
				case 1 : send_to_char( "There wasn't enough time this round to complete the casting.\n\r",    ch ); break;
				case 2 : send_to_char( "Despite your efforts you only produce worthless smoke.\n\r",          ch ); break;
				case 3 : send_to_char( "You lose control over your magic and only scorch the ground.\n\r",    ch ); break;
				case 4 : send_to_char( "The air sizzles with magic, but you fail to strike your victim.\n\r", ch ); break;
				case 5 : send_to_char( "You hesitate mid-way through the casting and nothing happens.\n\r",   ch ); break;
				default: send_to_char( "You lost your concentration.\n\r",                                    ch ); break;
			}
		}
		else
		{
			switch (number_bits(3))
			{
				case 0 : send_to_char( "A tickle in your nose prevents you from keeping your concentration.\n\r",    ch ); break;
				case 1 : send_to_char( "An itch on your leg keeps you from properly casting your spell.\n\r",        ch ); break;
				case 2 : send_to_char( "Something in your throat prevents you from uttering the proper phrase.\n\r", ch ); break;
				case 3 : send_to_char( "A twitch in your eye disrupts your concentration for a moment.\n\r",         ch ); break;
				case 4 : send_to_char( "You get a mental block mid-way through the casting.\n\r",                    ch ); break;
				default: send_to_char( "You lost your concentration.\n\r",                                           ch ); break;
			}
		}
		ch->mana -= mana / 2;
	}
	else
	{
		if (!IS_IMMORTAL(ch))
		{
			ch->mana -= mana;
		}

		if (number_bits(1) == 0 && is_affected(ch, gsn_mana_shackles))
		{
			switch (number_bits(2))
			{
				case 0:
					act("Your magical shackles send searing lightning through you as you cast.", ch, NULL, NULL, TO_CHAR);
					act("$n's magical shackles send searing lightning through $m as $e casts.",  ch, NULL, NULL, TO_ROOM);
					break;
				case 1:
					act("You reel in shock as your magical shackles claw into you as you cast.", ch, NULL, NULL, TO_CHAR);
					act("$n reels in shock as $s magical shackles claw into $m as $e casts.",    ch, NULL, NULL, TO_ROOM);
					break;
				case 2:
					act("Your magical shackles shock you to the core as you cast.", ch, NULL, NULL, TO_CHAR);
					act("$n's magical shackles shock $m to the core as $e casts.",  ch, NULL, NULL, TO_ROOM);
					break;
				case 3:
					act("You scream out in agony as your magical shackles shock you as you cast.", ch, NULL, NULL, TO_CHAR);
					act("$n screams out in agony as $s magical shackles shock $m as $e casts.",    ch, NULL, NULL, TO_ROOM);
					break;
			}
			ch->hit -= number_range(mana*3, mana*6);
		}

		if (victim && victim != ch && !IS_NPC(victim) && target == TAR_CHAR_OFFENSIVE)
		{
			if (IS_AWAKE(victim) && number_percent() < learned(victim, gsn_lightning_reflexes) / 4)
			{
				switch (number_bits(2))
				{
					case 0:
						act("You dive away from $n's $t with a lightning fast reflex.", ch, skill_table[sn].noun_damage, victim, TO_VICT);
						act("$N dives away from your $t with a lightning fast reflex.", ch, skill_table[sn].noun_damage, victim, TO_CHAR);
						break;
					case 1:
						act("You avoid $n's $t with a lightning fast side step.", ch, skill_table[sn].noun_damage, victim, TO_VICT);
						act("$N avoids your $t with a lightning fast side step.", ch, skill_table[sn].noun_damage, victim, TO_CHAR);
						break;
					case 2:
						act("You throw your body aside escaping from $n's $t.", ch, skill_table[sn].noun_damage, victim, TO_VICT);
						act("$N throws $S body aside escaping from your $t.",   ch, skill_table[sn].noun_damage, victim, TO_CHAR);
						break;
					case 3:
						act("You deftly move aside watching $n's $t go past.", ch, skill_table[sn].noun_damage, victim, TO_VICT);
						act("$N deftly moves aside watching your $t go past.", ch, skill_table[sn].noun_damage, victim, TO_CHAR);
						break;
				}
				check_improve(victim, gsn_lightning_reflexes);
				pop_call();
				return;
			}
		}

		if (victim && victim != ch && number_range(0, 4) == 0)
		{
			if (is_affected(victim, gsn_magic_mirror))
			{
				act("White lightning surrounds you, reflecting $n's magic.", ch, NULL, victim, TO_VICT    );
				act("White lightning surrounds $N, reflecting your magic.",  ch, NULL, victim, TO_CHAR    );
				act("White lightning surrounds $N, reflecting $n's magic.",  ch, NULL, victim, TO_NOTVICT );
				vo = (void *) ch;
				ch = victim;
			}
		}

		level = magic_modify(ch, victim, sn, level);

		(*skill_table[sn].spell_fun) (sn, level, ch, vo, target);

		check_improve(ch, sn);
	}

	if (target == TAR_CHAR_OFFENSIVE)
	{
		if (victim != ch && victim->fighting == NULL)
		{
			make_char_fight_char( ch, victim );
			multi_hit( victim, ch, TYPE_UNDEFINED );
		}
	}
	pop_call();
	return;
}

/*
	Cast spells at targets using a magical object.
*/

void obj_cast_spell( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj )
{
	void *vo;
	int target;

	push_call("obj_cast_spell(%p,%p,%p,%p,%p)",sn,level,ch,victim,obj);

	if (sn <= 0)
	{
		pop_call();
		return;
	}

	if (sn <= MAX_SKILL && skill_table[sn].slot == -1)
	{
		pop_call();
		return;
	}

	if (is_safe_magic(ch, NULL, sn))
	{
		pop_call();
		return;
	}

	if (sn >= MAX_SKILL || skill_table[sn].spell_fun == 0)
	{
		if (obj != NULL)
		{
			log_printf("obj_cast_spell: Vnum %u bad sn %d.", obj->pIndexData->vnum, sn);
		}
		else
		{
			log_printf("Unknown item cast sn %d", sn);
		}
		pop_call();
		return;
	}

	level  = level * caster_levels(ch) / UMAX(1, ch->level);
	target = skill_table[sn].target;

	switch (target)
	{
		default:
			bug( "obj_cast_spell: bad target for sn %d.", sn );
			pop_call();
			return;

		case TAR_IGNORE:
			vo = NULL;
			break;

		case TAR_CHAR_OFFENSIVE:

			if (victim == ch || victim == NULL)
			{
				if (who_fighting(ch))
				{
					victim = ch->fighting->who;
				}
			}

			if (victim == NULL)
			{
				send_to_char( "You can't do that.\n\r", ch );
				pop_call();
				return;
			}

			if (is_safe_magic(ch, victim, sn))
			{
				pop_call();
				return;
			}
			vo = (void *) victim;
			break;

		case TAR_CHAR_DEFENSIVE:

			if (victim == NULL)
			{
				victim = ch;
			}

			if (is_safe_magic(ch, victim, sn))
			{
				pop_call();
				return;
			}

			vo = (void *) victim;
			break;

		case TAR_CHAR_SELF:

			if (victim == NULL)
			{
				victim = ch;
			}

			vo = (void *) ch;
			break;

		case TAR_OBJ_INV:
			if (obj == NULL)
			{
				send_to_char( "You can't do that.\n\r", ch );
				pop_call();
				return;
			}
			vo = (void *) obj;
			break;

		case TAR_OBJ_CHAR_DEF:

			if (victim)
			{
				if (is_safe_magic(ch, victim, sn))
				{
					pop_call();
					return;
				}
				target = TAR_CHAR_DEFENSIVE;
				vo     = (void *) victim;
			}
			else
			{
				target = TAR_OBJ_INV;
				vo     = (void *) obj;
			}
			break;

		case TAR_OBJ_CHAR_OFF:

			if (victim)
			{
				if (victim == ch && who_fighting(ch))
				{
					victim = ch->fighting->who;
				}

				if (is_safe_magic(ch, victim, sn))
				{
					pop_call();
					return;
				}
				target = TAR_CHAR_OFFENSIVE;
				vo     = (void *) victim;
			}
			else
			{
				target = TAR_OBJ_INV;
				vo     = (void *) obj;
			}
			break;
	}

	target_name = "";

	level = magic_modify(ch, victim, sn, level);

	(*skill_table[sn].spell_fun) ( sn, level, ch, vo, target );

	if (target == TAR_CHAR_OFFENSIVE)
	{
		if (victim != ch && victim->fighting == NULL)
		{
			make_char_fight_char( ch, victim );
			multi_hit( victim, ch, TYPE_UNDEFINED );
		}
	}
	pop_call();
	return;
}

/*
	Magical Totem casting spells on the room
*/

void totem_cast_spell( OBJ_DATA *obj )
{
	CHAR_DATA *totem, *rch, *rch_next;
	int sn, level, target;

	push_call("totem_cast_spell(%p)",obj);

	if (obj->in_room == NULL)
	{
		pop_call();
		return;
	}

	if (obj->in_room->area->nplayer == 0)
	{
		pop_call();
		return;
	}

	if ((totem = mob_index[MOB_VNUM_TOTEM]->first_instance) == NULL)
	{
		pop_call();
		return;
	}

	target_name = "";

	STRFREE(totem->short_descr);
	totem->short_descr = STRDUPE(obj->short_descr);

	char_to_room(totem, obj->in_room->vnum);

	act("The air thickens with magic as $n starts to hum.", totem, NULL, NULL, TO_ROOM);

	for (rch = obj->in_room->first_person ; rch ; rch = rch_next)
	{
		rch_next = rch->next_in_room;

		if (rch == totem)
		{
			continue;
		}

		if (IS_NPC(rch) && IS_AFFECTED(rch, AFF_ETHEREAL))
		{
			continue;
		}

		if (is_affected(rch, gsn_anti_magic_shell))
		{
			continue;
		}

		sn = obj->value[number_range(0,3)];

		if (!is_spell(sn))
		{
			continue;
		}

		level  = obj->level;
		level  = magic_modify(totem, rch, sn, level);
		target = skill_table[sn].target;

		switch (target)
		{
			case TAR_OBJ_CHAR_OFF:
				target = TAR_CHAR_OFFENSIVE;
				break;
			case TAR_OBJ_CHAR_DEF:
				target = TAR_CHAR_DEFENSIVE;
				break;
		}

		(*skill_table[sn].spell_fun) (sn, level, totem, rch, target);
	}

	char_to_room(totem, ROOM_VNUM_TOTEM);

	pop_call();
	return;
}

/*
	Takes any offensive spell for a sorcerer and defensive spell
	for a shaman and applies it to all the appropriate char's.
	Costs more mana than the combined casts, but takes less time.
*/

void do_mass( CHAR_DATA *ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	CHAR_DATA *victim, *next_vict;
	int sn, sha_mlv, sor_mlv, level, mana, target;

	push_call("do_mass(%p,%p)",ch,argument);

	argument = one_argument( argument, arg1 );

	if (arg1[0] == '\0')
	{
		send_to_char( "Cast which what where?\n\r", ch );
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && multi(ch, gsn_mass) == -1)
	{
		send_to_char( "You can't do that!\n\r", ch );
		pop_call();
		return;
	}

	if (number_percent() > learned(ch, gsn_mass))
	{
		send_to_char("You failed to enter the proper state of mind to amass that much energy!\n\r",ch);
		wait_state(ch, skill_table[gsn_mass].beats);
		pop_call();
		return;
	}

	sn = skill_lookup( arg1 );

	if (sn < 0 || !is_spell(sn))
	{
		send_to_char( "That is not a spell.\n\r", ch );
		if (!IS_NPC(ch) && ch->desc == NULL)
		{
			log_printf("[%u] massing unknown spell: %s", ch->pIndexData->vnum, arg1);
		}
		pop_call();
		return;
	}

	if ((level = can_cast_spell(ch, sn)) == -1)
	{
		pop_call();
		return;
	}

	sha_mlv = (IS_NPC(ch) || IS_IMMORTAL(ch)) ? 100 : ch->pcdata->mclass[CLASS_MONK];
	sor_mlv = (IS_NPC(ch) || IS_IMMORTAL(ch)) ? 100 : ch->pcdata->mclass[CLASS_ILLUSIONIST];

	target  = skill_table[sn].target;

	switch (target)
	{
		case TAR_CHAR_OFFENSIVE:
		case TAR_OBJ_CHAR_OFF:
			if (sor_mlv < skill_table[gsn_mass].skill_level[CLASS_ILLUSIONIST])
			{
				send_to_char( "You don't have enough skill in the proper class!\n\r", ch );
				pop_call();
				return;
			}
			target = TAR_CHAR_OFFENSIVE;
			break;
		case TAR_CHAR_DEFENSIVE:
		case TAR_OBJ_CHAR_DEF:
			if (sha_mlv < skill_table[gsn_mass].skill_level[CLASS_MONK])
			{
				send_to_char( "You don't have enough skill in the proper class!\n\r", ch );
				pop_call();
				return;
			}
			target = TAR_CHAR_DEFENSIVE;
			break;
		default:
			send_to_char("You would kill yourself trying to amass that much energy!\n\r",ch);
			pop_call();
			return;
 	}

	if (is_safe_magic(ch, NULL, sn))
	{
		pop_call();
		return;
	}

	mana = get_mana(ch, sn) * 5 / 4;

	/*
		Locate targets.
	*/

	switch (target)
	{
		case TAR_CHAR_OFFENSIVE:
			send_to_char("You unleash your magic upon your foes!\n\r", ch);
			break;
		case TAR_CHAR_DEFENSIVE:
			send_to_char("You sent forth your magic to aid your group.\n\r", ch);
			break;
	}

	for (victim = ch->in_room->last_person ; victim ; victim = next_vict)
	{
		next_vict = victim->prev_in_room;

		switch (target)
		{
			case TAR_CHAR_OFFENSIVE:

				if (!can_mass_cast(ch, victim))
				{
					continue;
				}

				if (!IS_IMMORTAL(ch) && ch->mana < mana)
				{
					act("You don't have enough mana for $N.", ch, NULL, victim, TO_CHAR);
					wait_state( ch, skill_table[sn].beats );
					pop_call();
					return;
				}
				if (!IS_IMMORTAL(ch) && number_percent() > learned(ch, sn))
				{
					act("You lost your concentration on $N.", ch, NULL, victim, TO_CHAR);
					ch->mana -= mana / 2;
					continue;
				}

				if (!IS_IMMORTAL(ch))
				{
					ch->mana -= mana;
				}

				level = magic_modify(ch, victim, sn, level);

				(*skill_table[sn].spell_fun) (sn, level, ch, victim, target);

				check_improve(ch, sn);

				if (victim != ch && victim->fighting == NULL)
				{
					make_char_fight_char( ch, victim );
					multi_hit( victim, ch, TYPE_UNDEFINED );
				}
				break;

			case TAR_CHAR_DEFENSIVE:

				if (!is_same_group(ch, victim))
				{
					continue;
				}

				if (is_safe_magic(ch, victim, sn))
				{
					continue;
				}

				if (!IS_IMMORTAL(ch) && ch->mana < mana)
				{
					act("You don't have enough mana for $N.",ch,NULL,victim,TO_CHAR);
					wait_state( ch, skill_table[sn].beats );
					pop_call();
					return;
				}

				if (!IS_IMMORTAL(ch) && number_percent() > learned(ch, sn))
				{
					act("You lost your concentration on $N.",ch,NULL,victim,TO_CHAR);
					ch->mana -= mana / 2;
					continue;
				}
				if (!IS_IMMORTAL(ch))
				{
    					ch->mana -= mana;
    				}

				level = magic_modify(ch, victim, sn, level);

				(*skill_table[sn].spell_fun) ( sn, level, ch, victim, target );

				check_improve(ch, sn);

				break;

		}
	}
	if (IS_AFFECTED(ch, AFF2_QUICKEN))
	{
		wait_state( ch, skill_table[sn].beats * 2 * 4 / 5);
	}
	else
	{
		wait_state( ch, skill_table[sn].beats * 2);
	}
	check_improve(ch, gsn_mass);

	pop_call();
	return;
}

/*
	Finds target in adjusting room and casts the spell on it for
	double mana cost.
*/

void do_range_cast( CHAR_DATA *ch, char *argument )
{
	char *arg1, buf1[MAX_INPUT_LENGTH];
	char *arg2, buf2[MAX_INPUT_LENGTH];
	char buf3[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *temp_room;
	EXIT_DATA *pexit;
	CHAR_DATA *victim, *next_victim;
	void *vo;
	int mana, sn, dir, level, target;
	long long actn;

	push_call("do_range_cast(%p,%p)",ch,argument);

	if (!IS_NPC(ch) && multi(ch, gsn_range_cast) == -1)
	{
		send_to_char("Better leave the envisioning to the rangecasters.\n\r", ch);
		pop_call();
		return;
	}

	if (strlen( argument) > 120 )
	{
		*(argument+120)='\0';
	}

	target_name = one_argument( argument, buf1 );
	target_name = one_argument( target_name, buf2 );
	target_name = one_argument( target_name, buf3 );

	/*
		Get proper room for given direction
	*/

	if ((dir = direction_door(buf1)) == -1)
	{
		if ((dir = direction_door(buf2)) == -1)
		{
			if ((dir = direction_door(buf3)) == -1)
			{
				send_to_char("Range cast which direction?\n\r",ch);
				pop_call();
				return;
			}

			/* buf3 is direction */

			arg1=buf1;
			arg2=buf2;
//			arg3=buf3;
		}

		/* buf2 is direction */

		arg1=buf1;
		arg2=buf3;
//		arg3=buf2;
	}
	else
	{
		/* buf1 is direction */

		arg1=buf2;
		arg2=buf3;
//		arg3=buf1;
	}

	pexit = get_exit(ch->in_room->vnum, dir);

	if (pexit == NULL
	||  room_index[pexit->to_room]->area != ch->in_room->area
	||  IS_SET(ch->in_room->exit[dir]->flags, EX_CLOSED))
	{
		send_to_char( "You can't range cast in that direction!\n\r", ch );
		pop_call();
		return;
	}

	for (victim = room_index[pexit->to_room]->first_person ; victim ; victim = victim->next_in_room)
	{
		if (IS_NPC(victim) && (victim->pIndexData->pShop != NULL))
		{
			send_to_char( "You can't range cast into a shop!\n\r", ch );
			pop_call();
			return;
		}
	}

	if (arg1[0] == '\0')
	{
		send_to_char( "Cast which what where?\n\r", ch );
		pop_call();
		return;
	}

	sn = skill_lookup( arg1 );

	if (sn < 0 || !is_spell(sn))
	{
		if (!IS_NPC(ch) && ch->desc == NULL)
		{
			log_printf("[%u] rangecast unknown spell: %s", ch->pIndexData->vnum, arg1);
		}
		send_to_char( "That is not a spell.\n\r", ch );
		pop_call();
		return;
	}

	if ((level = can_cast_spell(ch, sn)) == -1)
	{
		pop_call();
		return;
	}

	if (is_safe_magic(ch, NULL, sn))
	{
		pop_call();
		return;
	}

	target = skill_table[sn].target;

	switch (target)
	{
		case TAR_OBJ_CHAR_DEF:
			target = TAR_CHAR_DEFENSIVE;
			break;
		case TAR_OBJ_CHAR_OFF:
			target = TAR_CHAR_OFFENSIVE;
			break;
	}

	if (IS_SET(room_index[pexit->to_room]->room_flags, ROOM_SAFE) && (target != TAR_CHAR_DEFENSIVE))
	{
		send_to_char( "You can't range cast in that direction!\n\r", ch );
		pop_call();
		return;
	}

	mana = 3 * get_mana(ch, sn);

	/*
		Locate targets.
	*/

	victim	= NULL;
	vo		= NULL;

	temp_room = ch->in_room;

	switch ( target )
	{
		default:
			bug( "do_range_cast: bad target for sn %d.", sn );
			pop_call();
			return;

		case TAR_CHAR_SELF:
		case TAR_OBJ_INV:
			send_to_char("That type of target is not allowed at range.\n\r",ch);
			pop_call();
			return;

		case TAR_IGNORE:
			break;

		case TAR_CHAR_OFFENSIVE:

			if (arg2[0] == '\0')
			{
				send_to_char( "Range cast whom?\n\r", ch );
				pop_call();
				return;
			}

			ch->in_room = room_index[pexit->to_room];

			if ((victim = get_char_room(ch, arg2)) == NULL)
			{
				send_to_char( "They aren't there.\n\r", ch );
				ch->in_room = temp_room;

				pop_call();
				return;
			}
			ch->in_room = temp_room;

			if (victim == ch)
			{
				send_to_char( "They aren't there.\n\r", ch );
				pop_call();
				return;
			}

			if (is_safe_magic(ch, victim, sn))
			{
				pop_call();
				return;
			}
			vo = (void *) victim;
			break;

		case TAR_CHAR_DEFENSIVE:
			if (arg2[0] == '\0')
			{
				send_to_char( "Range cast whom?\n\r", ch );
				pop_call();
				return;
			}

			ch->in_room = room_index[pexit->to_room];

			if ((victim = get_char_room( ch, arg2 ) ) == NULL)
			{
				send_to_char( "They aren't there.\n\r", ch );
				ch->in_room = temp_room;

				pop_call();
				return;
			}
			ch->in_room = temp_room;

			if (victim == ch)
			{
				send_to_char( "They aren't there.\n\r", ch );
				pop_call();
				return;
			}

			if (is_safe_magic(ch, victim, sn))
			{
				pop_call();
				return;
			}

			vo = (void *) victim;
			break;
	}

	if (!IS_IMMORTAL(ch) && ch->mana < mana)
	{
		send_to_char( "You don't have enough mana.\n\r", ch );
		pop_call();
		return;
	}

	if (number_percent() > learned(ch, gsn_range_cast))
	{
		send_to_char("You envisioned the area but failed to remain focussed.\n\r",ch);
		wait_state(ch, skill_table[sn].beats/2);
		pop_call();
		return;
	}

	wait_state( ch, skill_table[sn].beats);

	send_to_char("You close your eyes, envision the area, and cast!\n\r",ch);

	if (!IS_IMMORTAL(ch) && number_percent() > learned(ch, sn))
	{
		send_to_char( "You lost your concentration.\n\r", ch );
		ch->mana -= mana / 2;
		pop_call();
		return;
	}

	if (!IS_IMMORTAL(ch))
	{
		ch->mana -= mana;
	}

	char_from_room(ch);
	char_to_room(ch, pexit->to_room);

	/* turn off autosac, autoloot */

	actn = ch->act;
	REMOVE_BIT(ch->act, PLR_AUTOSAC|PLR_AUTOLOOT|PLR_AUTO_SPLIT);

	level = magic_modify(ch, victim, sn, level);

	(*skill_table[sn].spell_fun) ( sn, level, ch, vo, target );

	check_improve(ch, sn);

	if (target == TAR_CHAR_OFFENSIVE)
	{
		make_char_fight_char( ch, victim );
	}

	ch->act = actn;

	char_from_room(ch);
	char_to_room(ch, temp_room->vnum);

	/*
		Move some creatures into room of caster
	*/

	if (target == TAR_CHAR_DEFENSIVE)
	{
		pop_call();
		return;
	}

	if (target == TAR_IGNORE)
	{
		if (skill_table[sn].position != POS_FIGHTING)
		{
			pop_call();
			return;
		}
	}

	for (victim = room_index[pexit->to_room]->last_person ; victim ; victim = next_victim)
	{
		next_victim = victim->prev_in_room;

		if (victim->fighting != NULL && victim->fighting->who != ch)
		{
			continue;
		}

		if (victim->fighting == NULL && number_bits(2) != 0)
		{
			continue;
		}

		if (!IS_NPC(victim) || !IS_IMMORTAL(victim) || IS_AFFECTED(victim, AFF_ETHEREAL) || number_bits(3) == 0)
		{
			continue;
		}

		char_from_room(victim);
		char_to_room(victim, ch->in_room->vnum);

		act("$n arrives, glaring around angrily!", victim, NULL, NULL, TO_ROOM);
		if (!IS_NPC(ch) && victim->npcdata->pvnum_last_hit == ch->pcdata->pvnum)
		{
			make_char_fight_char( ch, victim );
			multi_hit( victim, ch, TYPE_UNDEFINED );
		}
	}
	pop_call();
	return;
}

/*
	Returns -1 if one cannot cast the spell, otherwise it Returns the
	spell level - Scandum 01/04/2002
*/

int can_cast_spell( CHAR_DATA *ch, int sn )
{
	int level;

	push_call("can_cast_spell(%p,%p)",ch,sn);

	if (IS_NPC(ch))
	{
		level = UMAX(ch->level * 3 / 4, 1);

		pop_call();
		return(level);
	}

	if (!IS_IMMORTAL(ch))
	{
		if (!IS_SET(skill_table[sn].race, 1 << ch->race) && multi(ch, sn) == -1)
		{
			send_to_char( "You cannot cast that spell.\n\r", ch );
			pop_call();
			return(-1);
		}
	}

	if (IS_IMMORTAL(ch) || IS_SET(skill_table[sn].race, 1 << ch->race))
	{
		level = ch->level;
	}
	else
	{
		level = multi_spell_level(ch, sn);

		level = level * ch->pcdata->learned[sn] / 100 + 1;
	}

	/*
		Added SFOL check and bonus based on int of caster, ranging from
		103% to 139% for a 0 to 10 reinc lvl 95 human - Scandum 30-03-2002
	*/

	if (IS_IMMORTAL(ch) || (skill_table[sn].follower == SFOL_NONE)
	|| (skill_table[sn].follower & SFOL_CHAOS && ch->pcdata->god == GOD_CHAOS)
	|| (skill_table[sn].follower & SFOL_ORDER && ch->pcdata->god == GOD_ORDER))
	{
		if (get_curr_int(ch) > 28)
		{
				level = level * get_curr_int(ch) / 28;
		}
	}
	else
	{
		send_to_char( "You lack the required skill with the primordial force to cast that spell.\n\r", ch );

		pop_call();
		return(-1);
	}
	pop_call();
	return(level);
}

/*
	Returns TRUE and gives appropriate spams if one cannot cast a spell
	on the victim, if victim is NULL only room and ch based stuff is
	checked - Scandum 01-04-2002
*/

bool is_safe_magic( CHAR_DATA *ch, CHAR_DATA *victim, int sn)
{
	push_call("is_safe_magic(%p,%p)",ch,victim);

	if (is_affected(ch, gsn_anti_magic_shell))
	{
		if ((victim && victim != ch) || skill_table[sn].target == TAR_IGNORE)
		{
			send_to_char("The flows of magic shatter against the shell surrounding you.\n\r", ch);
			pop_call();
			return TRUE;
		}
	}

	if (find_keeper(ch) != NULL)
	{
		ch_printf(ch, "%s%s tells you 'No magic in here! You punk!'\n\r", get_color_string(ch,COLOR_SPEACH,VT102_BOLD), capitalize(find_keeper(ch)->short_descr));
		pop_call();
		return TRUE;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		if (skill_table[sn].target == TAR_IGNORE)
		{
			if (skill_table[sn].position == POS_FIGHTING)
			{
				send_to_char( "You cannot do that here.\n\r", ch);
				pop_call();
				return TRUE;
			}
		}
		if (skill_table[sn].target == TAR_CHAR_OFFENSIVE
		||  skill_table[sn].target == TAR_OBJ_CHAR_OFF)
		{
			send_to_char( "You cannot do that here.\n\r", ch);
			pop_call();
			return TRUE;
		}
	}

	if (ch->position < skill_table[sn].position)
	{
		send_to_char( "You can't concentrate enough.\n\r", ch );
		pop_call();
		return TRUE;
	}

	if (victim == NULL)
	{
		pop_call();
		return FALSE;
	}

	if (victim == ch)
	{
		pop_call();
		return FALSE;
	}

	if (is_affected(victim, gsn_anti_magic_shell))
	{
		act("The flows of magic shatter against the shell surrounding $N.", ch, NULL, victim, TO_CHAR);
		pop_call();
		return TRUE;
	}

	if (ch->fighting != NULL && ch->fighting->who == victim)
	{
		pop_call();
		return FALSE;
	}

	if (ch->in_room->area->low_r_vnum == ROOM_VNUM_ARENA)
	{
		pop_call();
		return FALSE;
	}

	switch (skill_table[sn].target)
	{
		default:

			bug( "is_safe_magic: bad target for sn %d.", sn );
			pop_call();
			return TRUE;
			break;

		case TAR_CHAR_OFFENSIVE:
		case TAR_OBJ_CHAR_OFF:
			if (IS_NPC(ch) && ch->fighting == NULL && ch->hit < ch->max_hit / 2)
			{
				if (ch->desc == NULL)
				{
					pop_call();
					return TRUE;
				}
			}

			if (ch->desc)
			{
				if (!IS_NPC(victim))
				{
					send_to_char( "You must MURDER a player.\n\r", ch);
					pop_call();
					return TRUE;
				}
			}

			if (!IS_NPC(ch))
			{
				ch->pcdata->just_died_ctr = 0;
			}

			if (IS_NPC(victim) && IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL)
			{
				if (victim->master == ch)
				{
					raw_kill( victim );
					pop_call();
					return TRUE;
				}
			}

			if (ch->fighting == NULL || ch->fighting->who != victim )
			{
				if (!IS_NPC(ch) && IS_NPC(victim) && IS_AFFECTED(victim, AFF_CHARM))
				{
					send_to_char( "You cannot kill a slave.\n\r", ch);
					pop_call();
					return TRUE;
				}
			}

			if (IS_NPC(victim) && victim->fighting != NULL
			&& !IS_NPC(ch) && !is_same_group(ch, victim->fighting->who))
			{
				ch_printf(ch, "%s seems to be busy!\n\r", capitalize(get_name(victim)));
				pop_call();
				return TRUE;
			}

			if (check_recently_fought(ch, victim))
			{
				ch_printf(ch, "%s was recently fought.  Try later.\n\r", capitalize(get_name(victim)));
				pop_call();
				return TRUE;
			}
			break;

		case TAR_CHAR_DEFENSIVE:
		case TAR_OBJ_CHAR_DEF:

			if (!IS_NPC(ch) && victim->fighting != NULL)
			{
				if (!IS_NPC(victim) && !IS_NPC(victim->fighting->who))
				{
					ch_printf(ch ,"%s seems to be busy!\n\r", capitalize(get_name(victim)));
					pop_call();
					return TRUE;
				}
				if (!is_same_group(ch, victim))
				{
					ch_printf(ch ,"%s seems to be busy!\n\r", capitalize(get_name(victim)));
					pop_call();
					return TRUE;
				}
			}
			break;
	}
	pop_call();
	return FALSE;
}

/*
	true if victim is a legal target for a mass attack - Scandum 01/04/2002
*/

bool can_mass_cast( CHAR_DATA *ch, CHAR_DATA *victim )
{
	push_call("can_mass_cast(%p,%p)",ch,victim);

	if (victim == ch)
	{
		pop_call();
		return FALSE;
	}

	if (is_affected(victim, gsn_anti_magic_shell))
	{
		act("The flows of magic shatter against a shell surrounding $N.", ch, NULL, victim, TO_CHAR);
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

	if (IS_NPC(ch) && ch->desc && !IS_NPC(victim))
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

void do_mana( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	int sn;

	push_call("do_mana(%p,%p)",ch,argument);

	if (*argument == '\0')
	{
		send_to_char("Usage: mana <spell name>.\n\r", ch);
		pop_call();
		return;
	}

	sn = skill_lookup( argument );
	if (sn < 0 || !is_spell(sn))
	{
		send_to_char( "That is not a spell.\n\r", ch );
		pop_call();
		return;
	}

	if ((can_cast_spell(ch, sn)) == -1)
	{
		pop_call();
		return;
	}

	sprintf( buf, "Mana usage for '%s' is: %d\n\r",
	skill_table[sn].name, get_mana( ch, sn));
	send_to_char( buf, ch);
	pop_call();
	return;
}

int get_mana( CHAR_DATA *ch, int sn)
{
	int mana,mnx,cnt;

	push_call("get_mana(%p,%p)",ch,sn);

	if (IS_NPC(ch) || IS_SET(skill_table[sn].race, 1 << ch->race))
	{
		mana = skill_table[sn].cost;
	}
	else
	{
		cnt = multi(ch, sn);

		mnx = skill_table[sn].skill_level[cnt] / 2 + 1;

		mana = 50 * mnx / (ch->pcdata->mclass[cnt] + mnx - skill_table[sn].skill_level[cnt]) - (get_curr_wis(ch)-13) / 2;

		mana = UMAX(skill_table[sn].cost, mana);
	}

	if (IS_AFFECTED(ch, AFF2_QUICKEN))
	{
		mana += mana;
	}

	pop_call();
	return(mana);
}

void do_move( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	int sn, cnt, move;

	push_call("do_move(%p,%p)",ch,argument);

	sn = skill_lookup( argument );
	if( sn < 0 || is_spell(sn))
	{
		send_to_char( "That doesn't cost any movement.\n\r", ch );
		pop_call();
		return;
	}

	cnt = multi( ch, sn);

	if(cnt==-1 && !IS_NPC(ch))
	{
		send_to_char( "You can't do that.\n\r", ch );
		pop_call();
		return;
	}
	move = IS_NPC(ch) ? 0 : skill_table[sn].cost;

	sprintf(buf,"Movement usage for '%s' is: %d\n\r",skill_table[sn].name,move);
	send_to_char(buf,ch);
	pop_call();
	return;
}

bool is_spell(int sn)
{
	push_call("is_spell(%p)",sn);

	if (sn > 0 && sn < MAX_SKILL && IS_SET(skill_table[sn].flags, FSKILL_SPELL))
	{
		pop_call();
  		return TRUE;
  	}

	pop_call();
	return FALSE;
}

int caster_levels( CHAR_DATA *ch )
{
	int levels;

	push_call("caster_levels(%p)",ch);

	if (IS_NPC(ch))
	{
		pop_call();
		return( ch->level );
	}

	levels  = ch->pcdata->mclass[CLASS_ELEMENTALIST];
	levels += ch->pcdata->mclass[CLASS_ILLUSIONIST];
	levels += ch->pcdata->mclass[CLASS_MONK];
	levels += ch->pcdata->mclass[CLASS_NECROMANCER];
	/*
		Non specialist caster-levels count only half	21/10/2000 - Manwe
	*/
	levels += ch->pcdata->mclass[CLASS_RANGER]    / 2;
	levels += ch->pcdata->mclass[CLASS_MARAUDER]  / 2;
	levels += ch->pcdata->mclass[CLASS_GLADIATOR] / 2;
	levels += ch->pcdata->mclass[CLASS_NINJA]     / 2;

	if (levels > ch->level)
	{
		levels = ch->level;
	}
	pop_call();
	return( levels );
}

int magic_modify( CHAR_DATA *ch, CHAR_DATA *victim, int sn, int level )
{
	int dam_type = DAM_THRUST;

	push_call("damage_modify(%p,%p,%p,%p)",ch,victim,sn,level);

	if (level <= 0)
	{
		pop_call();
		return 0;
	}

	if (victim == NULL)
	{
		pop_call();
		return level;
	}

	switch (skill_table[sn].target)
	{
		case TAR_CHAR_DEFENSIVE:
		case TAR_OBJ_CHAR_DEF:
			if (!IS_IMMORTAL(ch) && victim != ch)
			{
				if (level > victim->level * 2)
				{
					level = victim->level * 2;
				}
			}
			break;
	}

	dam_type = skill_table[sn].dam_type;

	if (dam_type == DAM_NONE)
	{
		pop_call();
		return level;
	}

	if (dam_type == god_table[which_god(victim)].resistance)
	{
		level = level * 75 / 100;
	}

	if (IS_UNDEAD(victim))
	{
		if (IS_SET(DAM_LIFE|DAM_POISON|DAM_EVIL, dam_type))
		{
			level = level * 75 / 100;
		}
	}

	if (IS_SET(race_table[victim->race].res_low, dam_type))
	{
		level = level * 90 / 100;
	}

	if (IS_SET(race_table[victim->race].res_high, dam_type))
	{
		level = level * 80 / 100;
	}

	if (IS_SET(race_table[victim->race].vul_low, dam_type))
	{
		level = level * 110 / 100;
	}

	if (IS_SET(race_table[victim->race].vul_high, dam_type))
	{
		level = level * 120 / 100;
	}

	level = UMAX(1, level);

	pop_call();
	return level;
}

int multi_spell_level( CHAR_DATA *ch, int sn)
{
	int cnt, mlv;

	push_call("multi_spell_level(%p,%p)",ch,sn);

	mlv = 0;

	if (IS_NPC(ch))
	{
		mlv = ch->level;
		pop_call();
		return(mlv);
	}

	for (cnt = 0 ; cnt < MAX_CLASS ; cnt++)
	{
		if (skill_table[sn].skill_level[cnt] < MAX_LEVEL)
		{
			mlv += ch->pcdata->mclass[cnt];
		}
		else if (cnt >= 4)
		{
			mlv += ch->pcdata->mclass[cnt] * 3 / 4;
		}
		else
		{
			mlv += ch->pcdata->mclass[cnt] * 1 / 4;
		}
	}

	if (mlv > ch->level)
	{
		mlv = ch->level;
	}

	mlv += ch->pcdata->reincarnation;

	pop_call();
	return(mlv);
}

ROOM_INDEX_DATA *get_rift_room( CHAR_DATA *ch, int rift_number)
{
	ROOM_INDEX_DATA *rift_room;

	push_call("get_rift_room(%p,%p)",ch,rift_number);

	number_seed(value % 50 * 10000 + ch->pcdata->pvnum + 1);

	while (TRUE)
	{
		rift_room = room_index[number_range(100, MAX_VNUM-1)];

		if (rift_room == NULL)
		{
			continue;
		}
		if (IS_SET(rift_room->room_flags, ROOM_SAFE))
		{
			continue;
		}
		if (IS_SET(rift_room->room_flags, ROOM_IS_CASTLE))
		{
			continue;
		}
		if (IS_SET(rift_room->room_flags, ROOM_PET_SHOP))
		{
			continue;
		}
		if (IS_SET(rift_room->room_flags, ROOM_PRIVATE))
		{
			continue;
		}
		if (IS_SET(rift_room->room_flags, ROOM_SOLITARY))
		{
			continue;
		}
		if (IS_SET(rift_room->room_flags, ROOM_NO_RECALL))
		{
			continue;
		}
		if (rift_room->sector_type == SECT_ETHEREAL)
		{
			continue;
		}
		if (rift_room->sector_type == SECT_ASTRAL)
		{
			continue;
		}
		if (IS_SET(rift_room->area->flags, AFLAG_NOTELEPORT))
		{
			continue;
		}
		if (IS_SET(rift_room->area->flags, AFLAG_NORECALL))
		{
			continue;
		}
		if (IS_SET(rift_room->area->flags, AFLAG_NORIP))
		{
			continue;
		}

		break;
	}
	pop_call();
	return rift_room;
}

/*
	Spell functions.
*/

DO_SPELL(spell_acid_blast)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_acid_blast(%p,%p,%p,%p)",sn,level,ch,vo);

	dam = number_range(level*3+50, level*5+100);

	if ( saves_spell( level, ch, victim ) )
	{
		dam /= 2;
	}
	damage( ch, victim, dam, sn );

	pop_call();
	return;
}

DO_SPELL(spell_armor)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_armor(%p,%p,%p,%p)",sn,level,ch,vo);

	if ( is_affected( victim, sn ) )
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}

	af.type = sn;

	if (multi(ch, sn) != -1)
	{
		af.duration  = 48;
		af.modifier  = -15;
	}
	else
	{
		af.duration  = 24;
		af.modifier  = -5;
	}
	af.location  = APPLY_AC;
	af.bittype   = AFFECT_TO_NONE;
	af.bitvector = 0;
	affect_to_char( victim, &af );

	act( "$n's armor begins to glow softly as it is enhanced by a cantrip.", victim, NULL, NULL, TO_ROOM );
	send_to_char( "Your armor begins to glow softly as it is enhanced by a cantrip.\n\r", victim );

	pop_call();
	return;
}

DO_SPELL(spell_bless)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_bless(%p,%p,%p,%p)",sn,level,ch,vo);

	if( is_affected( victim, sn ) )
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}
	af.type      = sn;
	af.duration  = 6+level/2;
	af.bittype   = AFFECT_TO_NONE;
	af.bitvector = 0;

	af.location  = APPLY_HITROLL;
	af.modifier  = level/9;
	affect_to_char( victim, &af );

	af.location  = APPLY_SAVING_SPELL;
	af.modifier  = 0 - level / 8;
	affect_to_char( victim, &af );

	act( "A powerful blessing is laid upon $n.", victim, NULL, NULL, TO_ROOM );
	send_to_char( "A powerful blessing is laid upon you.\n\r", victim );
	pop_call();
	return;
}

DO_SPELL(spell_blindness)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_blindness(%p,%p,%p,%p)",sn,level,ch,vo);

	if ( IS_AFFECTED(victim, AFF_BLIND) )
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}

	if (IS_AFFECTED(victim, AFF_TRUESIGHT ))
	{
		act( "$N sees through the veil on reality you try to impose.", ch, NULL, victim, TO_CHAR );
		act( "You see through the veil on reality $n attempts to impose.", ch, NULL, victim, TO_VICT );
		pop_call();
		return;
	}

	if ( saves_spell( level, ch, victim ) )
	{
		send_to_char( "Nothing happens.\n\r", ch );
		pop_call();
		return;
	}

	af.type      = sn;
	af.location  = APPLY_HITROLL;
	af.modifier  = -2*level/5;
	af.duration  = 1+level/5;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF_BLIND;
	affect_to_char( victim, &af );

	send_to_char( "You are blinded!\n\r", victim );

	if ( ch != victim )
	{
		send_to_char( "Your foe is now blinded.\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_burning_hands)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_burning_hands(%p,%p,%p,%p)",sn,level,ch,vo);

	dam = 50 + level + number_range(0, 50);

	if (saves_spell(level, ch, victim))
	{
		dam /= 2;
	}
	damage(ch, victim, dam, sn);

	pop_call();
	return;
}

DO_SPELL(spell_call_lightning)
{
	CHAR_DATA *vch, *vch_next;
	int dam, door, temp_vnum;
	EXIT_DATA *pexit;

	push_call("spell_call_lightning(%p,%p,%p,%p)",sn,level,ch,vo);

	if ( !IS_OUTSIDE(ch) )
	{
		send_to_char( "You must be out of doors.\n\r", ch );
		pop_call();
		return;
	}

	if (NO_WEATHER_SECT(ch->in_room->sector_type))
	{
		send_to_char( "You must be out of doors.\n\r", ch );
		pop_call();
		return;
	}

	if ( ch->in_room->area->weather_info->sky < SKY_RAINING )
	{
		send_to_char( "You need bad weather.\n\r", ch );
		pop_call();
		return;
	}

	send_to_char( "Elemental lightning strikes your foes!\n\r", ch );
	act( "$n calls elemental lightning to strike $s foes!", ch, NULL, NULL, TO_ROOM );

	for (vch = ch->in_room->last_person ; vch ; vch = vch_next)
	{
		vch_next = vch->prev_in_room;

		if (!can_mass_cast(ch, vch))
		{
			continue;
		}
		dam = number_range(level*1, level*4);
		damage( ch, vch, saves_spell(level, ch, vch) ? dam/2 : dam, sn );
	}

	temp_vnum = ch->in_room->vnum;

	for (door = 0 ; door < 6 ; door++)
	{
		if ((pexit = get_exit(temp_vnum, door)) && pexit->to_room != temp_vnum)
		{
			ch->in_room = room_index[pexit->to_room];
			act("Lightning flashes in the sky.", ch, NULL, NULL, TO_ROOM );
		}
	}
	ch->in_room = room_index[temp_vnum];

	pop_call();
	return;
}

DO_SPELL(spell_chain_lightning)
{
	CHAR_DATA *vch, *vch_next;
	ROOM_INDEX_DATA *temp_room = ch->in_room;
	int dam, targets;

	push_call("spell_chain_lightning(%p,%p,%p,%p)",sn,level,ch,vo);

	if (!IS_OUTSIDE(ch) || NO_WEATHER_SECT(ch->in_room->sector_type))
	{
		send_to_char( "You must be out of doors.\n\r", ch );
		pop_call();
		return;
	}

	if (ch->in_room->area->weather_info->sky < SKY_RAINING)
	{
		level = level * 2 / 3;
	}

	if (ch->in_room->area->weather_info->sky < SKY_LIGHTNING)
	{
		level = level * 2 / 3;
	}

	act( "You call forth a bolt of chain lightning!", ch, NULL, NULL, TO_CHAR );
	act( "$n calls forth a bolt of chain lightning!", ch, NULL, NULL, TO_ROOM );

	while (level > 0)
	{
		for (targets = 0, vch = ch->in_room->last_person ; vch ; vch = vch_next)
		{
			vch_next = vch->prev_in_room;

			if (!can_mass_cast(ch, vch))
			{
				continue;
			}

			dam     = saves_spell(level, ch, vch) ? dice(level, 3) : dice(level, 6);
			level  -= 20;
			targets += 1;

			switch (number_bits(2))
			{
				case 0:
					act("The searing bolt of chain lightning arcs to you.", vch, NULL, NULL, TO_CHAR);
					act("The searing bolt of chain lightning arcs to $n.",  vch, NULL, NULL, TO_ROOM);
					break;
				case 1:
					act("The sizzling bolt of chain lightning arcs to you.", vch, NULL, NULL, TO_CHAR);
					act("The sizzling bolt of chain lightning arcs to $n.",  vch, NULL, NULL, TO_ROOM);
					break;
				case 2:
					act("The crackling bolt of chain lightning arcs to you.", vch, NULL, NULL, TO_CHAR);
					act("The crackling bolt of chain lightning arcs to $n.",  vch, NULL, NULL, TO_ROOM);
					break;
				case 3:
					act("The fiery bolt of chain lightning arcs to you.", vch, NULL, NULL, TO_CHAR);
					act("The fiery bolt of chain lightning arcs to $n.",  vch, NULL, NULL, TO_ROOM);
					break;
			}
			damage( ch, vch, dam, sn );

			if (level <= 0)
			{
				break;
			}
		}
		if (level > 0)
		{
			if (targets == 1)
			{
				vch     = ch;
				dam     = saves_spell(level, ch, vch) ? dice(level, 6) : dice(level, 3);
				level  -= 20;

				switch (number_bits(2))
				{
					case 0:
						act("The searing bolt of chain lightning arcs to you.", vch, NULL, NULL, TO_CHAR);
						act("The searing bolt of chain lightning arcs to $n.",  vch, NULL, NULL, TO_ROOM);
						break;
					case 1:
						act("The sizzling bolt of chain lightning arcs to you.", vch, NULL, NULL, TO_CHAR);
						act("The sizzling bolt of chain lightning arcs to $n.",  vch, NULL, NULL, TO_ROOM);
						break;
					case 2:
						act("The crackling bolt of chain lightning arcs to you.", vch, NULL, NULL, TO_CHAR);
						act("The crackling bolt of chain lightning arcs to $n.",  vch, NULL, NULL, TO_ROOM);
						break;
					case 3:
						act("The fiery bolt of chain lightning arcs to you.", vch, NULL, NULL, TO_CHAR);
						act("The fiery bolt of chain lightning arcs to $n.",  vch, NULL, NULL, TO_ROOM);
						break;
				}
				damage( ch, vch, dam, sn );
			}

			if (ch->in_room != temp_room)
			{
				send_to_room("The bolt of chain lightning loses power and fizzles out.\n\r", temp_room);
				pop_call();
				return;
			}

			if (targets == 0)
			{
				act("You catch the bolt of lightning and send it back into the sky.",   ch, NULL, NULL, TO_CHAR);
				act("$n catches the bolt of lightning and sends it back into the sky.", ch, NULL, NULL, TO_ROOM);
				pop_call();
				return;
			}
		}
	}

	switch (number_bits(2))
	{
		case 0: send_to_room("The searing bolt of chain lightning loses power and fizzles out.\n\r",          temp_room); break;
		case 1: send_to_room("The sizzling bolt of chain lightning arcs to the ground and dissipates.\n\r",   temp_room); break;
		case 2: send_to_room("The crackling arc of chain lightning strikes the ground and vanishes.\n\r",     temp_room); break;
		case 3: send_to_room("The fiery bolt of chain lightning loses power before reaching its target.\n\r", temp_room); break;
	}

	pop_call();
	return;
}

DO_SPELL(spell_cause_light)
{
	push_call("spell_cause_light(%p,%p,%p,%p)",sn,level,ch,vo);

	damage( ch, (CHAR_DATA *) vo, dice(1, 15) + level / 2, sn );

	pop_call();
	return;
}

DO_SPELL(spell_cause_serious)
{
	push_call("spell_cause_serious(%p,%p,%p,%p)",sn,level,ch,vo);

	damage( ch, (CHAR_DATA *) vo, dice(1, 25) + level * 2 / 3, sn );

	pop_call();
	return;
}

DO_SPELL(spell_cause_critical)
{
	push_call("spell_cause_critical(%p,%p,%p,%p)",sn,level,ch,vo);

	damage( ch, (CHAR_DATA *) vo, dice(1, 50) + level, sn );

	pop_call();
	return;
}

DO_SPELL(spell_change_sex)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_change_sex(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_char("You are forbidden from casting that here.\n\r", ch);
		pop_call();
		return;
	}
	if ( is_affected( victim, sn ) )
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}
	af.type      = sn;
	af.duration  = level/4;
	af.location  = APPLY_SEX;

	if (victim->sex <= 0)
	{
		if (number_bits(1) == 0)
		{
			af.modifier  = 1;
		}
		else
		{
			af.modifier  = 2;
		}
	}
	else if (victim->sex == 1)
	{
		if (number_bits(1) == 0)
		{
			af.modifier  = 1;
		}
		else
		{
			af.modifier  = -1;
		}
	}
	else if (victim->sex >= 2)
	{
		if (number_bits(1) == 0)
		{
			af.modifier  = -1;
		}
		else
		{
			af.modifier  = -2;
		}
	}
	af.bittype   = AFFECT_TO_NONE;
	af.bitvector = 0;
	affect_to_char( victim, &af );

	send_to_char( "You writhe in agony as your gender changes.\n\r", victim );
	act( "$n writhes in agony as $s gender changes.", victim, NULL, NULL, TO_ROOM);

	pop_call();
	return;
}

DO_SPELL(spell_charm_person)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_charm_person(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_char("You are forbidden from casting that here.\n\r", ch);
		pop_call();
		return;
	}

	if (victim == ch)
	{
		send_to_char( "You like yourself even better!\n\r", ch );
		pop_call();
		return;
	}

	if (!IS_NPC(victim) || !can_mass_cast(ch, victim))
	{
		send_to_char( "They seem to be unaffected by your magic.\n\r", ch );
		pop_call();
		return;
	}

	if (IS_AFFECTED(victim, AFF_CHARM))
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}

	if (level / 3 < victim->level || saves_spell(level, ch, victim) || get_pets(ch) >= 2)
	{
		do_say(victim, "I don't want to be charmed!");
		act( "$n suddenly looks VERY angry!",victim,NULL,NULL,TO_ROOM);

		if (victim->fighting == NULL)
		{
			make_char_fight_char( ch, victim );
			multi_hit(victim, ch, TYPE_UNDEFINED);
		}
		pop_call();
		return;
	}

	if ( victim->master )
	{
		stop_follower( victim );
	}

	add_follower( victim, ch );
	af.type      = sn;
	af.duration  = number_fuzzy( level / 4 );
	af.location  = 0;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF_CHARM;
	affect_to_char( victim, &af );

	act( "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );
	act( "$N looks at you with adoring eyes.", ch, NULL, victim, TO_CHAR );
	act( "$N looks at $n with adoring eyes.",  ch, NULL, victim, TO_NOTVICT );
	pop_call();
	return;
}

DO_SPELL(spell_chill_touch)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	int dam;

	push_call("spell_chill_touch(%p,%p,%p,%p)",sn,level,ch,vo);

	dam = 20 + level + number_range(0, 130);

	if (saves_spell(level, ch, victim))
	{
		dam /= 2;
	}
	else if (number_bits(1) == 0)
	{
		AFFECT_DATA af;

		af.type      = sn;
		af.duration  = 1;
		if (!IS_NPC(victim))
		{
			af.location  = APPLY_STR;
		}
		else
		{
			af.location  = APPLY_DAMROLL;
		}
		af.modifier  = -1;
		af.bittype   = AFFECT_TO_NONE;
		af.bitvector = 0;
		affect_join( victim, &af );
	}

	damage( ch, victim, dam, sn );
	pop_call();
	return;
}

DO_SPELL(spell_stone_fist)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	int body_part = 0;

	push_call("spell_stone_fist(%p,%p,%p,%p)",sn,level,ch,vo);

	if ( is_affected( victim, sn ) )
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}
	af.type = sn;

	af.duration  = 8;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bittype   = AFFECT_TO_NONE;
	af.bitvector = 0;
	affect_to_char( victim, &af );

	if (IS_NPC(victim) && IS_SET(victim->act, ACT_BODY))
	{
		body_part = victim->pIndexData->attack_parts;
	}
	else if (!IS_NPC(victim) || IS_SET(victim->act, ACT_RACE))
	{
		body_part = race_table[victim->race].attack_parts;
	}

	if (IS_SET(body_part, BODY_TENTACLE))
	{
		act( "$n's tentacles turn to stone.", victim, NULL, NULL, TO_ROOM );
		send_to_char( "Your tentacles turn to stone.\n\r", victim );
	}
	else if (IS_SET(body_part, BODY_CLAW))
	{
		act( "$n's claws turn to stone.", victim, NULL, NULL, TO_ROOM );
		send_to_char( "Your claws turn to stone.\n\r", victim );
	}
	else if (IS_SET(body_part, BODY_BRANCH))
	{
		act( "$n's branches turn to stone.", victim, NULL, NULL, TO_ROOM );
		send_to_char( "Your branches turn to stone.\n\r", victim );
	}
	else
	{
		act( "$n's fists turn to stone.", victim, NULL, NULL, TO_ROOM );
		send_to_char( "Your fists turn to stone.\n\r", victim );
	}
	pop_call();
	return;
}

DO_SPELL(spell_color_spray)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_color_spray)",sn,level,ch,vo);

	dam = 35 + number_range(level * 3, level * 5);

	if (saves_spell(level, ch, victim))
	{
		damage(ch, victim, dam / 2, sn);
	}
	else
	{
		/*
			Added blindness as an extra effect to this spell,
			21/10/2000 - Manwe
			Lets not make it too easy, merged the two spells into one
			01/04/2002 - Scandum
		*/

		damage (ch, victim, dam, sn);

		if (ch->in_room == victim->in_room && number_bits(3) == 0 && !IS_AFFECTED(victim, AFF_BLIND))
		{
			AFFECT_DATA af;

			af.type      = gsn_blindness;
			af.location  = APPLY_HITROLL;
			af.modifier  = -1 * level / 5;
			af.duration  = 0;
			af.bittype   = AFFECT_TO_CHAR;
			af.bitvector = AFF_BLIND;

			affect_to_char( victim, &af );

			send_to_char("You are blinded by a dazzling ray of light!\n\r", victim);

			if (ch != victim)
			{
				send_to_char("A dazzling ray of light blinds your foe.\n\r", ch);
			}
		}
	}

	pop_call();
	return;
}

DO_SPELL(spell_continual_light)
{
	OBJ_DATA *light;

	push_call("spell_continual_light(%p,%p,%p,%p)",sn,level,ch,vo);

	light = create_object( get_obj_index( OBJ_VNUM_LIGHT_BALL ), 0 );
	light->timer = level / 2;
	light->level = level;
	act( "Shards of iridescent light collide to form a dazzling ball.", ch, NULL, NULL, TO_CHAR );
	act( "Shards of iridescent light collide to form a dazzling ball.", ch, NULL, NULL, TO_ROOM );
	obj_to_room( light, ch->in_room->vnum );
	pop_call();
	return;
}

DO_SPELL(spell_control_weather)
{
	push_call("spell_control_weather(%p,%p,%p,%p)",sn,level,ch,vo);

	if (!strcasecmp(target_name, "wet"))
	{
		ch->in_room->area->weather_info->change		+= 1 + level / 33;
	}
	else if (!strcasecmp(target_name, "dry"))
	{
		ch->in_room->area->weather_info->change		-= 1 + level / 33;
	}
	else if (!strcasecmp(target_name, "windy"))
	{
		ch->in_room->area->weather_info->wind_speed	+= 1 + level / 33;
	}
	else if (!strcasecmp(target_name, "calm"))
	{
		ch->in_room->area->weather_info->wind_speed	-= 1 + level / 33;
	}
	else
	{
		send_to_char ("Do you want the weather calm, windy, wet, or dry?\n\r", ch );
		pop_call();
		return;
	}
	send_to_char( "Ok.\n\r", ch );
	pop_call();
	return;
}

DO_SPELL(spell_feast)
{
	CHAR_DATA *fch;

	push_call("spell_feast(%p,%p,%p,%p)",sn,level,ch,vo);

	/*
		Added blood feast for undead and vampiric races - Scandum 01/04/2002
	*/

	if (rspec_req(ch, RSPEC_VAMPIRIC) || rspec_req(ch, RSPEC_UNDEAD))
	{
		act( "$n spreads $s arms and blood spurts forth from $s opened hands.",     ch, NULL, NULL, TO_ROOM );
		act( "You spread your arms and blood spurts forth from your opened hands.", ch, NULL, NULL, TO_CHAR );
		act( "Everyone in the room joins you as you drink your fill.",              ch, NULL, NULL, TO_CHAR );

		gain_condition( ch, COND_FULL,   50 );
		gain_condition( ch, COND_THIRST, 50 );

		for (fch = ch->in_room->first_person ; fch ; fch = fch->next_in_room)
		{
			if (!IS_NPC(fch) && fch->position >= POS_RESTING && fch != ch)
			{
				if (!rspec_req(fch,RSPEC_VAMPIRIC) && !rspec_req(fch,RSPEC_UNDEAD))
				{
					send_to_char( "You turn away your head in dismay as the undeads drink.\n\r", fch );
				}
				else
				{
					gain_condition( fch, COND_FULL,   50 );
					gain_condition( fch, COND_THIRST, 50 );
					send_to_char( "You join in the blood feast and drink your fill.\n\r", fch );
				}
			}
		}
	}
	else
	{
		act( "$n creates a large selection of food and drinks.",     ch, NULL, NULL, TO_ROOM );
		act( "You create a large selection of food and drinks.",     ch, NULL, NULL, TO_CHAR );
		act( "Everyone in the room joins you as you eat your fill.", ch, NULL, NULL, TO_CHAR );

		gain_condition( ch, COND_FULL, 50 );
		gain_condition( ch, COND_THIRST, 50 );

		for (fch = ch->in_room->first_person ; fch ; fch = fch->next_in_room)
		{
			if (!IS_NPC(fch) && fch->position >= POS_RESTING && fch != ch)
			{
				if (!rspec_req(fch,RSPEC_VAMPIRIC) && !rspec_req(fch,RSPEC_UNDEAD))
				{
					gain_condition( fch, COND_FULL, 50 );
					gain_condition( fch, COND_THIRST, 50 );
					send_to_char( "You join in the feast and eat your fill.\n\r", fch );
				}
				else
				{
					send_to_char( "You turn away your head in dismay as the mortals eat.\n\r", fch );
				}
			}
		}
	}
	pop_call();
	return;
}

DO_SPELL(spell_restore)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int heal, scale_top, scale_bot;
	char buf[MAX_INPUT_LENGTH];

	push_call("spell_restore(%p,%p,%p,%p)",sn,level,ch,vo);

	scale_top = 10;  /* The fraction for the hp/mana ratio */
	scale_bot = 7;

	if (value == -1 || value > ch->mana )
	{
		value = ch->mana;
	}

	heal = value * scale_top / scale_bot ;

	if (victim->hit + heal > victim->max_hit)
	{
		heal = victim->max_hit - victim->hit ;
	}

	value = heal * scale_bot / scale_top;

	sprintf( buf, "Restoring %d hitpoints for %d mana.\n\r", heal, value + get_mana(ch,sn));
	send_to_char( buf, ch);

	victim->hit += heal;
	ch->mana -= value;

	update_pos( victim );
	send_to_char( "You are filled with an overwhelming feeling of warmth.\n\r", victim );
	affect_strip(victim, gsn_critical_hit);
	if ( ch != victim )
	{
		send_to_char( "You restore them.\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_energy_shift)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int heal, scale_top, scale_bot;
	char buf[MAX_INPUT_LENGTH];

	push_call("spell_energy_shift(%p,%p,%p,%p)",sn,level,ch,vo);

	scale_top = 65;  /* The fraction for the mana_to/mana_from ratio */
	scale_bot = 100;

	if( value == -1 || value > ch->mana )
	{
		value = ch->mana;
	}

	heal = value * scale_top / scale_bot ;
	if( victim->mana + heal > victim->max_mana )
	{
		heal = victim->max_mana - victim->mana ;
	}
	value = ( heal * scale_bot / scale_top );

	sprintf( buf, "Shifting %d mana for %d mana.\n\r", heal, value + get_mana(ch,sn));
	send_to_char( buf, ch);

	victim->mana += heal;
	ch->mana -= value;

	update_pos( victim );
	send_to_char( "You feel a surge of power.\n\r", victim );
	if ( ch != victim )
	{
		send_to_char( "You transfer energy to them.\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_induction)
{
	int heal, scale_top, scale_bot;
	char buf[MAX_INPUT_LENGTH];

	push_call("spell_induction(%p,%p,%p,%p)",sn,level,ch,vo);

	scale_top = 7;  /* The fraction for the mana_to/hp_from ratio */
	scale_bot = 10;

	if (value < 0 || value > ch->hit - 10 )
	{
		value = ch->hit-10;
	}

	heal = value * scale_top / scale_bot;

	if (ch->mana + heal > ch->max_mana)
	{
		heal = ch->max_mana - ch->mana ;
     }

	if (heal < 0)
	{
		heal = 0;
	}
	value = heal * scale_bot / scale_top;

	sprintf( buf, "Inducing %d mana points for %d hitpoints.\n\r", heal, value );
	send_to_char( buf, ch);

	ch->mana += heal;
	ch->hit -= value;

	update_pos( ch );
	if (heal > 0)
	{
		send_to_char( "You feel a surge of power.\n\r", ch );
		act( "$n drains $s strength to further $s magic.", ch, NULL, NULL, TO_ROOM);
	}
	pop_call();
	return;
}

DO_SPELL(spell_create_food)
{
	OBJ_DATA *mushroom;

	push_call("spell_create_food(%p,%p,%p,%p)",sn,level,ch,vo);

	mushroom = create_object( get_obj_index( OBJ_VNUM_MUSHROOM ), 0 );
	mushroom->value[0] = 5 + level;
	act( "$p suddenly appears.", ch, mushroom, NULL, TO_ROOM );
	act( "$p suddenly appears.", ch, mushroom, NULL, TO_CHAR );
	obj_to_room( mushroom, ch->in_room->vnum );

	pop_call();
	return;
}

DO_SPELL(spell_create_spring)
{
	OBJ_DATA *spring;

	push_call("spell_create_spring(%p,%p,%p,%p)",sn,level,ch,vo);

	spring = create_object( get_obj_index( OBJ_VNUM_SPRING ), 0 );
	spring->timer = level;
	act( "Tracing a ring before you, the graceful flow of a mystical spring emerges.", ch, spring, NULL, TO_CHAR );
	act( "As $n traces a ring through the air, the graceful flow of a mystical spring emerges.", ch, spring, NULL, TO_ROOM );
	obj_to_room( spring, ch->in_room->vnum );

	pop_call();
	return;
}

DO_SPELL(spell_ice_arrow)
{
	OBJ_DATA *arrow;
	int type;

	push_call("spell_ice_arrow(%p,%p,%p,%p)",sn,level,ch,vo);

	if ((type = get_flag(target_name, bow_types)) == -1)
	{
		ch_printf(ch, "Create what kind of arrow, %s?\n\r", give_flags(bow_types));
		pop_call();
		return;
	}
	arrow = create_object(get_obj_index(OBJ_VNUM_ICE_ARROW), 0);

	arrow->value[0] = type;
	arrow->value[1] = level;
	arrow->value[2] = URANGE(10, level/4, 20);
	arrow->value[3] = 5;
	arrow->level    = level / 2;

	SET_BIT(arrow->extra_flags, ITEM_MODIFIED);

	act("$p of frigid, dangerous beauty takes shape in your hand.", ch, arrow, NULL, TO_CHAR);
	act("$p of frigid, dangerous beauty takes shape in $n's hand.", ch, arrow, NULL, TO_ROOM);

	if (ch->carry_number < can_carry_n(ch) && ch->carry_weight < can_carry_w(ch))
	{
		obj_to_char(arrow, ch);
	}
	else
	{	
		obj_to_room(arrow, ch->in_room->vnum);
	}
	pop_call();
	return;
}

DO_SPELL(spell_cure_blindness)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	push_call("spell_cure_blindness(%p,%p,%p,%p)",sn,level,ch,vo);

	if (!is_affected(victim, gsn_blindness))
	{
		if (!is_affected(victim, gsn_gouge))
		{
			if (!is_affected(victim, gsn_flash_powder))
			{
				send_to_char( "They aren't blinded.\n\r", ch );
	
				pop_call();
				return;
			}
		}
	}

	affect_strip( victim, gsn_blindness );
	affect_strip( victim, gsn_gouge );
	affect_strip( victim, gsn_flash_powder );

	send_to_char( "Your vision returns!\n\r", victim );

	if (ch != victim)
	{
		send_to_char( "They are no longer blinded.\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_cure_light)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int heal;

	push_call("spell_cure_light(%p,%p,%p,%p)",sn,level,ch,vo);

	heal = dice(1, 15) + level / 2;

	victim->hit = UMIN( victim->hit + heal, victim->max_hit );
	update_pos( victim );

	send_to_char( "Your light wounds mend and your pain ebbs slightly.\n\r", victim );

	if (ch != victim)
	{
		send_to_char( "Ok.\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_cure_serious)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int heal;

	push_call("spell_cure_serious(%p,%p,%p,%p)",sn,level,ch,vo);

	heal = dice(1, 25) + level * 2 / 3 ;

	victim->hit = UMIN( victim->hit + heal, victim->max_hit );
	update_pos( victim );

	send_to_char( "Your serious wounds mend and your pain ebbs away.\n\r", victim );

	if ( ch != victim )
	{
		send_to_char( "Ok.\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_cure_critical)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int heal;

	push_call("spell_cure_critical(%p,%p,%p,%p)",sn,level,ch,vo);

	heal = level + number_range(0, 50);

	victim->hit = UMIN( victim->hit + heal, victim->max_hit );
	update_pos(victim);

	send_to_char( "Your critical wounds close and your pain ebbs away.\n\r", victim );
	affect_strip(victim,gsn_critical_hit);

	if ( ch != victim )
	{
		send_to_char( "Ok.\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_cure_poison)
{
	CHAR_DATA	*victim = (CHAR_DATA *) vo;
	bool		cured   = FALSE;

	push_call("spell_cure_poison(%p,%p,%p,%p)",sn,level,ch,vo);

	if ( is_affected( victim, gsn_poison ) )
	{
		affect_strip( victim, gsn_poison );
		cured = TRUE;
	}

	if (victim->poison != NULL)
	{
		POISON_DATA *npd, *pd;
		cured = TRUE;

		pd = victim->poison;
		victim->poison = NULL;

		while (pd != NULL)
		{
			npd = pd->next;
			FREEMEM( pd );
			pd = npd;
		}
	}

	if ( cured )
	{
		act( "$N looks better.", ch, NULL, victim, TO_CHAR );
		act( "$N looks better.", ch, NULL, victim, TO_ROOM );
		send_to_char( "A warm feeling runs through your body.\n\r", victim );
	}
	else
	{
		act( "$N does not seem to be poisoned.", ch, NULL, victim, TO_CHAR );
	}
	pop_call();
	return;
}

DO_SPELL(spell_curse)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_curse(%p,%p,%p,%p)",sn,level,ch,vo);

	if ( IS_AFFECTED(victim, AFF_CURSE) )
	{
		send_to_char( "They are already cursed.\n\r", ch );
		pop_call();
		return;
	}

	if ( saves_spell( level, ch, victim ) )
	{
		send_to_char( "Your curse did not strike them.\n\r", ch );
		pop_call();
		return;
	}

	af.type      = sn;
	af.duration  = level/15;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF_CURSE;

	af.location  = APPLY_SAVING_SPELL;
	af.modifier  = UMAX(level/20, 3);
	affect_to_char( victim, &af );

	af.location  = APPLY_HITROLL;
	af.modifier  = UMIN(level/-10, -6);
	affect_to_char( victim, &af );

	send_to_char( "You feel unclean.\n\r", victim );

	if ( ch != victim )
	{
		send_to_char( "Your foe is now cursed.\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_detect_evil)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_detect_evil(%p,%p,%p,%p)",sn,level,ch,vo);

	if (is_affected(victim, sn))
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}
	af.type      = sn;
	af.duration  = level;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bittype   = AFFECT_TO_NONE;
	af.bitvector = 0;
	affect_to_char( victim, &af );

	send_to_char( "Traces of red outline all evil in plain sight.\n\r", victim );

	if ( ch != victim )
	{
		send_to_char( "Ok.\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_detect_hidden)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_detect_hidden(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_AFFECTED(victim, AFF_DETECT_HIDDEN) || IS_AFFECTED(victim, AFF_TRUESIGHT))
	{
		send_to_char( "You are already affected.\n\r", ch );
		pop_call();
		return;
	}
	af.type      = sn;
	af.duration  = level;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF_DETECT_HIDDEN;
	affect_to_char( victim, &af );

	send_to_char( "Your senses are heightened to those of an animal.\n\r", victim );

	pop_call();
	return;
}

DO_SPELL(spell_detect_invis)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_detect_invis(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_AFFECTED(victim, AFF_DETECT_INVIS) || IS_AFFECTED(victim, AFF_TRUESIGHT))
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}
	af.type      = sn;
	af.duration  = level;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF_DETECT_INVIS;
	affect_to_char( victim, &af );

	send_to_char( "Your eyes fixate as they gain the ability to see the unseen.\n\r", victim );

	if ( ch != victim )
	{
		send_to_char( "Ok.\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_detect_poison)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;

	push_call("spell_detect_poison(%p,%p,%p,%p)",sn,level,ch,vo);

	if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD )
	{
		if ( obj->value[3] != 0 )
		{
			send_to_char( "You smell poisonous fumes.\n\r", ch );
		}
		else
		{
			send_to_char( "It looks very delicious.\n\r", ch );
		}
	}
	else
	{
		send_to_char( "It doesn't look poisoned.\n\r", ch );
	}
	pop_call();
	return;
}

/*
	Changed dispel spells slightly, you now damage the victim and make
	it gain or lose alignment, when the victim hits neutral alignment
	their gear will be checked for zappers - Scandum 01/04/2002
*/

DO_SPELL(spell_dispel_evil)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_dispel_evil(%p,%p,%p,%p)",sn,level,ch,vo);

	if (!IS_NPC(ch) && IS_EVIL(ch))
	{
		victim = ch;
  	}

	if (!IS_EVIL(victim))
	{
		act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
		pop_call();
		return;
	}

	dam = number_range(level, level * 5);

	if ( saves_spell( level, ch, victim ) )
	{
		dam /= 2;
	}

	damage( ch, victim, dam, sn );

	victim->alignment += 5;

	if (IS_NEUTRAL(victim))
	{
		check_zap(victim, TRUE);
	}
	pop_call();
	return;
}

DO_SPELL(spell_dispel_good)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_dispel_good(%p,%p,%p,%p)",sn,level,ch,vo);

	if (!IS_NPC(ch) && IS_GOOD(ch))
	{
		victim = ch;
  	}

	if (!IS_GOOD(victim))
	{
		act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
		pop_call();
		return;
	}

	dam = number_range (level, level * 5);

	if ( saves_spell( level, ch, victim ) )
	{
		dam /= 2;
	}

	damage( ch, victim, dam, sn );

	victim->alignment -= 5;

	if (IS_NEUTRAL(victim))
	{
		check_zap(victim, TRUE);
	}
	pop_call();
	return;
}

DO_SPELL(spell_dispel_undead)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_dispel_undead(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_NPC(victim) && !IS_SET(victim->act , ACT_UNDEAD))
	{
		if (!rspec_req(victim, RSPEC_UNDEAD))
		{
			act( "$N is not affected.", ch, NULL, victim, TO_CHAR);
			pop_call();
			return;
		}
	}

	dam = number_range(level*3, level*9);

	if ( saves_spell( level/2, ch, victim ) )
	{
		dam /= 3;
	}
	damage( ch, victim, dam, sn );
	pop_call();
	return;
}

DO_SPELL(spell_dispel_magic)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA *paf, *paf_prev;
	int paf_type;

	push_call("spell_dispel_magic(%p,%p,%p,%p)",sn,level,ch,vo);

	if (target == TAR_CHAR_DEFENSIVE)
	{
		if (saves_spell(level, ch, victim))
		{
			send_to_char( "You failed.\n\r", ch );
			pop_call();
			return;
		}

		for (paf_type = 0, paf = victim->last_affect ; paf ; paf = paf_prev)
		{
			paf_prev = paf->prev;

			if (!is_spell(paf->type))
			{
				continue;
			}

			switch (paf->bitvector)
			{
				case AFF2_POSSESS:
				case AFF_CHARM:
					continue;
			}

			if (paf_type && paf->type != paf_type)
			{
				continue;
			}

			if (paf_type == 0 && paf->duration >= 0)
			{
				if (skill_table[paf->type].msg_off)
				{
					act(skill_table[paf->type].msg_off, victim, NULL, NULL, TO_CHAR);
				}
			}
			paf_type = paf->type;
			affect_from_char(victim, paf);
		}

		if (paf_type == 0)
		{
			send_to_char( "Nothing appears to happen.\n\r", ch );
		}
		else if (victim != ch)
		{
			send_to_char( "Their magic spells seem to vanish slowly.\n\r", ch );
		}
	}
	else
	{
		for (paf_type = 0, paf = obj->last_affect ; paf ; paf = paf_prev)
		{
			paf_prev = paf->prev;

			if (!is_spell(paf->type))
			{
				continue;
			}

			if (paf_type && paf->type != paf_type)
			{
				continue;
			}

			if (paf_type == 0 && paf->duration >= 0)
			{
				if (skill_table[paf->type].msg_obj_off)
				{
					act(skill_table[paf->type].msg_obj_off, ch, obj, NULL, TO_CHAR);
				}
			}
			paf_type = paf->type;
			affect_from_obj(obj, paf);
		}

		if (paf_type == 0)
		{
			send_to_char( "Nothing appears to happen.\n\r", ch );
		}
	}
	pop_call();
	return;
}

DO_SPELL(spell_earthquake)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	EXIT_DATA *pexit;
	int door, temp_vnum;

	push_call("spell_earthquake(%p,%p,%p,%p)",sn,level,ch,vo);

	/*
		no earthquakes in the air
	*/

	if (ch->in_room->sector_type == SECT_AIR)
	{
		send_to_char("The earth trembles far beneath you.\n\r",ch);
		pop_call();
		return;
	}

	send_to_char( "The earth trembles beneath your feet!\n\r", ch );
	act( "$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM );

	temp_vnum = ch->in_room->vnum;

	for (door = 0 ; door <= 5 ; door++)
	{
		if ((pexit = get_exit(temp_vnum, door)) && pexit->to_room != temp_vnum)
		{
			ch->in_room = room_index[pexit->to_room];
			act("The earth trembles and shivers.", ch, NULL, NULL, TO_ROOM );
		}
	}
	ch->in_room = room_index[temp_vnum];

	for (vch = ch->in_room->last_person ; vch ; vch = vch_next)
	{
		vch_next = vch->prev_in_room;

		if (CAN_FLY(vch))
		{
			continue;
		}
		if (!can_mass_cast(ch, vch))
		{
			continue;
		}
		damage( ch, vch, level + dice(2, 8), sn );
	}
	pop_call();
	return;
}

DO_SPELL(spell_tremor)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	EXIT_DATA *pexit;
	int door, temp_vnum;

	push_call("spell_tremor(%p,%p,%p,%p)",sn,level,ch,vo);

	if (ch->in_room->sector_type == SECT_AIR)
	{
		send_to_char("The earth tremors far beneath you.\n\r",ch);
		pop_call();
		return;
	}

	temp_vnum = ch->in_room->vnum;

	for (door = 0 ; door <= 5 ; door++)
	{
		if ((pexit = get_exit(temp_vnum, door)) && pexit->to_room != temp_vnum)
		{
			ch->in_room = room_index[pexit->to_room];
			act("You hear a loud wrenching noise from the ground.", ch, NULL, NULL, TO_ROOM );
		}
	}
	ch->in_room = room_index[temp_vnum];

	send_to_char( "The earth tremors beneath your feet!\n\r", ch );
	act( "$n makes the earth shake.", ch, NULL, NULL, TO_ROOM );

	for (vch = ch->in_room->last_person ; vch ; vch = vch_next)
	{
		vch_next	= vch->prev_in_room;

		if (CAN_FLY(vch))
		{
			continue;
		}
		if (!can_mass_cast(ch, vch))
		{
			continue;
		}
		if (number_bits(3) == 0)
		{
			vch->position = POS_RESTING;
		}
		damage(ch, vch, number_range(level * 2, level * 3) + dice(5, 20), sn);
	}
	pop_call();
	return;
}

DO_SPELL(spell_enchant_weapon)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_enchant_weapon(%p,%p,%p,%p)",sn,level,ch,vo);

	if (!IS_NPC(ch) && multi(ch, sn) == -1)
	{
		send_to_char( "You can't do that!\n\r", ch);
		pop_call();
		return;
	}

	if (obj->item_type != ITEM_WEAPON || IS_OBJ_STAT(obj, ITEM_MAGIC) || obj->first_affect != NULL)
	{
		send_to_char( "That object cannot be enchanted.\n\r", ch);
		pop_call();
		return;
	}

	SET_BIT(obj->extra_flags, ITEM_MODIFIED);
	SET_BIT(obj->extra_flags, ITEM_MAGIC);

	af.type		= sn;
	af.duration	= -1;
	af.bittype	= AFFECT_TO_NONE;
	af.bitvector	= 0;

	af.location	= APPLY_HITROLL;
	af.modifier	= number_fuzzy(level / number_fuzzy(5));

	affect_to_obj(obj, &af);

	af.location	= APPLY_DAMROLL;
	af.modifier	= number_fuzzy(level / number_fuzzy(10));

	affect_to_obj(obj, &af);

	obj->level     += number_fuzzy(level / number_fuzzy(3));


	if (IS_GOOD(ch))
	{
		SET_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
		SET_BIT(obj->extra_flags, ITEM_ANTI_NEUTRAL);
		DEL_BIT(obj->extra_flags, ITEM_EVIL);
		SET_BIT(obj->extra_flags, ITEM_BLESS);
		act("$p glows yellow.", ch, obj, NULL, TO_ROOM);
	}
	else if (IS_EVIL(ch))
	{
		SET_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
		SET_BIT(obj->extra_flags, ITEM_ANTI_NEUTRAL);
		DEL_BIT(obj->extra_flags, ITEM_BLESS);
		SET_BIT(obj->extra_flags, ITEM_EVIL);

		act("$p glows red.", ch, obj, NULL, TO_ROOM);
	}
	else
	{
		SET_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
		SET_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
		DEL_BIT(obj->extra_flags, ITEM_BLESS);
		DEL_BIT(obj->extra_flags, ITEM_EVIL);

		act("$p glows blue.", ch, obj, NULL, TO_ROOM);
	}

	act("$p has been enchanted.", ch, obj, NULL, TO_CHAR);
	pop_call();
	return;
}

DO_SPELL(spell_energy_bolt)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_energy_bolt(%p,%p,%p,%p)",sn,level,ch,vo);

	dam = 40 + level * 2 + number_range(0, 50);

	if (saves_spell(level, ch, victim))
	{
		dam /= 2;
	}

	damage(ch, victim, dam, sn);

	pop_call();
	return;
}

DO_SPELL(spell_energy_drain)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_energy_drain(%p,%p,%p,%p)",sn,level,ch,vo);

	if (saves_spell(level * 1.5, ch, victim))
	{
		switch (number_range(1, 3))
		{
			case 1: act( "$N shrugs off your deadly embrace.", ch, NULL, victim, TO_CHAR); break;
			case 2: act( "$N flinches but is unharmed.", ch, NULL, victim, TO_CHAR); break;
			case 3: act( "$N groans from your spine chilling spell, but is otherwise unharmed.", ch, NULL, victim, TO_CHAR); break;
		}
		pop_call();
		return;
	}

	dam = 50 + number_range(level * 2, level * 4);

	if (!saves_spell(level, ch, victim))
	{
		if (!saves_spell(level, ch, victim))
		{
			act("You drain $N's vitality.", ch, NULL, victim, TO_CHAR);
			act("$N drains your vitality.", victim, NULL, ch, TO_CHAR), 

			dam = UMIN(victim->hit + 1, dam);

			ch->hit      = UMIN(ch->hit + dam, ch->max_hit);

			victim->move = UMAX(0, victim->move - dam);
			victim->mana = UMAX(0, victim->mana - dam);
		}
	}
	else
	{
		dam /= 2;
	}

	damage (ch, victim, dam, sn);

	pop_call();
	return;
}

DO_SPELL(spell_fireball)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_fireball(%p,%p,%p,%p)",sn,level,ch,vo);

	dam   = 50 + number_range(level * 3, level * 5);

	if (saves_spell(level, ch, victim))
	{
		dam /= 2;
	}
	damage( ch, victim, dam, sn );

	pop_call();
	return;
}

DO_SPELL(spell_flamestrike)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_flamestrike(%p,%p,%p,%p)",sn,level,ch,vo);

	dam = level + number_range(1, level * 2);

	if ( saves_spell( level, ch, victim ) )
	{
		dam /= 2;
	}
	damage( ch, victim, dam, sn );

	pop_call();
	return;
}

DO_SPELL(spell_faerie_fire)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_faerie_fire(%p,%p,%p,%p)",sn,level,ch,vo);

	if (is_affected(victim, gsn_faerie_fire))
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}

	af.type      = sn;
	af.duration  = 1 + level / 25;
	af.location  = APPLY_AC;
	af.modifier  = 2 * level;
	af.bittype   = AFFECT_TO_NONE;
	af.bitvector = 0;
	affect_to_char( victim, &af );

	send_to_char( "You are surrounded by a pink outline.\n\r", victim );
	act( "$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM );

	pop_call();
	return;
}

DO_SPELL(spell_faerie_fog)
{
	CHAR_DATA *ich;

	push_call("spell_faerie_fog(%p,%p,%p,%p)",sn,level,ch,vo);

	act( "$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM );
	send_to_char( "You conjure a cloud of purple smoke.\n\r", ch );

	for ( ich = ch->in_room->first_person; ich != NULL; ich = ich->next_in_room )
	{
		if ( !IS_NPC(ich) && IS_SET(ich->act, PLR_WIZINVIS) )
			continue;

		if ( !IS_NPC(ich) && IS_SET(ich->act, PLR_WIZCLOAK) )
			continue;

		if( IS_NPC(ich) && IS_AFFECTED(ich, AFF_ETHEREAL) )
			continue;

		if ( ich == ch || saves_spell( level, ch, ich ) )
			continue;

		affect_strip ( ich, gsn_invis					);
		affect_strip ( ich, gsn_sneak					);
		affect_strip ( ich, gsn_stealth				);
		REMOVE_BIT   ( ich->affected_by, AFF_HIDE		);
		REMOVE_BIT   ( ich->affected_by, AFF_INVISIBLE	);
		REMOVE_BIT   ( ich->affected_by, AFF_SNEAK		);
		REMOVE_BIT   ( ich->affected_by, AFF_STEALTH		);
		REMOVE_BIT   ( ich->affected_by, AFF_IMP_INVISIBLE);
		act( "$n is revealed!", ich, NULL, NULL, TO_ROOM );
		send_to_char( "You are revealed!\n\r", ich );
	}
	pop_call();
	return;
}

DO_SPELL(spell_fly)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_fly(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_AFFECTED(victim, AFF2_EARTHBIND))
	{
		send_to_char("They are bound to the earth.\n\r", ch);
		pop_call();
		return;
	}

	if (CAN_FLY(victim))
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}
	af.type      = sn;
	af.duration  = level + 3;
	af.location  = 0;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF_FLYING;
	affect_to_char( victim, &af );
	send_to_char( "Your feet rise off the ground.\n\r", victim );
	act( "$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM );

	pop_call();
	return;
}

DO_SPELL(spell_gate)
{
	CHAR_DATA *victim;
	OBJ_DATA *gate;

	push_call("spell_gate(%p,%p,%p,%p)",sn,level,ch,vo);

	if (target_name[0] == '\0')
	{
		send_to_char("Whom would you want to gate to?\n\r", ch);
		pop_call();
		return;
	}

	if ((victim = get_char_area(ch, target_name)) == NULL)
	{
		send_to_char("There is none named like that.\n\r", ch);
		pop_call();
		return;
	}

	if  (IS_SET(ch->in_room->room_flags, ROOM_IS_CASTLE)
	||  (IS_NPC(victim) && level / 3 < victim->level)
	|| (!IS_NPC(victim) && level - 3 < victim->level))
	{
		send_to_char( "They are protected from your power.\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_NPC(victim) && is_affected(victim, gsn_anti_magic_shell))
	{
		act("The flows of magic shatter against a shell surrounding $N.", ch, NULL, victim, TO_CHAR);
		pop_call();
		return;
	}

	if (victim == ch
	|| victim->in_room == NULL
	|| IS_SET(victim->in_room->room_flags, ROOM_SAFE)
	|| IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
	|| IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
	|| IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
	|| IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
	|| victim->in_room->sector_type == SECT_ETHEREAL
	|| victim->in_room->sector_type == SECT_ASTRAL
	|| ch->in_room->sector_type == SECT_ETHEREAL
	|| ch->in_room->sector_type == SECT_ASTRAL
	|| IS_SET(ch->in_room->area->flags, AFLAG_NORECALL)
	|| IS_SET(ch->in_room->area->flags, AFLAG_NOSUMMON)
	|| victim->level > UMIN(ch->level-3, level - 3)
	|| find_keeper(victim) != NULL)
	{
		send_to_char( "You failed.\n\r", ch );
		pop_call();
		return;
	}

	if (ch->in_room == victim->in_room)
	{
		send_to_char("You are already there.\n\r",ch);
		pop_call();
		return;
	}

	if (saves_spell(level,ch,victim))
	{
		send_to_char("You failed to envision your victim.\n\r", ch);
		pop_call();
		return;
	}

	gate = create_object(get_obj_index(OBJ_VNUM_GATE), 0);

	if (!gate)
	{
		ch_printf(ch,"You try to create a gate, but it does not seem possible somehow.\n\r");
		pop_call();
		return;
	}
	gate->value[3] = victim->in_room->vnum;

	obj_to_room(gate, ch->in_room->vnum);

	act( "$p opens before you.", ch, gate, NULL, TO_CHAR);
	act( "$p opens before $n.",  ch, gate, NULL, TO_ROOM);

	do_enter(ch, "i23");
	obj_from_room(gate);
	junk_obj(gate);
	pop_call();
	return;
}

/*
 * Spell for mega1.are from Glop/Erkenbrand.
 */

DO_SPELL(spell_general_purpose)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_general_purpose(%p,%p,%p,%p)",sn,level,ch,vo);

	dam = number_range( 25, 100 );
	if ( saves_spell( level, ch, victim ) )
		dam /= 2;
	damage( ch, victim, dam, sn );
	pop_call();
	return;
}

DO_SPELL(spell_giant_strength)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_giant_strength(%p,%p,%p,%p)",sn,level,ch,vo);

	if ( is_affected( victim, sn ) )
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}
	af.type      = sn;
	af.duration  = level;
	af.location  = APPLY_STR;
	af.modifier  = 1 + (level >= 18) + (level >= 25);
	af.bittype   = AFFECT_TO_NONE;
	af.bitvector = 0;
	affect_to_char( victim, &af );
	send_to_char( "You feel the strength of a giant.\n\r", victim );
	if ( ch != victim )
	{
		send_to_char( "Ok.\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_harm)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_harm(%p,%p,%p,%p)",sn,level,ch,vo);

	dam = 100 + number_range(0, 4) * 50;

	if (saves_spell(level, ch, victim))
	{
		dam /= 2;
	}

	damage (ch, victim, number_fuzzy(dam), sn);

	pop_call();
	return;
}

DO_SPELL(spell_heal)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	push_call("spell_heal(%p,%p,%p,%p)",sn,level,ch,vo);

	victim->hit = UMIN(victim->hit + 100, victim->max_hit);

	update_pos(victim);

	send_to_char( "A warm feeling fills your body.\n\r", victim);

	affect_strip(victim,gsn_critical_hit);
	affect_strip(victim, 631); /* HALLUCINATE */

	if (ch != victim)
	{
		send_to_char("Healing powers at work.\n\r", ch );
	}
	pop_call();
	return;
}

/*
 * Spell for mega1.are from Glop/Erkenbrand.
 */

DO_SPELL(spell_high_explosive)
{

	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_high_explosive(%p,%p,%p,%p)",sn,level,ch,vo);

	dam = number_range( 30, 120 );
	if ( saves_spell( level, ch, victim ) )
		dam /= 2;
	damage( ch, victim, dam, sn );
	pop_call();
	return;
}

DO_SPELL(spell_identify)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	char buf[MAX_STRING_LENGTH];
	AFFECT_DATA *paf;

	push_call("spell_identify(%p,%p,%p,%p)",sn,level,ch,vo);

	ch_printf(ch, "%s is type %s, extra flags %s.\n\rWeight is %d, value is %d, level is %d.\n\r",
		capitalize(obj->short_descr),
		item_type_name(obj),
		flag_string(obj->extra_flags, o_flags),
		get_obj_weight(obj),
		obj->cost,
		obj->level);

	switch ( obj->item_type )
	{
		case ITEM_SCROLL:
		case ITEM_POTION:
		case ITEM_PILL:
			sprintf(buf, "Level %d spells of:", obj->value[0]);

			if (obj->value[1] > 0 && obj->value[1] < MAX_SKILL)
			{
				cat_sprintf(buf, " '%s'", skill_table[obj->value[1]].name);
			}

			if (obj->value[2] > 0 && obj->value[2] < MAX_SKILL)
			{
				cat_sprintf(buf, " '%s'", skill_table[obj->value[2]].name);
			}

			if (obj->value[3] > 0 && obj->value[3] < MAX_SKILL)
			{
				cat_sprintf(buf, " '%s'", skill_table[obj->value[3]].name);
			}
			ch_printf(ch, "%s\n\r", buf);
			break;

		case ITEM_WAND:
		case ITEM_STAFF:
			sprintf(buf, "Has %d/%d charges of level %d",
				obj->value[2],
				obj->value[1],
				obj->value[0]);

			if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL)
			{
				cat_sprintf(buf, " '%s'", skill_table[obj->value[3]].name);
			}
			ch_printf(ch, "%s\n\r", buf);
			break;

		case ITEM_WEAPON:
			switch (obj->value[0])
			{
				case WEAPON_TYPE_WEAPON:
					ch_printf(ch, "Damage is %dd%d.\n\r", obj->value[1], obj->value[2]);
					break;
				case WEAPON_TYPE_SWORD:
					ch_printf(ch, "This weapon is a sword. Damage is %dd%d.\n\r", obj->value[1], obj->value[2]);
					break;
				case WEAPON_TYPE_DAGGER:
					ch_printf(ch, "This weapon is a dagger. Damage is %dd%d.\n\r", obj->value[1], obj->value[2]);
					break;
				case WEAPON_TYPE_AXE:
					ch_printf(ch, "This weapon is an axe. Damage is %dd%d.\n\r", obj->value[1], obj->value[2]);
					break;
				case WEAPON_TYPE_MACE:
					ch_printf(ch, "This weapon is a mace. Damage is %dd%d.\n\r", obj->value[1], obj->value[2]);
					break;
				case WEAPON_TYPE_STAFF:
					ch_printf(ch, "This weapon is a staff. Damage is %dd%d.\n\r", obj->value[1], obj->value[2]);
					break;
				case WEAPON_TYPE_WHIP:
					ch_printf(ch, "This weapon is a whip. Damage is %dd%d.\n\r", obj->value[1], obj->value[2]);
					break;
				case WEAPON_TYPE_FLAIL:
					ch_printf(ch, "This weapon is a flail. Damage is %dd%d.\n\r", obj->value[1], obj->value[2]);
					break;
				case WEAPON_TYPE_SPEAR:
					ch_printf(ch, "This weapon is a spear. Damage is %dd%d.\n\r", obj->value[1], obj->value[2]);
					break;
				case WEAPON_TYPE_CLAW:
					ch_printf(ch, "This weapon is a claw. Damage is %dd%d.\n\r", obj->value[1], obj->value[2]);
					break;
				case WEAPON_TYPE_SHORTBOW:
					ch_printf(ch, "This weapon is a short bow. Damage is %dd%d.\n\r", obj->value[1], obj->value[2]);
					break;
				case WEAPON_TYPE_LONGBOW:
					ch_printf(ch, "This weapon is a long bow. Damage is %dd%d.\n\r", obj->value[1], obj->value[2]);
					break;
				case WEAPON_TYPE_CROSSBOW:
					ch_printf(ch, "This weapon is a cross bow. Damage is %dd%d.\n\r", obj->value[1], obj->value[2]);
					break;
				case WEAPON_TYPE_BLOWPIPE:
					ch_printf(ch, "This weapon is a blow pipe. Damage is %dd%d.\n\r", obj->value[1], obj->value[2]);
					break;
				default:
					ch_printf(ch, "This weapon is faulty. Talk to Demise. Damage is %dd%d.\n\r", obj->value[1], obj->value[2]);
					break;
			}
			break;

		case ITEM_ARMOR:
			ch_printf(ch, "Armor class is %d.\n\r", obj->value[0] );
			break;

		case ITEM_AMMO:
			switch (obj->value[0])
			{
				case WEAPON_TYPE_SHORTBOW:
					ch_printf(ch, "Ammo type for short bows, Damage is %d, Speed is %d, Range is %d.\n\r",
						obj->value[1], obj->value[2], obj->value[3]);
					break;
				case WEAPON_TYPE_LONGBOW:
					ch_printf(ch, "Ammo type for long bows, Damage is %d, Speed is %d, Range is %d.\n\r",
						obj->value[1], obj->value[2], obj->value[3]);
					break;
				case WEAPON_TYPE_CROSSBOW:
					ch_printf(ch, "Ammo type for cross bows, Damage is %d, Speed is %d, Range is %d.\n\r",
						obj->value[1], obj->value[2], obj->value[3]);
					break;
				case WEAPON_TYPE_BLOWPIPE:
					ch_printf(ch, "Ammo type for blow pipes, Damage is %d, Speed is %d, Range is %d.\n\r",
						obj->value[1], obj->value[2], obj->value[3]);
					break;
				default:
					ch_printf(ch, "Ammo type is faulty. Talk to Demise. Damage is %d, Speed is %d, Range is %d.\n\r",
						obj->value[1], obj->value[2], obj->value[3]);
					break;
			}
			break;

		case ITEM_CONTAINER:
			ch_printf(ch, "Capacity is %d weight and %d items.\n\r", obj->value[0], obj->value[3] ? obj->value[3] : MAX_OBJECTS_IN_CONTAINER);
			break;

		case ITEM_LIGHT:
			ch_printf(ch, "Brightness is %d and duration is %d hours.\n\r", obj->value[0], obj->value[2]);
			break;
	}

	for (paf = obj->pIndexData->first_affect ; paf ; paf = paf->next)
	{
		switch (paf->location)
		{
			case APPLY_NONE:
				break;

			default:
				ch_printf(ch, "Affects %s by %d.\n\r", affect_loc_name(paf->location), paf->modifier);
				break;
		}
		switch (paf->bittype)
		{
			case AFFECT_TO_NONE:
				break;

			case AFFECT_TO_CHAR:
				ch_printf(ch, "Affects wearer with %s.\n\r", affect_bit_name(paf->bitvector));
				break;

			case AFFECT_TO_OBJ:
				ch_printf(ch, "Affects object with %s.\n\r", flag_string(paf->bitvector, o_flags));
				break;
		}
	}

	for (paf = obj->first_affect ; paf != NULL ; paf = paf->next)
	{
		switch (paf->location)
		{
			case APPLY_NONE:
				break;

			default:
				ch_printf(ch, "Affects %s by %d.\n\r", affect_loc_name(paf->location), paf->modifier);
				break;
		}
		switch (paf->bittype)
		{
			case AFFECT_TO_NONE:
				break;

			case AFFECT_TO_CHAR:
				ch_printf(ch, "Affects wearer with %s.\n\r", affect_bit_name(paf->bitvector));
				break;

			case AFFECT_TO_OBJ:
				ch_printf(ch, "Affects object with %s.\n\r", flag_string(paf->bitvector, o_flags));
				break;
		}
	}
	sprintf(buf, "Wear locations:");

	if (!IS_SET(obj->wear_flags, ITEM_TAKE))
	{
		strcat( buf, " Cannot take");
	}
	if (IS_SET(obj->wear_flags, ITEM_WEAR_FINGER))
	{
		strcat( buf, " Fingers");
	}
	if (IS_SET(obj->wear_flags, ITEM_WEAR_NECK))
	{
		strcat( buf, " Neck");
	}
	if (IS_SET(obj->wear_flags, ITEM_WEAR_BODY))
	{
		strcat( buf, " Body");
	}
	if (IS_SET(obj->wear_flags, ITEM_WEAR_HEAD))
	{
		strcat( buf, " Head");
	}
	if (IS_SET(obj->wear_flags, ITEM_WEAR_LEGS))
	{
		strcat( buf, " Legs");
	}
	if (IS_SET(obj->wear_flags, ITEM_WEAR_FEET))
	{
		strcat( buf, " Feet");
	}
	if (IS_SET(obj->wear_flags, ITEM_WEAR_HANDS))
	{
		strcat( buf, " Hands");
	}
	if (IS_SET(obj->wear_flags, ITEM_WEAR_ARMS))
	{
		strcat( buf, " Arms");
	}
	if (IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
	{
		strcat( buf, " Shield");
	}
	if (IS_SET(obj->wear_flags, ITEM_WEAR_ABOUT))
	{
		strcat( buf, " About");
	}
	if (IS_SET(obj->wear_flags, ITEM_WEAR_WAIST))
	{
		strcat( buf, " Waist");
	}
	if (IS_SET(obj->wear_flags, ITEM_WEAR_WRIST))
	{
		strcat( buf, " Wrist");
	}
	if (IS_SET(obj->wear_flags, ITEM_WEAR_WIELD))
	{
		strcat( buf, " Wield");
	}
	if (IS_SET(obj->wear_flags, ITEM_WEAR_HOLD))
	{
		strcat( buf, " Hold");
	}
	if (IS_SET(obj->wear_flags, ITEM_WEAR_HEART))
	{
		strcat( buf, " Heart");
	}
	if (strlen(buf) < 17)
	{
		strcat( buf, " Carry only");
	}
	ch_printf(ch, "%s.\n\r", buf);

	pop_call();
	return;
}

DO_SPELL(spell_infravision)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_infravision(%p,%p,%p,%p)",sn,level,ch,vo);

	if ( IS_AFFECTED(victim, AFF_INFRARED) )
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}
	act( "$n's eyes glow red.", ch, NULL, NULL, TO_ROOM );
	af.type      = sn;
	af.duration  = 2 * level;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF_INFRARED;
	affect_to_char( victim, &af );
	send_to_char( "Your eyes glow red.\n\r", victim );
	if ( ch != victim )
	{
		send_to_char( "Ok.\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_enhanced_heal)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_enhanced_heal(%p,%p,%p,%p)",sn,level,ch,vo);

	if ( IS_AFFECTED(victim, AFF2_ENHANCED_HEAL) )
	{
		send_to_char("They are already healing easily.\n\r", ch);
		pop_call();
		return;
	}

	act( "$n heals easily.", victim, NULL, NULL, TO_ROOM );
	af.type      = sn;
	af.duration  = 24;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF2_ENHANCED_HEAL;
	affect_to_char( victim, &af );
	send_to_char( "You heal easily.\n\r", victim );
	if ( ch != victim )
	{
		send_to_char( "Ok.\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_enhanced_revive)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_enhanced_revive(%p,%p,%p,%p)",sn,level,ch,vo);

	if ( IS_AFFECTED(victim, AFF2_ENHANCED_REVIVE) )
	{
		send_to_char("They are already reviving easily.\n\r", ch);
		pop_call();
		return;
      }

	act( "$n revives easily.", victim, NULL, NULL, TO_ROOM );
	af.type      = sn;
	af.duration  = 24;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF2_ENHANCED_REVIVE;
	affect_to_char( victim, &af );
	send_to_char( "You revive easily.\n\r", victim );
	if ( ch != victim )
	{
		send_to_char( "Ok.\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_enhanced_rest)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_enhanced_rest(%p,%p,%p,%p)",sn,level,ch,vo);

	if ( IS_AFFECTED(victim, AFF2_ENHANCED_REST) )
	{
		send_to_char("They are already resting easily.\n\r", ch);
		pop_call();
		return;
	}

	act( "$n rests easily.", victim, NULL, NULL, TO_ROOM );
	af.type      = sn;
	af.duration  = 24;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF2_ENHANCED_REST;
	affect_to_char( victim, &af );
	send_to_char( "You rest easily.\n\r", victim );
	if ( ch != victim )
	{
		send_to_char( "Ok.\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_remove_fear)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_remove_fear(%p,%p,%p,%p)",sn,level,ch,vo);

	if (saves_spell(level, ch, victim))
	{
		send_to_char( "They are not made fearless.\n\r", ch );
		pop_call();
		return;
	}

	if (!IS_NPC(victim))
	{
		if (is_affected(victim, sn))
		{
			send_to_char("They are already fearless.\n\r", ch);
			pop_call();
			return;
		}

		af.type		= sn;
		af.duration	= level / 3;
		af.bittype	= AFFECT_TO_NONE;
		af.bitvector	= 0;
		af.location	= APPLY_NONE;
		af.modifier	= 0;

		affect_to_char(victim, &af);

		affect_strip(victim, gsn_fear);

		victim->pcdata->wimpy = 0;
	}
	else
	{
		if (!IS_SET(victim->act, ACT_WIMPY))
		{
			send_to_char( "They are already fearless.\n\r", ch );
			pop_call();
			return;
		}
		REMOVE_BIT(victim->act, ACT_WIMPY);
	}
	act("You become fearless!", victim, NULL, NULL, TO_CHAR );
	act("$n becomes fearless!", victim, NULL, NULL, TO_ROOM );

	pop_call();
	return;
}

DO_SPELL(spell_fear)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_fear(%p,%p,%p,%p)",sn,level,ch,vo);

	if (saves_spell(level, ch, victim))
	{
		send_to_char( "They are not made frightened.\n\r", ch );
		pop_call();
		return;
	}

	if (!IS_NPC(victim))
	{
		if (is_affected(victim, sn))
		{
			send_to_char("They are already frightened.\n\r", ch);
			pop_call();
			return;
		}

		af.type		= sn;
		af.duration	= level / 3;
		af.bittype	= AFFECT_TO_NONE;
		af.bitvector	= 0;
		af.location	= APPLY_NONE;
		af.modifier	= 0;

		affect_to_char(victim, &af);

		affect_strip(victim, gsn_remove_fear);

		victim->pcdata->wimpy = victim->max_hit / 2;
	}
	else
	{
		if (IS_SET(victim->act, ACT_WIMPY))
		{
			send_to_char( "They are already frightened.\n\r", ch );
			pop_call();
			return;
		}
		SET_BIT(victim->act, ACT_WIMPY);
	}
	act("You become filled with fear!", victim, NULL, NULL, TO_CHAR );
	act("$n becomes filled with fear!", victim, NULL, NULL, TO_ROOM );

	pop_call();
	return;
}

DO_SPELL(spell_haste)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_haste(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_AFFECTED(victim, AFF_HASTE))
	{
		send_to_char("They are already fast.\n\r", ch);
		pop_call();
		return;
	}

	af.type      = sn;
	af.location  = APPLY_DEX;

	af.duration = URANGE(12, level / 4, 48);
	af.modifier = 1 + (level > 49) + (level > 99);
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF_HASTE;
	affect_to_char(victim, &af);

	send_to_char("You speed up.\n\r", victim);
	act("$n speeds up.", victim, NULL, NULL, TO_ROOM);

	pop_call();
	return;
}

DO_SPELL(spell_invis)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_invis(%p,%p,%p,%p)",sn,level,ch,vo);

	if (target == TAR_CHAR_DEFENSIVE)
	{
		if (IS_AFFECTED(victim, AFF_INVISIBLE) || IS_AFFECTED(victim, AFF_IMP_INVISIBLE))
		{
			send_to_char("They are already invisible.\n\r", ch);
			pop_call();
			return;
		}

		act("You fade out of existence.", victim, NULL, NULL, TO_CHAR);
		act("$n fades out of existence.", victim, NULL, NULL, TO_ROOM);

		af.type      = sn;
		af.duration  = level;
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		af.bittype   = AFFECT_TO_CHAR;
		af.bitvector = AFF_INVISIBLE;
		affect_to_char( victim, &af );
	}
	else if (target == TAR_OBJ_INV)
	{
		if (IS_SET(obj->extra_flags, ITEM_INVIS))
		{
			act("$p is already invisible.", ch, obj, NULL, TO_CHAR);
			pop_call();
			return;
		}

		af.type      = sn;
		af.duration  = level;
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		af.bittype   = AFFECT_TO_OBJ;
		af.bitvector = ITEM_INVIS;

		affect_to_obj(obj, &af );

		SET_BIT(obj->extra_flags, ITEM_MODIFIED);

		act("$p fades out of existance.", ch, obj, NULL, TO_CHAR);
	}
	pop_call();
	return;
}

DO_SPELL(spell_invis_obj)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;

	push_call("spell_invis_obj(%p,%p,%p,%p)",sn,level,ch,vo);

	if (number_fuzzy(obj->level) > level)
	{
		send_to_char( "You failed.\n\r", ch);
		pop_call();
		return;
	}

	if (IS_SET(obj->extra_flags, ITEM_INVIS))
	{
		send_to_char( "It is already invisible.\n\r", ch);
		pop_call();
		return;
	}

	SET_BIT(obj->extra_flags, ITEM_INVIS);
	SET_BIT(obj->extra_flags, ITEM_MODIFIED);

	send_to_char( "Ok.\n\r", ch );
	pop_call();
	return;
}

DO_SPELL(spell_tongues)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_tongues(%p,%p,%p,%p)",sn,level,ch,vo);

	if ( IS_AFFECTED(victim, AFF_TONGUES))
	{
		send_to_char( "That person is already speaking in tongues.\n\r", ch);
		pop_call();
		return;
	}

	act( "$n starts speaking in tongues.", victim, NULL, NULL, TO_ROOM);
	af.type      = sn;
	af.duration  = 24;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF_TONGUES;
	affect_to_char( victim, &af);
	send_to_char( "You start speaking in tongues.\n\r", victim);
	if(ch!=victim)
	{
		send_to_char( "Ok.\n\r", ch);
	}
	pop_call();
	return;
}

DO_SPELL(spell_understand)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_understand(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_AFFECTED(victim, AFF_UNDERSTAND))
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}
	act( "$n starts understanding everyone.", victim, NULL, NULL, TO_ROOM);
	af.type      = sn;
	af.duration  = 24;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF_UNDERSTAND;
	affect_to_char( victim, &af);
	send_to_char( "You start understanding everyone.\n\r", victim);
	if(ch!=victim)
	{
		send_to_char( "Ok.\n\r", ch);
	}
	pop_call();
	return;
}

DO_SPELL(spell_petrifying_touch)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_petrifying_touch(%p,%p,%p,%p)",sn,level,ch,vo);

	dam   = number_range(level+30, level+110);

	if ( saves_spell( level, ch, victim ) )
	{
		dam /= 2;
	}
	else if (number_bits(1) == 0)
	{
		AFFECT_DATA af;

		af.type      = sn;
		af.duration  = 1;
		if (!IS_NPC(victim))
		{
			af.location = APPLY_DEX;
			af.modifier = -1;
		}
		else
		{
			af.location = APPLY_AC;
			af.modifier = 2;
		}
		af.bittype   = AFFECT_TO_NONE;
		af.bitvector = 0;
		affect_join( victim, &af );
	}
	damage( ch, victim, dam, sn );

	pop_call();
	return;
}

DO_SPELL(spell_locate_object)
{
	OBJ_DATA *obj;
	OBJ_DATA *in_obj;
	int count;

	push_call("spell_locate_object(%p,%p,%p,%p)",sn,level,ch,vo);

	if (*target_name == '\0')
	{
		send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
		pop_call();
		return;
	}

	if (strlen(target_name) > 20)
	{
		target_name[20] = '\0';
	}

	count = 0;

	for (obj = mud->f_obj ; obj ; obj = obj->next)
	{
		if (obj->level > level || !is_multi_name_list_short(target_name, short_to_name(obj->short_descr, 1)) || !can_see_obj(ch, obj) )
		{
			continue;
		}

		for (in_obj = obj ; in_obj->in_obj ; in_obj = in_obj->in_obj);

		if (in_obj->carried_by && !can_see_world(ch, in_obj->carried_by))
		{
			continue;
		}

		count++;

		if (in_obj->carried_by != NULL)
		{
			ch_printf(ch, "%s%s is carried by %s.\n\r", get_color_string(ch, COLOR_TEXT, VT102_DIM), capitalize(obj->short_descr), PERS(in_obj->carried_by, ch));
		}
		else
		{
			ch_printf(ch, "%s%s is in %s.\n\r", get_color_string(ch, COLOR_TEXT, VT102_DIM), capitalize(obj->short_descr), in_obj->in_room == NULL ? "somewhere" : in_obj->in_room->name);
		}
		if (count > UMAX(20, 10 + level / 10))
		{
			break;
		}
	}

	if (count == 0)
	{
		send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
	}

	pop_call();
	return;
}

DO_SPELL(spell_magic_missile)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_magic_missile(%p,%p,%p,%p)",sn,level,ch,vo);

	dam = number_range(level / 2 + 40, level / 2 + 60);

	if (!IS_AFFECTED(ch, AFF2_QUICKEN))
	{
		switch (level / 25)
		{
			case 0:	wait_state(ch, 1 + skill_table[sn].beats * 3);	break;
			case 1:	wait_state(ch, 1 + skill_table[sn].beats * 2);	break;
			default:	wait_state(ch, 1 + skill_table[sn].beats * 1);	break;
		}
	}

	if ( saves_spell( level, ch, victim ) )
	{
		dam /= 2;
	}

	damage( ch, victim, dam, sn );
	pop_call();
	return;
}

DO_SPELL(spell_mass_invis)
{
	AFFECT_DATA af;
	CHAR_DATA *gch;

	push_call("spell_mass_invis(%p,%p,%p,%p)",sn,level,ch,vo);

	for (gch = ch->in_room->first_person ; gch ; gch = gch->next_in_room)
	{
		if (!is_same_group(gch, ch))
		{
			continue;
		}
		if (IS_AFFECTED(gch, AFF_INVISIBLE) || IS_AFFECTED(gch, AFF_IMP_INVISIBLE))
		{
			continue;
		}

		act("$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM);
		act("You slowly fade out of existence.", gch, NULL, NULL, TO_CHAR);
		af.type      = gsn_invis;
		af.duration  = 24;
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		af.bittype   = AFFECT_TO_CHAR;
		af.bitvector = AFF_INVISIBLE;
		affect_to_char( gch, &af );
	}
	send_to_char( "Ok.\n\r", ch );

	pop_call();
	return;
}

DO_SPELL(spell_null)
{
	push_call("spell_null(%p,%p,%p,%p)",sn,level,ch,vo);

	send_to_char( "That's not a spell!\n\r", ch );
	pop_call();
	return;
}

DO_SPELL(spell_pass_door)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_pass_door(%p,%p,%p,%p)",sn,level,ch,vo);

	if ( IS_AFFECTED(victim, AFF_PASS_DOOR) || rspec_req(victim,RSPEC_PASS_DOOR))
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}
	af.type      = sn;
	af.duration  = number_fuzzy( level / 4 );
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF_PASS_DOOR;
	affect_to_char( victim, &af );
	act( "$n turns translucent.", victim, NULL, NULL, TO_ROOM );
	send_to_char( "You turn translucent.\n\r", victim );

	pop_call();
	return;
}

DO_SPELL(spell_poison)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_poison(%p,%p,%p,%p)",sn,level,ch,vo);

	if ( IS_AFFECTED(victim, AFF_POISON) )
	{
		send_to_char( "They are already poisoned.\n\r", ch );
		pop_call();
		return;
	}

	if ( saves_spell( level, ch, victim ) )
	{
		send_to_char( "They turn slightly green but it passes.\n\r", ch );
		pop_call();
		return;
	}

	if (IS_NPC(victim))
	{
		af.type      = sn;
		af.duration  = level;
		af.location  = APPLY_AC;
		af.modifier  = level;
		af.bittype   = AFFECT_TO_CHAR;
		af.bitvector = AFF_POISON;
	}
	else
	{
		af.type      = sn;
		af.duration  = level/4;
		af.location  = APPLY_STR;
		af.modifier  = -2;
		af.bittype   = AFFECT_TO_CHAR;
		af.bitvector = AFF_POISON;
	}
	affect_to_char(victim, &af );
	send_to_char( "You feel poison coursing through your vains.\n\r", victim );
	if ( ch != victim )
	{
		send_to_char( "They now feel your poison.\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_mage_blast)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_mage_blast(%p,%p,%p,%p)",sn,level,ch,vo);

	if (is_affected(victim, gsn_mage_blast))
	{
		send_to_char( "Your blast was ineffective.\n\r", ch );
		pop_call();
		return;
	}
	if (saves_spell(level, ch, victim))
	{
		af.type      = sn;
		af.duration  = 1;
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		af.bittype   = AFFECT_TO_NONE;
		af.bitvector = 0;
		affect_to_char( victim, &af );
		send_to_char( "Your blast was repelled.\n\r", ch );
		pop_call();
		return;
	}

	af.type      = sn;
	af.duration  = level/20;
	af.location  = APPLY_SAVING_SPELL;
	af.modifier  = level/8;
	af.bittype   = AFFECT_TO_NONE;
	af.bitvector = 0;
	affect_to_char( victim, &af );
	send_to_char( "Your magic aura has been weakened.\n\r", victim );
	if ( ch != victim )
	{
		send_to_char( "You blast through their magical defenses.\n\r", ch );
	}
	pop_call();
	return;
}

/*
	Checked spells till here - Scandum
*/

DO_SPELL(spell_protection_evil)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_protection_evil(%p,%p,%p,%p)",sn,level,ch,vo);

	if ( IS_AFFECTED(victim, AFF_PROTECT_EVIL) )
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}

	af.type      = sn;
	af.duration  = level/4;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF_PROTECT_EVIL;
	affect_to_char( victim, &af );
	send_to_char( "You feel protected from evil.\n\r", victim );
	if ( ch != victim )
	{
		send_to_char( "Protection has been applied from evil.\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_protection_good)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_protection_good(%p,%p,%p,%p)",sn,level,ch,vo);

	if ( IS_AFFECTED(victim, AFF_PROTECT_GOOD) )
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}

	af.type      = sn;
	af.duration  = level/5;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF_PROTECT_GOOD;
	affect_to_char( victim, &af );
	send_to_char( "You feel protected from good.\n\r", victim );
	if ( ch != victim )
	{
		send_to_char( "Protection from good is now working.\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_refresh)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	push_call("spell_refresh(%p,%p,%p,%p)",sn,level,ch,vo);

	victim->move = UMIN( victim->move + level, victim->max_move );
	send_to_char( "You feel less tired.\n\r", victim );
	if ( ch != victim )
	{
		send_to_char( "Movement is now restoring.\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_remove_curse)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj;
	char buf[MAX_STRING_LENGTH];

	push_call("spell_remove_curse(%p,%p,%p,%p)",sn,level,ch,vo);

	if (is_affected(victim, gsn_curse))
	{
		affect_strip( victim, gsn_curse );
		send_to_char( "You feel better.\n\r", victim );
		if (ch != victim)
		{
			send_to_char( "Your spell has taken the curse away.\n\r", ch );
		}
		pop_call();
		return;
	}
	for(obj=victim->first_carrying; obj!=NULL ; obj=obj->next_content)
	if((!IS_SET(obj->extra_flags, ITEM_INVENTORY)) &&
		(IS_SET(obj->extra_flags, ITEM_NODROP) ||
		IS_SET(obj->extra_flags, ITEM_NOREMOVE)))
	{
		REMOVE_BIT( obj->extra_flags, ITEM_NOREMOVE);
		REMOVE_BIT( obj->extra_flags, ITEM_NODROP);
		if( victim!=ch)
			send_to_char( "Ok.\n\r", ch);
		sprintf( buf, "%s is no longer cursed.\n\r", obj->short_descr);
		if( buf[0]>='a' && buf[0]<='z')
			buf[0]-=('a'-'A');
		send_to_char( buf, victim);
		pop_call();
		return;
	}
	send_to_char( "That had no effect.\n\r", ch);

	pop_call();
	return;
}

DO_SPELL(spell_ethereal_travel)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA *paf, *paf_next;
	AFFECT_DATA af;

	push_call("spell_ethereal_travel(%p,%p,%p,%p)",sn,level,ch,vo);

	if( IS_NPC( victim ) )
	{
		send_to_char( "You may only cast that on players.\n\r", ch );
		pop_call();
		return;
	}

	if ( IS_AFFECTED(victim, AFF2_ETHEREAL) )
	{
		for ( paf = victim->first_affect; paf != NULL; paf = paf_next )
		{
			paf_next = paf->next;
			if( paf->bitvector==AFF2_ETHEREAL )
			{
				paf->duration  = number_fuzzy( level / 2 );
				send_to_char( "You faze out a bit more.\n\r", victim );
				if( victim != ch )
					send_to_char( "Ok.\n\r", ch );
				pop_call();
				return;
			}
		}
	}

	af.type      = sn;
	af.duration  = number_fuzzy( level / 2 );
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF2_ETHEREAL;
	affect_to_char( victim, &af );
	act( "$n partially fades out.", victim, NULL, NULL, TO_ROOM );
	send_to_char( "You partially fade out.\n\r", victim );

	pop_call();
	return;
}

DO_SPELL(spell_astral_projection)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af, *paf, *paf_next;

	push_call("spell_astral_projection(%p,%p,%p,%p)",sn,level,ch,vo);

	if( IS_NPC( victim ) )
	{
		send_to_char( "You may only cast that on players.\n\r", ch );
		pop_call();
		return;
	}

	if ( IS_AFFECTED(victim, AFF2_ASTRAL) )
	{
		for ( paf = victim->first_affect; paf != NULL; paf = paf_next )
		{
			paf_next = paf->next;
			if (paf->bitvector == AFF2_ASTRAL)
			{
				paf->duration  = number_fuzzy( level / 3 );
				send_to_char( "You no longer have the urge to return to your body.\n\r", victim );
				if (ch != victim)
				{
					act( "$N no longer has the urge to return to $S body.", ch, NULL, victim, TO_CHAR );
				}
				pop_call();
				return;
			}
		}
	}

	af.type      = sn;
	af.duration  = number_fuzzy( level / 3 );
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF2_ASTRAL;
	affect_to_char( victim, &af );
	act( "$n steps outside of $s body.", victim, NULL, NULL, TO_ROOM );
	send_to_char( "You step outside your body.\n\r", victim );
	pop_call();
	return;
}

DO_SPELL(spell_breath_water)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af, *paf, *paf_next;

	push_call("spell_breath_water(%p,%p,%p,%p)",sn,level,ch,vo);

	if( IS_NPC( victim ) )
	{
		send_to_char( "You may only cast that on players.\n\r", ch );
		pop_call();
		return;
	}

	if ( IS_AFFECTED(victim, AFF2_BREATH_WATER) )
	{
		for ( paf = victim->first_affect; paf != NULL; paf = paf_next )
		{
			paf_next = paf->next;
			if( paf->bitvector == AFF2_BREATH_WATER )
			{
				paf->duration  = number_fuzzy( level / 3 );
				act( "$n takes another deep breath.", victim, NULL, NULL, TO_ROOM );
				send_to_char( "You take another deep breath.\n\r", victim );
				/*if( victim != ch )
				send_to_char( "Gills appear so water can be breathed.\n\r", victim );*/

				pop_call();
				return;
			}
		}
	}

	af.type      = sn;
	af.duration  = number_fuzzy( level / 3 );
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF2_BREATH_WATER;
	affect_to_char( victim, &af );
	act( "$n takes a deep breath.", victim, NULL, NULL, TO_ROOM );
	send_to_char( "You take a deep breath.\n\r", victim );

	pop_call();
	return;
}

DO_SPELL(spell_sanctuary)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_sanctuary(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_AFFECTED(victim, AFF_SANCTUARY))
	{
		send_to_char( "That person already has Sanctuary.\n\r", ch);
		pop_call();
		return;
	}
	af.type      = sn;
	af.duration  = number_fuzzy( level / 4 );
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF_SANCTUARY;
	affect_to_char( victim, &af );

	act( "$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM );
	send_to_char( "You are surrounded by a white aura.\n\r", victim );

	pop_call();
	return;
}

DO_SPELL(spell_shield)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_shield(%p,%p,%p,%p)",sn,level,ch,vo);

	if ( is_affected( victim, sn ) )
	{
		send_to_char( "That person is already Shielded.\n\r", ch);
		pop_call();
		return;
	}
	af.type      = sn;
	af.duration  = 8 + level/2;
	af.location  = APPLY_AC;
	af.modifier  = -20;
	af.bittype   = AFFECT_TO_NONE;
	af.bitvector = 0;
	affect_to_char( victim, &af );
	act( "$n is surrounded by a force shield.", victim, NULL, NULL, TO_ROOM );
	send_to_char( "You are surrounded by a force shield.\n\r", victim );
	pop_call();
	return;
}

DO_SPELL(spell_shocking_grasp)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_shocking_grasp(%p,%p,%p,%p)",sn,level,ch,vo);

	dam   = number_range(level+60, level+90);

	if ( saves_spell( level, ch, victim ) )
	{
		dam /= 2;
	}
	damage( ch, victim, dam, sn );

	pop_call();
	return;
}

DO_SPELL(spell_paralyzing_embrace)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_paralyzing_embrace(%p,%p,%p,%p)",sn,level,ch,vo);

	dam   = number_range(level+20, level+120);

	if ( saves_spell( level, ch, victim ) )
	{
		dam /= 2;
	}
	else if (number_bits(1) == 0)
	{
		AFFECT_DATA af;

		af.type      = sn;
		af.duration  = 1;
		if (!IS_NPC(victim))
		{
			if (number_bits(1) == 0)
			{
				af.location = APPLY_INT;
				af.modifier = -1;
			}
			else
			{
				af.location = APPLY_WIS;
				af.modifier = -1;
			}
		}
		else
		{
			af.location = APPLY_SAVING_SPELL;
			af.modifier = 1;
		}
		af.bittype   = AFFECT_TO_NONE;
		af.bitvector = 0;
		affect_join( victim, &af );
	}
	damage( ch, victim, dam, sn );

	pop_call();
	return;
}

DO_SPELL(spell_sleep)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_sleep(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_char("You are forbidden from casting that here.\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && !IS_NPC(victim))
	{
		if (!can_attack(ch, victim))
		{
			send_to_char("You cannot cast sleep on that person.\n\r", ch);
			pop_call();
			return;
		}
		check_killer(ch, victim);
	}

	if ( IS_AFFECTED(victim, AFF_SLEEP) )
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}

	if (saves_spell(level, ch, victim))
	{
		act("$n glares icily at you as your sleep spell fails.", victim, NULL, ch, TO_VICT);
		act("$n glares icily at $N as $S sleep spell fails.",  victim, NULL, ch, TO_NOTVICT);
		act("You glare icily at $N as $S sleep spell fails.", victim, NULL, ch, TO_CHAR);

		if (IS_NPC(victim) && victim->fighting == NULL)
		{
			make_char_fight_char(ch, victim);
			multi_hit(victim, ch, TYPE_UNDEFINED);
		}
		pop_call();
		return;
	}

	af.type      = sn;
	af.duration  = level / 5;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF_SLEEP;
	affect_to_char( victim, &af );

	if (IS_AWAKE(victim))
	{
		send_to_char( "You feel very sleepy ..... zzzzzz.\n\r", victim);
		act( "$n goes to sleep.", victim, NULL, NULL, TO_ROOM);
		victim->position = POS_SLEEPING;
	}

	pop_call();
	return;
}

DO_SPELL(spell_stone_skin)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_stone_skin(%p,%p,%p,%p)",sn,level,ch,vo);

	if ( is_affected( victim, sn ) )
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}
	af.type      = sn;
	af.duration  = level/2;
	af.location  = APPLY_AC;
	af.modifier  = -40;
	af.bittype   = AFFECT_TO_NONE;
	af.bitvector = 0;
	affect_to_char( victim, &af );
	act( "$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM );
	send_to_char( "Your skin turns to stone.\n\r", victim );

	pop_call();
	return;
}

DO_SPELL(spell_demon)
{
	MOB_INDEX_DATA *pMob;
	CHAR_DATA *mh;

	push_call("spell_demon(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_char("You are forbidden from casting that here.\n\r", ch);
		pop_call();
		return;
	}

	if (get_pets(ch) > 0)
	{
		send_to_char("You have too many pets.\n\r", ch);
		pop_call();
		return;
	}

	if (ch->position == POS_FIGHTING)
	{
		send_to_char( "You cannot cast this during combat.\n\r", ch);
		pop_call();
		return;
	}

	pMob = get_mob_index( 9901 );   /* Hard coded in emud.are */
	mh = create_mobile( pMob );
	char_to_room( mh, ch->in_room->vnum );

	mh->level				= level * 3 / 4;
	mh->npcdata->damnodice	= mh->level;
	mh->npcdata->damsizedice	= 4;
	mh->npcdata->damplus	= 3;
	mh->max_hit			= 80 + mh->level * 4;
	mh->hit				= mh->max_hit;

	if (number_percent() < 66)
	{
		SET_BIT( mh->affected_by , AFF_CHARM );
		send_to_char( "A demon appears out of a rip in space.\n\r", ch);
		act("A demon appears out of a rip in space.", ch, NULL, NULL, TO_ROOM);
		add_follower( mh , ch );

		pop_call();
		return;
	}
	mh->level		= level;
	mh->max_hit	= 30 + mh->level * 10;
	send_to_char( "A demon appears out of a rip in space.\n\rHe seems angry.\n\r", ch);
	multi_hit(mh, ch, TYPE_UNDEFINED);

	pop_call();
	return;
}

DO_SPELL(spell_animate_dead)
{
	CHAR_DATA *mh;
	OBJ_DATA	*obj;

	push_call("spell_animate_dead(%p,%p,%p,%p)",sn,level,ch,vo);

	if (get_pets(ch) > 0)
	{
		send_to_char( "You have too many pets.\n\r", ch);
		pop_call();
		return;
	}

	if (ch->position == POS_FIGHTING)
	{
		send_to_char( "You cannot cast this during combat.\n\r", ch);
		pop_call();
		return;
	}

	for (obj = ch->in_room->first_content ; obj ; obj = obj->next_content)
	{
		if (obj->item_type == ITEM_CORPSE_NPC && obj->level <= level)
		{
			break;
		}
	}

	if (obj == NULL)
	{
		send_to_char( "You find no suitable corpse.\n\r", ch);
		pop_call();
		return;
	}

	mh		= create_mobile(mob_index[9903]);
	char_to_room( mh, ch->in_room->vnum );

	mh->level				= obj->level;
	mh->hitroll			= 0;
	mh->npcdata->damnodice	= mh->level;
	mh->npcdata->damsizedice	= 3;
	mh->npcdata->damplus	= 2;
	mh->max_hit			= 50 + mh->level * 5;
	mh->hit				= mh->max_hit;

	SET_BIT( mh->affected_by , AFF_CHARM );
	SET_BIT( mh->act, ACT_UNDEAD );

	if (IS_SET(obj->extra_flags, ITEM_MOUNT))
	{
		SET_BIT( mh->act, ACT_MOUNT );
	}

	act("$n raises $p from death.", ch, obj, NULL, TO_ROOM );

	RESTRING(mh->name,			obj->name);
	RESTRING(mh->short_descr,	obj->short_descr);
	RESTRING(mh->long_descr,		"");
	RESTRING(mh->description,	"");

	while (obj->first_content)
	{
		if (number_bits(1))
		{
			obj_to_char(obj->first_content, mh);
		}
		else
		{
			junk_obj(obj->first_content);
		}
	}
	junk_obj(obj);

	do_wear(mh, "all");

	add_follower( mh , ch );

	pop_call();
	return;
}

DO_SPELL(spell_beast)
{
	MOB_INDEX_DATA *pMob;
	CHAR_DATA *mh;

	push_call("spell_beast(%p,%p,%p,%p)",sn,level,ch,vo);

	if (get_pets(ch) > 0)
	{
		send_to_char( "You have too many pets.\n\r", ch);
		pop_call();
		return;
	}

	if (ch->position == POS_FIGHTING)
	{
		send_to_char( "You cannot cast this during combat.\n\r", ch);
		pop_call();
		return;
	}

	pMob = get_mob_index( 9902 );   /* Hard coded in emud.are */
	mh = create_mobile( pMob );
	char_to_room( mh, ch->in_room->vnum );

	mh->level				= level * 3 / 4;
	mh->npcdata->damnodice	= mh->level;
	mh->npcdata->damsizedice	= 2;
	mh->npcdata->damplus	= 5;
	mh->max_hit			= 30 + level * 4;
	mh->hit				= 30 + level * 4;

	SET_BIT(mh->affected_by, AFF_CHARM);

	send_to_char( "A shadow beast appears out of a ripple in the wall.\n\r", ch);
	add_follower( mh , ch );

	if (ch->fighting != NULL)
	{
		multi_hit( mh, ch->fighting->who, TYPE_UNDEFINED);
	}
	pop_call();
	return;
}

DO_SPELL(spell_shade)
{
	MOB_INDEX_DATA *pMob;
	CHAR_DATA *mh;

	push_call("spell_shade(%p,%p,%p,%p)",sn,level,ch,vo);

	if (get_pets(ch) > 0)
	{
		send_to_char("You have too many pets.\n\r", ch);
		pop_call();
		return;
	}

	pMob = get_mob_index( 9903 );   /* Hard coded in emud.are */
	mh = create_mobile( pMob );
	char_to_room( mh, ch->in_room->vnum );

	mh->level				= level * 2 / 3;
	mh->npcdata->damnodice	= mh->level / 2;
	mh->npcdata->damsizedice	= 2;
	mh->npcdata->damplus	= 5;
	mh->max_hit			= 30 + level * 3;
	mh->hit				= 30 + level * 3;

	SET_BIT(mh->affected_by , AFF_CHARM);

	act("A Shade appears out of a ripple in the floor.", ch, NULL, NULL, TO_CHAR);
	act("A Shade appears out of a ripple in the floor.", ch, NULL, NULL, TO_ROOM);

	add_follower( mh , ch );

	pop_call();
	return;
}

DO_SPELL(spell_phantasm)
{
	MOB_INDEX_DATA *pMob;
	CHAR_DATA *mh;

	push_call("spell_phantasm(%p,%p,%p,%p)",sn,level,ch,vo);

	if (get_pets(ch) > 0)
	{
		send_to_char( "You have too many pets.\n\r", ch);
		pop_call();
		return;
	}

	pMob = get_mob_index( 9904 );   /* Hard coded in emud.are */
	mh = create_mobile( pMob );
	char_to_room( mh, ch->in_room->vnum );

	mh->level				= level * 1 / 2;
	mh->npcdata->damnodice	= mh->level;
	mh->npcdata->damsizedice	= 5;
	mh->npcdata->damplus	= 50;
	mh->max_hit			= 50 + mh->level * 6;
	mh->hit				= 50 + mh->level * 6;

	SET_BIT( mh->affected_by , AFF_CHARM );

	send_to_char( "A phantasmal killer appears out of the palm of your hand.\n\r", ch);
	add_follower( mh , ch );

	if (ch->fighting != NULL)
	{
		multi_hit(mh, ch->fighting->who, TYPE_UNDEFINED );
	}
	pop_call();
	return;
}

DO_SPELL(spell_summon)
{
	CHAR_DATA *victim;

	push_call("spell_summon(%p,%p,%p,%p)",sn,level,ch,vo);

	if (target_name[0] == '\0')
	{
		send_to_char("Whom would you like to summon?\n\r", ch);
		pop_call();
		return;
	}

	if ((victim = get_char_area(ch, target_name)) == NULL)
	{
		send_to_char( "There is nothing named like that.\n\r", ch);
		pop_call();
		return;
	}

	if (IS_NPC(victim) && level / 3 < victim->level)
	{
		send_to_char( "You cannot summon those that do not wish to leave.\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_NPC(victim) && is_affected(victim, gsn_anti_magic_shell))
	{
		act("The flows of magic shatter against a shell surrounding $N.", ch, NULL, victim, TO_CHAR);
		act("Flows of magic shatter against your anti magic shell.", ch, NULL, victim, TO_VICT);
		pop_call();
		return;
	}

	if (victim == ch
	|| victim->in_room == NULL
	|| IS_SET(victim->in_room->room_flags, ROOM_SAFE)
	|| IS_SET(ch->in_room->room_flags, ROOM_SAFE)
	|| IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
	|| victim->in_room->sector_type == SECT_ETHEREAL
	|| victim->in_room->sector_type == SECT_ASTRAL
	|| ch->in_room->sector_type == SECT_ETHEREAL
	|| ch->in_room->sector_type == SECT_ASTRAL
	|| IS_SET(ch->in_room->area->flags, AFLAG_NORECALL)
	|| IS_SET(ch->in_room->area->flags, AFLAG_NOSUMMON)
	|| victim->level > level - 3
	|| victim->fighting != NULL
	|| find_keeper(victim) != NULL)
	{
		send_to_char( "You failed.\n\r", ch );
		pop_call();
		return;
	}

	if (IS_AFFECTED(victim, AFF_STABILITY) || saves_spell(level / 2, ch, victim))
	{
		send_to_char( "They resist your summoning spell.\n\r", ch);
		act("You feel a strange magical force pulling at you.", ch, NULL, victim, TO_VICT);

		pop_call();
		return;
	}

	act( "$n disappears suddenly.", victim, NULL, NULL, TO_ROOM );

	char_from_room( victim );
	char_to_room( victim, ch->in_room->vnum );

	act( "$n arrives suddenly.", victim, NULL, NULL, TO_ROOM );
	ch_printf(victim, "%s has summoned you!\n\r", get_name(ch));

	do_look( victim, "auto" );

	pop_call();
	return;
}

DO_SPELL(spell_banish)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	CHAR_DATA *rch;
	ROOM_INDEX_DATA *pRoomIndex;
	int cnt = 0;

	push_call("spell_banish(%p,%p,%p,%p)",sn,level,ch,vo);

	if (victim->in_room == NULL
	||  victim == ch
	||  IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
	||  IS_SET(ch->in_room->area->flags, AFLAG_NORECALL)
	||  IS_SET(ch->in_room->room_flags,  AFLAG_NOSUMMON)
	||  saves_spell(level, ch, victim))
	{
		send_to_char( "You failed.\n\r", ch );
		pop_call();
		return;
	}

	if (saves_spell(level, ch, victim))
	{
		victim = ch;
	}

	for ( ; ; )
	{
		/*
			Better safe than freezing, added cnt - Scandum 01-04-2002
		*/
		if (cnt++ > 10000)
		{
			send_to_char("Nothing happens", ch);
			pop_call();
			return;
		}
		pRoomIndex = get_room_index(number_range(victim->in_room->area->low_r_vnum, victim->in_room->area->hi_r_vnum));

		if ( pRoomIndex != NULL )
		{
			if (IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)
			||  IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY)
			||  IS_SET(pRoomIndex->room_flags, ROOM_SAFE)
			||  pRoomIndex == victim->in_room)
			{
				continue;
			}
			for (rch = pRoomIndex->first_person ; rch ; rch = rch->next_in_room)
			{
				if (IS_NPC(rch) && rch->pIndexData->pShop != NULL)
				{
					break;
				}
			}
			if (rch)
			{
				continue;
			}
			break;
		}
	}

	act( "$n is banished from existence.", victim, NULL, NULL, TO_ROOM );

	if (victim->position == POS_FIGHTING)
	{
		stop_fighting(victim, TRUE);
	}
	char_from_room( victim );
	char_to_room( victim, pRoomIndex->vnum );

	if (victim == ch)
	{
		send_to_char("You failed and banished yourself from existance.\n\r", victim);
	}
	else
	{
		send_to_char("You've been banished from existence.\n\r", victim);
	}
	do_look( victim, "auto" );

	pop_call();
	return;
}

DO_SPELL(spell_teleport)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	ROOM_INDEX_DATA *pRoomIndex;
	int attempts;

	push_call("spell_teleport(%p,%p,%p,%p)",sn,level,ch,vo);

	victim = ch;

	if (victim->in_room == NULL
	|| IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
	|| IS_SET(victim->in_room->area->flags, AFLAG_NORECALL)
	|| (!IS_NPC(ch) && victim->fighting != NULL)
	|| (IS_NPC(ch) && saves_spell(level, ch, victim)))
	{
		send_to_char( "You failed.\n\r", ch );
		pop_call();
		return;
	}

	for (attempts = 0 ; attempts < 1000 ; attempts++)
	{

		pRoomIndex = get_room_index( number_range(1, MAX_VNUM-1));
		if(pRoomIndex != NULL)
		{
			if(!IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)
			&& !IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY)
			&& !IS_SET(pRoomIndex->room_flags, ROOM_IS_CASTLE)
			&& !IS_SET(pRoomIndex->room_flags, ROOM_PET_SHOP)
			&& !IS_SET(pRoomIndex->room_flags, ROOM_RIP)
			&& !IS_SET(pRoomIndex->room_flags, ROOM_NO_RECALL)
			&& !IS_SET(pRoomIndex->area->flags, AFLAG_NOTELEPORT)
			&& !IS_SET(pRoomIndex->area->flags, AFLAG_NORECALL)
			&& pRoomIndex->area->average_level*2 < victim->level+15
			&& pRoomIndex->sector_type != SECT_ETHEREAL
			&& pRoomIndex->sector_type != SECT_ASTRAL
			&& !((!IS_IMMORTAL(victim))
			&& (victim->level<pRoomIndex->area->low_hard_range
			||  victim->level>pRoomIndex->area->hi_hard_range)))
			{
				break;
			}
		}
	}
	if (attempts >= 1000)
	{
		send_to_char("The earth decides you should remain where you are.\n\r", ch);
		pop_call();
		return;
	}

	act("$n slowly fades out of existence.", victim, NULL, NULL, TO_ROOM);

	if (victim->position == POS_FIGHTING && !IS_NPC(ch))
	{
		stop_fighting(victim, TRUE);
	}
	char_from_room(victim);
	char_to_room(victim, pRoomIndex->vnum);
	act("$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM);
	do_look(victim, "auto");
	pop_call();
	return;
}

DO_SPELL(spell_ventriloquate)
{
	char buf1[MAX_STRING_LENGTH];
	char speaker[MAX_INPUT_LENGTH];
	CHAR_DATA *vch;

	push_call("spell_ventriloquate(%p,%p,%p,%p)",sn,level,ch,vo);

	if( ch->in_room == NULL )
	{
		pop_call();
		return;
	}
	for( vch = ch->in_room->first_person; vch != NULL; vch=vch->next_in_room )
	{
		if( !IS_NPC( vch ) && vch->level >= 99 )
		{
			sprintf( buf1, "You may not cast that with %s in the room.\n\r",
			capitalize( vch->name ) );
			send_to_char( buf1, ch );
			pop_call();
			return;
		}
	}

	target_name = one_argument( target_name, speaker );

	for ( vch = ch->in_room->first_person; vch != NULL; vch = vch->next_in_room )
	{
		if( vch->position >= POS_RESTING )
		{
			if ( !is_name_short( speaker, vch->name ) )
			{
				if( !saves_spell( level, ch, vch ) )
					sprintf( buf1, "%s%s says '%s'\n\r", get_color_string(vch,COLOR_SPEACH,VT102_DIM),
				capitalize( speaker ), target_name );
				else
					sprintf( buf1, "%sSomeone makes %s say '%s'\n\r", get_color_string(vch,COLOR_SPEACH,VT102_DIM),
				capitalize(speaker), target_name );
				send_to_char( buf1, vch );
			}
		}
	}

	pop_call();
	return;
}

DO_SPELL(spell_weaken)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_weaken(%p,%p,%p,%p)",sn,level,ch,vo);

	if ( is_affected( victim, sn ))
	{
		send_to_char( "They are already weakened.\n\r", ch );
		pop_call();
		return;
	}

	if( saves_spell( level, ch, victim ) )
	{
		send_to_char( "Nothing happens.\n\r", ch );
		pop_call();
		return;
	}

	af.type      = sn;
	af.duration  = level / 10;
	if(IS_NPC(victim))
	{
		af.location  = APPLY_DAMROLL;
		af.modifier  = 0-(level/10)-2;
	}
	else
	{
		af.location  = APPLY_STR;
		af.modifier  = 0-(level/45)-2;
	}
	af.bittype   = AFFECT_TO_NONE;
	af.bitvector = 0;

	affect_to_char( victim, &af );
	send_to_char( "Your strength flows out of your body.\n\r", victim );
	if ( ch != victim )
	{
		send_to_char( "They now lost the strength to fight.\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_word_of_recall)
{

	push_call("spell_word_of_recall(%p,%p,%p,%p)",sn,level,ch,vo);

	do_recall( (CHAR_DATA *) vo, NULL );

	pop_call();
	return;
}

/*
	Breath Spells
*/

DO_SPELL(spell_acid_breath)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_acid_breath(%p,%p,%p,%p)",sn,level,ch,vo);

	dam = number_range(level*3+50, level*6+100);

	if (saves_spell(level, ch, victim))
	{
		dam /= 2;
	}

	damage( ch, victim, dam, sn );

	pop_call();
	return;
}

DO_SPELL(spell_fire_breath)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj_lose;
	OBJ_DATA *obj_next;
	int dam;

	push_call("spell_fire_breath(%p,%p,%p,%p)",sn,level,ch,vo);

	if (number_percent() < level / 2 && !saves_spell(level, ch, victim))
	{
		for (obj_lose = victim->first_carrying ; obj_lose ; obj_lose = obj_next)
		{
			obj_next = obj_lose->next_content;

			if (number_bits(2) != 0 || obj_lose->first_content)
			{
				continue;
			}

			switch (obj_lose->item_type)
			{
				default:			continue;
				case ITEM_POTION:	act("$p bubbles and boils!", victim, obj_lose, NULL, TO_CHAR);	break;
				case ITEM_SCROLL:	act("$p crackles and burns!", victim, obj_lose, NULL, TO_CHAR);	break;
				case ITEM_STAFF:	act("$p smokes and chars!", victim, obj_lose, NULL, TO_CHAR);	break;
				case ITEM_WAND:	act("$p sparks and sputters!", victim, obj_lose, NULL, TO_CHAR);	break;
				case ITEM_FOOD:	act("$p blackens and crisps!", victim, obj_lose, NULL, TO_CHAR);	break;
				case ITEM_PILL:	act("$p melts and drips!", victim, obj_lose, NULL, TO_CHAR);	break;
			}
			junk_obj( obj_lose );
		}
	}

	dam = number_range(level*10, level*12);

	if (saves_spell(level, ch, victim))
	{
		dam /= 2;
	}

	damage( ch, victim, dam, sn );

	pop_call();
	return;
}

DO_SPELL(spell_frost_breath)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj_lose;
	OBJ_DATA *obj_next;
	int dam;

	push_call("spell_frost_breath(%p,%p,%p,%p)",sn,level,ch,vo);

	if (number_percent() < level && !saves_spell(level, ch, victim))
	{
		for (obj_lose = victim->first_carrying ; obj_lose ; obj_lose = obj_next)
		{
			obj_next = obj_lose->next_content;

			if (number_bits(2) != 0 || obj_lose->first_content)
			{
				continue;
			}

			switch (obj_lose->item_type)
			{
				default:				continue;
				case ITEM_DRINK_CON:
				case ITEM_POTION:		act("$p freezes and shatters!", victim, obj_lose, NULL, TO_CHAR);	break;
			}
			junk_obj( obj_lose );
		}
	}

	dam = number_range(level*4, level*18);

	if (saves_spell(level, ch, victim))
	{
		dam /= 2;
	}
	damage( ch, victim, dam, sn );

	pop_call();
	return;
}

DO_SPELL(spell_gas_breath)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_gas_breath(%p,%p,%p,%p)",sn,level,ch,vo);

	dam = number_range(level*8, level*14);

	if (saves_spell(level, ch, victim))
	{
		dam /= 2;
	}
	else
	{
		if (number_bits(3) == 0 && !IS_AFFECTED(victim, AFF_POISON))
		{
			AFFECT_DATA af;

			af.type      = gsn_poison;
			af.location  = APPLY_AC;
			af.modifier  = level;
			af.duration  = level/10;
			af.bittype   = AFFECT_TO_CHAR;
			af.bitvector = AFF_POISON;

			affect_to_char( victim, &af );

			send_to_char( "You feel poison coursing through your vains.\n\r", victim );
			if (ch != victim)
			{
				send_to_char( "They now feel your poison.\n\r", ch );
			}
		}
	}
	damage(ch, victim, dam, sn);

	pop_call();
	return;
}

DO_SPELL(spell_lightning_breath)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_lightning_breath(%p,%p,%p,%p)",sn,level,ch,vo);

	dam = number_range(level*6, level*16);

	if (saves_spell(level, ch, victim))
	{
		dam /= 2;
	}
	damage( ch, victim, dam, sn );

	pop_call();
	return;
}

DO_SPELL(spell_psionic_shockwave)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_psionic_shockwave(%p,%p,%p,%p)",sn,level,ch,vo);

	dam = number_range(level*2, level*20);

	if (saves_spell(level, ch, victim))
	{
		dam /= 2;
	}
	damage( ch, victim, dam, sn );

	pop_call();
	return;
}

DO_SPELL(spell_block_area)
{
	push_call("spell_block_area(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_char("You are restricted from blocking this area.\n\r",ch);
		pop_call();
		return;
	}

	if (ch->in_room->sector_type < 2 || ch->in_room->sector_type > 4)
	{
		send_to_char("The area pulsates with life but no plant life appears.\n\r", ch);
		pop_call();
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_BLOCK))
	{
		send_to_char("The area pulsates with life but nothing new appears.\n\r",ch);
		pop_call();
		return;
	}

	SET_BIT(ch->in_room->room_flags, ROOM_BLOCK);

	if (!del_room_timer(ch->in_room->vnum, ROOM_TIMER_UNBLOCK))
	{
		set_room_timer(ch->in_room->vnum, ROOM_TIMER_BLOCK, level / 2);
	}
	act( "All sorts of plant growth sprout up everywhere!", ch, NULL, NULL, TO_ROOM);
	act( "Your magic fills the area with plant life!", 	 ch, NULL, NULL, TO_CHAR);

	pop_call();
	return;
}

DO_SPELL(spell_write_spell)
{
	char buf[MAX_INPUT_LENGTH];
	char buf2[MAX_INPUT_LENGTH];
	OBJ_DATA *scroll;
	int sn2,i;

	push_call("spell_write_spell(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_NPC(ch) || multi(ch, sn) == -1)
	{
		send_to_char( "You can't do that!\n\r", ch);
		pop_call();
		return;
	}

	if ((scroll = get_eq_char(ch, WEAR_HOLD)) == NULL || scroll->item_type != ITEM_SCROLL)
	{
		send_to_char("You need to hold the scroll you wish to write upon.\n\r",ch);
		pop_call();
		return;
	}

	for (i = 0 ; i < 100 ; i++)
	{
		sn2 = number_range(0, MAX_SKILL-1);

		if (skill_table[sn2].slot <= 0)
		{
			continue;
		}
		if (!IS_SET(skill_table[sn2].flags, FSKILL_SPELL))
		{
			continue;
		}
		if (ch->pcdata->learned[sn2] == 0)
		{
			continue;
		}
		break;
	}

	if (i >= 100)
	{
		send_to_char("Your mind goes blank!\n\r", ch);
		ch->mana = 0;
		pop_call();
		return;
	}

	if (level < 21)
	{
		sprintf(buf, "a haphazardly written scroll of %s", skill_table[sn2].name);
	}
	else if (level < 31)
	{
		sprintf(buf, "a decently scribed scroll of %s", skill_table[sn2].name);
	}
	else
	{
		sprintf(buf, "a skillfully crafted scroll of %s", skill_table[sn2].name);
	}

	sprintf(buf2, "scroll %s", skill_table[sn2].name);
	RESTRING(scroll->name, buf2);
	RESTRING(scroll->short_descr, buf);
	RESTRING(scroll->description, buf);

	scroll->cost		= 0;
	scroll->level		= level * 3 / 4;
	scroll->value[0]	= level;
	scroll->value[1]	= sn2;
	scroll->value[2]	= 0;
	scroll->value[3]	= 0;

	SET_BIT(scroll->extra_flags, ITEM_MODIFIED);

	act("$n writes what is hoped to be a useful spell.",	ch, scroll, NULL, TO_ROOM);
	act("You enter a trance and.. ..you now hold $p!",	ch, scroll, NULL, TO_CHAR);

	pop_call();
	return;
}

DO_SPELL(spell_homonculous)
{
	MOB_INDEX_DATA *pMob;
	CHAR_DATA *mh;

	push_call("spell_homonculous(%p,%p,%p,%p)",sn,level,ch,vo);

	if (ch->desc == NULL || ch->desc->original != NULL)
	{
		send_to_char( "You already are switched into something.\n\r", ch);
		pop_call();
		return;
	}

	pMob	= get_mob_index( 9900 );   /* Hard coded in emud.are */
	mh	= create_mobile( pMob );
	char_to_room(mh, ch->in_room->vnum);

	act("A clap of thunder and a small, ugly creature appears!",	ch, NULL, NULL, TO_ROOM);
	act("A clap of thunder and you suddenly feel smaller!",		ch, NULL, NULL, TO_CHAR);

	ch->desc->character 	= mh;
	ch->desc->original  	= ch;
	mh->desc            	= ch->desc;
	ch->desc            	= NULL;
	ch->pcdata->switched	= TRUE;

	pop_call();
	return;
}

DO_SPELL(spell_enhance_object)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA af;
	int stat;

	push_call("spell_enhance_object(%p,%p,%p,%p)",sn,level,ch,vo);

	if (obj->owned_by)
	{
		send_to_char( "You cannot enhance that object.\n\r", ch);
		pop_call();
		return;
	}

	stat = UMIN(1 + (level-1) / 25, 6);

	switch (obj->item_type)
	{
		case ITEM_ARMOR:
			af.location = APPLY_AC;
			af.modifier = 0-stat;
			break;
		case ITEM_WEAPON:
			af.location = APPLY_HITROLL;
			af.modifier = stat;
			break;
		default:
			send_to_char("You cannot enhance that kind of an object.\n\r", ch);
			pop_call();
			return;
	}

	obj->owned_by	= ch->pcdata->pvnum;

	SET_BIT(obj->extra_flags, ITEM_MODIFIED);

	af.type		= sn;
	af.duration	= -1;
	af.bittype	= AFFECT_TO_NONE;
	af.bitvector	= 0;

	affect_to_obj(obj, &af);

	char_reset( ch );

	act("$p has been enhanced.", ch, obj, NULL, TO_CHAR);

	pop_call();
	return;
}

DO_SPELL(spell_mage_shield)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_mage_shield(%p,%p,%p,%p)",sn,level,ch,vo);

	if (is_affected(victim, sn))
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}
	af.type      = sn;
	af.duration  = level/3;
	af.location  = APPLY_SAVING_SPELL;
	af.modifier  = -level/5;
	af.bittype   = AFFECT_TO_NONE;
	af.bitvector = 0;
	affect_to_char( victim, &af );

	act( "$n is quickly covered by a shimmering glow.", victim, NULL, NULL, TO_ROOM );
	send_to_char( "You are quickly covered by a shimmering glow.\n\r", victim );

	pop_call();
	return;
}

DO_SPELL(spell_rift)
{
	char buf[MAX_INPUT_LENGTH];
	EXIT_DATA *pexit;
	ROOM_INDEX_DATA *to_room;
	int door, rip_door;

	push_call("spell_rift(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (!IS_SET(ch->in_room->room_flags, ROOM_RIP))
	{
		send_to_char("You can only create rifts from inside of a rip.\n\r",ch);
		pop_call();
		return;
	}

	to_room = get_rift_room(ch, value);

	/*
		pick a random exit
	*/

	for (door = 0 ; door < 6 ; door++)
	{
		if (ch->in_room->exit[door] && IS_SET(ch->in_room->exit[door]->flags, EX_RIFT))
		{
			send_to_char("Sorry, only one rift per rip.\n\r", ch);
			pop_call();
			return;
		}
	}

	for (rip_door = 0 ; rip_door < 6 ; rip_door++)
	{
		door = number_door();

		if (to_room->exit[door] == NULL && ch->in_room->exit[rev_dir[door]] == NULL)
		{
			rip_door = rev_dir[door];
			break;
		}
	}

	if (rip_door == 6)
	{
		send_to_char("You failed to open a rift using that wrinkle in this room.\n\r",ch);

		pop_call();
		return;
	}

	/*
		Create 2-way connection
	*/

	create_exit(to_room, door);
	pexit = to_room->exit[door];

	RESTRING(pexit->description, "You see a rift in space and time.");

	pexit->flags     = EX_RIP;
	pexit->to_room   = ch->in_room->vnum;


	create_exit(ch->in_room, rip_door);
	pexit = ch->in_room->exit[rip_door];

	sprintf(buf, "You see a wrinkle in time and space leading toward %s.", to_room->area->name);
	RESTRING(pexit->description, buf);

	pexit->to_room   = to_room->vnum;
	pexit->flags     = EX_RIFT;

	act("Space and time appear to warp for a split second!", ch, NULL, NULL, TO_ROOM);

	switch (rip_door)
	{
		case DIR_UP:   sprintf(buf, "You cause space and time to warp irreparably above you."); 					break;
		case DIR_DOWN: sprintf(buf, "You cause space and time to warp irreparably below you."); 					break;
		default:       sprintf(buf, "You cause space and time to warp irreparably to the %s.", dir_name[rip_door]);	break;
	}
	act (buf, ch, NULL, NULL, TO_CHAR );

	pop_call();
	return;
}

DO_SPELL(spell_rip)
{
	char buf[MAX_INPUT_LENGTH];
	EXIT_DATA *pexit;
	ROOM_INDEX_DATA *pRoomIndex;
	int door, rip_door, vnum, range;

	push_call("spell_rip(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (!IS_SET(ch->in_room->room_flags, ROOM_RIP))
	{
		if (IS_SET(ch->in_room->room_flags,	ROOM_SAFE)
		||  IS_SET(ch->in_room->room_flags,	ROOM_NO_RIP)
		||  IS_SET(ch->in_room->area->flags,	AFLAG_NORIP)
		||  IS_SET(ch->in_room->room_flags,	ROOM_NO_RECALL)
		||  IS_SET(ch->in_room->area->flags,	AFLAG_NORECALL))
		{
			send_to_char("You are prevented from ripping space here!\n\r",ch);
			pop_call();
			return;
		}
	}

	range = 0;

	for (door = 0 ; door < 6 ; door++)
	{
		if (ch->in_room->exit[door] == NULL)
		{
			range++;

			continue;
		}

		if (!HAS_BIT(ch->in_room->exit[door]->flags, EX_RIP))
		{
			continue;
		}

		if (ch->pcdata->pvnum == room_index[ch->in_room->exit[door]->to_room]->creator_pvnum)
		{
			send_to_char("You failed to create a rip here.\n\r", ch);

			pop_call();
			return;
		}
	}

	if (range == 0)
	{
		send_to_char("You failed to create a rip here.\n\r", ch);

		pop_call();
		return;
	}
	vnum     = number_range(1, range);
	rip_door = 0;

	for (door = 0 ; door < 6 ; door++)
	{
		if (ch->in_room->exit[door] == NULL)
		{
			if (--vnum == 0)
			{
				rip_door = door;
				break;
			}
		}
	}

	/* find valid vnum */

	for (vnum = ROOM_VNUM_RIFT ; vnum < MAX_VNUM ; vnum++)
	{
		if (room_index[vnum] == NULL)
		{
			break;
		}
	}

	if (vnum >= MAX_VNUM)
	{
		send_to_char("You are prevented from ripping up the Realm!\n\r",ch);
		pop_call();
		return;
	}

	pRoomIndex = create_room(vnum);

	sprintf(buf, "Haven a la %s.", ch->name);
	pRoomIndex->name          = STRALLOC(buf);
	pRoomIndex->area          = room_index[ROOM_VNUM_LIMBO]->area;
	pRoomIndex->vnum          = vnum;
	pRoomIndex->description   = STRALLOC("You see an area that defies all description.\n\r");
	pRoomIndex->room_flags    = ROOM_RIP|ROOM_NO_MOB|ROOM_INDOORS|ROOM_NO_RECALL|ROOM_NO_SAVE|ROOM_SAFE;
	pRoomIndex->sector_type   = SECT_INSIDE;
	pRoomIndex->creator_pvnum = ch->pcdata->pvnum;

	for (door = 0 ; door < MAX_LAST_LEFT ; door++)
	{
		pRoomIndex->last_left_name[door] = STRDUPE(str_empty);
	}

	/* new room exit points to old room */

	create_exit(pRoomIndex, rev_dir[rip_door]);
	pexit = pRoomIndex->exit[rev_dir[rip_door]];

	RESTRING(pexit->description, "You see an opening exiting the fabric of space and time.");

	pexit->to_room   = ch->in_room->vnum;


	create_exit(ch->in_room, rip_door);
	pexit = ch->in_room->exit[rip_door];

	RESTRING(pexit->description, "You see an opening in the fabric of space and time.");

	pexit->flags     = EX_RIP;
	pexit->to_room   = pRoomIndex->vnum;


	act("$n creates a room of haven $twards.", ch, dir_name[rip_door], NULL, TO_ROOM);
	ch_printf(ch, "You create a room of haven %swards.\n\r", dir_name[rip_door]);

	pop_call();
	return;
}

DO_SPELL(spell_envision)
{
	ROOM_INDEX_DATA *rift_room, *temp_room;

	push_call("spell_envision(%p,%p,%p,%p)",sn,level,ch,vo);

	rift_room	= get_rift_room(ch, value);
	temp_room	= ch->in_room;

	char_from_room(ch);
	char_to_room(ch, rift_room->vnum);

	ch_printf(ch, "Time space coordinates echo through your mind.\n\r");

	do_look(ch, "");

	char_from_room(ch);
	char_to_room(ch, temp_room->vnum);

	pop_call();
	return;
}

DO_SPELL(spell_illusion)
{
	CHAR_DATA *mh, *victim;
	char illusionname[MAX_INPUT_LENGTH];
	int i;

	push_call("spell_illusion(%p,%p,%p,%p)",sn,level,ch,vo);

	if (get_pets(ch) > 0)
	{
		send_to_char("You have too many pets.\n\r", ch);
		pop_call();
		return;
	}

	for (i = 0 ; target_name[i] != '\0' ; i++)
	{
		if (ispunct(target_name[i]) || isdigit(target_name[i]))
		{
			send_to_char("You failed to create such an abstract illusion.\n\r", ch);
			pop_call();
			return;
		}
	}
	mh	= create_mobile( get_mob_index( MOB_VNUM_ILLUSION ) );
	char_to_room( mh, ch->in_room->vnum );

	mh->level				= 3*level/4;
	mh->npcdata->damnodice	= mh->level;
	mh->npcdata->damsizedice	= 2;
	mh->npcdata->damplus	= 3;
	mh->max_hit			= 1;
	mh->hit				= mh->max_hit;

	if (target_name[0] == '\0')
	{
		STRFREE (mh->name);
		STRFREE (mh->short_descr);
		STRFREE (mh->long_descr);
		STRFREE (mh->description);

		sprintf(illusionname, "%s^", ch->name);
		mh->name			= STRALLOC(illusionname);

		if (IS_NPC(ch))
		{
			mh->short_descr = STRALLOC(ch->short_descr);
		}
		else
		{
			mh->short_descr = STRALLOC(ch->name);
		}

		mh->long_descr		= STRALLOC(ch->long_descr);
		mh->description	= STRALLOC(ch->description);
		mh->race			= ch->race;
		mh->sex			= ch->sex;
	}
	else if ((victim = get_char_room(ch, target_name)) != NULL && victim != mh)
	{
		for (illusionname[0] = '\0', i = 0 ; victim->name[i] != '\0' ; i++)
		{
			if (victim->name[i] == ' ')
			{
				strcat(illusionname, "^ ");
			}
			else
			{
				cat_sprintf(illusionname, "%c", victim->name[i]);
			}
		}
		strcat(illusionname, "^");

		STRFREE(mh->name);
		STRFREE(mh->short_descr);
		STRFREE(mh->long_descr);
		STRFREE(mh->description);

		if (IS_NPC(victim))
		{
			mh->short_descr = STRALLOC(victim->short_descr);
		}
		else
		{
			mh->short_descr = STRALLOC(victim->name);
		}
		mh->name			= STRALLOC(illusionname);
		if (IS_NPC(victim))
		{
			if (victim->long_descr[0] != '\0')
			{
				sprintf(illusionname, "%s", ansi_strip(ansi_translate(victim->long_descr)));
			}
			else
			{
				illusionname[0] = '\0';
			}
		}
		else
		{
			strcpy(illusionname, victim->long_descr);
		}

		mh->long_descr		= STRALLOC(illusionname);
		mh->description	= STRALLOC(victim->description);
		mh->race			= victim->race;
		mh->sex			= victim->sex;
	}
	else
	{
		for (illusionname[0] = '\0', i = 0 ; target_name[i] != '\0' ; i++)
		{
			if (target_name[i] == ' ')
			{
				strcat(illusionname, "^ ");
			}
			else
			{
				cat_sprintf(illusionname, "%c", target_name[i]);
			}
		}
		strcat(illusionname, "^");

		STRFREE (mh->name);
		STRFREE (mh->short_descr);
		STRFREE (mh->long_descr);
		STRFREE (mh->description);

		mh->name			= STRALLOC(illusionname);
		mh->short_descr	= STRALLOC(target_name);
		mh->long_descr		= STRALLOC("");
		mh->description	= STRALLOC("");

		mh->race			= ch->race;
		mh->sex			= ch->sex;
	}

	SET_BIT( mh->affected_by , AFF_CHARM );
	act("$N takes shape before your eyes.",ch,NULL,mh,TO_CHAR);
	act("$N takes shape before your eyes.",ch,NULL,mh,TO_ROOM);
	add_follower( mh , ch );
	pop_call();
	return;
}

DO_SPELL(spell_winged_call)
{
	CHAR_DATA *mob;

	push_call("spell_winged_call(%p,%p,%p,%p)",sn,level,ch,vo);

	if (get_pets(ch) > 0)
	{
		send_to_char("You have too many pets.\n\r", ch);
		pop_call();
		return;
	}

	mob = create_mobile(mob_index[MOB_VNUM_WINGED_CALL]);

	char_to_room(mob, ch->in_room->vnum);

	mob->level				= level / 2;
	mob->npcdata->damnodice		= level / 2;
	mob->npcdata->damsizedice	= 2;
	mob->npcdata->damplus		= level / 2;
	mob->max_hit				= level * 5;
	mob->hit					= mob->max_hit;

	SET_BIT(mob->affected_by , AFF_CHARM);

	act("You summon $N from the high mountains.", ch, NULL, mob, TO_CHAR);
	act("$n summons $N from the high mountains.", ch, NULL, mob, TO_ROOM);

	add_follower(mob, ch);

	pop_call();
	return;
}

DO_SPELL(spell_mirror_image)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_mirror_image(%p,%p,%p,%p)",sn,level,ch,vo);

	if ( is_affected( victim, sn ) )
	{
		send_to_char( "Mirror images can't be compounded!\n\r", ch );
		pop_call();
		return;
	}
	af.type = sn;

	if (multi(ch, sn) != -1)
	{
		af.duration = (ch == victim) ? 10 : 5;
		af.modifier = (ch == victim) ? number_fuzzy(level / 8) : number_fuzzy(level / 20);
	}
	else
	{
		af.duration  = 10;
		af.modifier  = 1 + level / 30;
	}
	af.location  = APPLY_NONE;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF2_MIRROR_IMAGE;
	affect_to_char( victim, &af );
	send_to_char("Several of you take a step out and away from yourself.\n\r", victim );
	if ( ch != victim )
	{
		send_to_char( "Ok.\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_hallucinate)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_hallucinate(%p,%p,%p,%p)",sn,level,ch,vo);

	if ( is_affected( victim, sn ) )
	{
		send_to_char( "They can't get any worse!\n\r", ch );
		pop_call();
		return;
	}

	if (IS_AFFECTED(victim, AFF_TRUESIGHT))
	{
		act( "$N sees through the veil on reality you try to impose.", ch, NULL, victim, TO_CHAR );
		act( "You see through the veil on reality $n attempts to impose.", ch, NULL, victim, TO_VICT );
		pop_call();
		return;
	}
	af.type      = sn;
	af.duration  = level/10; /* Used to be level/31 , longer trip for those halu dudes now :)  -  Scandum */
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF2_HALLUCINATE;
	affect_to_char( victim, &af );
	send_to_char("Wow!  You see lot's of pretty colors!\n\r", victim );
	if(ch!=victim)
	{
		send_to_char( "Ok.\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_stability)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_stability(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_AFFECTED(victim, AFF_STABILITY))
	{
		send_to_char("They already feel the solidity of the earth.\n\r", ch);
		pop_call();
		return;
	}

	af.type      = sn;
	af.duration  = level/4;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF_STABILITY;
	affect_to_char( victim, &af );

	send_to_char("The weight of the earth begins to creep into your soul.\n\r", victim );

	if (ch != victim)
	{
		send_to_char( "You fill their soul with the weight of the earth.\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_confusion)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_confusion(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_AFFECTED(victim, AFF2_CONFUSION) || saves_spell(level, ch, victim))
	{
		send_to_char( "You cast confusion!\n\r", ch );
		pop_call();
		return;
	}

	af.type      = sn;
	af.duration  = level/30+1;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF2_CONFUSION;

	affect_to_char( victim, &af );
	send_to_char( "Your eyes shimmer.\n\r", victim );
	if (ch != victim)
	{
		send_to_char( "You cast confusion!\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_sanctify)
{
	ROOM_INDEX_DATA *room;
	CHAR_DATA *fch;
	char buf[MAX_INPUT_LENGTH];

	push_call("spell_sanctify(%p,%p,%p,%p)",sn,level,ch,vo);

	if (ch->in_room == NULL)
	{
		pop_call();
		return;
	}

	room = ch->in_room;

	if (IS_SET(room->room_flags, ROOM_SAFE))
	{
		send_to_char( "This room is already safe enough.\n\r", ch );
		pop_call();
		return;
	}

	for (fch = room->first_person ; fch ; fch = fch->next_in_room)
	{
		if (fch->position == POS_FIGHTING || fch->fighting != NULL)
		{
			send_to_char( "There is too much violence in the room to cast sanctify.\n\r", ch );
			pop_call();
			return;
		}
	}

	/* Can't sanctify the arena.  Presto  8-1-98 */

	if (ch->in_room->area->low_r_vnum == ROOM_VNUM_ARENA)
	{
		send_to_char("The arena may not be sanctified.\n\r", ch);
		pop_call();
		return;
	}

	set_room_timer(room->vnum, ROOM_TIMER_SANCTIFY, 1 + level / 5);
	room->sanctify_char = ch;

	SET_BIT(room->room_flags, ROOM_SAFE);

	send_to_char( "The room becomes sanctified, and is a sanctuary for all.\n\r", ch);
	sprintf(buf, "$n prays to %s and makes this small area a sanctuary to all.", get_god_name(which_god(ch)));
	act(buf, ch, NULL, NULL, TO_ROOM);

	pop_call();
	return;
}

DO_SPELL(spell_globe_of_darkness)
{
	push_call("spell_globe_of_darkness(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_SET(ch->in_room->room_flags, ROOM_GLOBE))
	{
		send_to_char("This room is already shrouded in darkness.\n\r", ch );
		pop_call();
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_char( "The darkness dissipates immediately.\n\r", ch );
		pop_call();
		return;
	}

	SET_BIT(ch->in_room->room_flags, ROOM_GLOBE);

	set_room_timer(ch->in_room->vnum, ROOM_TIMER_GLOBE, 1 + level/20);

	act("The light around you dissolves into darkness.", ch, NULL, NULL, TO_CHAR);
	act("The light around you dissolves into darkness.", ch, NULL, NULL, TO_ROOM);

	pop_call();
	return;
}

DO_SPELL(spell_ice_layer)
{
	push_call("spell_ice_layer(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_SET(ch->in_room->room_flags, ROOM_ICE))
	{
		send_to_char("This room is already covered by ice.\n\r", ch );
		pop_call();
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_char( "The ice melts immediately.\n\r", ch );
		pop_call();
		return;
	}

	switch (ch->in_room->sector_type)
	{
		case SECT_ETHEREAL:
		case SECT_ASTRAL:
			send_to_char("The ice slowly drifts away into the unknown.\n\r", ch);
			pop_call();
			return;

		case SECT_UNDER_WATER:
			send_to_char("The ice slowly floats up toward the surface.\n\r", ch);
			pop_call();
			return;
	}

	/*
		Possible to add duration based on temperature, farenheit to celcius
		is: -32 times 5 div by 9
	*/

	SET_BIT(ch->in_room->room_flags, ROOM_ICE);

	set_room_timer(ch->in_room->vnum, ROOM_TIMER_ICE, 1 + level/30);

	act("Water emerges at your command, freezing into sheets of ice.", ch, NULL, NULL, TO_CHAR);
	act("Water emerges at $n's command, freezing into sheets of ice.", ch, NULL, NULL, TO_ROOM);

	pop_call();
	return;
}

DO_SPELL(spell_benediction)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_benediction(%p,%p,%p,%p)",sn,level,ch,vo);

	if (victim->position == POS_FIGHTING)
	{
		send_to_char( "They are fighting.\n\r", ch );
		pop_call();
		return;
	}

	if (is_affected(victim, sn))
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}

	if (ch->level < victim->level*3/4
	||  ch->level > victim->level*4/3)
	{
		ch_printf(ch,  "Your god feels that %s is unworthy of your benediction.\n\r", get_name(victim) );
		pop_call();
		return;
	}

	af.type      = sn;
	af.location  = APPLY_HIT;
	af.modifier  = level * 4;

	if (victim != ch)
	{
		af.modifier /= 2;
	}

	af.duration  = level / 8;
	af.bittype   = AFFECT_TO_NONE;
	af.bitvector = 0;

	affect_to_char( victim, &af );

	if (victim != ch)
	{
		act("You make a sign of benediction over $N's head.", ch, NULL, victim, TO_CHAR);
		act("$n makes a sign of benediction over your head.", ch, NULL, victim, TO_VICT);
	}
	else
	{
		act("You make the sign of benediction over your head.", ch, NULL, NULL, TO_CHAR);
		act("$n makes the sign of benediction over $s head.",   ch, NULL, NULL, TO_ROOM);
	}
	pop_call();
	return;
}

DO_SPELL(spell_righteous_fury)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_righteous_fury(%p,%p,%p,%p)",sn,level,ch,vo);

	if ( victim->position == POS_FIGHTING )
	{
		send_to_char( "They are fighting.\n\r", ch );
		pop_call();
		return;
	}
	if( is_affected( victim, sn ) )
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}
	af.type      = sn;
	af.duration  = number_fuzzy (level /10);
	af.location  = APPLY_HITROLL;
	af.modifier  = number_fuzzy (level / 7);
	if (victim != ch)
	{
		af.modifier /= 2;
	}
	af.bittype   = AFFECT_TO_NONE;
	af.bitvector = 0;
	affect_to_char( victim, &af );

	af.location  = APPLY_DAMROLL;
	af.modifier  = number_fuzzy (level / 8);
	if (victim != ch)
	{
		af.modifier /= 2;
	}
	affect_to_char( victim, &af );

	if (ch != victim)
	{
		act("You invoke the wrath of your god, filling $N with righteous fury.", ch, NULL, victim, TO_CHAR);
		act("$n invokes the wrath of $s god, filling you with righteous fury.",  ch, NULL, victim, TO_VICT);
		act("$n invokes the wrath of $s god, filling $N with righteous fury.",   ch, NULL, victim, TO_NOTVICT);
	}
	else
	{
		act("You invoke the wrath of your god, filling yourself with divine fury.", ch, NULL, victim, TO_CHAR);
		act("$n invokes the wrath of $s god, filling $mself with divine fury.",     ch, NULL, victim, TO_NOTVICT);
	}

	pop_call();
	return;
}

DO_SPELL(spell_soothing_touch)
{
	CHAR_DATA *gch;
	int heal;

	push_call("spell_soothing_touch(%p,%p,%p,%p)",sn,level,ch,vo);

	heal = dice(10, 10) + level*5/2 ;

	for (gch = ch->in_room->first_person ; gch ; gch = gch->next_in_room)
	{
		if (!is_same_group(gch, ch))
		{
			continue;
		}
		if (is_safe_magic(ch, gch, sn))
		{
			continue;
		}
		gch->hit = UMIN( gch->hit + heal, gch->max_hit );
		update_pos( gch );

		if (gch != ch)
		{
			act("$n's soothing touch makes your wounds close and your pain fade.", ch, NULL, gch, TO_VICT );
		}
		else
		{
			act("Your soothing touch makes wounds close and pain fade.", ch, NULL, NULL, TO_CHAR );
		}
	}
	pop_call();
	return;
}

DO_SPELL(spell_farheal)
{
	CHAR_DATA *victim;
	int heal;

	push_call("spell_farheal(%p,%p,%p,%p)",sn,level,ch,vo);

	if ( ( victim = get_player_world( ch, target_name ) ) == NULL)
	{
		send_to_char( "Farheal who ?\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && ch->pcdata->mclass[ch->class] != ch->level)
	{
		send_to_char("You lack the single minded devotion to reach through the void.\n\r", ch);
		pop_call();
		return;
     }

	if (victim->in_room == NULL
		||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
		||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
		||   victim->in_room->sector_type == SECT_ETHEREAL
		||   victim->in_room->sector_type == SECT_ASTRAL
		||   ch->in_room->sector_type == SECT_ETHEREAL
		||   ch->in_room->sector_type == SECT_ASTRAL
		||   IS_NPC(victim)
		||   victim->fighting != NULL )
	{
		ch_printf(ch, "You can not reach %s through the void.\n\r",
		get_name(victim));
		pop_call();
		return;
	}
	if (victim == ch)
	{
		send_to_char( "You can't farheal yourself!\n\r", ch);
		pop_call();
		return;
	}
	if (victim->in_room == ch->in_room)
	{
		ch_printf(ch, "%s is in the same room as you!\n\r",
		get_name(victim));
		pop_call();
		return;
	}
	heal = dice(11, 9) + level;
	victim->hit = UMIN( victim->hit + heal, victim->max_hit );
	update_pos( victim );

	act( "$n reaches through the void to fill you with healing energy.", ch, NULL, victim, TO_VICT );
	act ( "You reach through the void to $N, filling $M with healing energy.", ch, NULL, victim, TO_CHAR );

	pop_call();
	return;
}

DO_SPELL(spell_holy_word)
{
	push_call("spell_holy word(%p,%p,%p,%p)",sn,level,ch,vo);

	pop_call();
	return;
}

DO_SPELL(spell_unholy_word)
{
	push_call("spell_unholy_word(%p,%p,%p,%p)",sn,level,ch,vo);

	pop_call();
	return;
}

DO_SPELL(spell_invigorate)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int heal, scale_top, scale_bot;
	char buf[MAX_INPUT_LENGTH];

	push_call("spell_invigorate(%p,%p,%p,%p)",sn,level,ch,vo);

	if( victim==NULL || victim->in_room == NULL || ch->in_room==NULL
		|| victim->in_room != ch->in_room )
	{
		send_to_char( "That person is not here.\n\r", ch );
		pop_call();
		return;
      }

	scale_top = 29;  /* The fraction for the hp/mana ratio */
	scale_bot = 7;

	if( value == -1 || value > ch->mana )
		value = ch->mana;

	heal = value * scale_top / scale_bot ;
	if( victim->move + heal > victim->max_move )
		heal = victim->max_move - victim->move ;
	value = ( heal * scale_bot / scale_top );

	sprintf( buf, "Invigorating %d moves for %d mana.\n\r", heal, value + get_mana(ch,sn));
	send_to_char( buf, ch);

	victim->move += heal;
	ch->mana -= value;

	send_to_char( "A delicious chill runs up your spine, cleansing the lethargy from your limbs.\n\r", victim );

	pop_call();
	return;
}

DO_SPELL(spell_improved_invis)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_improved_invis(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_AFFECTED(victim, AFF_IMP_INVISIBLE))
	{
		send_to_char("They are already invisible.\n\r", ch);
		pop_call();
		return;
	}

	act( "Something unseen enshrouds $n.", victim, NULL, NULL, TO_ROOM );
	af.type      = sn;
	af.duration  = number_fuzzy(level / 4);
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF_IMP_INVISIBLE;
	affect_to_char( victim, &af );

	act("You are enshrouded by something unseen.", victim, NULL, NULL, TO_CHAR);
	act("$n is enshrouded by something unseen.",   victim, NULL, NULL, TO_ROOM);

	pop_call();
	return;
}

DO_SPELL(spell_truesight)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_truesight(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_AFFECTED(ch, AFF_TRUESIGHT))
	{
		send_to_char( "Your eyes already see through the facades of mundane reality.\n\r", ch);
		pop_call();
		return;
	}
	if (IS_AFFECTED(ch, AFF_BLIND))
	{
		affect_strip(victim, gsn_blindness);
	}
	af.type      = sn;
	af.duration  = number_fuzzy(level/4);
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF_TRUESIGHT;
	affect_to_char( victim, &af );

	send_to_char( "Your eyes gain the ability to see through the facades of mundane reality.\n\r", victim );

	pop_call();
	return;
}

DO_SPELL(spell_anti_magic_shell)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_anti_magic_shell(%p,%p,%p,%p)",sn,level,ch,vo);

	if (is_affected(victim, sn))
	{
		send_to_char( "You are already affected.\n\r", ch );
		pop_call();
		return;
	}

	af.type      = sn;
	af.duration  = URANGE(2, number_fuzzy(4), 6);
	af.location  = 0;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_NONE;
	af.bitvector = 0;
	affect_to_char( victim, &af );

	send_to_char( "The flows of magic recede from your body.\n\r", victim );

	pop_call();
	return;
}

DO_SPELL(spell_smoke)
{
	ROOM_INDEX_DATA *room;

	push_call("spell_smoke(%p,%p,%p,%p)",sn,level,ch,vo);

	if( ch->in_room == NULL )
	{
		pop_call();
		return;
	}

	room = ch->in_room;

	if( IS_SET(room->room_flags, ROOM_SAFE ) )
	{
		send_to_char( "The smoke dissipates immediately.\n\r", ch );
		pop_call();
		return;
	}

	SET_BIT( room->room_flags, ROOM_SMOKE );

	set_room_timer(room->vnum, ROOM_TIMER_SMOKE, 1 + level / 10);

	act( "You conjure up an impenetrable cloud of dark purple smoke.", ch, NULL, NULL, TO_CHAR );
	act( "$n conjures up an impenetrable cloud of dark purple smoke.", ch, NULL, NULL, TO_ROOM );

	pop_call();
	return;
}

DO_SPELL(spell_nightmare)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_nightmare(%p,%p,%p,%p)",sn,level,ch,vo);

	if (is_affected(victim, sn))
	{
		act( "$N is still recovering from $S last fright!", ch, NULL, victim, TO_CHAR );
		pop_call();
		return;
	}

	if (IS_EVIL(victim) && saves_spell(level/3, ch, victim))
	{
		act("$N smirks at your feeble attempt to scare $M.", ch, NULL, victim, TO_CHAR );
		pop_call();
		return;
	}
	else if( saves_spell( level, ch, victim ) )
	{
		send_to_char( "Nothing happens.\n\r", ch );
		pop_call();
		return;
	}

	if (IS_NPC(victim) )
	{
		af.type      = sn;
		af.duration  = number_fuzzy(level / 5);
		af.bittype   = AFFECT_TO_NONE;
		af.bitvector = 0;

		af.location  = APPLY_DAMROLL;
		af.modifier  = 0-(level/7);
		affect_to_char( victim, &af );

		af.location  = APPLY_HITROLL;
		af.modifier  = 0-(level/7);
		affect_to_char( victim, &af );

		af.location  = APPLY_SAVING_SPELL;
		af.modifier  = number_fuzzy(level/7);
		affect_to_char( victim, &af );
	}
	else
	{
		af.type      = sn;
		af.duration  = number_fuzzy(level/21);
		af.bitvector = 0;
		af.bittype   = AFFECT_TO_NONE;
		af.location  = APPLY_STR;
		af.modifier  = 0 - number_fuzzy(level/21);
		affect_to_char( victim, &af );

		af.location  = APPLY_DEX;
		af.modifier  = 0 - number_fuzzy(level/14);
		affect_to_char( victim, &af );

		af.location  = APPLY_SAVING_SPELL;
		af.modifier  = number_fuzzy(level/7);
		affect_to_char( victim, &af );
	}
	if (ch != victim)
	{
		ch_printf(victim, "A nightmarish apparition flies from %s's hands towards your face!\n\r", get_name(ch) );
		ch_printf(ch, "A nightmarish apparition flies from your hands towards %s's face!\n\r", get_name(victim) );
	}
	else
	{
		send_to_char("Ok.\n\r", ch);
	}

	damage( ch, victim, number_range(level/2, level*2), sn);

	pop_call();
	return;
}

DO_SPELL(spell_possess)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_posses(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_char("You are forbidden from casting that here.\n\r", ch);
		pop_call();
		return;
	}

	if (IS_NPC(ch) || ch->desc->original)
	{
		send_to_char("You are not in your original state.\n\r", ch);
		pop_call();
		return;
	}

	if (victim == ch)
	{
		send_to_char("You already have control over yourself.\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_NPC(victim) || !can_mass_cast(ch, victim))
	{
		send_to_char("They seem to be unaffected by your magic.\n\r", ch);
		pop_call();
		return;
	}

	if (victim->desc)
	{
		ch_printf(ch, "%s is already possessed.\n\r", victim->short_descr);
		pop_call();
		return;
	}

	if (IS_AFFECTED(victim, AFF2_POSSESS)
	||  level / 2 < victim->level
	||  saves_spell(level/2, ch, victim))
	{
		act("$N is unhappy with that you tried to seize control of $S brain.", ch, NULL, victim, TO_CHAR);
		multi_hit(victim, ch, TYPE_UNDEFINED);
		pop_call();
		return;
	}

	SET_BIT(victim->act,ACT_SENTINEL);
	REMOVE_BIT(victim->act, ACT_AGGRESSIVE);

	af.type      = sn;
	af.duration  = 12;
	af.location  = 0;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF2_POSSESS;
	affect_to_char( victim, &af );

	log_printf("%s has possessed %s",get_name(ch), victim->short_descr);

	ch->desc->character = victim;
	ch->desc->original  = ch;
	victim->desc        = ch->desc;
	ch->desc            = NULL;
	ch->pcdata->switched = TRUE;

	ch_printf(victim, "You have possessed %s.\n\r", get_name(victim));

	pop_call();
	return;
}

DO_SPELL(spell_transport)
{
	CHAR_DATA *victim;
	char arg3[MAX_STRING_LENGTH];
	OBJ_DATA *obj;

	push_call("spell_transport(%p,%p,%p,%p)",sn,level,ch,vo);

	target_name = one_argument(target_name, arg3 );

	if ((victim = get_player_world(ch, target_name)) == NULL
	||   victim == ch
	||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
	||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
	||   victim->in_room->sector_type == SECT_ASTRAL
	||   victim->in_room->sector_type == SECT_ETHEREAL
	||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
	||   IS_SET(ch->in_room->area->flags, AFLAG_NORECALL)
	||   victim->level >= level + 15
	||  (IS_NPC(victim) && saves_spell( level, ch, victim )) )
	{
		send_to_char("Your transport spell failed!\n\r", ch);
		pop_call();
		return;
	}

	if (victim->in_room == ch->in_room)
	{
		send_to_char("They are right beside you!", ch);
		pop_call();
		return;
	}

	if ((obj = get_obj_carry(ch, arg3)) == NULL
	||  (victim->carry_weight + get_obj_weight (obj)) > can_carry_w(victim))
	{
		send_to_char( "Your transport spell failed!\n\r", ch);
		pop_call();
		return;
	}

	if (IS_OBJ_STAT(obj, ITEM_NODROP))
	{
		send_to_char( "You can't seem to let go of it.\n\r", ch );
		pop_call();
		return;
	}

	act( "$p slowly dematerializes...", ch, obj, NULL, TO_CHAR );
	act( "$p slowly dematerializes from $n's hands..", ch, obj, NULL, TO_ROOM );

	obj_from_char( obj );
	obj_to_char( obj, victim );

	act( "$p from $n appears in your hands!", ch, obj, victim, TO_VICT);
	act( "$p appears in $n's hands!", victim, obj, NULL, TO_ROOM );

	pop_call();
	return;
}

DO_SPELL(spell_slow)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_slow(%p,%p,%p,%p)",sn,level,ch,vo);

	if(IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_char("You are forbidden from casting that here.\n\r", ch);
		pop_call();
		return;
	}

	if( is_affected( victim, sn ) )
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}

	if ( IS_AFFECTED(victim, AFF_HASTE) )
	{
		REMOVE_BIT(victim->act, AFF_HASTE);
	}

	act( "$n slows down.", victim, NULL, NULL, TO_ROOM );
	af.type      = sn;
	af.duration  = number_fuzzy(level/9);
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_NONE;
	af.bitvector = 0;
	affect_to_char( victim, &af );

	victim->speed = UMIN(victim->speed, get_max_speed(victim));

	send_to_char( "You slow down.\n\r", victim );
	if ( ch != victim )
		send_to_char( "Ok.\n\r", ch );

	pop_call();
	return;
}

DO_SPELL(spell_brew_potion)
{
	char buf[MAX_INPUT_LENGTH];
	char buf2[MAX_INPUT_LENGTH];
	OBJ_DATA *potion;
	int sn2,i;

	push_call("spell_brew_potion(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_NPC(ch) || multi(ch, sn) == -1)
	{
		send_to_char( "You can't do that!\n\r", ch);
		pop_call();
		return;
	}

	if ((potion = get_eq_char(ch, WEAR_HOLD)) == NULL || potion->item_type != ITEM_POTION)
	{
		send_to_char("You need to hold a vial for the potion you shall brew.\n\r", ch);
		pop_call();
		return;
	}

	for (i = 0 ; i < 100 ; i++)
	{
		sn2 = number_range(0, MAX_SKILL -1);

		if (skill_table[sn2].slot <= 0)
		{
			continue;
		}
		if (!IS_SET(skill_table[sn2].flags, FSKILL_SPELL))
		{
			continue;
		}
		if (ch->pcdata->learned[sn2] == 0)
		{
			continue;
		}
		break;
	}

	if (i >= 100)
	{
		send_to_char("Your mind goes blank!\n\r",ch);
		ch->mana = 0;
		pop_call();
		return;
	}

	if (level < 51)
	{
		sprintf(buf, "a murky potion of %s", skill_table[sn2].name);
		sprintf(buf2, "murky potion %s", skill_table[sn2].name);
	}
	else if (level < 61)
	{
		sprintf(buf, "a cloudy potion of %s", skill_table[sn2].name);
		sprintf(buf2, "cloudy potion %s", skill_table[sn2].name);
	}
	else
	{
		sprintf(buf, "a clear potion of %s", skill_table[sn2].name);
		sprintf(buf2, "clear potion %s", skill_table[sn2].name);
	}

	RESTRING(potion->name, buf2);
	RESTRING(potion->short_descr, buf);
	RESTRING(potion->long_descr, "");
	RESTRING(potion->description, buf);

	potion->cost		= 0;
	potion->level		= level * 3 / 4;
	potion->value[0]	= level;
	potion->value[1]	= sn2;
	potion->value[2]	= 0;
	potion->value[3]	= 0;

	SET_BIT(potion->extra_flags, ITEM_MODIFIED);

	act("$n brews up a potion.",						ch, potion, NULL, TO_ROOM);
	act("You enter a trance and...  ...you now hold $p!",	ch, potion, NULL, TO_CHAR);

	pop_call();
	return;
}

DO_SPELL(spell_elemental)
{
	MOB_INDEX_DATA *pMob;
	CHAR_DATA *mh;

	push_call("spell_elemental(%p,%p,%p,%p)",sn,level,ch,vo);

	if (get_pets(ch) > 0)
	{
		send_to_char( "You have too many pets.\n\r", ch);
		pop_call();
		return;
	}

	switch (ch->in_room->sector_type)
	{
		case SECT_LAVA:
			send_to_char( "A fiery shape emerges from the flames to do your bidding.\n\r", ch );
			pMob = get_mob_index(MOB_VNUM_FIRE_ELEMENTAL);
			break;
		case SECT_LAKE:
		case SECT_RIVER:
		case SECT_OCEAN:
		case SECT_UNDER_WATER:
			send_to_char( "A ripple in the water forms into a man sized shape.\n\r", ch );
			pMob = get_mob_index(MOB_VNUM_WATER_ELEMENTAL);
			break;
		case SECT_AIR :
			send_to_char( "A blast of wind circles you, ready to do your bidding.\n\r", ch );
			pMob = get_mob_index(MOB_VNUM_AIR_ELEMENTAL);
			break;
		case SECT_FOREST : case SECT_DESERT       : case SECT_MOUNTAIN   :
		case SECT_HILLS  : case SECT_UNDER_GROUND : case SECT_DEEP_EARTH :
			send_to_char( "A large mound of earth rises up out of the ground to do your bidding.\n\r", ch );
			pMob = get_mob_index(MOB_VNUM_EARTH_ELEMENTAL);
			break;
		default :
			send_to_char( "No elementals appear to do your bidding.\n\r", ch );
			pop_call();
			return;
	}

	if (pMob == NULL)
	{
		send_to_char( "You cannot create an elemental now.\n\r", ch );
		pop_call();
		return;
    }

	mh = create_mobile( pMob );
	char_to_room( mh, ch->in_room->vnum );

	mh->level				= level * 1 / 2;
	mh->npcdata->damnodice	= mh->level / 2;
	mh->npcdata->damsizedice	= 2;
	mh->npcdata->damplus	= 7;
	mh->max_hit			= 30 + level * 5;
	mh->hit				= 30 + level * 5;

	SET_BIT( mh->affected_by , AFF_CHARM );

	add_follower( mh , ch );

	if (ch->fighting != NULL)
	{
		multi_hit(mh, ch->fighting->who, TYPE_UNDEFINED);
	}
	pop_call();
	return;
}

DO_SPELL(spell_unbarring_ways)
{
	int door;
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	push_call("spell_unbarring_ways(%p,%p,%p,%p)",sn,level,ch,vo);

	if (target_name[0] == '\0')
	{
		send_to_char( "What direction do you wish to cast this spell in?\n\r", ch);
		pop_call();
		return;
	}

	if ((door = find_door(ch, target_name)) >= 0)
	{
 		pexit = ch->in_room->exit[door];

		if (pexit == NULL)
		{
			send_to_char( "There is no door in that direction.\n\r", ch);
			pop_call();
			return;
		}
		if (IS_SET(pexit->flags, EX_MAGICPROOF))
		{
			send_to_char( "It remains firm.\n\r",  ch );
			pop_call();
			return;
		}

		REMOVE_BIT(pexit->flags, EX_LOCKED);
		REMOVE_BIT(pexit->flags, EX_CLOSED);
		SET_BIT   (pexit->flags, EX_UNBARRED);

		if ((to_room = room_index[pexit->to_room]) != NULL
		&&  (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
		&&   room_index[pexit_rev->to_room] == ch->in_room )
		{
			REMOVE_BIT( pexit_rev->flags, EX_LOCKED);
			REMOVE_BIT( pexit_rev->flags, EX_CLOSED);
			SET_BIT   ( pexit_rev->flags, EX_UNBARRED);
		}
		act( "$n causes the $d to dissolve.", ch, NULL, pexit->keyword, TO_ROOM );
		act( "You cause the $d to dissolve.", ch, NULL, pexit->keyword, TO_CHAR );
	}
	else
	{
		send_to_char("There's no valid target in that direction!\n\r", ch);
	}
	pop_call();
	return;
}

DO_SPELL(spell_fireshield)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_fireshield(%p,%p,%p,%p)",sn,level,ch,vo);

	if (is_affected(victim, sn))
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}

	af.type      = sn;
	af.duration  = level/10;
	af.location  = 0;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF2_FIRESHIELD;
	affect_to_char( victim, &af );

	act( "$n bursts into flame!", victim, NULL, NULL, TO_ROOM );
	send_to_char( "You burst into flame!\n\r", victim );

	pop_call();
	return;
}

DO_SPELL(spell_recharge)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;

	push_call("spell_recharge(%p,%p,%p,%p)",sn,level,ch,vo);

	if (obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND)
	{
		if (obj->value[2] == obj->value[1] || obj->value[1] > (obj->pIndexData->value[1] * 4))
		{
			act( "$p bursts into flames, injuring you!", ch, obj, NULL, TO_CHAR );
			act( "$p bursts into flames, charring $n!", ch, obj, NULL, TO_ROOM);
			junk_obj(obj);
			damage(ch, ch, obj->level * 2, TYPE_UNDEFINED);

			pop_call();
			return;
		}

		if ( number_percent() <=2)
		{
			act( "$p glows with a blinding magical luminescence.", ch, obj, NULL, TO_CHAR);
			obj->value[1] *= 2;
			obj->value[2] = obj->value[1];

			pop_call();
			return ;
		}
		else if ( number_percent() <=5 )
		{
			act( "$p glows brightly for a few seconds...", ch, obj, NULL, TO_CHAR);
			obj->value[2] = obj->value[1];

			pop_call();
			return;
		}
		else if (number_percent() <= 10 )
		{
			act( "$p disintegrates into a void.", ch, obj, NULL, TO_CHAR);
			act( "$n's attempt at recharging fails, and $p disintegrates.", ch, obj, NULL, TO_ROOM);
			junk_obj(obj);

			pop_call();
			return;
		}
		else if (number_percent() <= 10 + level / 4)
		{
			send_to_char("Nothing happens.\n\r", ch);
			pop_call();
			return;
		}
		else
		{
			act( "$p feels warm to the touch.", ch, obj, NULL, TO_CHAR);
			--obj->value[1];
			obj->value[2] = obj->value[1];

			pop_call();
			return;
		}
	}
	else
	{
		send_to_char( "You can't recharge that!\n\r", ch);

		pop_call();
		return;
	}
	pop_call();
	return;
}

DO_SPELL(spell_vampiric_touch)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_vampiric_touch(%p,%p,%p,%p)",sn,level,ch,vo);

	if (check_hit(ch, victim, 0, gsn_vampiric_touch))
	{
		dam     = number_fuzzy(number_range(level*3, level*5));
		ch->hit = UMIN(ch->max_hit, ch->hit+dam/10);
		damage( ch, victim, dam, sn );
	}
	else
	{
		act( "$n's hand glows with dark energy, but fails to strike $s foe.", ch, NULL, victim, TO_ROOM );
		act( "The dark energies in your hand fail to strike $N.", ch, NULL, victim, TO_CHAR );
	}
	pop_call();
	return;
}

DO_SPELL(spell_windblast)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_windblast(%p,%p,%p,%p)",sn,level,ch,vo);

	dam   = number_range(level*3, level*5);

	if ( saves_spell( level, ch, victim ) )
		dam /= 2;
	damage( ch, victim, dam, sn );

	pop_call();
	return;
}

DO_SPELL(spell_waterburst)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_waterburst(%p,%p,%p,%p)",sn,level,ch,vo);

	dam   = number_range(level*2, level*6);

	if ( saves_spell( level, ch, victim ) )
		dam /= 2;
	damage( ch, victim, dam, sn );

	pop_call();
	return;
}

DO_SPELL(spell_mindblast)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_mindblast(%p,%p,%p,%p)",sn,level,ch,vo);

	dam   = number_range(level*1, level*7);

	if ( saves_spell( level, ch, victim ) )
		dam /= 2;
	damage( ch, victim, dam, sn );

	pop_call();
	return;
}

DO_SPELL(spell_snake_dart)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	push_call("spell_mindblast(%p,%p,%p,%p)",sn,level,ch,vo);

	dam   = number_range(level*1, level*7);

	if (saves_spell(level, ch, victim))
	{
		dam /= 2;
	}
	else
	{
		if (number_bits(3) == 0 && !IS_AFFECTED(victim, AFF_POISON))
		{
			AFFECT_DATA af;

			af.type      = gsn_poison;
			af.location  = APPLY_AC;
			af.modifier  = level/2;
			af.duration  = level/20;
			af.bittype   = AFFECT_TO_CHAR;
			af.bitvector = AFF_POISON;

			affect_to_char( victim, &af );

			send_to_char( "You feel poison coursing through your vains.\n\r", victim );
			if (ch != victim)
			{
				send_to_char( "They now feel your poison.\n\r", ch );
			}
		}
	}

	damage( ch, victim, dam, sn );

	pop_call();
	return;
}

DO_SPELL(spell_metamorphose_liquids)
{
	OBJ_DATA *obj = (OBJ_DATA *)vo;
	char arg3[MAX_INPUT_LENGTH];
	int liquid;

	push_call("spell_metamorphose_liquids(%p,%p,%p,%p)",sn,level,ch,vo);

	if (obj == NULL)
	{
		send_to_char("You are not carrying that.\n\r",ch);
		pop_call();
		return;
	}

	target_name = one_argument(target_name, arg3);
	target_name = one_argument(target_name, arg3);

	if (obj->item_type != ITEM_DRINK_CON)
	{
		send_to_char("That is not a drinkcontainer.\n\r",ch);
		pop_call();
		return;
	}

	if (!strlen(arg3))
	{
		send_to_char("Metamorphose to what kind of liquid?\n\r",ch);
		pop_call();
		return;
	}

	for (liquid = 0 ; liquid < LIQ_MAX ; liquid++)
	{
		if (!str_prefix(arg3, liq_table[liquid].liq_name))
		{
			break;
		}
	}

	if (liquid == LIQ_MAX)
	{
		send_to_char("There is no such liquid.\n\r", ch);
		pop_call();
		return;
	}

	if (obj->value[2] == liquid)
	{
		ch_printf(ch, "%s is already filled with %s\n\r", capitalize(obj->short_descr), liq_table[liquid].liq_name);
		pop_call();
		return;
	}
	else
	{
		ch_printf(ch, "You transform the %s in %s to %s.\n\r", liq_table[obj->value[2]].liq_name, obj->short_descr, liq_table[liquid].liq_name);
		obj->value[2] = liquid;
	}
	pop_call();
	return;
}

DO_SPELL(spell_detect_good)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_detect_good(%p,%p,%p,%p)",sn,level,ch,vo);

	if (is_affected(victim, sn))
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}
	af.type      = sn;
	af.duration  = level;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bittype   = AFFECT_TO_NONE;
	af.bitvector = 0;
	affect_to_char( victim, &af );

	send_to_char( "Traces of gold outline all good in plain sight.\n\r", victim );

	if (ch != victim)
	{
		send_to_char( "Ok.\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_firewalk)
{
	CHAR_DATA *victim = (CHAR_DATA *)vo;
	AFFECT_DATA af;

	push_call("spell_firewalk(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_NPC(victim))
	{
		send_to_char("You cannot cast this spell on mobs.\n",ch);
		pop_call();
		return;
	}

	if (IS_AFFECTED(victim, AFF2_FIREWALK))
	{
		send_to_char("They are already affected.\n\r", ch);
		pop_call();
		return;
	}
	af.type      = sn;
	af.duration  = level/2;
	af.location  = 0;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF2_FIREWALK;
	affect_to_char(victim, &af);

	act( "$n's feet glow a soft red.", victim, NULL, NULL, TO_ROOM );
	send_to_char( "Your feet tingle.\n\r", victim );
	pop_call();
	return;
}
	
DO_SPELL(spell_divine_inspiration)
{
	CHAR_DATA *victim = (CHAR_DATA *)vo;
	AFFECT_DATA af;

	push_call("spell_divine_inspiration(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_NPC(victim))
	{
		send_to_char("You cannot cast this spell on mobs.\n",ch);
		pop_call();
		return;
	}

	if (IS_AFFECTED(victim, AFF2_DIVINE_INSPIRATION))
	{
		send_to_char("They are already affected.\n\r", ch);
		pop_call();
		return;
	}

	af.type      = sn;
	af.location  = APPLY_INT;
	af.duration  = URANGE(2, level/10, 8);
	af.modifier  = 1 + (level > 49) + (level > 99);
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF2_DIVINE_INSPIRATION;
	affect_to_char(victim, &af);

	act( "$n's eyes shine brightly.", victim, NULL, NULL, TO_ROOM );
	send_to_char( "Your eyes shine brightly as divine inspiration sharpens your mind.\n\r", victim);
	pop_call();
	return;
}

DO_SPELL(spell_black_aura)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_black_aura(%p,%p,%p,%p)",sn,level,ch,vo);

	if (is_affected(victim, sn))
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}

	if (saves_spell(4*level/5, ch, victim))
	{
		act("Your shadow magic slithers around $N but fails to enshroud $M.", ch, NULL, victim, TO_CHAR);
		pop_call();
		return;
	}

	af.type      = sn;
	af.duration  = number_range(0, level/70);
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bittype   = AFFECT_TO_NONE;
	af.bitvector = 0;
	affect_to_char( victim, &af );

	act( "$n is surrounded by a black aura.", victim, NULL, NULL, TO_ROOM );
	send_to_char( "A bitter cloud of darkness envelopes you.\n\r", victim);

	pop_call();
	return;
}

DO_SPELL(spell_mana_shield)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_mana_shield(%p,%p,%p,%p)",sn,level,ch,vo);

	if (is_affected(victim, sn))
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}

	af.type      = sn;
	af.duration  = level/4;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bittype   = AFFECT_TO_NONE;
	af.bitvector = 0;
	affect_to_char( victim, &af );

	act( "A shield of sparkling blue energy coalesces around $n.", victim, NULL, NULL, TO_ROOM );
	send_to_char( "A shield of sparkling blue energy coalesces around you.\n\r", victim);

	pop_call();
	return;
}

DO_SPELL(spell_magic_mirror)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_magic_mirror(%p,%p,%p,%p)",sn,level,ch,vo);

	if (is_affected(victim, sn))
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}

	af.type      = sn;
	af.duration  = level/4;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bittype   = AFFECT_TO_NONE;
	af.bitvector = 0;
	affect_to_char( victim, &af );

	act( "A shield of reflective magic coalesces around $n.", victim, NULL, NULL, TO_ROOM );
	send_to_char( "A shield of reflective magic coalesces around you.\n\r", victim);

	pop_call();
	return;
}

DO_SPELL(spell_animate_object)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA *paf, af;
	CHAR_DATA *mob;
	char animatename[MAX_INPUT_LENGTH];
	int i;

	push_call("spell_animte_object(%p,%p,%p,%p)",sn,level,ch,vo);

	if (!IS_NPC(ch) && multi(ch, sn) == -1)
	{
		send_to_char( "You can't do that!\n\r", ch);
		pop_call();
		return;
	}

	if (obj->carried_by != ch || obj->wear_loc != WEAR_NONE)
	{
		send_to_char( "You are not carrying that.\n\r", ch);
		pop_call();
		return;
	}

	if (obj->level > level)
	{
		send_to_char("You failed.\n\r", ch);
		pop_call();
		return;
	}

	if (get_pets(ch) > 0)
	{
		send_to_char("You have too many pets.\n\r", ch);
		pop_call();
		return;
	}

	mob = create_mobile( get_mob_index( MOB_VNUM_ILLUSION ) );
	char_to_room( mob, ch->in_room->vnum );

	mob->level				= obj->level;
	mob->npcdata->damnodice		= 0;
	mob->npcdata->damsizedice	= 0;
	mob->npcdata->damplus		= mob->level;

	if (obj->item_type == ITEM_WEAPON)
	{
		mob->max_hit			= 100;
	}
	else
	{
		mob->max_hit			= UMAX(200, mob->level * 20);
	}
	mob->hit					= mob->max_hit;

	for (animatename[0] = '\0', i = 0 ; obj->name[i] != '\0' ; i++)
	{
		if (obj->name[i] == ' ')
		{
			strcat(animatename, "^ ");
		}
		else
		{
			cat_sprintf(animatename, "%c", obj->name[i]);
		}
	}
	strcat(animatename, "^");

	RESTRING(mob->name,			animatename);
	RESTRING(mob->short_descr,	obj->short_descr);
	RESTRING(mob->long_descr,	obj->long_descr);
	RESTRING(mob->description,	obj->description);

	REMOVE_BIT(mob->act, ACT_RACE);
	REMOVE_BIT(mob->act, ACT_SMART);
	mob->sex			= SEX_NEUTRAL;
	mob->race			= RACE_HUMAN;
	mob->gold			= obj->cost/2;

	af.type		= sn;
	af.duration	= -1;
	af.bittype   = AFFECT_TO_NONE;
	af.bitvector	= 0;

	for (paf = obj->first_affect ; paf ; paf = paf->next)
	{
		switch (paf->location)
		{
			case APPLY_STR:
				mob->npcdata->damplus	+= paf->modifier * 10;
				break;
			case APPLY_DAMROLL:
				mob->npcdata->damplus	+= paf->modifier * 5;
				break;
			case APPLY_HIT:
			case APPLY_CON:
				mob->hit = mob->max_hit	 = UMAX(100, mob->max_hit + paf->modifier * 10);
				break;
			case APPLY_MANA:
			case APPLY_WIS:
				mob->mana = mob->max_mana = UMAX(100, mob->max_mana + paf->modifier * 10);
				break;
			case APPLY_MOVE:
				mob->move = mob->max_move = UMAX(100, mob->max_move + paf->modifier * 10);
				break;
			case APPLY_DEX:
				af.modifier	= paf->modifier * 20;
				af.location	= APPLY_AC;
				affect_to_char(mob, &af);
				break;
			case APPLY_AC:
				af.modifier	= paf->modifier * 10;
				af.location	= APPLY_AC;
				affect_to_char(mob, &af);
				break;
			case APPLY_HITROLL:
				mob->hitroll			+= paf->modifier * 5;
				break;
			case APPLY_SAVING_PARA:
			case APPLY_SAVING_ROD:
			case APPLY_SAVING_PETRI:
			case APPLY_SAVING_BREATH:
			case APPLY_SAVING_SPELL:
			case APPLY_INT:
				mob->saving_throw		+= paf->modifier * 5;
				break;
		}
	}

	for (paf = obj->pIndexData->first_affect ; paf ; paf = paf->next)
	{
		switch (paf->location)
		{
			case APPLY_STR:
				mob->npcdata->damplus	+= paf->modifier * 10;
				break;
			case APPLY_DAMROLL:
				mob->npcdata->damplus	+= paf->modifier * 5;
				break;
			case APPLY_HIT:
			case APPLY_CON:
				mob->hit = mob->max_hit	 = UMAX(100, mob->max_hit + paf->modifier * 10);
				break;
			case APPLY_MANA:
			case APPLY_WIS:
				mob->mana = mob->max_mana = UMAX(100, mob->max_mana + paf->modifier * 10);
				break;
			case APPLY_MOVE:
				mob->move = mob->max_move = UMAX(100, mob->max_move + paf->modifier * 10);
				break;
			case APPLY_DEX:
				af.modifier	= paf->modifier * 20;
				af.location	= APPLY_AC;
				affect_to_char(mob, &af);
				break;
			case APPLY_AC:
				af.modifier	= paf->modifier * 10;
				af.location	= APPLY_AC;
				affect_to_char(mob, &af);
				break;
			case APPLY_HITROLL:
				mob->hitroll			+= paf->modifier * 5;
				break;
			case APPLY_SAVING_PARA:
			case APPLY_SAVING_ROD:
			case APPLY_SAVING_PETRI:
			case APPLY_SAVING_BREATH:
			case APPLY_SAVING_SPELL:
			case APPLY_INT:
				mob->saving_throw		+= paf->modifier * 5;
				break;
		}
	}

	if (obj->item_type == ITEM_ARMOR)
	{
		af.modifier	= obj->value[0] * -10;
		af.location	= APPLY_AC;
		affect_to_char(mob, &af);
	}
	if (IS_OBJ_STAT(obj, ITEM_INVIS))
	{
		SET_BIT(mob->affected_by, AFF_IMP_INVISIBLE);
	}
	if (IS_OBJ_STAT(obj, ITEM_BLESS))
	{
		mob->alignment = 1000;
	}
	if (IS_OBJ_STAT(obj, ITEM_EVIL))
	{
		mob->alignment = -1000;
	}

	while (obj->first_content != NULL)
	{
		obj_to_char(obj->first_content, mob);
	}

	act("$N flies from your hand and bows before you.", ch, NULL, mob, TO_CHAR);
	act("$N flies from $n's hand and bows before $m.",  ch, NULL, mob, TO_ROOM);

	SET_BIT(mob->affected_by, AFF_CHARM );
	add_follower( mob, ch );

	if (IS_SET(obj->extra_flags, ITEM_MOUNT))
	{
		SET_BIT( mob->act, ACT_MOUNT );
	}

	REMOVE_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
	REMOVE_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
	REMOVE_BIT(obj->extra_flags, ITEM_ANTI_NEUTRAL);

	SET_BIT(obj->extra_flags, ITEM_ETHEREAL);
	SET_BIT(obj->extra_flags, ITEM_NOREMOVE);
	SET_BIT(obj->extra_flags, ITEM_NODROP);
	SET_BIT(obj->extra_flags, ITEM_INVENTORY);

	switch (obj->item_type)
	{
		case ITEM_WEAPON:
			obj_to_char(obj, mob);
			equip_char(mob, obj, WEAR_WIELD);
			break;
		case ITEM_LIGHT:
			obj_to_char(obj, mob);
			equip_char(mob, obj, WEAR_LIGHT);
			break;
		default:
			if (IS_SET(obj->wear_loc, ITEM_WEAR_SHIELD))
			{
				obj_to_char(obj, mob);
				equip_char(mob, obj, WEAR_SHIELD);
			}
			else
			{
				junk_obj(obj);
			}
			break;
	}
	pop_call();
	return;
}

DO_SPELL(spell_mana_shackles)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_mana_shackles(%p,%p,%p,%p)",sn,level,ch,vo);

	if (is_affected(victim, sn))
	{
		send_to_char( "They are already shackled.\n\r", ch );
		pop_call();
		return;
	}

	if ( saves_spell( level, ch, victim ) )
	{
		act("Strings of lightning shoot from your hands but fail to entangle $N.",  ch, NULL, victim, TO_CHAR);
		act("Strings of lightning shoot from $n's hands but fail to entangle you.", ch, NULL, victim, TO_VICT);
		act("Strings of lightning shoot from $n's hands but fail to entangle $N.",  ch, NULL, victim, TO_NOTVICT);
		pop_call();
		return;
	}

	af.type      = sn;
	af.duration  = level/15;
	af.bittype   = AFFECT_TO_NONE;
	af.bitvector = 0;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	affect_to_char( victim, &af );

	act("Strings of lightning shoot from your hands, entangling $N.",  ch, NULL, victim, TO_CHAR);
	act("Strings of lightning shoot from $n's hands, entangling you.", ch, NULL, victim, TO_VICT);
	act("Strings of lightning shoot from $n's hands, entangling $N.",  ch, NULL, victim, TO_NOTVICT);

	pop_call();
	return;
}

DO_SPELL(spell_earthbind)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_earthbind(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_AFFECTED(victim, AFF2_EARTHBIND))
	{
		send_to_char("They are already bound to the earth.\n\r", ch);
		pop_call();
		return;
	}

	if (saves_spell(level, ch, victim))
	{
		act("Your shadow magic slithers around $N but fails to bind $M.", ch, NULL, victim, TO_CHAR);
		pop_call();
		return;
	}

	send_to_char("You have been bound to the earth.\n\r", victim);

	if (ch != victim)
	{
		send_to_char("You bind them to the earth.\n\r", ch);
	}

	if (CAN_FLY(victim))
	{
		act("You fall to the ground.", ch, NULL, victim, TO_VICT);
		act("$n falls to the ground.", victim, NULL, ch, TO_ROOM);

		damage(ch, victim, number_range(level, level*3), sn);
	}

	affect_strip(victim, gsn_fly);

	af.type      = sn;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.duration  = 1+level/5;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF2_EARTHBIND;
	affect_to_char( victim, &af );

	pop_call();
	return;
}

DO_SPELL(spell_quicken)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_quicken(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_AFFECTED(victim, AFF2_QUICKEN))
	{
		send_to_char("They are already quick of mind.\n\r", ch);
		pop_call();
		return;
	}

	act( "$n's eyes glitter as time wrinkles then slows to quicken $s mind.", victim, NULL, NULL, TO_ROOM );

	af.type      = sn;
	af.duration  = 12;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF2_QUICKEN;
	affect_to_char( victim, &af );

	send_to_char( "Time wrinkles then slows as your thoughts start racing.\n\r", victim );

	if (ch != victim)
	{
		send_to_char( "Ok.\n\r", ch );
	}
	pop_call();
	return;
}

DO_SPELL(spell_torrid_balm)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_torrid_balm(%p,%p,%p,%p)",sn,level,ch,vo);

	if (IS_AFFECTED(victim, AFF2_TORRID_BALM))
	{
		send_to_char( "They are already engulfed by fire.\n\r", ch );
		pop_call();
		return;
	}

	if (saves_spell(level, ch, victim))
	{
		send_to_char( "Flames ignite from your hands, but fail to surround your foe.\n\r", ch );
		pop_call();
		return;
	}

	af.type      = sn;
	af.duration  = level / 20;
	af.location  = IS_NPC(victim) ? APPLY_AC : APPLY_DEX;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF2_TORRID_BALM;
	affect_to_char(victim, &af );

	if (ch != victim)
	{
		act("Flames ignite from your hands, engulfing $N.",  ch, NULL, victim, TO_CHAR);
		act("Flames ignite from $n's hands, engulfing you.", ch, NULL, victim, TO_VICT);
		act("Flames ignite from $n's hands, engulfing $N.",  ch, NULL, victim, TO_NOTVICT);
	}
	else
	{
		act("Flames ignite from your hands, engulfing yourself.", ch, NULL, victim, TO_CHAR);
		act("Flames ignite from $n's hands, engulfing $mself.",   ch, NULL, victim, TO_NOTVICT);
	}
	pop_call();
	return;
}

DO_SPELL(spell_song_of_the_seas)
{
	CHAR_DATA *victim = (CHAR_DATA *)vo;
	AFFECT_DATA af;

	push_call("spell_song_of_the_seas(%p,%p,%p,%p)",sn,level,ch,vo);

	if (is_affected(victim, sn))
	{
		send_to_char("They are already affected.\n\r", ch);
		pop_call();
		return;
	}

	af.type      = sn;
	af.location  = APPLY_WIS;
	af.duration  = URANGE(2, level/10, 8);
	af.modifier  = 1 + (level > 49) + (level > 99);
	af.bittype   = AFFECT_TO_NONE;
	af.bitvector = 0;
	affect_to_char(victim, &af);

	send_to_char( "Your eyes shine sagaciously as the song of the seas guides your path.\n\r", victim);
	act( "$n's eyes shine sagaciously.", victim, NULL, NULL, TO_ROOM );

	pop_call();
	return;
}

DO_SPELL(spell_earthen_spirit)
{
	CHAR_DATA *victim = (CHAR_DATA *)vo;
	AFFECT_DATA af;

	push_call("spell_earthen_spirit(%p,%p,%p,%p)",sn,level,ch,vo);

	if (is_affected(victim, sn))
	{
		send_to_char("They are already affected.\n\r", ch);
		pop_call();
		return;
	}

	af.type      = sn;
	af.location  = APPLY_CON;
	af.duration  = URANGE(2, level/10, 8);
	af.modifier  = 1 + (level > 49) + (level > 99);
	af.bittype   = AFFECT_TO_NONE;
	af.bitvector = 0;
	affect_to_char(victim, &af);

	send_to_char("The spirit of the earth swirls around you.\n\r", victim);
	act( "The spirit of the earth swirls around $n", victim, NULL, NULL, TO_ROOM );

	pop_call();
	return;
}

DO_SPELL(spell_icicle_armor)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_icicle_armor(%p,%p,%p,%p)",sn,level,ch,vo);

	if (is_affected(victim, sn))
	{
		send_to_char("You are already encased in an icicle armor.\n\r", ch);
		pop_call();
		return;
	}
	af.type		= sn;
	af.duration	= level;
	af.modifier	= -1 * UMAX(1, level / 15) * 5;
	af.location	= APPLY_AC;
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector	= AFF2_ICICLE_ARMOR;

	affect_to_char(victim, &af);

	send_to_char("Spikes of ice rip through your skin, encasing you in armor.\n\r", victim);
	act("Spikes of ice rip through $n's skin, encasing $m in armor.", victim, NULL, NULL, TO_ROOM );

	pop_call();
	return;
}

DO_SPELL(spell_flamewave)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	EXIT_DATA *pexit;
	int door, temp_vnum;

	push_call("spell_flamewave(%p,%p,%p,%p)",sn,level,ch,vo);

	/*
		no flamewave under water
	*/

	if (ch->in_room->sector_type == SECT_UNDER_WATER)
	{
		send_to_char("The water boils around you.\n\r",ch);
		pop_call();
		return;
	}

	switch (ch->in_room->sector_type)
	{
		case SECT_LAKE:
		case SECT_RIVER:
		case SECT_OCEAN:
			level /= 2;
	}

	act("Tongues of fire and flame spew forth by your command!", ch, NULL, NULL, TO_CHAR);
	act("Tongues of fire and flame spew forth by $n's command.", ch, NULL, NULL, TO_ROOM);

	temp_vnum = ch->in_room->vnum;

	for (door = 0 ; door <= 5 ; door++)
	{
		if ((pexit = get_exit(temp_vnum, door)) && pexit->to_room != temp_vnum)
		{
			ch->in_room = room_index[pexit->to_room];
			act("The air about you sears with heat.", ch, NULL, NULL, TO_ROOM);
		}
	}
	ch->in_room = room_index[temp_vnum];

	for (vch = ch->in_room->last_person ; vch ; vch = vch_next)
	{
		vch_next	= vch->prev_in_room;

		if (!can_mass_cast(ch, vch))
		{
			continue;
		}
		damage( ch, vch, level + dice(1, level), sn );
	}
	pop_call();
	return;
}

DO_SPELL(spell_flame_blade)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_flame_blade(%p,%p,%p,%p)",sn,level,ch,vo);

	if (obj->item_type != ITEM_WEAPON)
	{
		act("You can only imbue weapons with fire.", ch, NULL, NULL, TO_CHAR);
		pop_call();
		return;
	}

	if (IS_SET(obj->extra_flags, ITEM_BURNING))
	{
		act("$p is already burning.", ch, obj, NULL, TO_CHAR);
		pop_call();
		return;
	}

	if (number_fuzzy(obj->level) >= level)
	{
		act("$p glows brightly, then returns to normal.", ch, obj, NULL, TO_CHAR);
		act("$p glows brightly, then returns to normal.", ch, obj, NULL, TO_ROOM);
		pop_call();
		return;
	}

	SET_BIT(obj->extra_flags, ITEM_MODIFIED);
	SET_BIT(obj->extra_flags, ITEM_MAGIC);

	act("Blue flames ignite from $p.", ch, obj, NULL, TO_CHAR);
	act("Blue flames ignite from $p.", ch, obj, NULL, TO_ROOM);

	af.type      = sn;
	af.duration  = level * 5;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bittype   = AFFECT_TO_OBJ;
	af.bitvector = ITEM_BURNING;

	affect_to_obj(obj, &af);

	pop_call();
	return;
}

DO_SPELL(spell_totem)
{
	OBJ_DATA *totem;

	push_call("spell_totem(%p,%p,%p,%p)",sn,level,ch,vo);

	for (totem = ch->in_room->first_content ; totem ; totem = totem->next_content)
	{
		if (totem->item_type == ITEM_TOTEM)
		{
			send_to_char("There is already a totem in this room.\n\r", ch);
			pop_call();
			return;
		}
	}

	switch (ch->in_room->sector_type)
	{
		case SECT_AIR:
		case SECT_ETHEREAL:
		case SECT_ASTRAL:
		case SECT_LAKE:
		case SECT_RIVER:
		case SECT_OCEAN:
			send_to_char("You cannot cast a totem in this room.\n\r", ch);
			pop_call();
			return;
	}

	totem = create_object( get_obj_index( OBJ_VNUM_TOTEM ), 0 );
	totem->timer = level;
	totem->level = level;

	act( "A totem rises up from the ground.", ch, NULL, NULL, TO_CHAR );
	act( "A totem rises up from the ground.", ch, NULL, NULL, TO_ROOM );

	obj_to_room( totem, ch->in_room->vnum );

	switch (tolower(target_name[0]))
	{
		case 'c':
			totem->value[0] = skill_lookup("cure poison");
			totem->value[1] = skill_lookup("cure poison");
			totem->value[2] = skill_lookup("remove curse");
			totem->value[3] = skill_lookup("cure critical");
			break;

		case 'p':
			totem->value[0] = skill_lookup("giant strength");
			totem->value[1] = skill_lookup("enhanced heal");
			totem->value[2] = skill_lookup("bless");
			if (ch->alignment < 0)
			{
				totem->value[3] = skill_lookup("protection good");
			}
			else
			{
				totem->value[3] = skill_lookup("protection evil");
			}
			break;
		default:
			totem->value[0] = skill_lookup("heal");
			totem->value[1] = skill_lookup("cure light");
			totem->value[2] = skill_lookup("cure serious");
			totem->value[3] = skill_lookup("cure critical");
			break;
	}
	pop_call();
	return;
}

/*
 * 11 sept 2004, Michiel Lange
 * First stonefist implementation
 */

DO_SPELL(spell_steelhand)
{
	CHAR_DATA *victim=(CHAR_DATA *) vo;
	AFFECT_DATA af;

	push_call("spell_steelhand(%p,%p,%p,%p)",sn,level,ch,vo);

	if ( is_affected( victim, sn ) )
	{
		send_to_char( "They are already affected.\n\r", ch );
		pop_call();
		return;
	}
	
	af.type      = sn;
	af.duration  = level/2;
	af.location  = 0;
	af.modifier  = ((level/10)+1) * ((level/25)+6);
	af.bittype   = AFFECT_TO_CHAR;
	af.bitvector = AFF2_STEELHAND;
	affect_to_char(victim, &af);

	act( "$n's hands become razorsharp steel.", victim, NULL, NULL, TO_ROOM );
	send_to_char( "Your hands turn to razorsharp steel blades.\n\r", victim );
	pop_call();
	return;
}

/* The sloth spell decreases the victim's agility for a certain amount of time 
   Created by Hypnos 20041114. */
DO_SPELL(spell_sloth)
{
	CHAR_DATA *victim=(CHAR_DATA *) vo;
	AFFECT_DATA af;
	
	push_call("spell_sloth(%d,%d,%p,%p,%d)", sn, level, ch, vo, target);
	
	if (is_affected(victim, sn))
	{
		if (victim == ch)
		{
			ch_printf(ch, "You are already affected by the sloth spell.\n\r");
		}
		else
		{
			ch_printf(ch, "%s is already affected by the sloth spell.\n\r", capitalize(get_name(victim)));
		}
		
		pop_call();
		return;
	}
	
	af.type = sn;
	
	if (!IS_NPC(victim) && IS_IMMORTAL(victim))
	{
		ch_printf(ch, "Immortal beings are immune to the sloth spell.\n\r");
		pop_call();
		return;
	}

	if (IS_NPC(victim))
	{
		af.duration = number_fuzzy(level / 10);
	}
	else
	{
		af.duration = number_fuzzy(level / 15);
	}

	af.location = APPLY_DEX;
	af.modifier = -(1 + ((level / 30)) * ((level / 10) + 1));
	af.bittype = AFFECT_TO_CHAR;
	af.bitvector = 0;

	affect_to_char(victim, &af);

	act( "$n's agility dwindles.", victim, NULL, NULL, TO_ROOM );
	send_to_char( "Your agility dwindles as sloth takes hold of you.\n\r", victim);
	pop_call();
	return;
}

/* The spell "a touch of idiocy" decrease's the poor victim's 
   Int and Wis scores by 1d6 
   Created by Hypnos 20041115 */
DO_SPELL(spell_touch_of_idiocy)
{
	CHAR_DATA *victim=(CHAR_DATA *) vo;
	AFFECT_DATA af;
	
	push_call("spell_touch_of_idiocy(%d,%d,%p,%p,%d)", sn, level, ch, vo, target);
	
	if (is_affected(victim, sn))
	{
		if(victim == ch)
		{
			ch_printf(ch, "You are already affected by the touch of idiocy spell.\n\r");
		}
		else
		{
			ch_printf(ch, "%s is already affected by the touch of idiocy spell.\n\r", capitalize(get_name(victim)));
		}
		
		pop_call();
		return;
	}
	
	af.type = sn;
	
	if (!IS_NPC(victim) && IS_IMMORTAL(victim))
	{
		ch_printf(ch, "The infinite wisdom and intellect of the Immortals are immune to the touch of idocy spell.\n\r");
		pop_call();
		return;
	}

	af.duration = number_fuzzy(level / 8);

	af.location = APPLY_INT;
	af.modifier = -1 - dice(level / 25, 6);
	af.bittype = AFFECT_TO_CHAR;
	af.bitvector = 0;
	
	affect_to_char(victim, &af);

	af.location = APPLY_WIS;
	af.modifier = -1 - dice(level / 25, 6);
	affect_to_char(victim, &af);

	act( "A powerful curse descends upon $s.", victim, NULL, NULL, TO_ROOM );
	send_to_char( "You suddenly feel a lot less certain of yourself.\n\r", victim);
	pop_call();
	return;
}

