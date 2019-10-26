/***************************************************************************
 * Lola  1.0 by Igor van den Hoven.                                        *
 *                                                                         *
 * Emud  2.2 by Igor van den Hoven, Michiel Lange.                         *
 *                                                                         *
 * MrMud 1.4 by David Bills and Dug Michael.                               *
 ***************************************************************************/
 
#include "mud.h"


void do_polymorph( CHAR_DATA *ch, char *argument )
{
	AFFECT_DATA af;

	push_call("do_polymorph(%p,%p)",ch,argument);

	if (!is_affected(ch, gsn_polymorph) && number_percent() > learned(ch, gsn_polymorph))
	{
		ch_printf(ch, "You failed to polymorph yourself.\n\r");
		pop_call();
		return;
	}

	if (argument && argument[0] == '\0')
	{
		if (is_affected(ch, gsn_polymorph))
		{
			argument = NULL;
		}
		else
		{
			ch_printf(ch, "Polymoprh yourself into what?\n\r");
			pop_call();
			return;
		}
	}

	if (argument)
	{
		if (ch->race == lookup_race(argument))
		{
			ch_printf(ch, "You are already a %s.\n\r", race_table[ch->race].race_name);
			pop_call();
			return;
		}

		if (lookup_race(argument) == -1)
		{
			ch_printf(ch, "You do not know how to change into a %s.\n\r", argument);
			pop_call();
			return;
		}

		if (!IS_NPC(ch) && race_table[lookup_race(argument)].enter > ch->pcdata->reincarnation)
		{
			ch_printf(ch, "Changing into %s %s is beyond your powers.\n\r", get_prefix(race_table[lookup_race(argument)].race_name), race_table[lookup_race(argument)].race_name);
			pop_call();
			return;
		}
	}
	affect_strip(ch, gsn_polymorph);

	if (argument)
	{
		af.type      = gsn_polymorph;
		af.duration  = number_range(12, 24);
		af.location  = APPLY_RACE;
		af.modifier  = lookup_race(argument) - ch->race;
		af.bittype   = AFFECT_TO_NONE;
		af.bitvector = 0;
		affect_to_char( ch, &af);
	}

	if (!IS_NPC(ch))
	{
		char_reset(ch);
	}

	act("Your bones make a grinding noise as you change into $t $T.", ch, get_prefix(race_table[ch->race].race_name), race_table[ch->race].race_name, TO_CHAR);
	act("$n's bones make a grinding noise as $e changes into $t $T.", ch, get_prefix(race_table[ch->race].race_name), race_table[ch->race].race_name, TO_ROOM);

	check_improve(ch, gsn_polymorph);
	pop_call();
	return;
}

void do_camp( CHAR_DATA *ch, char *argument )
{
	push_call("do_camp(%p,%p)",ch,argument);

	if(multi(ch, gsn_camp)==-1 && !IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if(ch->position<=POS_RESTING && in_camp(ch))
	{
		send_to_char("You are already camping.\n\r", ch);
		pop_call();
		return;
	}

	if(ch->position!=POS_STANDING)
	{
		send_to_char("You must be standing in order to establish a camp.\n\r", ch);
		pop_call();
		return;
	}
	if (number_percent() < learned(ch, gsn_camp))
	{
		if (!can_see_in_room(ch, ch->in_room))
		{
			if (number_percent() > 25)
			{
				send_to_char("You fumble around with your tools, but it looks nothing like a camp.\n\r",ch);
				act("$n fumbles around with $s tools...\n\r",ch,NULL,NULL,TO_ROOM);
			}
			else
			{
				send_to_char("You fumble around with your tools, and miraculously you establish a camp!\n\r",ch);
				act("$n fumbles around with $s tools, and establishes a camp!\n\r",ch,NULL,NULL,TO_ROOM);
				ch->position=POS_RESTING;
				SET_BIT(ch->affected2_by, 0-AFF2_CAMPING);
			}
		}
		else
		{
			send_to_char("You establish a camp and begin recuperating.\n\r", ch);
			act("$n establishes a camp.",ch,NULL,NULL,TO_ROOM);
			ch->position=POS_RESTING;
			SET_BIT(ch->affected2_by, 0-AFF2_CAMPING);
			check_improve(ch, gsn_camp);
		}
	}
	else
	{
		send_to_char("You fail to establish a good camp.\n\r", ch);
	}

	pop_call();
	return;
}

void do_ambush( CHAR_DATA *ch, char *argument )
{
	push_call("do_abmush(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	STRFREE(ch->pcdata->ambush);
	ch->pcdata->ambush = STRALLOC("");

	if (argument[0] == '\0')
	{
		if (strlen(ch->pcdata->ambush) > 0)
		{
			send_to_char("You step out of your ambush.\n\r", ch);
		}
		else
		{
			send_to_char("Ambush whom?\n\r", ch);
		}
		pop_call();
		return;
	}

	ch_printf(ch, "You move toward a strategic position and prepare to ambush %s.\n\r", capitalize(argument));

	if (number_percent() < learned(ch, gsn_ambush))
	{
		STRFREE(ch->pcdata->ambush);
		ch->pcdata->ambush = STRALLOC(argument);
	}
	pop_call();
	return;
}
