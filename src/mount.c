/***************************************************************************
 * Lola  1.0 by Igor van den Hoven.                                        *
 *                                                                         *
 * Emud  3.0 by Igor van den Hoven.                                        *
 ***************************************************************************/
 
#include "mud.h"

bool is_mounting( CHAR_DATA *ch )
{
	bool mounting;

	push_call("is_mounting(%p)", ch);

	mounting = (ch->mounting && ch->mounting->in_room == ch->in_room);

	pop_call();
	return mounting;
}

bool is_mounted( CHAR_DATA *ch )
{
	bool mounted;

	push_call("is_mounted(%p)", ch);

	mounted = (ch->master && ch->master->mounting == ch && ch->master->in_room == ch->in_room);

	pop_call();
	return mounted;
}

void do_mount( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	push_call("do_mount(%p,%p)",ch,argument);

	if (learned(ch, gsn_mount) == 0)
	{
		send_to_char("You are not skilled enough to mount anything.\n\r", ch);
		pop_call();
		return;
	}

	if (is_mounting(ch))
	{
		send_to_char("You are already mounted.\n\r", ch);
		pop_call();
		return;
	}

	if (*argument == 0)
	{
		send_to_char("Mount whom?\n\r", ch);
		pop_call();
		return;
	}

	if ((victim = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		pop_call();
		return;
	}

	if (is_mounted(victim))
	{
		send_to_char("They are already mounted.\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_NPC(victim) || victim->master != ch)
	{
		send_to_char("You can only mount pets.\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_SET(victim->act, ACT_MOUNT))
	{
		send_to_char("They cannot be mounted.\n\r", ch);
		pop_call();
		return;
	}

	if (victim->fighting)
	{
		send_to_char("Your mount is moving too fast to be mounted.\n\r", ch);
		pop_call();
		return;
	}

	if (victim->position != POS_STANDING)
	{
		send_to_char("Your mount must be standing to be mounted.\n\r", ch);
		pop_call();
		return;
	}

	wait_state(ch, skill_table[gsn_mount].beats);

	if (number_percent() > learned(ch, gsn_mount) || (CAN_FLY(victim) && number_range(50, 100) > learned(ch, gsn_mount)))
	{
		act("You lose your balance and fall to the ground as you try to mount $N.",	ch, NULL, victim, TO_CHAR);
		act("$n loses $s balance and falls to the ground as $e tries to mount you.",	ch, NULL, victim, TO_VICT);
		act("$n loses $s balance and falls to the ground as $e tries to mount $N.",	ch, NULL, victim, TO_NOTVICT);

		pop_call();
		return;
	}
	act("$N lowers $S head and kneels as you take your mount.",  ch, NULL, victim, TO_CHAR);
	act("You lower your head and kneel as $n mounts you.", ch, NULL, victim, TO_VICT);
	act("$N lowers $S head and kneels as $n mounts $M.",  ch, NULL, victim, TO_NOTVICT);

	ch->mounting  = victim;
	ch->furniture = NULL;

	check_improve(ch, gsn_mount);

	pop_call();
	return;
}

void do_dismount( CHAR_DATA *ch, char *argument )
{
	push_call("do_dismount(%p,%p)",ch,argument);

	if (!is_mounting(ch))
	{
		send_to_char("You are not mounted.\n\r", ch);
		pop_call();
		return;
	}

	act("You tug on the reins and dismount $N.",  ch, NULL, ch->mounting, TO_CHAR);
	act("$n yanks back on your reins and you halt quickly.", ch, NULL, ch->mounting, TO_VICT);
	act("$n pulls on $N's reins and jumps off.",  ch, NULL, ch->mounting, TO_NOTVICT);

	ch->mounting = NULL;

	pop_call();
	return;
}

void do_tame( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;

	push_call("do_mount(%p,%p)",ch,argument);

	if ((victim = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		pop_call();
		return;
	}

	if (learned(ch, gsn_tame) == 0)
	{
		send_to_char("You are not skilled enough to tame anything.\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_NPC(victim) || !IS_SET(victim->act, ACT_MOUNT))
	{
		send_to_char("They cannot be tamed.\n\r", ch);
		pop_call();
		return;
	}

	if (victim->master)
	{
		act("$N has already been tamed.", ch, NULL, victim, TO_CHAR);
		pop_call();
		return;
	}

	if (IS_UNDEAD(ch) && !IS_UNDEAD(victim))
	{
		act("$N backs away in fear as you try to tame $M",	ch, NULL, victim, TO_CHAR);
		act("You back away in fear as $n tries to tame you.",	ch, NULL, victim, TO_VICT);
		act("$N backs away in fear as $n tries to tame $M",	ch, NULL, victim, TO_NOTVICT);

		pop_call();
		return;
	}

	if (!IS_UNDEAD(ch) && IS_UNDEAD(victim) && !rspec_req(ch, RSPEC_INTIMIDATE))
	{
		act("Black smoke billows from $N's nostrils as $E rears up to you as you try to tame $M.",	ch, NULL, victim, TO_CHAR);
		act("Black smoke billows from your nostrils as you rear up to $n as $e tries to tame you.",	ch, NULL, victim, TO_VICT);
		act("Black smoke billows from $N's nostrils as $E rears up to $n who is trying to tame $M.",	ch, NULL, victim, TO_NOTVICT);

		pop_call();
		return;
	}

	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_KILLER) && !IS_EVIL(victim) && !IS_UNDEAD(victim))
	{
		act("$N cowers away in fear as you struggle to tame $M.",	ch, NULL, victim, TO_CHAR);
		act("You cower away in fear as $N struggles to tame you.",	ch, NULL, victim, TO_VICT);
		act("$N cowers away in fear as $N struggles to tame $M.",	ch, NULL, victim, TO_NOTVICT);

		pop_call();
		return;
	}

	if (victim->level > ch->level / 2 || number_percent() > learned(ch, gsn_tame))
	{
		send_to_char("You failed.\n\r", ch);
		pop_call();
		return;
	}

	if (get_pets(ch) >= 1)
	{
		send_to_char("You have too many pets.\n\r", ch);
		pop_call();
		return;
	}

	act("You tame $N.",  ch, NULL, victim, TO_CHAR);
	act("$n tames you.", ch, NULL, victim, TO_VICT);
	act("$n tames $N.",  ch, NULL, victim, TO_NOTVICT);

	add_follower(victim, ch);
	SET_BIT(victim->affected_by, AFF_CHARM);

	pop_call();
	return;
}
