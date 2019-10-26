/***************************************************************************
 * Lola  1.0 by Igor van den Hoven.                                        *
 *                                                                         *
 * Emud  2.2 by Igor van den Hoven, Michiel Lange.                         *
 *                                                                         *
 * MrMud 1.4 by David Bills and Dug Michael.                               *
 ***************************************************************************/

#include "mud.h"

/*
	race and class flags
*/

bool rspec_req( CHAR_DATA *ch, long long attr)
{
	push_call("rspec_req(%p,%p)",ch,attr);

	if (ch == NULL)
	{
		pop_call();
		return FALSE;
	}

	if (IS_NPC(ch) && !HAS_BIT(ch->act, ACT_RACE))
	{
		pop_call();
		return FALSE;
	}

	if (race_table[ch->race].flags & attr)
	{
		pop_call();
		return TRUE;
	}
	else
	{
		pop_call();
		return FALSE;
	}
}

bool cflag( CHAR_DATA *ch, long long attr)
{
	push_call("cspec_req(%p,%p)",ch,attr);

	if (IS_NPC(ch) && !HAS_BIT(ch->act, ACT_CLASS))
	{
		pop_call();
		return FALSE;
	}

	if (HAS_BIT(class_table[ch->class].spec, attr))
	{
		pop_call();
		return TRUE;
	}
	else
	{
		pop_call();
		return FALSE;
	}

	pop_call();
	return FALSE;
}

int get_age_bonus(CHAR_DATA *ch, int type)
{
	int bonus;

	push_call("get_age_bonus(%p,%p)",ch,type);

	bonus = 0;

	if (IS_SET(race_table[ch->race].flags, type))
	{
		switch (type)
		{
			case RSPEC_AGE_AC:
				bonus -= ch->level/6  + ch->pcdata->reincarnation*3/2;
				break;
			case RSPEC_AGE_HR:
				bonus += ch->level/6  + ch->pcdata->reincarnation*2/1;
				break;
			case RSPEC_AGE_DR:
				bonus += ch->level/18 + ch->pcdata->reincarnation*1/2;
				break;
		}
	}
	pop_call();
	return bonus;
}
