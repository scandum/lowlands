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

void obj_set_to_char( CHAR_DATA *, OBJ_DATA * );

/*
	Retrieve a character's trusted level for permission checking.
*/

int get_trust( CHAR_DATA *ch )
{
	push_call("get_trust(%p)",ch);

	if (IS_NPC(ch))
	{
		if (IS_AFFECTED(ch,AFF_CHARM))
		{
			pop_call();
			return -9;
		}
		if (IS_AFFECTED(ch, AFF2_POSSESS))
		{
			pop_call();
			return -8;
		}
		if (ch->desc)
		{
			switch (ch->pIndexData->vnum)
			{
				case 10:
				case 11:
					pop_call();
					return -3;
				default:
					pop_call();
					return -8;
			}
		}
		pop_call();
		return MAX_LEVEL-1;
	}

	if (ch->trust < ch->level)
	{
		ch->trust = ch->level;
	}
	pop_call();
	return ch->trust;
}

/*
	Retrieve amount of empty space available on character's screen
*/

int get_pager_breakpt( CHAR_DATA *ch )
{
	int breakpt = ch->pcdata->screensize_v;

	push_call("get_pager_breakpt(%p",ch);

	if (HAS_BIT(ch->pcdata->vt100_flags, VT100_INTERFACE))
	{
		breakpt -= 4 + ch->pcdata->tactical_size_v;
	}
	else if (IS_SET(ch->act, PLR_BLANK))
	{
		breakpt -= 4;
	}
	else
	{
		breakpt -= 3;
	}
	pop_call();
	return breakpt;
}

int get_page_width( CHAR_DATA *ch )
{
	push_call("get_page_width(%p)",ch);

	if (ch->desc)
	{
		pop_call();
		return (CH(ch->desc)->pcdata->screensize_h);
	}
	pop_call();
	return 80;
}

/*
	Retrieve a character's age.
*/

int get_age( CHAR_DATA *ch )
{
	int age = 17;

	push_call("get_age(%p)",ch);

	if (!IS_NPC(ch)) age += ch->pcdata->played / 20000;

	pop_call();
	return age;
}


/*
	Retrieve character's current strength.
*/

int get_max_str( CHAR_DATA *ch )
{
	int max;

	push_call("get_max_str(%p)",ch);

	if (IS_NPC(ch))
	{
		max = 15 + ch->level / 10;
	}
	else
	{
		max = 19 + ch->level / 9 + ch->pcdata->reincarnation;

		if (class_table[ch->class].attr_prime == APPLY_STR)
		{
			max += class_table[ch->class].prime_mod;
		}
		if (class_table[ch->class].attr_second == APPLY_STR)
		{
			max += class_table[ch->class].sec_mod;
		}
		max += race_table[ch->race].race_mod[0];

	}
	pop_call();
	return UMIN(max, MAX_STAT);
}

int get_curr_str( CHAR_DATA *ch )
{
	int max;

	push_call("get_curr_str(%p)",ch);

	max = get_max_str(ch);

	if (IS_NPC(ch))
	{
		pop_call();
		return 13 + ch->level / 15;
	}
	else
	{
		pop_call();
		return URANGE(1, ch->pcdata->perm_str + ch->pcdata->mod_str, max);
	}
}

/*
	Retrieve character's current intelligence.
*/

int get_max_int( CHAR_DATA *ch )
{
	int max;

	push_call("get_max_int(%p)",ch);

	if (IS_NPC(ch))
	{
		max = 15 + ch->level / 10;
	}
	else
	{
		max = 19 + ch->level/9 + ch->pcdata->reincarnation;

		if (class_table[ch->class].attr_prime == APPLY_INT)
		{
			max += class_table[ch->class].prime_mod;
		}

		if (class_table[ch->class].attr_second == APPLY_INT)
		{
			max += class_table[ch->class].sec_mod;
		}

		max += race_table[ch->race].race_mod[2];
	}

	pop_call();
	return UMIN(max, MAX_STAT);
}

int get_curr_int( CHAR_DATA *ch )
{
	int max;

	push_call("get_curr_int(%p)",ch);

	max = get_max_int(ch);

	if (IS_NPC(ch))
	{
		pop_call();
		return max;
	}

	pop_call();
	return URANGE(1, ch->pcdata->perm_int + ch->pcdata->mod_int, max);
}

/*
	Retrieve character's current wisdom.
*/

int get_max_wis( CHAR_DATA *ch )
{
	int max;

	push_call("get_max_wis(%p)",ch);

	if (IS_NPC(ch))
	{
		max = 15 + ch->level / 10;
	}
	else
	{
		max = 19 + ch->level/9 + ch->pcdata->reincarnation;

		if (class_table[ch->class].attr_prime == APPLY_WIS)
		{
			max += class_table[ch->class].prime_mod;
		}
		if (class_table[ch->class].attr_second == APPLY_WIS)
		{
			max += class_table[ch->class].sec_mod;
		}

		max += race_table[ch->race].race_mod[3];
	}
	pop_call();
	return UMIN(max, MAX_STAT);
}

int get_curr_wis( CHAR_DATA *ch )
{
	int max;

	push_call("get_curr_wis(%p)",ch);

	max = get_max_wis(ch);

	if (IS_NPC(ch))
	{
		pop_call();
		return max;
	}
	else
	{
		pop_call();
		return URANGE(1, ch->pcdata->perm_wis + ch->pcdata->mod_wis, max);
	}
}

/*
	Retrieve character's current dexterity.
*/

int get_max_dex( CHAR_DATA *ch )
{
	int max;

	push_call("get_max_dex(%p)",ch);

	if (IS_NPC(ch))
	{
		max = 15 + ch->level / 10;
	}
	else
	{
		max = 19 + ch->level/9 + ch->pcdata->reincarnation;

		if (class_table[ch->class].attr_prime == APPLY_DEX)
		{
			max += class_table[ch->class].prime_mod;
		}
		if (class_table[ch->class].attr_second == APPLY_DEX)
		{
			max += class_table[ch->class].sec_mod;
		}

		max += race_table[ch->race].race_mod[1];
	}
	pop_call();
	return UMIN(max, MAX_STAT);
}

int get_curr_dex( CHAR_DATA *ch )
{
	int max;

	push_call("get_curr_dex(%p)",ch);

	max = get_max_dex(ch);

	if (IS_NPC(ch))
	{
		pop_call();
		return max;
	}
	else
	{
		pop_call();
		return URANGE(1, ch->pcdata->perm_dex + ch->pcdata->mod_dex, max);
	}
}

/*
	Retrieve character's current constitution.
*/



int get_max_con( CHAR_DATA *ch )
{
	int max;

	push_call("get_max_con(%p)",ch);

	if (IS_NPC(ch))
	{
		max = 15 + ch->level / 10;
	}
	else
	{
		max = 19 + ch->level/9 + ch->pcdata->reincarnation;

		if (class_table[ch->class].attr_prime == APPLY_CON)
		{
			max += class_table[ch->class].prime_mod;
		}
		if (class_table[ch->class].attr_second == APPLY_CON)
		{
			max += class_table[ch->class].sec_mod;
		}

		max += race_table[ch->race].race_mod[4];
	}
	pop_call();
	return UMIN(max, MAX_STAT);
}

int get_curr_con( CHAR_DATA *ch )
{
	int max;

	push_call("get_curr_con(%p)",ch);

	max = get_max_con(ch);

	if (IS_NPC(ch))
	{
		pop_call();
		return max;
	}
	else
	{
		pop_call();
		return URANGE(1, ch->pcdata->perm_con + ch->pcdata->mod_con, max);
	}
}

/*
	Retrieve a character's carry capacity.
*/

int can_carry_n( CHAR_DATA *ch )
{
	int items;

	push_call("can_carry_n(%p)",ch);

	if (IS_NPC(ch) && HAS_BIT(ch->act, ACT_WEAK))
	{
		pop_call();
		return 0;
	}

	items = MAX_WEAR + get_max_dex(ch) + ch->level / 2;

	pop_call();
	return items;
}



/*
	Retrieve a character's carry capacity.
*/

int can_carry_w( CHAR_DATA *ch )
{
	push_call("can_carry_w(%p)",ch);

	if (IS_NPC(ch))
	{
		if (IS_SET(ch->act, ACT_WEAK))
		{
			pop_call();
			return 0;
		}
		pop_call();
		return 1000;
	}
	pop_call();
	return str_app[get_curr_str(ch)].carry;
}


/*
	See if a string is one of the names of an object.
*/

bool is_name( const char *str, char *namelist )
{
	char name[MAX_INPUT_LENGTH];

	push_call("is_name(%p,%p)",str,namelist);

	for ( ; ; )
	{
		namelist = one_argument_nolower(namelist, name);

		if (*name == '\0')
		{
			pop_call();
			return FALSE;
		}

		if (!strcasecmp(str, name))
		{
			pop_call();
			return TRUE;
		}
	}
	pop_call();
	return FALSE;
}

bool is_name_short( const char *str, char *namelist )
{
	char name[MAX_INPUT_LENGTH];

	push_call("is_name_short(%p,%p)",str,namelist);

	if (strlen(str) < 2)
	{
		pop_call();
		return FALSE;
	}

	for ( ; ; )
	{
		namelist = one_argument_nolower(namelist, name);

		if (*name == '\0')
		{
			pop_call();
			return FALSE;
		}

		if (!str_prefix(str, name))
		{
			pop_call();
			return TRUE;
		}
	}
	pop_call();
	return FALSE;
}


/*
	Checks if str is a name in namelist supporting multiple keywords
*/

bool is_name_list( char *str, char *namelist )
{
	char name[MAX_INPUT_LENGTH];

	push_call("is_name_list(%p,%p)",str,namelist);

	for ( ; ; )
	{
		str = one_argument_nolower(str, name);

		if (name[0] == '\0')
		{
			pop_call();
			return FALSE;
		}

		if (is_name(name, namelist))
		{
			pop_call();
			return TRUE;
		}
	}
	pop_call();
	return FALSE;
}

/*
	Lookup a skill by name.
*/

int skill_lookup( char *name )
{
	int sn, bot, top, srt;

	push_call("skill_lookup(%p)",name);

	bot = 0;
	top = MAX_SKILL -1;
	sn  = 0;

	while (bot <= top)
	{
		srt = strcmp(name, skill_table[sn].name);

		if (srt == 0)
		{
			pop_call();
			return sn;
		}

		if (srt < 0)
		{
			top = sn - 1;
		}
		else
		{
			bot = sn + 1;
		}
		sn = bot + (top - bot) / 2;
	}

	for (sn = 0 ; sn < MAX_SKILL ; sn++)
	{
		if (tolower(name[0]) == skill_table[sn].name[0])
		{
			if (is_multi_name_short(name, skill_table[sn].name))
			{
				pop_call();
				return sn;
			}
		}
	}

	pop_call();
	return -1;
}

/*
	Check if all words in str appear in the namelist - Scandum
*/

bool is_multi_name_list( char *str, char *namelist )
{
	char name[MAX_INPUT_LENGTH];

	push_call("is_multi_name(%p,%p)",str,namelist);

	if (namelist[0] == '\0')
	{
		pop_call();
		return FALSE;
	}

	for ( ; ; )
	{
		str = one_argument_nolower(str, name);

		if (name[0] == '\0')
		{
			pop_call();
			return TRUE;
		}

		if (!is_name(name, namelist))
		{
			pop_call();
			return FALSE;
		}
	}
	pop_call();
	return TRUE;
}

bool is_multi_name_list_short( char *str, char *namelist )
{
	char name[MAX_INPUT_LENGTH];

	push_call("is_multi_name_list(%p,%p)",str,namelist);

	if (namelist[0] == '\0')
	{
		pop_call();
		return FALSE;
	}

	for ( ; ; )
	{
		str = one_argument_nolower(str, name);

		if (name[0] == '\0')
		{
			pop_call();
			return TRUE;
		}

		if (!is_name_short(name, namelist))
		{
			pop_call();
			return FALSE;
		}
	}
	pop_call();
	return TRUE;
}


/*
	Some nice abreviation for multi key words - Scandum
*/

bool is_multi_name_short( char *str, char *namelist )
{
	char name[MAX_INPUT_LENGTH];
	char list[MAX_INPUT_LENGTH];

	push_call("is_name_list(%p,%p)",str,namelist);

	if (*str == '\0' || *(str+1) == '\0')
	{
		pop_call();
		return FALSE;
	}

	for ( ; ; )
	{
		str		= one_argument(str, name);
		namelist	= one_argument(namelist, list);

		if (name[0] == '\0')
		{
			break;
		}

		if (str_prefix(name, list))
		{
			pop_call();
			return FALSE;
		}
	}
	pop_call();
	return TRUE;
}

/*
	Apply or remove an affect to a character.
*/

void affect_modify( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd )
{
	int mod;

	push_call("affect_modify(%p,%p,%p)",ch,paf,fAdd);

	if (fAdd)
	{
		if (paf->bitvector)
		{
			switch (paf->bittype)
			{
				case AFFECT_TO_NONE:
					paf->bittype = AFFECT_TO_CHAR;
					log_printf("affect_modify: bad bittype");
					dump_stack();
				case AFFECT_TO_CHAR:
					if (paf->bitvector < 0)
					{
						SET_BIT(ch->affected2_by, 0 - paf->bitvector);
					}
					else
					{
						SET_BIT(ch->affected_by, paf->bitvector);
					}
					break;
				case AFFECT_TO_OBJ:
					break;
				default:
					log_printf("affect_modify: unknown bittype %d", paf->bittype);
					dump_stack();
					break;
			}
		}
		mod = paf->modifier;
	}
	else
	{
		if (paf->bitvector)
		{
			switch (paf->bittype)
			{
				case AFFECT_TO_NONE:
					log_printf("affect_modify: bad bittype");
					paf->bittype = AFFECT_TO_CHAR;
					dump_stack();
				case AFFECT_TO_CHAR:
					if (paf->bitvector < 0)
					{
						REMOVE_BIT(ch->affected2_by, 0 - paf->bitvector);
					}
					else
					{
						REMOVE_BIT(ch->affected_by, paf->bitvector);
					}
					break;
				case AFFECT_TO_OBJ:
					break;
				default:
					log_printf("affect_modify: unknown bittype", paf->bittype);
					dump_stack();
					break;
			}
		}
		mod = 0 - paf->modifier;
	}

	if (IS_SET(paf->bitvector, AFF_HASTE) && paf->bitvector > 0)
	{
		ch->speed = get_max_speed(ch);
	}

	switch (paf->location)
	{
		default:
			bug( "affect_modify: unknown location %d.", paf->location );
			pop_call();
			return;

		case APPLY_NONE:
			break;

		case APPLY_STR:
			if (!IS_NPC(ch))
			{
				ch->pcdata->mod_str       += mod;
			}
			break;
		case APPLY_DEX:
			if (!IS_NPC(ch))
			{
				ch->pcdata->mod_dex       += mod;
			}
			break;
		case APPLY_INT:
			if (!IS_NPC(ch))
			{
				ch->pcdata->mod_int       += mod;
			}
			break;
		case APPLY_WIS:
			if (!IS_NPC(ch))
			{
				ch->pcdata->mod_wis       += mod;
			}
			break;
		case APPLY_CON:
			if (!IS_NPC(ch))
			{
				ch->pcdata->mod_con       += mod;
			}
			break;
		case APPLY_SEX:
			ch->sex				+= mod;
			break;
		case APPLY_RACE:
			ch->race				+= mod;
			break;
		case APPLY_AGE:
			break;
		case APPLY_MANA:
			ch->max_mana	= UMAX(1, ch->max_mana + mod);
			break;
		case APPLY_HIT:
			ch->max_hit	= UMAX(1, ch->max_hit + mod);
			break;
		case APPLY_MOVE:
	    		ch->max_move	= UMAX(1, ch->max_move + mod);
			break;
		case APPLY_AC:
			if (!IS_NPC(ch))
			{
				ch->pcdata->armor		+= mod;
			}
			break;
		case APPLY_HITROLL:
			ch->hitroll				+= mod;
			if (!IS_NPC(ch) && IS_SET(mud->flags, MUD_EQAFFECTING))
			{
				ch->pcdata->eqhitroll	+= mod;
			}
			break;
		case APPLY_DAMROLL:
			ch->damroll			+= mod;

			if (!IS_NPC(ch) && IS_SET(mud->flags, MUD_EQAFFECTING))
			{
				ch->pcdata->eqdamroll	+= mod;
			}
			break;
		case APPLY_SAVING_PARA:
		case APPLY_SAVING_ROD:
		case APPLY_SAVING_PETRI:
		case APPLY_SAVING_BREATH:
		case APPLY_SAVING_SPELL:
			ch->saving_throw			+= mod;
			if (IS_SET(mud->flags, MUD_EQAFFECTING) && !IS_NPC(ch))
			{
				ch->pcdata->eqsaves		+= mod;
			}
			break;
	}

	pop_call();
	return;
}


void affect_obj_modify( OBJ_DATA *obj, AFFECT_DATA *paf, bool fAdd )
{
//	int mod;

	push_call("affect_obj_modify(%p,%p,%p)",obj,paf,fAdd);

	if (fAdd)
	{
		if (paf->bitvector)
		{
			switch (paf->bittype)
			{
				case AFFECT_TO_NONE:
					log_printf("affect_obj_modify: bad bittype");
					dump_stack();
					break;
				case AFFECT_TO_CHAR:
					break;
				case AFFECT_TO_OBJ:
					SET_BIT(obj->extra_flags, paf->bitvector);
					break;
				default:
					log_printf("affect_modify: unknown bittype %d", paf->bittype);
					dump_stack();
					break;
			}
		}
//		mod = paf->modifier;
	}
	else
	{
		if (paf->bitvector)
		{
			switch (paf->bittype)
			{
				case AFFECT_TO_NONE:
					log_printf("affect_modify: bad bittype");
					dump_stack();
					break;
				case AFFECT_TO_CHAR:
					break;
				case AFFECT_TO_OBJ:
					REMOVE_BIT(obj->extra_flags, paf->bitvector);
					break;
				default:
					log_printf("affect_modify: unknown bittype", paf->bittype);
					dump_stack();
					break;
			}
		}
//		mod = 0 - paf->modifier;
	}

	switch (paf->location)
	{
		case APPLY_NONE:
			break;
	}

	pop_call();
	return;
}
/*
	Give an affect to a char.
*/


void affect_to_char( CHAR_DATA *ch, AFFECT_DATA *paf )
{
	AFFECT_DATA *paf_new;

	push_call("affect_to_char(%p,%p)",ch,paf);

	ALLOCMEM(paf_new, AFFECT_DATA, 1);

	paf_new->type       = paf->type;
	paf_new->duration   = paf->duration;
	paf_new->location   = paf->location;
	paf_new->modifier   = paf->modifier;
	paf_new->bittype	= paf->bittype;
	paf_new->bitvector  = paf->bitvector;

	LINK( paf_new, ch->first_affect, ch->last_affect, next, prev );

	affect_modify(ch, paf_new, TRUE);

	pop_call();
	return;
}


void affect_to_obj( OBJ_DATA *obj, AFFECT_DATA *paf )
{
	AFFECT_DATA *paf_new;

	push_call("affect_to_obj(%p,%p)",obj,paf);

	ALLOCMEM(paf_new, AFFECT_DATA, 1);

	paf_new->type       = paf->type;
	paf_new->duration   = paf->duration;
	paf_new->location   = paf->location;
	paf_new->modifier   = paf->modifier;
	paf_new->bittype	= paf->bittype;
	paf_new->bitvector  = paf->bitvector;

	LINK( paf_new, obj->first_affect, obj->last_affect, next, prev );

	affect_obj_modify(obj, paf_new, TRUE);

	if (obj->wear_loc != WEAR_NONE && obj->carried_by)
	{
		SET_BIT(mud->flags, MUD_EQAFFECTING);

		affect_modify(obj->carried_by, paf, TRUE);

		REMOVE_BIT(mud->flags, MUD_EQAFFECTING);
	}
	pop_call();
	return;
}


void affect_from_obj( OBJ_DATA *obj, AFFECT_DATA *paf )
{
	push_call("affect_from_obj(%p,%p)",obj,paf);

	if (obj->wear_loc != WEAR_NONE && obj->carried_by)
	{
		SET_BIT(mud->flags, MUD_EQAFFECTING);

		affect_modify(obj->carried_by, paf, FALSE);

		REMOVE_BIT(mud->flags, MUD_EQAFFECTING);
	}

	affect_obj_modify(obj, paf, FALSE);

	UNLINK(paf, obj->first_affect, obj->last_affect, next, prev);
	FREEMEM(paf);

	pop_call();
	return;
}

/*
	Remove an affect from a char.
*/

void affect_from_char( CHAR_DATA *ch, AFFECT_DATA *paf )
{
	push_call("affect_from_char(%p,%p)",ch,paf);

	if ((paf->prev == NULL && paf != ch->first_affect)
	||  (paf->next == NULL && paf != ch->last_affect))
	{
		log_printf("UNLINK ERROR affect_from_char: affect not found on %s", ch->name);
		dump_stack();
		pop_call();
		return;
	}

	affect_modify(ch, paf, FALSE);

	UNLINK(paf, ch->first_affect, ch->last_affect, next, prev);

	FREEMEM( paf );

	pop_call();
	return;
}


/*
	Strip all affects of a given sn.
*/

void affect_strip( CHAR_DATA *ch, int sn )
{
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	push_call("affect_strip(%p,%p)",ch,sn);

	for (paf = ch->first_affect ; paf ; paf = paf_next)
	{
		paf_next = paf->next;

		if (paf->type == sn)
		{
			affect_from_char( ch, paf );
		}
	}
	pop_call();
	return;
}

/*
	Return true if a char is first_affect by a spell.
*/

bool is_affected( CHAR_DATA *ch, int sn )
{
	AFFECT_DATA *paf;

	push_call("is_affected(%p,%p)",ch,sn);

	if (ch->first_affect)
	{
		for (paf = ch->first_affect ; paf ; paf = paf->next)
		{
			if (paf->type == sn)
			{
				pop_call();
				return TRUE;
			}
		}
	}
	pop_call();
	return FALSE;
}


/*
	Add or enhance an affect.
*/

void affect_join( CHAR_DATA *ch, AFFECT_DATA *paf )
{
	AFFECT_DATA *paf_old;

	push_call("affect_join(%p,%p)",ch,paf);

	for (paf_old = ch->first_affect ; paf_old ; paf_old = paf_old->next)
	{
		if (paf_old->type == paf->type && paf_old->location == paf->location)
		{
			affect_modify(ch, paf_old, FALSE);

			if (paf->duration == -1 || paf_old->duration == -1)
			{
				paf_old->duration = -1;
			}
			else
			{
				paf_old->duration += paf->duration;
			}
			paf_old->modifier += paf->modifier;

			affect_modify(ch, paf_old, TRUE);

			pop_call();
			return;
		}
	}
	affect_to_char( ch, paf );
	pop_call();
	return;
}

/*
	Move a char out of a room.
*/

void char_from_room( CHAR_DATA *ch )
{
	OBJ_DATA *obj;

	push_call("char_from_room(%p)",ch);

	if (ch->in_room == NULL)
	{
		log_printf( "char_from_room: ch->in_room == NULL");
		dump_stack();
		pop_call();
		return;
	}

	/*
		Takes care of fingered and half reconnected chars - Scandum
	*/

	if ((ch->prev_in_room == NULL && ch != ch->in_room->first_person)
	||  (ch->next_in_room == NULL && ch != ch->in_room->last_person))
	{
		pop_call();
		return;
	}

	if (ch == ch->in_room->sanctify_char)
	{
		REMOVE_BIT(ch->in_room->room_flags, ROOM_SAFE);
		del_room_timer(ch->in_room->vnum, ROOM_TIMER_SANCTIFY);
		ch->in_room->sanctify_char = NULL;
		act("The area does not look safe now.", ch, NULL, NULL, TO_CHAR);
		act("The area does not look safe now.", ch, NULL, NULL, TO_ROOM);
	}

	if (mud->update_rch == ch)
	{
		mud->update_rch = ch->next_in_room;
	}

	if (IS_AFFECTED(ch, AFF2_CAMPING))
	{
		REMOVE_BIT(ch->affected2_by, 0-AFF2_CAMPING);
	}

	if (ch->furniture)
	{
		ch->furniture = NULL;
	}

	if (ch->mounting)
	{
		ch->mounting = NULL;
	}

	if (!IS_NPC(ch))
	{
		--ch->in_room->area->nplayer;
	}

	if ((obj = get_eq_char(ch, WEAR_LIGHT)) != NULL && obj->item_type == ITEM_LIGHT)
	{
		if (obj->value[0] == 0)
		{
			ch->in_room->light -= 100;
		}
		else
		{
			ch->in_room->light -= obj->value[0];
		}
	}

	UNLINK(ch, ch->in_room->first_person, ch->in_room->last_person, next_in_room, prev_in_room);

	ch->in_room = NULL;

	pop_call();
	return;
}

/*
	Move a char into a room.
*/

void char_to_room( CHAR_DATA *ch, int vnum )
{
	ROOM_INDEX_DATA *pRoomIndex;
	OBJ_DATA *obj;

	push_call("char_to_room(%p,%p)",ch,vnum);

	if ((pRoomIndex = get_room_index(vnum)) == NULL)
	{
		log_printf("char_to_room: room_index %d == NULL", vnum);
		dump_stack();
		pRoomIndex = room_index[ROOM_VNUM_TEMPLE];
	}

	if (ch->in_room)
	{
		char_from_room(ch);
	}

	ch->in_room = pRoomIndex;

	LINK(ch, pRoomIndex->first_person, pRoomIndex->last_person, next_in_room, prev_in_room);

	if (!IS_NPC(ch))
	{
		++ch->in_room->area->nplayer;
	}

	if ((obj = get_eq_char(ch, WEAR_LIGHT)) != NULL && obj->item_type == ITEM_LIGHT)
	{
		if (obj->value[0] == 0)
		{
			ch->in_room->light += 100;
		}
		else
		{
			ch->in_room->light += obj->value[0];
		}
	}

	if (ch->desc)
	{
		if (HAS_BIT(CH(ch->desc)->pcdata->vt100_flags, VT100_INTERFACE))
		{
			vt100prompt(ch);
		}

		if (ch->desc->mth->msdp_data)
		{
			char exits[MAX_INPUT_LENGTH];
			EXIT_DATA *pexit;
			int exit;

			// Update the ROOM variable for automappers

			sprintf(exits, "%c", MSDP_TABLE_OPEN);

			for (exit = 0 ; exit < 6 ; exit++)
			{
				if (is_valid_exit(ch, ch->in_room, exit))
				{
					pexit = ch->in_room->exit[exit];

					if (can_use_exit(ch, pexit) && !HAS_BIT(room_index[pexit->to_room]->room_flags, ROOM_RIP))
					{
						cat_sprintf(exits, "\001%s\002%d", dir_name_short[exit], ch->in_room->exit[exit]->to_room);
					}
				}
			}
			cat_sprintf(exits, "%c", MSDP_TABLE_CLOSE);

			msdp_update_var_instant(ch->desc, "ROOM", "%c\001%s\002%d\001%s\002%s\001%s\002%s\001%s\002%s\001%s\002%s%c",
				MSDP_TABLE_OPEN,
					"VNUM", ch->in_room->vnum,
					"NAME", ch->in_room->name,
					"AREA", ch->in_room->area->name,
					"TERRAIN", sector_table[ch->in_room->sector_type].sector_name,
					"EXITS", exits,
				MSDP_TABLE_CLOSE);

			// Update the ROOM_EXITS variable for tactical interfaces

			sprintf(exits, "%c", MSDP_TABLE_OPEN);

			for (exit = 0 ; exit < 6 ; exit++)
			{
				if (is_valid_exit(ch, ch->in_room, exit))
				{
					pexit = ch->in_room->exit[exit];

					if (can_use_exit(ch, pexit) && !HAS_BIT(room_index[pexit->to_room]->room_flags, ROOM_RIP))
					{
						if (IS_SET(pexit->flags, EX_CLOSED))
						{
							cat_sprintf(exits, "\001%s\002%s", dir_name_short[exit], "CLOSED");
						}
						else
						{
							cat_sprintf(exits, "\001%s\002%s", dir_name_short[exit], "OPEN");
						}
					}
				}
			}
			cat_sprintf(exits, "%c", MSDP_TABLE_CLOSE);

			msdp_update_var(ch->desc, "ROOM_EXITS", exits);
		}
	}

	pop_call();
	return;
}

/*
	Give an obj to a char.
*/

void obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch )
{
	OBJ_DATA *otmp;

	push_call("obj_to_char(%p,%p)",obj,ch);

	if (obj->in_room != NULL)
	{
		obj_from_room(obj);
	}

	if (obj->in_obj != NULL)
	{
		obj_from_obj(obj);
	}

	if (obj->carried_by != NULL)
	{
		obj_from_char( obj );
	}

	if (!IS_NPC(ch) || ch->pIndexData->pShop == NULL || ch->first_carrying == NULL)
	{
		LINK(obj, ch->first_carrying, ch->last_carrying, next_content, prev_content);
	}
	else	/* Sort by level for shop keepers */
	{
		for (otmp = ch->last_carrying ; otmp ; otmp = otmp->prev_content)
		{
			if (obj->level >= otmp->level)
			{
				INSERT_RIGHT(obj, otmp, ch->last_carrying, next_content, prev_content);
				break;
			}
		}
		if (otmp == NULL)
		{
			INSERT_LEFT(obj, ch->first_carrying, ch->first_carrying, next_content, prev_content);
		}
	}
	obj->carried_by	= ch;
	obj->sac_timer		= 0;
	ch->carry_weight	+= get_obj_weight(obj);
	ch->carry_number++;

	if (!IS_NPC(ch))
	{
		obj_set_to_char( ch, obj );
	}
	pop_call();
	return;
}

void obj_aff_to_obj(OBJ_DATA *obj, int type, int value)
{
	AFFECT_DATA *paf;

	ALLOCMEM(paf, AFFECT_DATA, 1);
	paf->type       = gsn_object_rape;
	paf->duration   = -1;
	paf->modifier   = value;
	paf->bitvector  = 0;
	paf->location   = type;

	LINK(paf, obj->first_affect, obj->last_affect, next, prev);
}

void obj_set_to_char( CHAR_DATA *ch, OBJ_DATA *obj )
{
	push_call("obj_set_to_char(%p,%p)",ch,obj);

	/*
		Auto engrave stuff and randomization
	*/

	if (!HAS_BIT(obj->extra_flags, ITEM_MODIFIED) && ch->level < MAX_LEVEL)
	{
		if (HAS_BIT(obj->pIndexData->extra_flags, ITEM_MYTHICAL) || HAS_BIT(obj->pIndexData->extra_flags, ITEM_EPICAL))
		{
			AFFECT_DATA *paf;
			int dizzy;

			SET_BIT(obj->extra_flags, ITEM_MODIFIED);

			number_seed(obj->pIndexData->vnum + ch->pcdata->pvnum + ch->pcdata->reincarnation * MAX_LEVEL + ch->level);

			obj->weight = number_fuzzy(obj->weight);
			obj->level  = number_fuzzy(obj->level);

			switch ( obj->item_type )
			{
				case ITEM_WEAPON:
					if (obj->value[1] <= obj->value[2])
					{
						obj->value[2] = number_fuzzy(obj->value[2]);
					}
					else
					{
						obj->value[1] = number_fuzzy(obj->value[1]);
					}
					break;
				case ITEM_ARMOR:
					obj->value[0] = number_fuzzy(obj->value[0]);
					break;
				case ITEM_CONTAINER:
					obj->value[0] = number_fuzzy(obj->value[0]);
					obj->value[3] = obj->value[3] ? number_fuzzy(obj->value[3]) : number_fuzzy(MAX_OBJECTS_IN_CONTAINER);
					break;
				case ITEM_LIGHT:
					obj->value[0] = obj->value[0] ? number_fuzzy(obj->value[0]) : number_fuzzy(100);
					break;

				case ITEM_LOCKPICK:
					obj->value[0] = number_fuzzy(obj->value[0]);
					obj->value[1] = number_fuzzy(obj->value[1]);
					break;
				default:
					break;
			}

			for (paf = obj->pIndexData->first_affect ; paf ; paf = paf->next)
			{
				switch (paf->location)
				{
					case APPLY_NONE:
					case APPLY_SEX:
					case APPLY_RACE:
						break;

					default:
						dizzy = number_dizzy(paf->modifier);

						if (dizzy)
						{
							log_printf("obj_rape (%d): loc: %d mod: %d dizzy mod: %d", obj->pIndexData->vnum, paf->location, paf->modifier, dizzy);
							obj_aff_to_obj(obj, paf->location, dizzy);
						}
						break;
				}
			}
		}
	}

	if (IS_SET(obj->pIndexData->extra_flags, ITEM_AUTO_ENGRAVE) && !obj->owned_by && ch->level < MAX_LEVEL)
	{
		obj->owned_by = ch->pcdata->pvnum;

		SET_BIT(obj->extra_flags, ITEM_MODIFIED);
	}
	
	pop_call();
	return;
}

/*
	Take an obj from its character.
*/

void obj_from_char( OBJ_DATA *obj )
{
	CHAR_DATA *ch;

	push_call("obj_from_char(%p)",obj);

	if (obj->in_obj)
	{
		log_printf("obj_from_char: obj->in_obj");
		obj_from_obj(obj);
		pop_call();
		return;
	}

	if (obj->in_room)
	{
		log_printf("obj_from_char: obj->in_room");
		obj_from_room( obj );
		pop_call();
		return;
	}

	if ((ch = obj->carried_by) == NULL)
	{
		log_printf("obj_from_char: obj->carried_by == NULL");
		pop_call();
		return;
	}

	if (obj->wear_loc != WEAR_NONE)
	{
		unequip_char( ch, obj );
	}

	if ((obj->prev_content == NULL && obj != ch->first_carrying)
	||  (obj->next_content == NULL && obj != ch->last_carrying))
	{
		log_printf("UNLINK ERROR object %s not carried by %s", obj->name, ch->name);
		dump_stack();
	}
	else
	{
		UNLINK( obj, ch->first_carrying, ch->last_carrying, next_content, prev_content );
	}

	ch->carry_number--;
	ch->carry_weight -= get_obj_weight( obj );

	if (obj->reset)
	{
		if (obj->first_content)
		{
			OBJ_DATA *c_obj;

			for (c_obj = obj->first_content ; c_obj ; c_obj = c_obj->next_content)
			{
				if (c_obj->reset != NULL)
				{
					c_obj->reset->obj = NULL;
					c_obj->reset = NULL;
				}
			}
		}

		if (obj->reset->obj != obj)
		{
			log_printf("obj_from_char: obj->reset->obj != obj");
		}
		else
		{
			obj->reset->obj = NULL;
			obj->reset = NULL;
		}
	}

	obj->carried_by = NULL;

	pop_call();
	return;
}



/*
	Find the ac value of an obj, including position effect.
*/


int apply_ac( OBJ_DATA *obj, int iWear )
{
	push_call("apply_ac(%p,%p)",obj,iWear);

	if (obj->item_type != ITEM_ARMOR)
	{
		pop_call();
		return 0;
	}

	if (iWear == WEAR_BODY && multi(obj->carried_by, gsn_armor_usage) != -1)
	{
		pop_call();
		return 3.0 * (3 + learned(obj->carried_by, gsn_armor_usage)) / 100 * obj->value[0];
	}
	pop_call();
	return class_eq_table[obj->carried_by->class][iWear] * obj->value[0];
}

/*
	Find a piece of eq on a character.
*/

OBJ_DATA *get_eq_char( CHAR_DATA *ch, int iWear )
{
	OBJ_DATA *obj;

	push_call("get_eq_char(%p,%p)",ch,iWear);

	for (obj = ch->first_carrying ; obj ; obj = obj->next_content)
	{
		if (obj->wear_loc == iWear)
		{
			pop_call();
			return obj;
		}
	}
	pop_call();
	return NULL;
}

int count_users( OBJ_DATA * obj )
{
	CHAR_DATA *fch;
	int count = 0;

	push_call("count_users(%p)",obj);

	if (obj->in_room == NULL)
	{
		log_printf("count_users: obj->in_room == NULL");
		pop_call();
		return -1;
	}

	for (fch = obj->in_room->first_person ; fch ; fch = fch->next_in_room)
	{
		if (fch->furniture == obj)
		{
			count++;
		}
	}
	pop_call();
	return count;
}


void check_zap( CHAR_DATA *ch, bool fDisplay )
{
	OBJ_DATA *obj;
	int alignment;

	push_call("check_zap(%p,%p)",ch,fDisplay);

	if (IS_NEUTRAL(ch))
	{
		alignment = ITEM_ANTI_NEUTRAL;
	}
	else if (IS_GOOD(ch))
	{
		alignment = ITEM_ANTI_GOOD;
	}
	else
	{
		alignment = ITEM_ANTI_EVIL;
	}

	for (obj = ch->first_carrying ; obj ; obj = obj->next_content)
	{
		if (obj->wear_loc != WEAR_NONE && IS_SET(obj->extra_flags, alignment))
		{
			if (fDisplay)
			{
				act("You are zapped by $p.", ch, obj, NULL, TO_CHAR);
				act("$n is zapped by $p.",   ch, obj, NULL, TO_ROOM);
			}
			unequip_char(ch, obj);
		}
	}
	pop_call();
	return;
}

int objref(char *argument, OBJ_DATA *obj)
{
	return atoll(argument) == obj->obj_ref_key;
}

/*
	Equip a char with an obj.
*/

void equip_char( CHAR_DATA *ch, OBJ_DATA *obj, int iWear )
{
	AFFECT_DATA *paf;

	push_call("equip_char(%p,%p,%p)",ch,obj,iWear);

	if (obj->wear_loc != WEAR_NONE)
	{
		log_printf("equip_char: obj->wear_loc != WEAR_NONE");
		dump_stack();
	}

	if (iWear < 0 || iWear >= MAX_WEAR)
	{
		log_printf("Equip: obj: %d iWear: %d", obj->pIndexData->vnum, iWear);
		pop_call();
		return;
	}

	if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch))
	||  (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch))
	||  (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch)))
	{
		act( "You are zapped by $p.", ch, obj, NULL, TO_CHAR );
		act( "$n is zapped by $p.",  ch, obj, NULL, TO_ROOM );
		pop_call();
		return;
	}

	if (get_eq_char(ch, iWear) != NULL)
	{
		log_printf("Equip_char: already equipped (vnum %u).", IS_NPC(ch) ? ch->pIndexData->vnum : 0 );
		pop_call();
		return;
	}

	if (!IS_NPC(ch))
	{
		ch->pcdata->armor -= apply_ac( obj, iWear );
	}

	obj->wear_loc       = iWear;

	SET_BIT(mud->flags, MUD_EQAFFECTING);

	for (paf = obj->pIndexData->first_affect ; paf ; paf = paf->next)
	{
		affect_modify(ch, paf, TRUE);
	}

	for (paf = obj->first_affect ; paf ; paf = paf->next)
	{
		affect_modify(ch, paf, TRUE);
	}

	REMOVE_BIT(mud->flags, MUD_EQAFFECTING);

	if (obj->item_type == ITEM_LIGHT && ch->in_room && obj->wear_loc == WEAR_LIGHT)
	{
		if (obj->value[0] == 0)
		{
			ch->in_room->light += 100;
		}
		else
		{
			ch->in_room->light += obj->value[0];
		}
	}

	pop_call();
	return;
}

/*
	Unequip a char with an obj.
*/

void unequip_char( CHAR_DATA *ch, OBJ_DATA *obj )
{
	AFFECT_DATA *paf;

	push_call("unequip_char(%p,%p)",ch,obj);

	if (obj->wear_loc == WEAR_NONE)
	{
		log_printf("unequip_char: obj->wear_loc == WEAR_NONE");
		dump_stack();
		pop_call();
		return;
	}

	if (!IS_NPC(ch))
	{
		ch->pcdata->armor += apply_ac( obj, obj->wear_loc );
	}

	if (ch->in_room && obj->item_type == ITEM_LIGHT && obj->wear_loc == WEAR_LIGHT)
	{
		if (IS_NPC(ch) || get_char_pvnum(ch->pcdata->pvnum) != NULL)
		{
			if (obj->value[0] == 0)
			{
				ch->in_room->light -= 100;
			}
			else
			{
				ch->in_room->light -= obj->value[0];
			}
		}
	}

	obj->wear_loc              = WEAR_NONE;

	SET_BIT(mud->flags, MUD_EQAFFECTING);

	for (paf = obj->pIndexData->first_affect ; paf ; paf = paf->next)
	{
		affect_modify(ch, paf, FALSE);
	}

	for (paf = obj->first_affect ; paf ; paf = paf->next )
	{
		affect_modify(ch, paf, FALSE);
	}

	REMOVE_BIT(mud->flags, MUD_EQAFFECTING);

	pop_call();
	return;
}


/*
	Move an obj out of a room.
*/

void obj_from_room( OBJ_DATA *obj )
{
	push_call("obj_from_room(%p)",obj);

	if (obj->in_room == NULL)
	{
		log_printf("obj_from_room: obj->in_room == NULL");
		pop_call();
		return;
	}

	if ((obj->prev_content == NULL && obj != obj->in_room->first_content)
	||  (obj->next_content == NULL && obj != obj->in_room->last_content))
	{
		log_printf("UNLINK ERROR obj_from_room: object %s not found.", obj->name);
		dump_stack();
	}
	else
	{
		UNLINK(obj, obj->in_room->first_content, obj->in_room->last_content, next_content, prev_content);
	}


	if (obj->reset != NULL)
	{
		if (obj->first_content != NULL)
		{
			OBJ_DATA *c_obj;

			for (c_obj = obj->first_content ; c_obj ; c_obj = c_obj->next_content)
			{
				if (c_obj->reset != NULL)
				{
					c_obj->reset->obj = NULL;
					c_obj->reset = NULL;
				}
			}
		}

		if (obj->reset->obj != obj)
		{
			log_printf("obj_from_room: obj->reset->obj != obj");
		}
		else
		{
			obj->reset->obj = NULL;
			obj->reset = NULL;
		}
	}
	obj->in_room->content_count--;

	obj->in_room = NULL;

	pop_call();
	return;
}

/*
	Move an obj into a room.
*/

void obj_to_room( OBJ_DATA *obj, int vnum)
{
	ROOM_INDEX_DATA *pRoomIndex;

	push_call("obj_to_room(%p,%p)",obj, vnum);

	if (obj->in_room)
	{
		obj_from_room(obj);
	}

	if (obj->in_obj)
	{
		obj_from_obj(obj);
	}

	if (obj->carried_by)
	{
		obj_from_char(obj);
	}

	if ((pRoomIndex = get_room_index(vnum)) == NULL)
	{
		log_printf("obj_to_room: index %d == NULL", vnum);
		pop_call();
		return;
	}

	if (pRoomIndex != room_index[ROOM_VNUM_JUNK])
	{
		if (pRoomIndex->content_count >= MAX_OBJECTS_IN_ROOM)
		{
			junk_obj(obj);
			pop_call();
			return;
		}
	}
	LINK(obj, pRoomIndex->first_content, pRoomIndex->last_content, next_content, prev_content);

	obj->in_room = pRoomIndex;

	pRoomIndex->content_count++;

	pop_call();
	return;
}

/*
	Move an object into an object.
*/

void obj_to_obj( OBJ_DATA *obj, OBJ_DATA *obj_to )
{
	push_call("obj_to_obj(%p,%p)",obj,obj_to);

	if (obj->in_room)
	{
		obj_from_room(obj);
	}

	if (obj->in_obj)
	{
		obj_from_obj(obj);
	}

	if (obj->carried_by)
	{
		obj_from_char(obj);
	}

	if (obj_to->carried_by != NULL)
	{
		obj_to->carried_by->carry_weight += get_obj_weight(obj);
	}

	LINK(obj, obj_to->first_content, obj_to->last_content, next_content, prev_content);

	obj_to->content_weight += get_obj_weight(obj);

	obj->in_obj = obj_to;

	pop_call();
	return;
}

/*
	Move an object out of an object.
*/

void obj_from_obj( OBJ_DATA *obj )
{
	push_call("obj_from_obj(%p)",obj);

	if (obj->in_obj == NULL)
	{
		log_printf("obj_from_obj: obj->in_obj == null");
		dump_stack();
		pop_call();
		return;
	}

	if ((obj->prev_content == NULL && obj != obj->in_obj->first_content)
	||  (obj->next_content == NULL && obj != obj->in_obj->last_content))
	{
		log_printf("UNLINK ERROR obj_from_obj: object %s not found", obj->name);
		dump_stack();
	}
	else
	{
		UNLINK(obj, obj->in_obj->first_content, obj->in_obj->last_content, next_content, prev_content);
	}

	obj->in_obj->content_weight -= get_obj_weight(obj);

	if (obj->in_obj->carried_by)
	{
		obj->in_obj->carried_by->carry_weight -= get_obj_weight(obj);
	}

	obj->in_obj       = NULL;

	if (obj->reset != NULL)
	{
		if (obj->reset->obj != obj)
		{
			log_printf("obj_from_obj: obj->reset->obj != obj");
		}
		else
		{
			obj->reset->obj = NULL;
			obj->reset = NULL;
		}
	}
	pop_call();
	return;
}


/*
	Extract an obj from the world.
*/

void extract_obj( OBJ_DATA *obj )
{
	CHAR_DATA *rch;

	push_call("extract_obj(%p)",obj);

	if (obj->item_type == ITEM_FURNITURE && obj->in_room)
	{
		for (rch = obj->in_room->first_person ; rch ; rch = rch->next_in_room)
		{
			if (rch->furniture == obj)
			{
				rch->furniture = NULL;
			}
		}
	}

	if (obj->in_room)
	{
		obj_from_room(obj);
	}

	if (obj->in_obj)
	{
		obj_from_obj(obj);
	}

	if (obj->carried_by)
	{
		obj_from_char(obj);
	}

	if (obj->next_content || obj->prev_content)
	{
		log_printf("extract_obj: object has next or prev content");
	}

	while (obj->first_content)
	{
		extract_obj(obj->first_content);
	}

	if (mud->update_obj == obj)
	{
		mud->update_obj = obj->next;
	}

	rem_obj_ref_hash(obj);

	if ((obj->prev == NULL && obj != mud->f_obj)
	||  (obj->next == NULL && obj != mud->l_obj))
	{
		log_printf("UNLINK ERROR object %s not in mud object list.", obj->name);
		dump_stack();
	}
	else
	{
		UNLINK(obj, mud->f_obj, mud->l_obj, next, prev);
		UNLINK(obj, obj->pIndexData->first_instance, obj->pIndexData->last_instance, next_instance, prev_instance);
	}

	{
		AFFECT_DATA *paf;

		while ((paf = obj->first_affect) != NULL)
		{
			UNLINK(paf, obj->first_affect, obj->last_affect, next, prev);
			FREEMEM(paf);
		}
	}

	if (obj->reset != NULL)
	{
		obj->reset->obj = NULL;
		obj->reset      = NULL;
	}

	if (obj->poison != NULL)
	{
		FREEMEM( obj->poison );
		obj->poison = NULL;
	}

	STRFREE( obj->name        );
	STRFREE( obj->description );
	STRFREE( obj->short_descr );
	STRFREE( obj->long_descr );

	if (obj->obj_quest != NULL)
	{
		FREEMEM( obj->obj_quest );
	}

	if (obj->pIndexData != NULL)
	{
		--mud->total_obj;
		--obj->pIndexData->total_objects;
	}

	FREEMEM( obj );

	pop_call();
	return;
}

void junk_obj( OBJ_DATA *obj )
{
	JUNK_DATA *junk;

	push_call("junk_obj(%p)",obj);

	if (obj->in_room && obj->in_room->vnum == ROOM_VNUM_JUNK && IS_SET(obj->extra_flags, ITEM_NOT_VALID))
	{
		pop_call();
		return;
	}
	SET_BIT(obj->extra_flags, ITEM_NOT_VALID);
	obj_to_room(obj, ROOM_VNUM_JUNK);

	ALLOCMEM(junk, JUNK_DATA, 1);
	junk->obj = obj;
	LINK(junk, mud->f_junk, mud->l_junk, next, prev);

	pop_call();
	return;
}


void junk_mob( CHAR_DATA *ch )
{
	JUNK_DATA *junk;

	push_call("junk_mob(%p)",ch);

	if (!IS_NPC(ch))
	{
		log_printf("junk_mob: trying to junk PC");
		dump_stack();

		pop_call();
		return;
	}

	if (IS_SET(ch->act, ACT_WILL_DIE) && ch->in_room->vnum == ROOM_VNUM_JUNK)
	{
		pop_call();
		return;
	}

	SET_BIT(ch->act, ACT_WILL_DIE);

	char_to_room(ch, ROOM_VNUM_JUNK);

	if (ch->fighting)
	{
		stop_fighting(ch, TRUE);
	}

	ALLOCMEM(junk, JUNK_DATA, 1);
	junk->mob = ch;
	LINK(junk, mud->f_junk, mud->l_junk, next, prev);

	pop_call();
	return;
}

/*
	Extract a char from the world.
*/

void extract_char( CHAR_DATA *ch )
{
	PLAYER_GAME *gpl;

	push_call("extract_char(%p)",ch);

	if (ch == NULL || ch->name == NULL)
	{
		log_printf("found nullified data in extract_char.");
		dump_stack();
		pop_call();
		return;
	}

	if (ch->desc && ch->desc->original)
	{
		do_return(ch, NULL);
	}

	die_follower(ch);

	if (!IS_NPC(ch))
	{
		if (ch->pcdata->shadowing)
		{
			stop_shadow(ch);
		}
		if (ch->pcdata->shadowed_by)
		{
			stop_shadow(ch->pcdata->shadowed_by);
		}
		check_bad_desc(ch);
	}
	stop_fighting(ch, TRUE);

	if (mud->update_rch == ch)
	{
		mud->update_rch = ch->next_in_room;
	}

	if (mud->update_wch == ch)
	{
		mud->update_wch = ch->next;
	}

	if (mud->update_ich == ch)
	{
		mud->update_ich = ch->next_instance;
	}

	if (ch->in_room)
	{
		char_from_room( ch );
	}

	if (IS_NPC(ch))
	{
		--ch->pIndexData->total_mobiles;
		--mud->total_mob;

		if (ch->reset != NULL && ch->reset->mob == ch)
		{
			ch->reset->mob = NULL;
		}
	}

	if (!IS_NPC(ch) && is_desc_valid(ch))
	{
		for (gpl = mud->f_player ; gpl ; gpl = gpl->next)
		{
			if (is_desc_valid(gpl->ch) && gpl->ch->desc->snoop_by == ch->desc)
			{
				gpl->ch->desc->snoop_by = NULL;
			}
		}
	}

	if (!IS_NPC(ch) && ch->pcdata->corpse)
	{
		extract_obj(ch->pcdata->corpse);
	}

	if ((ch->prev == NULL && ch != mud->f_char)
	||  (ch->next == NULL && ch != mud->l_char))
	{
		if (IS_NPC(ch))
		{
			log_printf( "UNLINK ERROR character %s not found in char_list.", ch->name);
			dump_stack();
		}
	}
	else
	{
		UNLINK(ch, mud->f_char, mud->l_char, next, prev);
	}

	if (!IS_NPC(ch))
	{
		sub_player(ch);
	}
	else
	{
		UNLINK(ch, ch->pIndexData->first_instance, ch->pIndexData->last_instance, next_instance, prev_instance);
	}
	
	free_char( ch );

	pop_call();
	return;
}


CHAR_DATA *get_char_room( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *rch;
	int number;
	int count;

	push_call("get_char_room(%p,%p)",ch,argument);

	number = number_argument(argument, arg);
	count  = 0;

	if (!strcasecmp(arg, "self"))
	{
		pop_call();
		return ch;
	}

	for (rch = ch->in_room->first_person ; rch != NULL ; rch = rch->next_in_room)
	{
		if (!is_name(arg, rch->name))
		{
			continue;
		}
		if (!can_see(ch, rch) && (!IS_NPC(ch) || !IS_IMMORTAL(ch)))
		{
			continue;
		}
		if (++count != number)
		{
			continue;
		}
		pop_call();
		return rch;
	}
	count = 0;

	for (rch = ch->in_room->first_person ; rch != NULL ; rch = rch->next_in_room)
	{
		if (!is_name_short(arg, rch->name))
		{
			continue;
		}
		if (!can_see(ch, rch) || ++count != number)
		{
			continue;
		}
		pop_call();
		return rch;
	}
	pop_call();
	return NULL;
}


CHAR_DATA *get_char_area( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *ach;
	int number, count, vnum;

	push_call("get_char_area(%p,%p)",ch,argument);

	number = number_argument(argument, arg);
	count  = 0;

	for (vnum = ch->in_room->area->low_r_vnum ; vnum <= ch->in_room->area->hi_r_vnum ; vnum++)
	{
		if (room_index[vnum] == NULL)
		{
			continue;
		}

		for (ach = room_index[vnum]->first_person ; ach ; ach = ach->next_in_room)
		{
			if (!is_name(arg, ach->name))
			{
				continue;
			}
			if (!can_see(ch, ach) && (!IS_NPC(ch) || !IS_IMMORTAL(ch)))
			{
				continue;
			}
			if (++count != number)
			{
				continue;
			}
			pop_call();
			return ach;
		}
	}

	count = 0;

	for (vnum = ch->in_room->area->low_r_vnum ; vnum <= ch->in_room->area->hi_r_vnum ; vnum++)
	{
		if (room_index[vnum] == NULL)
		{
			continue;
		}

		for (ach = room_index[vnum]->first_person ; ach ; ach = ach->next_in_room)
		{
			if (!is_name_short(arg, ach->name))
			{
				continue;
			}
			if (!can_see(ch, ach) && (!IS_NPC(ch) || !IS_IMMORTAL(ch)))
			{
				continue;
			}
			if (++count != number)
			{
				continue;
			}
			pop_call();
			return ach;
		}
	}
	pop_call();
	return NULL;
}

/*
	Find a char in the world.
*/

CHAR_DATA *get_char_world( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *wch;
	int number;
	int count;

	push_call("get_char_world(%p,%p)",ch,argument);

	if ((wch = get_char_room(ch, argument)) != NULL)
	{
		pop_call();
		return wch;
	}

	if ((wch = get_player_world(ch, argument)) != NULL)
	{
		pop_call();
		return wch;
	}

	number = number_argument( argument, arg );
	count  = 0;

	for (wch = mud->f_char ; wch ; wch = wch->next)
	{
		if (!is_name(arg, wch->name))
		{
			continue;
		}
		if (!can_see(ch, wch) && (!IS_NPC(ch) || !IS_IMMORTAL(ch)))
		{
			continue;
		}
		if (!can_see_world(ch, wch) && (!IS_NPC(ch) || !IS_IMMORTAL(ch)))
		{
			continue;
		}
		if (++count != number)
		{
			continue;
		}
		pop_call();
		return wch;
	}
	count = 0;

	for (wch = mud->f_char ; wch ; wch = wch->next)
	{
		if (!is_name_short(arg, wch->name))
		{
			continue;
		}
		if (!can_see(ch, wch) || !can_see_world(ch, wch) || ++count != number)
		{
			continue;
		}
		else
		{
			pop_call();
			return wch;
		}
	}
	pop_call();
	return NULL;
}

CHAR_DATA *get_mob_vnum( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *ich;
	char arg[MAX_INPUT_LENGTH];
	int count, number, vnum;

	push_call("get_mob_vnum(%p,%p)",ch,argument);

	number = number_argument(argument, arg);
	count  = 0;
	vnum   = atol(arg);

	if (get_mob_index(vnum) == NULL)
	{
		pop_call();
		return NULL;
	}

	for (ich = mob_index[vnum]->first_instance ; ich ; ich = ich->next_instance)
	{
		if (++count == number && can_see(ch, ich))
		{
			pop_call();
			return ich;
		}
	}
	pop_call();
	return NULL;
}

OBJ_DATA *get_obj_vnum(CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *iobj;
	char arg[MAX_INPUT_LENGTH];
	int count, number, vnum;

	push_call("get_obj_vnum(%p,%p)",ch,argument);

	number = number_argument(argument, arg);
	count  = 0;
	vnum   = atol(arg);

	if (get_obj_index(vnum) == NULL)
	{
		pop_call();
		return NULL;
	}

	for (iobj = obj_index[vnum]->first_instance ; iobj ; iobj = iobj->next_instance)
	{
		if (++count == number && can_see_obj(ch, iobj))
		{
			pop_call();
			return iobj;
		}
	}
	pop_call();
	return NULL;
}
		
/*
	Returns TRUE if a PC with the specified name exists
*/

int char_exists(char *arg)
{
	FILE            *charfile;
	DESCRIPTOR_DATA *d;
	char pfile[MAX_INPUT_LENGTH], buf1[200], buf2[200];

	push_call("char_exists(%p)",arg);

	if (*arg == '\0' || *arg == ' ')
	{
		pop_call();
		return FALSE;
	}

	for (d = mud->f_desc ; d ; d = d->next)
	{
		if (is_desc_valid(d->character))
		{
			if (is_name(arg, d->character->name))
			{
				pop_call();
				return TRUE;
			}
		}
	}

	sprintf(buf1, "%s/%c/%s",	PLAYER_DIR, tolower(arg[0]), capitalize_name(arg));
	sprintf(buf2, "%s/%c/%c/%s",	PLAYER_DIR, tolower(arg[0]), tolower(arg[1]), capitalize_name(arg));

	if ((charfile = my_fopen(buf1, "r", TRUE)) != NULL)
	{
		rename(buf1, buf2);

		my_fclose(charfile);
	}

	sprintf(pfile,  "%s/%c/%c/%s", PLAYER_DIR, tolower(arg[0]), tolower(arg[1]), capitalize(arg));

	if ((charfile = my_fopen(pfile, "r", TRUE)) == NULL)
	{
		pop_call();
		return FALSE;
	}
	if (charfile)
	{
		my_fclose(charfile);
	}
	pop_call();
	return TRUE;
}

/*
	Find a player in the world
*/

CHAR_DATA *get_player_world( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	PLAYER_GAME *fpl;
	CHAR_DATA   *wch;

	push_call("get_player_world(%p,%p)",ch,argument);

	if ((wch = get_char_room(ch, argument)) != NULL && !IS_NPC(wch))
	{
		pop_call();
		return wch;
	}

	argument = one_argument( argument, arg );

	for (fpl = mud->f_player ; fpl ; fpl = fpl->next)
	{
		if (!is_name(arg, fpl->ch->name)) 
		{
			continue;
		}
		if (!can_see(ch, fpl->ch) && (!IS_NPC(ch) || !IS_IMMORTAL(ch)))
		{
			continue;
		}
		if (!can_see_world(ch, fpl->ch) && (!IS_NPC(ch) || !IS_IMMORTAL(ch)))
		{
			continue;
		}
		pop_call();
		return fpl->ch;
	}

	for (fpl = mud->f_player ; fpl ; fpl = fpl->next)
	{
		if (!is_name_short(arg, fpl->ch->name) || !can_see_world(ch, fpl->ch) || !can_see(ch, fpl->ch))
		{
			continue;
		}
		pop_call();
		return fpl->ch;
	}
	pop_call();
	return NULL;
}

/*
	pvnum_index based routines - Scandum
*/

CHAR_DATA * get_char_pvnum(int pvnum)
{
	push_call("get_char_pvnum(%p)",pvnum);

	if (pvnum < 0 || pvnum >= MAX_PVNUM)
	{
		pop_call();
		return NULL;
	}

	if (pvnum_index[pvnum] == NULL)
	{
		pop_call();
		return NULL;
	}

	pop_call();
	return pvnum_index[pvnum]->ch;
}


char *get_name_pvnum(int pvnum)
{
	push_call("get_name_pvnum(%p)",pvnum);

	if (pvnum < 0 || pvnum >= MAX_PVNUM)
	{
		pop_call();
		return NULL;
	}

	if (pvnum_index[pvnum] == NULL)
	{
		pop_call();
		return NULL;
	}

	pop_call();
	return pvnum_index[pvnum]->name;
}

int get_pvnum_name(char *name)
{
	int pvnum;

	push_call("get_pvnum_name(%p)",name);

	for (pvnum = 0 ; pvnum < MAX_PVNUM ; pvnum++)
	{
		if (pvnum_index[pvnum] == NULL)
		{
			continue;
		}
		if (IS_SET(pvnum_index[pvnum]->flags, PVNUM_DELETED))
		{
			continue;
		}
		if (!strcasecmp(pvnum_index[pvnum]->name, name))
		{
			pop_call();
			return pvnum;
		}
	}
	pop_call();
	return -1;
}

/*
	Find an obj in a list.
*/

OBJ_DATA *get_obj_list( CHAR_DATA *ch, char *argument, OBJ_DATA *list )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int number;
	int count;

	push_call("get_obj_list(%p,%p,%p)",ch,argument,list);

	number = number_argument( argument, arg );
	count  = 0;

	for (obj = list ; obj ; obj = obj->next_content)
	{
		if (objref(arg, obj))
		{
			pop_call();
			return obj;
		}
		if (can_see_obj(ch, obj))
		{
			if (is_name(arg, obj->name) && ++count == number)
			{
				pop_call();
				return obj;
			}
		}
	}
	count = 0;

	for (obj = list ; obj ; obj = obj->next_content)
	{
		if (can_see_obj(ch, obj) && is_name_short(arg, obj->name))
		{
			if (++count == number)
			{
				pop_call();
				return obj;
			}
		}
	}
	pop_call();
	return NULL;
}

/*
	Find an obj in player's inventory - used for the steal skill.
*/

OBJ_DATA *get_obj_carry_even_invis(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int number;
	int count;

	push_call("get_obj_carry_even_invis(%p,%p)",ch,argument);

	number = number_argument( argument, arg );
	count  = 0;

	for (obj = ch->first_carrying ; obj ; obj = obj->next_content)
	{
		if (objref(arg, obj))
		{
			pop_call();
			return obj;
		}
		if (obj->wear_loc == WEAR_NONE && is_name(arg, obj->name))
		{
			if (++count == number)
			{
				pop_call();
				return obj;
			}
		}
	}

	count = 0;

	for (obj = ch->first_carrying ; obj ; obj = obj->next_content)
	{
		if (obj->wear_loc == WEAR_NONE && is_name_short(arg, obj->name))
		{
			if (++count == number)
			{
				pop_call();
				return obj;
			}
		}
	}
	pop_call();
	return NULL;
}

OBJ_DATA *get_obj_carry( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int number;
	int count;

	push_call("get_obj_carry(%p,%p)",ch,argument);

	number = number_argument( argument, arg );
	count  = 0;

	for (obj = ch->first_carrying ; obj ; obj = obj->next_content)
	{
		if (obj->wear_loc == WEAR_NONE)
		{
			if (objref(arg, obj))
			{
				pop_call();
				return obj;
			}
			if (can_see_obj(ch, obj) && is_name(arg, obj->name) && ++count == number)
			{
				pop_call();
				return obj;
			}
		}
	}

	count = 0;

	for (obj = ch->first_carrying ; obj ; obj = obj->next_content)
	{
		if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj) && is_name_short(arg, obj->name))
		{
			if (++count == number)
			{
				pop_call();
				return obj;
			}
		}
	}
	pop_call();
	return NULL;
}

/*
	Find an obj in keeper's inventory.
*/

OBJ_DATA *get_obj_carry_keeper( CHAR_DATA *keeper, char *argument, CHAR_DATA *ch)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj, *oobj;
	int number, count;

	push_call("get_obj_carry_keeper(%p,%p,%p)",keeper, argument, bargain);

	number = number_argument( argument, arg );
	count  = 0;

	for (obj = keeper->first_carrying ; obj ; obj = obj->next_content)
	{
		if (obj->wear_loc != WEAR_NONE || !can_see_obj(ch, obj) || !is_name(arg, obj->name))
		{
			continue;
		}
		if (bargain(ch, get_cost(keeper, obj, TRUE)) < 1)
		{
			continue;
		}
		for (oobj = keeper->first_carrying ; oobj != obj ; oobj = oobj->next_content)
		{
			if (oobj->wear_loc != WEAR_NONE || !can_see_obj(ch, oobj) || !is_name(arg, oobj->name))
			{
				continue;
			}
			if (bargain(ch, get_cost(keeper, oobj, TRUE)) < 1)
			{
				continue;
			}
			if (oobj->pIndexData->vnum == obj->pIndexData->vnum && !IS_SET(obj->extra_flags, ITEM_MODIFIED))
			{
				break;
			}
		}
		if (oobj != obj)
		{
			continue;
		}
		if (++count == number)
		{
			pop_call();
			return obj;
		}
	}

	count = 0;

	for (obj = keeper->first_carrying ; obj ; obj = obj->next_content)
	{
		if (obj->wear_loc != WEAR_NONE || !can_see_obj(ch, obj) || !is_name_short(arg, obj->name))
		{
			continue;
		}
		if (bargain(ch, get_cost(keeper, obj, TRUE)) < 1)
		{
			continue;
		}

		for (oobj = keeper->first_carrying ; oobj != obj ; oobj = oobj->next_content)
		{
			if (oobj->wear_loc != WEAR_NONE || !can_see_obj(ch, oobj) || !is_name_short(arg, oobj->name))
			{
				continue;
			}
			if (bargain(ch, get_cost(keeper, oobj, TRUE)) < 1)
			{
				continue;
			}
			if (oobj->pIndexData->vnum == obj->pIndexData->vnum && !IS_SET(obj->extra_flags, ITEM_MODIFIED))
			{
				break;
			}
		}
		if (oobj != obj)
		{
			continue;
		}
		if (++count == number)
		{
			pop_call();
			return obj;
		}
	}
	pop_call();
	return NULL;
}


/*
	Find an obj in player's equipment.
*/

OBJ_DATA *get_obj_wear( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	char arg[MAX_INPUT_LENGTH];
	int number, count;

	push_call("get_obj_wear(%p,%p)",ch,argument);

	number = number_argument(argument, arg);
	count  = 0;

	for (obj = ch->first_carrying ; obj ; obj = obj->next_content)
	{
		if (obj->wear_loc != WEAR_NONE)
		{
			if (objref(arg, obj))
			{
				pop_call();
				return obj;
			}
			if (can_see_obj(ch, obj) && is_name(arg, obj->name) && ++count == number)
			{
				pop_call();
				return obj;
			}
		}
	}

	count = 0;

	for (obj = ch->first_carrying ; obj ; obj = obj->next_content)
	{
		if (obj->wear_loc != WEAR_NONE && can_see_obj(ch, obj) && is_name_short(arg, obj->name))
		{
			if (++count == number)
			{
				pop_call();
				return obj;
			}
		}
	}
	pop_call();
	return NULL;
}

/*
	Find an obj vnum in player's equipment.
*/

OBJ_DATA *get_obj_wear_vnum( CHAR_DATA *ch, int vnum)
{
	OBJ_DATA *obj;

	push_call("get_obj_wear_vnum(%p,%p)",ch,vnum);

	for (obj = ch->first_carrying ; obj ; obj = obj->next_content)
	{
		if (obj->wear_loc != WEAR_NONE && obj->pIndexData->vnum == vnum)
		{
			pop_call();
			return obj;
		}
	}
	pop_call();
	return NULL;
}

/*
	Find an obj vnum in a players inventory.
*/

OBJ_DATA *get_obj_carry_vnum( CHAR_DATA *ch, int vnum)
{
	OBJ_DATA *obj;

	push_call("get_obj_carry_vnum(%p,%p)",ch,vnum);

	for (obj = ch->first_carrying ; obj ; obj = obj->next_content)
	{
		if (obj->pIndexData->vnum == vnum)
		{
			pop_call();
			return obj;
		}
	}
	pop_call();
	return NULL;
}

/*
	Now scans as if 1 list - Scandum
*/

OBJ_DATA *get_obj_here_vnum( CHAR_DATA *ch, int vnum, int number )
{
	OBJ_DATA *obj;
	int count;

	push_call("get_obj_here_vnum(%p,%p)",ch,vnum,number);

	count  = 0;

	for (obj = ch->first_carrying ; obj ; obj = obj->next_content)
	{
		if (obj->wear_loc != WEAR_NONE && obj->pIndexData->vnum == vnum && can_see_obj(ch, obj))
		{
			if (++count == number)
			{
				pop_call();
				return obj;
			}
		}
	}

	for (obj = ch->first_carrying ; obj ; obj = obj->next_content)
	{
		if (obj->wear_loc == WEAR_NONE && obj->pIndexData->vnum == vnum && can_see_obj(ch, obj))
		{
			if (++count == number)
			{
				pop_call();
				return obj;
			}
		}
	}

	for (obj = ch->in_room->first_content ; obj ; obj = obj->next_content)
	{
		if (obj->pIndexData->vnum == vnum && can_see_obj(ch, obj))
		{
			if (++count == number)
			{
				pop_call();
				return obj;
			}
		}
	}

	pop_call();
	return NULL;
}


OBJ_DATA *get_obj_here( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int number;
	int count;

	push_call("get_obj_here(%p,%p)",ch,argument);

	number = number_argument( argument, arg );

	if (arg[0] == 'i' && atol(&arg[1]))
	{
		pop_call();
		return get_obj_here_vnum(ch, atol(&arg[1]), number);
	}

	count  = 0;

	for (obj = ch->first_carrying ; obj ; obj = obj->next_content)
	{
		if (objref(arg, obj))
		{
			pop_call();
			return obj;
		}

		if (obj->wear_loc != WEAR_NONE && can_see_obj(ch, obj) && is_name(arg, obj->name))
		{
			if (++count == number)
			{
				pop_call();
				return obj;
			}
		}
	}

	for (obj = ch->first_carrying ; obj ; obj = obj->next_content)
	{
		if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj) && is_name(arg, obj->name))
		{
			if (++count == number)
			{
				pop_call();
				return obj;
			}
		}
	}

	for (obj = ch->in_room->first_content ; obj ; obj = obj->next_content)
	{
		if (objref(arg, obj))
		{
			pop_call();
			return obj;
		}

		if (can_see_obj(ch, obj) && is_name(arg, obj->name))
		{
			if (++count == number)
			{
				pop_call();
				return obj;
			}
		}
	}

	count = 0;

	for (obj = ch->first_carrying ; obj ; obj = obj->next_content)
	{
		if (obj->wear_loc != WEAR_NONE && can_see_obj(ch, obj) && is_name_short(arg, obj->name))
		{
			if (++count == number)
			{
				pop_call();
				return obj;
			}
		}
	}

	for (obj = ch->first_carrying ; obj ; obj = obj->next_content)
	{
		if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj) && is_name_short(arg, obj->name))
		{
			if (++count == number)
			{
				pop_call();
				return obj;
			}
		}
	}

	for (obj = ch->in_room->first_content ; obj ; obj = obj->next_content)
	{
		if (can_see_obj(ch, obj) && is_name_short(arg, obj->name))
		{
			if (++count == number)
			{
				pop_call();
				return obj;
			}
		}
	}
	pop_call();
	return NULL;
}



/*
	Find an obj in the world.
*/

OBJ_DATA *get_obj_world( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int number;
	int count;

	push_call("get_obj_world(%p,%p)",ch,argument);

	if ((obj = get_obj_here(ch, argument)) != NULL)
	{
		pop_call();
		return obj;
	}

	number = number_argument( argument, arg );
	count  = 0;

	for (obj = mud->f_obj ; obj ; obj = obj->next)
	{
		if (objref(arg, obj))
		{
			pop_call();
			return obj;
		}

		if (can_see_obj(ch, obj) && is_name(arg, obj->name))
		{
			if ( ++count == number )
			{
				pop_call();
				return obj;
			}
		}
	}

	count = 0;

	for (obj = mud->f_obj ; obj ; obj = obj->next)
	{
		if (can_see_obj(ch, obj) && is_name_short(arg, obj->name))
		{
			if (++count == number)
			{
				pop_call();
				return obj;
			}
		}
	}
	pop_call();
	return NULL;
}

/*
	Create a 'money' obj.
*/

OBJ_DATA *create_money( int amount )
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;

	push_call("create_money(%p)",amount);

	if ( amount <= 0 )
	{
		bug( "Create_money: zero or negative money %d.", amount );
		amount = 1;
	}

	if ( amount == 1 )
	{
		obj = create_object( get_obj_index( OBJ_VNUM_MONEY_ONE ), 0 );
	}
	else
	{
		obj = create_object( get_obj_index( OBJ_VNUM_MONEY_SOME ), 0 );
		sprintf( buf, obj->short_descr, amount );
		STRFREE( obj->short_descr );
		obj->short_descr        = STRALLOC( buf );
		obj->value[0]           = amount;
	}
	pop_call();
	return obj;
}

/*
	Return weight of an object, including weight of first_content.
	But this linked list is extremely slow.
*/

int get_obj_weight( OBJ_DATA *obj )
{
	push_call("get_obj_weight(%p)",obj);

	pop_call();
	return(obj->weight + obj->content_weight);
}

/*
	True if room is dark.
*/

bool room_is_dark( ROOM_INDEX_DATA *pRoomIndex )
{
	push_call("room_is_dark(%p)",pRoomIndex);

	if (IS_SET(pRoomIndex->room_flags, ROOM_GLOBE))
	{
		pop_call();
		return TRUE;
	}

	if (pRoomIndex->light > 0)
	{
		pop_call();
		return FALSE;
	}

	if (IS_SET(pRoomIndex->room_flags, ROOM_DARK))
	{
		pop_call();
		return TRUE;
	}

	if (pRoomIndex->sector_type == SECT_INSIDE
	||  pRoomIndex->sector_type == SECT_CITY)
	{
		pop_call();
		return FALSE;
	}

	if (mud->sunlight == SUN_DARK)
	{
		pop_call();
		return TRUE;
	}
	pop_call();
	return FALSE;
}

/*
	True if room is private.
*/

bool room_is_private( ROOM_INDEX_DATA *pRoomIndex )
{
	CHAR_DATA *rch;
	int count;

	count = 0;

	push_call("room_is_private(%p)",pRoomIndex);

	for (rch = pRoomIndex->first_person ; rch ; rch = rch->next_in_room)
	{
		if (!IS_NPC(rch))
		{
			count++;
		}
	}

	if (IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE) && count >= 2 )
	{
		pop_call();
		return TRUE;
	}

	if (IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) && count >= 1)
	{
		pop_call();
		return TRUE;
	}
	pop_call();
	return FALSE;
}


/*
	True if char can see victim.
*/

bool can_see( CHAR_DATA *ch, CHAR_DATA *victim )
{
	push_call("can_see(%p,%p)",ch,victim);

	if (ch == victim)
	{
		pop_call();
		return TRUE;
	}

	if (IS_NPC(victim) && show_build_vnum(ch, victim->pIndexData->vnum))
	{
		pop_call();
		return TRUE;
	}

	if (IS_NPC(victim) && IS_AFFECTED(victim, AFF_ETHEREAL))
	{
		pop_call();
		return FALSE;
	}

	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT) && victim->trust <= ch->trust)
	{
		pop_call();
		return TRUE;
	}

	if (!IS_NPC(victim) && IS_SET(victim->act, PLR_WIZINVIS))
	{
		pop_call();
		return FALSE;
	}

	if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_ETHEREAL))
	{
		pop_call();
		return TRUE;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_GLOBE))
	{
		if (race_table[ch->race].vision < 2)
		{
			pop_call();
			return FALSE;
		}
	}

	if (IS_AFFECTED(ch, AFF_TRUESIGHT))
	{
		pop_call();
		return TRUE;
	}

	if (IS_AFFECTED(ch, AFF_BLIND))
	{
		pop_call();
		return FALSE;
	}

	if (!IS_NPC(ch) && room_is_dark(ch->in_room) && !IS_AFFECTED(ch, AFF_INFRARED))
	{
		if (race_table[ch->race].vision == 0)
		{
			pop_call();
			return FALSE;
		}

		if (race_table[ch->race].vision == 1
		&& (ch->in_room->sector_type == SECT_INSIDE
		|| IS_SET(ch->in_room->room_flags, ROOM_INDOORS)))
		{
			pop_call();
			return FALSE;
		}
	}

	if (IS_AFFECTED(victim, AFF_INVISIBLE)
	&& !IS_AFFECTED(ch, AFF_DETECT_INVIS))
	{
		pop_call();
		return FALSE;
	}

	if (IS_AFFECTED(victim, AFF_IMP_INVISIBLE)
	&& !IS_AFFECTED(ch, AFF_DETECT_INVIS))
	{
		pop_call();
		return FALSE;
	}

	if (IS_AFFECTED(victim, AFF_HIDE)
	&& !IS_AFFECTED(ch, AFF_DETECT_HIDDEN))
	{
		pop_call();
		return FALSE;
	}

	if (IS_AFFECTED(victim, AFF_STEALTH)
	&& !IS_AFFECTED(ch, AFF_DETECT_HIDDEN))
	{
		pop_call();
		return FALSE;
	}

	pop_call();
	return TRUE;
}

/*
	True if char can see victim in who list and alike
*/

bool can_see_world( CHAR_DATA *ch, CHAR_DATA *victim )
{
	push_call("can_see_world(%p,%p)",ch,victim);

	if (ch == victim)
	{
		pop_call();
		return TRUE;
	}

	if (IS_NPC(victim) && IS_AFFECTED(victim, AFF_ETHEREAL))
	{
		if (IS_NPC(ch) || !IS_IMMORTAL(ch) || !can_olc_modify(ch, victim->pIndexData->vnum))
		{
			pop_call();
			return FALSE;
		}
	}

	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT) && victim->trust <= ch->trust)
	{
		pop_call();
		return TRUE;
	}

	if (!IS_NPC(victim) && IS_SET(victim->act, PLR_WIZINVIS))
	{
		pop_call();
		return FALSE;
	}

	if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_ETHEREAL))
	{
		pop_call();
		return TRUE;
	}

	if (!IS_NPC(victim) && IS_SET(victim->act, PLR_WIZCLOAK))
	{
		pop_call();
		return FALSE;
	}

	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT))
	{
		pop_call();
		return TRUE;
	}

	if (!IS_NPC(victim) && IS_AFFECTED(victim, AFF_STEALTH))
	{
		if (victim->pcdata->learned[gsn_greater_stealth] > 0)
		{
			if (!IS_AFFECTED(ch, AFF_TRUESIGHT) || victim->level > ch->level)
			{
				pop_call();
				return FALSE;
			}
		}
	}

	pop_call();
	return TRUE;
}

/*
	True if char can see stuff with infravision - Scandum 14-05-2002
*/

bool can_see_infra( CHAR_DATA *ch, CHAR_DATA *victim )
{
	push_call("can_see_infra(%p,%p)",ch,victim);

	if (IS_NPC(ch) && IS_IMMORTAL(ch))
	{
		pop_call();
		return TRUE;
	}

	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT))
	{
		pop_call();
		return TRUE;
	}

	if (!IS_AFFECTED(ch, AFF_INFRARED))
	{
		pop_call();
		return FALSE;
	}

	if (IS_UNDEAD(victim))
	{
		pop_call();
		return FALSE;
	}
	pop_call();
	return TRUE;
}

/*
	True if char can see in smoke rooms
*/

bool can_see_smoke( CHAR_DATA *ch )
{
	push_call("can_see_smoke(%p)",ch);

	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT))
	{
		pop_call();
		return TRUE;
	}

	if (IS_AFFECTED(ch, AFF_TRUESIGHT))
	{
		pop_call();
		return TRUE;
	}

	if (number_percent() > 50)
	{
		pop_call();
		return TRUE;
	}
	pop_call();
	return FALSE;
}


/*
	True if char can see hidden stuff (used for hidden exists)
*/

bool can_see_hidden( CHAR_DATA *ch )
{
	push_call("can_see_hidden(%p)",ch);

	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT))
	{
		pop_call();
		return TRUE;
	}

	if (IS_AFFECTED(ch, AFF_DETECT_HIDDEN))
	{
		pop_call();
		return TRUE;
	}

	if (IS_AFFECTED(ch, AFF_TRUESIGHT))
	{
		pop_call();
		return TRUE;
	}

	if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_ETHEREAL))
	{
		pop_call();
		return TRUE;
	}
	pop_call();
	return FALSE;
}

/*
	True if char can hear victim.
*/

bool can_hear( CHAR_DATA *ch, CHAR_DATA *victim )
{
	push_call("can_hear(%p,%p)",ch,victim);

	if (ch != NULL)
	{
		if (ch == victim)
		{
			pop_call();
			return TRUE;
		}

		if (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT) && victim->trust <= ch->trust)
		{
			pop_call();
			return TRUE;
		}
	}

	if (IS_NPC(victim) && IS_AFFECTED(victim, AFF_ETHEREAL))
	{
		pop_call();
		return FALSE;
	}

	if (!IS_NPC(victim))
	{
		if (IS_SET(victim->act, PLR_WIZINVIS) || IS_SET(victim->act, PLR_WIZCLOAK))
		{
			pop_call();
			return FALSE;
		}
	}

	if (IS_AFFECTED(victim, AFF_HIDE))
	{
		pop_call();
		return FALSE;
	}

	if (IS_AFFECTED(victim, AFF_STEALTH))
	{
		pop_call();
		return FALSE;
	}


	if (IS_AFFECTED(victim, AFF_SNEAK))
	{
		pop_call();
		return FALSE;
	}

	if (victim->race == RACE_HOBBIT && get_eq_char(victim, WEAR_FEET) == NULL)
	{
		pop_call();
		return FALSE;
	}

	pop_call();
	return TRUE;
}

/*
	True if char can see obj
*/

bool can_see_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
	push_call("can_see_obj(%p,%p)",ch,obj);

	if (show_build_vnum(ch, obj->pIndexData->vnum))
	{
		pop_call();
		return TRUE;
	}

	if (!IS_NPC(ch))
	{
		if (!IS_IMMORTAL(ch) && IS_SET(obj->extra_flags, ITEM_ETHEREAL))
		{
			pop_call();
			return FALSE;
		}

		if (IS_SET(ch->act, PLR_HOLYLIGHT))
		{
			pop_call();
			return TRUE;
		}
	}

	if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_ETHEREAL))
	{
		pop_call();
		return TRUE;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_GLOBE))
	{
		if (race_table[ch->race].vision < 2)
		{
			pop_call();
			return FALSE;
		}
	}

	if (IS_AFFECTED(ch, AFF_TRUESIGHT))
	{
		pop_call();
		return TRUE;
	}

	if (IS_AFFECTED(ch, AFF_BLIND))
	{
		pop_call();
		return FALSE;
	}

	if (obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
	{
		pop_call();
		return TRUE;
	}

	if (!can_see_in_room(ch, ch->in_room))
	{
		if (!IS_SET(obj->extra_flags, ITEM_GLOW))
		{
			pop_call();
			return FALSE;
		}
	}

	if (IS_SET(obj->extra_flags, ITEM_INVIS)
	&& !IS_AFFECTED(ch, AFF_DETECT_INVIS))
	{
		pop_call();
		return FALSE;
	}

	pop_call();
	return TRUE;
}

/*
	True if char can drop obj.
*/

bool can_drop_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
	push_call("can_drop_obj(%p,%p)",ch,obj);

	if (IS_IMMORTAL(ch))
	{
		pop_call();
		return TRUE;
	}

	if (!IS_SET(obj->extra_flags, ITEM_NODROP))
	{
		pop_call();
		return TRUE;
	}
	pop_call();
	return FALSE;
}

/*
	Return ascii name of an item type.
*/

char *item_type_name( OBJ_DATA *obj )
{
	push_call("item_type_name(%p)",obj);

	switch ( obj->item_type )
	{
		case ITEM_LIGHT:
			pop_call();
			return "light";
		case ITEM_SCROLL:
			pop_call();
			return "scroll";
		case ITEM_WAND:
			pop_call();
			return "wand";
		case ITEM_STAFF:      
			pop_call();
			return "staff";
		case ITEM_WEAPON:     
			pop_call();
 			return "weapon";
		case ITEM_TREASURE:   
			pop_call();
 			return "treasure";
		case ITEM_ARMOR:      
			pop_call();
 			return "armor";
		case ITEM_POTION:     
			pop_call();
 			return "potion";
		case ITEM_FURNITURE:  
			pop_call();
 			return "furniture";
		case ITEM_TRASH:      
			pop_call();
 			return "trash";
		case ITEM_CONTAINER:  
			pop_call();
 			return "container";
		case ITEM_DRINK_CON:  
			pop_call();
 			return "drink container";
		case ITEM_KEY:        
			pop_call();
 			return "key";
		case ITEM_FOOD:       
			pop_call();
 			return "food";
		case ITEM_MONEY:      
			pop_call();
 			return "money";
		case ITEM_BOAT:       
			pop_call();
 			return "boat";
		case ITEM_CORPSE_NPC: 
			pop_call();
 			return "npc corpse";
		case ITEM_CORPSE_PC:  
			pop_call();
 			return "pc corpse";
		case ITEM_FOUNTAIN:   
			pop_call();
 			return "fountain";
		case ITEM_PILL:       
			pop_call();
 			return "pill";
		case ITEM_PORTAL:     
			pop_call();
 			return "portal";
 		case ITEM_ARTIFACT:
 			pop_call();
 			return "artifact";
 		case ITEM_LOCKPICK:
 			pop_call();
 			return "lockpick";
		case ITEM_AMMO:       
			pop_call();
 			return "ammunition";
 		case ITEM_TOTEM:
 			pop_call();
 			return "totem";
	}

	bug( "Item_type_name: unknown type %d.", obj->item_type );

	pop_call();
	return "(unknown)";
}

/*
	Return ascii name of an affect location.
*/

char *affect_loc_name( int location )
{
	push_call("affect_loc_name(%p)",location);

	switch ( location )
	{
		case APPLY_NONE:          
			pop_call();
			return "none";
		case APPLY_STR:           
			pop_call();
			return "strength";
		case APPLY_DEX:           
			pop_call();
			return "dexterity";
		case APPLY_INT:           
			pop_call();
			return "intelligence";
		case APPLY_WIS:           
			pop_call();
			return "wisdom";
		case APPLY_CON:           
			pop_call();
			return "constitution";
		case APPLY_SEX:           
			pop_call();
			return "sex";
		case APPLY_RACE:          
			pop_call();
			return "race";
		case APPLY_LEVEL:         
			pop_call();
			return "level";
		case APPLY_AGE:           
			pop_call();
			return "age";
		case APPLY_MANA:          
			pop_call();
			return "mana";
		case APPLY_HIT:           
			pop_call();
			return "hp";
		case APPLY_MOVE:          
			pop_call();
			return "moves";
		case APPLY_EXP:           
			pop_call();
			return "experience";
		case APPLY_AC:            
			pop_call();
			return "armor class";
		case APPLY_HITROLL:       
			pop_call();
			return "hit roll";
		case APPLY_DAMROLL:       
			pop_call();
			return "damage roll";
		case APPLY_SAVING_PARA:   
			pop_call();
			return "save vs paralysis";
		case APPLY_SAVING_ROD:    
			pop_call();
			return "save vs rod";
		case APPLY_SAVING_PETRI:  
			pop_call();
			return "save vs petrification";
		case APPLY_SAVING_BREATH: 
			pop_call();
			return "save vs breath";
		case APPLY_SAVING_SPELL:  
			pop_call();
			return "save vs spell";
	}

	bug( "Affect_location_name: unknown location %d.", location );
	pop_call();
	return "(unknown)";
}

/*
	Return ascii name of an affect bit vector.
*/

char *affect_bit_name( int vector )
{
	static char buf[512];

	push_call("affect_bit_name(%p)",vector);

	buf[0] = '\0';
	if (vector > 0)
	{
		if ( vector & AFF_BLIND				) strcat( buf, " blind"			);
		if ( vector & AFF_INVISIBLE			) strcat( buf, " invisible"		);
		if ( vector & AFF_IMP_INVISIBLE		) strcat( buf, " improved_invis"   );
		if ( vector & AFF_DETECT_INVIS		) strcat( buf, " detect_invis"	);
		if ( vector & BV05					) strcat( buf, " BV05"			);
		if ( vector & AFF_DETECT_HIDDEN		) strcat( buf, " detect_hidden"	);
		if ( vector & AFF_TRUESIGHT			) strcat( buf, " truesight"		);
		if ( vector & AFF_SANCTUARY			) strcat( buf, " sanctuary"		);
		if ( vector & BV09					) strcat( buf, " BV09"			);
		if ( vector & AFF_INFRARED			) strcat( buf, " infrared"		);
		if ( vector & AFF_CURSE				) strcat( buf, " curse"			);
		if ( vector & AFF_POISON				) strcat( buf, " poison"			);
		if ( vector & AFF_PROTECT_EVIL		) strcat( buf, " protect_evil"	);
		if ( vector & AFF_PROTECT_GOOD		) strcat( buf, " protect_good"	);
		if ( vector & AFF_SLEEP				) strcat( buf, " sleep"			);
		if ( vector & AFF_SNEAK				) strcat( buf, " sneak"			);
		if ( vector & AFF_HIDE				) strcat( buf, " hide"			);
		if ( vector & AFF_CHARM				) strcat( buf, " charm"			);
		if ( vector & AFF_FLYING				) strcat( buf, " flying"			);
		if ( vector & AFF_PASS_DOOR			) strcat( buf, " pass_door"		);
		if ( vector & AFF_STEALTH			) strcat( buf, " stealth"		);
		if ( vector & AFF_HASTE				) strcat( buf, " haste"			);
	}
	else
	{
		vector = 0-vector;

		if ( vector & (0-AFF2_ENHANCED_REST)		) strcat( buf, " enhanced rest"		);
		if ( vector & (0-AFF2_ENHANCED_HEAL)		) strcat( buf, " enhanced heal"		);
		if ( vector & (0-AFF2_ENHANCED_REVIVE)		) strcat( buf, " enhanced revive"		);
		if ( vector & (0-AFF2_DIVINE_INSPIRATION)	) strcat( buf, " divine inspiration"	);
		if ( vector & (0-AFF2_CAMPING)			) strcat( buf, " camping"			);
		if ( vector & (0-AFF2_BERSERK)			) strcat( buf, " berserk"			);
		if ( vector & (0-AFF2_MIRROR_IMAGE)		) strcat( buf, " mirror image"		);
		if ( vector & (0-AFF2_HALLUCINATE)			) strcat( buf, " hallucinate"			);
		if ( vector & (0-AFF2_BLEEDING)			) strcat( buf, " bleeding"			);
		if ( vector & (0-AFF2_EARTHBIND)			) strcat( buf, " earthbind"			);
		if ( vector & (0-AFF2_ETHEREAL)			) strcat( buf, " ethereal travel"		);
		if ( vector & (0-AFF2_ASTRAL)				) strcat( buf, " astral projection"	);
		if ( vector & (0-AFF2_BREATH_WATER)		) strcat( buf, " breath water"		);
		if ( vector & (0-AFF2_QUICKEN)			) strcat( buf, " quicken"			);
		if ( vector & (0-AFF2_CONFUSION)			) strcat( buf, " confusion"			);
		if ( vector & (0-AFF2_HAS_FLASH)			) strcat( buf, " flash powder"		);
		if ( vector & (0-AFF2_FIRESHIELD)			) strcat( buf, " fireshield"			);
		if ( vector & (0-AFF2_TORRID_BALM)			) strcat( buf, " torrid balm"			);
		if ( vector & (0-AFF2_FIREWALK)			) strcat( buf, " firewalk"			);
	}
	pop_call();
	return ( buf[0] != '\0' ) ? buf+1 : "none";
}


/*
	Chaos 10/10/93
	Determines which class has highest level in a skill
	Return -1 is none.
*/

int multi( CHAR_DATA *ch, int sn)
{
	int cnt, mlv, tst;

	push_call("multi(%p,%p)",ch,sn);

	mlv = -1;
	tst = -1;

	if (IS_NPC(ch))
	{
		pop_call();
		return -1;
	}

	for (cnt = 0 ; cnt < MAX_CLASS ; cnt++)
	{
		if (ch->pcdata->mclass[cnt] >= skill_table[sn].skill_level[cnt])
		{
			if (mlv < ch->pcdata->mclass[cnt])
			{
				tst = cnt;
				mlv = ch->pcdata->mclass[cnt];
			}
		}
	}
	pop_call();
	return(tst);
}

/*
	This is the variant of multi that selects the highest adept level.
	Chaos - 9/14/98
*/

int multi_pick(CHAR_DATA *ch, int sn)
{
	int cnt, mlv, tst;

	push_call("multi_pick(%p,%p)",ch,sn);

	mlv = -1;
	tst = -1;

	if (IS_NPC(ch))
	{
		if (HAS_BIT(ch->act, ACT_CLASS))
		{
			if (ch->level >= skill_table[sn].skill_level[ch->class])
			{
				tst = ch->class;
			}
		}
	}
	else
	{
		for (cnt = 0 ; cnt < MAX_CLASS ; cnt++)
		{
			if (ch->pcdata->mclass[cnt] >= skill_table[sn].skill_level[cnt])
			{
				if (mlv < class_table[cnt].skill_master)
				{
					tst = cnt;
					mlv = class_table[cnt].skill_master;
				}
			}
		}
	}
	pop_call();
	return(tst);
}

/*
	Returns sum of multiclass levels regarding the given skill.
	100% if you can practise the skill.
	 50% if you can practise the skill at a higher level.
	 25% if you do not meet previous requirements
	It is asumed there is checked if ch has the skill practised
	Scandum 07/02/02
*/

int multi_skill_level( CHAR_DATA *ch, int sn)
{
	int cnt, mlv;

	push_call("multi_skill_level(%p,%p)",ch,sn);

	mlv = 0;

	if (IS_NPC(ch))
	{
		mlv = ch->level;

		pop_call();
		return(mlv);
	}

	for (cnt = 0 ; cnt < MAX_CLASS ; cnt++)
	{
		if (ch->pcdata->mclass[cnt] >= skill_table[sn].skill_level[cnt])
		{
			mlv += ch->pcdata->mclass[cnt];
		}
		else if (skill_table[sn].skill_level[cnt] < MAX_LEVEL)
		{
			mlv += ch->pcdata->mclass[cnt] / 2;
		}
		else
		{
			mlv += ch->pcdata->mclass[cnt] / 4;
		}
	}
	if (mlv > ch->level)
	{
		mlv = ch->level;
	}
	if (ch->race == RACE_HUMAN)
	{
		mlv += ch->pcdata->reincarnation;
	}
	pop_call();
	return(mlv);
}

/*
	Return percentage learned of skill/spell
*/

int learned( CHAR_DATA *ch, int sn )
{
	int total;

	push_call("learned(%p,%p)",ch,sn);

	if (IS_NPC(ch))
	{
		pop_call();
		return 100;
	}

	if (is_spell(sn))
	{
		total = int_app[get_curr_int(ch)].learn;
	}
	else
	{
		total = dex_app[get_curr_dex(ch)].learn;
	}

	total += ch->pcdata->reincarnation;

	if (IS_SET(skill_table[sn].race, 1 << ch->race))
	{
		pop_call();
		return 100 + total;
	}

	if (ch->pcdata->learned[sn] == 0)
	{
		pop_call();
		return 0;
	}

	pop_call();
	return (ch->pcdata->learned[sn] + total);
}

int weapon_skill( CHAR_DATA *ch, int type )
{
	int skill;

	push_call("weapon_skill(%p,%p)",ch,type);

	switch (type)
	{
		case WEAPON_TYPE_SWORD:
			skill = learned(ch, gsn_sword_fighting);
			break;
		case WEAPON_TYPE_DAGGER:
			skill = learned(ch, gsn_dagger_fighting);
			if (ch->race == RACE_VAMPIRE)
			{
				skill = UMAX(75, skill + 10);
			}
			break;
		case WEAPON_TYPE_AXE:
			skill = learned(ch, gsn_axe_fighting);
			if (ch->race == RACE_DWARF)
			{
				skill = UMAX(75, skill + 10);
			}
			break;
		case WEAPON_TYPE_MACE:
			skill = learned(ch, gsn_mace_fighting);
			if (ch->race == RACE_TITAN)
			{
				skill = UMAX(75, skill + 10);
			}
			break;
		case WEAPON_TYPE_STAFF:
			skill = learned(ch, gsn_staff_fighting);
			break;
		case WEAPON_TYPE_WHIP:
			skill = learned(ch, gsn_whip_fighting);
			break;
		case WEAPON_TYPE_FLAIL:
			skill = learned(ch, gsn_flail_fighting);
			break;
		case WEAPON_TYPE_SPEAR:
			skill = learned(ch, gsn_spear_fighting);
			break;
		case WEAPON_TYPE_CLAW:
			skill = learned(ch, gsn_claw_fighting);
			break;
		case WEAPON_TYPE_SHORTBOW:
		case WEAPON_TYPE_LONGBOW:
		case WEAPON_TYPE_CROSSBOW:
		case WEAPON_TYPE_BLOWPIPE:
			skill = learned(ch, gsn_shoot);
			if (ch->race == RACE_ELF)
			{
				skill += 10;
			}
			break;

		default:
			skill = 90;
			skill += dex_app[get_curr_dex(ch)].learn;
                        skill += IS_NPC(ch) ? 0 : ch->pcdata->reincarnation;
			break;
	}
	pop_call();
	return skill;
}

/*
	See if skill/spell improves
*/

void check_improve( CHAR_DATA *ch, int sn )
{
	int cnt;

	push_call("check_improve(%p, %d)",ch, sn);

	if (ch == NULL || IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (ch->pcdata->learned[sn] == 0)
	{
		pop_call();
		return;
	}

	if ((cnt = multi_pick(ch, sn)) == -1)
	{
		pop_call();
		return;
	}

	if (IS_AFFECTED(ch, AFF2_DIVINE_INSPIRATION))
	{
		if (ch->pcdata->last_improve + 86400 * 1 > mud->current_time)
		{
			pop_call();
			return;
		}

		if (number_range(1, 500) >= get_curr_int(ch))
		{
			pop_call();
			return;
		}
	}
	else
	{
		if (ch->pcdata->last_improve + 86400 * 2 > mud->current_time)
		{
			pop_call();
			return;
		}

		if (number_range(1, 1000) >= get_curr_int(ch))
		{
			pop_call();
			return;
		}
	}

	if (ch->pcdata->learned[sn] < class_table[cnt].skill_adept)
	{
		if (is_spell(sn))
		{
			ch_printf(ch, "By studying hard on %s you mastered it a bit more.\n\r", skill_table[sn].name);
		}
		else
		{
			ch_printf(ch, "You became better at %s.\n\r", skill_table[sn].name);
		}
		ch->pcdata->learned[sn]++;
		ch->pcdata->last_improve = mud->current_time;
	}
	pop_call();
	return;
}

int get_pets( CHAR_DATA *ch)
{
	PET_DATA *pet;
	int cnt = 0;

	push_call("get_pets(%p)",ch);

	for (pet = mud->f_pet ; pet ; pet = pet->next)
	{
		if (pet->ch->master == ch)
		{
			if (IS_AFFECTED(pet->ch, AFF_CHARM))
			{
				cnt++;
			}
		}
	}
	pop_call();
	return( cnt );
}

void char_reset(CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	AFFECT_DATA *paf;

	int old_max_hit, old_max_mana, old_max_move;

	push_call("char_reset(%p)",ch);

	ch->hitroll           = get_age_bonus(ch, RSPEC_AGE_HR);
	ch->damroll           = get_age_bonus(ch, RSPEC_AGE_DR);
	ch->pcdata->armor     = get_age_bonus(ch, RSPEC_AGE_AC) + 100;
	ch->saving_throw      = 0;
	ch->pcdata->eqhitroll = 0;
	ch->pcdata->eqdamroll = 0;
	ch->pcdata->eqsaves   = 0;
	ch->pcdata->mod_str   = 0;
	ch->pcdata->mod_dex   = 0;
	ch->pcdata->mod_wis   = 0;
	ch->pcdata->mod_con   = 0;
	ch->pcdata->mod_int   = 0;
	old_max_hit           = ch->max_hit;
	old_max_mana          = ch->max_mana;
	old_max_move          = ch->max_move;
	ch->max_hit           = ch->pcdata->actual_max_hit;
	ch->max_mana          = ch->pcdata->actual_max_mana;
	ch->max_move          = ch->pcdata->actual_max_move;
	ch->affected_by       = 0;
	ch->affected2_by      = 0;
	ch->sex               = ch->pcdata->actual_sex;

	SET_BIT(mud->flags, MUD_EQAFFECTING);

	for (obj = ch->first_carrying ; obj ; obj = obj->next_content)
	{
		if (obj->wear_loc != WEAR_NONE)
		{
			ch->pcdata->armor -= apply_ac(obj, obj->wear_loc);

			for (paf = obj->first_affect ; paf ; paf = paf->next)
			{
				switch(paf->location)
				{
					default:
						affect_modify(ch, paf, TRUE);
						break;
				}
			}

			for (paf = obj->pIndexData->first_affect ; paf ; paf = paf->next)
			{
				switch(paf->location)
				{
					case APPLY_RACE:
						break;
					default:
						affect_modify(ch, paf, TRUE);
						break;
				}
			}
		}
	}

	REMOVE_BIT(mud->flags, MUD_EQAFFECTING);

	for (paf = ch->first_affect ; paf ; paf = paf->next)
	{
		switch(paf->location)
		{
			default:
				affect_modify(ch, paf, TRUE);
				break;
		}
	}

	/* Fix limits    -  Chaos  1/31/97 */

	if (ch->max_mana < 1)
	{
		ch->max_mana = 1;
	}
	if (ch->max_mana < ch->mana)
	{
		ch->mana = ch->max_mana;
	}
	if (ch->max_hit < 1)
	{
		ch->max_hit = 1;
	}
	if (ch->max_hit < ch->hit)
	{
		ch->hit = ch->max_hit;
	}
	if (ch->max_move < 1)
	{
		ch->max_move = 1;
	}
	if (ch->max_move < ch->move)
	{
		ch->move = ch->max_move;
	}
	if (ch->pcdata->actual_max_hit == 0)  /* Reconstruct Actuals  - Chaos 12/22/94 */
	{
		ch->pcdata->actual_max_hit = old_max_hit - ch->max_hit;
		ch->max_hit += ch->pcdata->actual_max_hit;
		ch->pcdata->actual_max_mana = old_max_mana - ch->max_mana;
		ch->max_mana += ch->pcdata->actual_max_mana;
		ch->pcdata->actual_max_move = old_max_move - ch->max_move;
		ch->max_move += ch->pcdata->actual_max_move;
	}
	pop_call();
	return;
}


int mob_armor(CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	AFFECT_DATA *paf;
	int AC;

	push_call("mob_armor(%p)",ch);

	AC = 0;

	for (obj = ch->first_carrying ; obj ; obj = obj->next_content)
	{
		if (obj->wear_loc != WEAR_NONE)
		{
			AC += apply_ac(obj, obj->wear_loc);

			for (paf = obj->first_affect ; paf ; paf = paf->next)
			{
				if (paf->location == APPLY_AC)
				{
					AC = AC - paf->modifier;
				}
			}
			for (paf = obj->pIndexData->first_affect ; paf ; paf = paf->next)
			{
				if (paf->location == APPLY_AC)
				{
					AC = AC - paf->modifier;
				}
			}
		}
	}

	for (paf = ch->first_affect ; paf ; paf = paf->next)
	{
		if (paf->location == APPLY_AC)
		{
			AC = AC - paf->modifier;
		}
	}

	pop_call();
	return(AC);
}

int get_vision(CHAR_DATA *ch)
{
	int vision;

	push_call("get_vision(%p)",ch);

	vision = 100;

	if (IS_AFFECTED(ch, AFF_INFRARED))
	{
		vision += 50;
	}

	vision += race_table[ch->race].vision * 50;

	pop_call();
	return vision;
}

int get_sight(CHAR_DATA *ch)
{
	int sight;

	push_call("get_sight(%p)",ch);

	sight = 200;

	pop_call();
	return sight;
}

int get_room_light(CHAR_DATA *ch, ROOM_INDEX_DATA *room, bool door)
{
	int light;

	push_call("get_room_light(%p,%p,%p)",ch,room,door);

	light = room->light;

	if (IS_SET(room->room_flags, ROOM_SMOKE))
	{
		light -= 50;
	}

	if (IS_SET(room->room_flags, ROOM_DARK))
	{
		light -= 200;
	}
	else if (!IS_SET(sector_table[room->sector_type].flags, SFLAG_NOWEATHER) && !IS_SET(room->room_flags, ROOM_INDOORS))
	{
		switch (room->area->weather_info->sky)
		{
			case SKY_CLOUDLESS:
				light += sector_table[room->sector_type].light;
				break;
			case SKY_CLOUDY:
				light += sector_table[room->sector_type].light / 2;
				break;
			case SKY_RAINING:
			case SKY_LIGHTNING:
				light += sector_table[room->sector_type].light / 4;
				break;
		}
		switch (mud->sunlight)
		{
			case SUN_DARK:
				light -= 100;
				break;
			case SUN_SET:
			case SUN_RISE:
				light -= 50;
				break;
		}
	}
	else
	{
		light += sector_table[room->sector_type].light;
	}

	light = URANGE(-200, light, 200);

	if (door == FALSE)
	{
		pop_call();
		return light;
	}

	for (door = 0 ; door < 6 ; door++)
	{
		if (!is_valid_exit(ch, room, door))
		{
			continue;
		}
		if (!can_use_exit(ch, room->exit[door]))
		{
			continue;
		}
		light = UMAX(light, get_room_light(ch, room_index[room->exit[door]->to_room], FALSE) - 50);
	}
	pop_call();
	return light;
}

bool can_see_in_room( CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
	int vision, light;

	push_call("can_see_in_room(%p,%p)",ch,room);

	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT))
	{
		pop_call();
		return TRUE;
	}

	if (IS_SET(room->room_flags, ROOM_GLOBE) && race_table[ch->race].vision < 2 && !IS_AFFECTED(ch, AFF_ETHEREAL))
	{
		pop_call();
		return FALSE;
	}

	if (IS_AFFECTED(ch, AFF_TRUESIGHT))
	{
		pop_call();
		return TRUE;
	}

	if (IS_AFFECTED(ch, AFF_BLIND))
	{
		pop_call();
		return FALSE;
	}

	if (IS_NPC(ch) && ch->desc == NULL)
	{
		pop_call();
		return TRUE;
	}

	vision = get_vision(ch);
	light  = get_room_light(ch, ch->in_room, TRUE);

	if (ch->in_room == room)
	{
		if (vision + light >= 0)
		{
			pop_call();
			return TRUE;
		}
	}
	else
	{
		light = UMIN(light, light + get_room_light(ch, room, TRUE));

		if (vision + light >= 0)
		{
			pop_call();
			return TRUE;
		}
	}
	pop_call();
	return FALSE;
}

int obj_level_estimate(OBJ_INDEX_DATA *objIndex)
{
	int lvl_neg, lvl_pos, lvl;

	lvl = obj_level(objIndex, &lvl_neg, &lvl_pos);

	return lvl;
}

int obj_level(OBJ_INDEX_DATA *objIndex, int *lvl_neg, int *lvl_pos)
{
	AFFECT_DATA *aff;
	int level;
	int value[4];

	push_call("obj_level(%p)",objIndex);

	*lvl_neg = 0;
	*lvl_pos = 1;

	for (aff = objIndex->first_affect ; aff ; aff = aff->next)
	{
		switch (aff->location)
		{
			case APPLY_STR:
			case APPLY_WIS:
			case APPLY_CON:
			case APPLY_DEX:
			case APPLY_INT:
				if (aff->modifier > 0)
				{
					*lvl_pos += aff->modifier * aff->modifier * 4/3;
				}
				else
				{
					*lvl_neg += aff->modifier * aff->modifier / 2;
				}
				break;
			case APPLY_MANA:
				if (aff->modifier > 0)
				{
					*lvl_pos += aff->modifier / 3;
				}
				break;
			case APPLY_HIT:
				if (aff->modifier > 0)
				{
					*lvl_pos += aff->modifier / 2;
				}
				else
				{
					*lvl_neg += aff->modifier / 4;
				}
				break;
			case APPLY_MOVE:
				if (aff->modifier > 0)
				{
					*lvl_pos += aff->modifier / 4;
				}
				break;
			case APPLY_AC:
				if (aff->modifier < 0)
				{
					*lvl_pos += aff->modifier * aff->modifier / 4;
				}
				else
				{
					*lvl_neg += aff->modifier / 2;
				}
				break;
			case APPLY_HITROLL:
				if (aff->modifier > 0)
				{
					*lvl_pos += aff->modifier * aff->modifier / 2;
				}
				break;
			case APPLY_DAMROLL:
				if (aff->modifier > 0)
				{
					*lvl_pos += aff->modifier * aff->modifier;
				}
				break;
			case APPLY_SAVING_PARA:
			case APPLY_SAVING_ROD:
			case APPLY_SAVING_PETRI:
			case APPLY_SAVING_BREATH:
			case APPLY_SAVING_SPELL:
				if (aff->modifier < 0)
				{
					*lvl_pos += aff->modifier * aff->modifier / 3;
				}
				else
				{
					*lvl_neg += aff->modifier * aff->modifier / 6;
				}
				break;
			default:
				break;
		}
	}

	value[0] = objIndex->value[0];
	value[1] = objIndex->value[1];
	value[2] = objIndex->value[2];
	value[3] = objIndex->value[3];

	switch(objIndex->item_type)
	{
		case ITEM_LIGHT:
			if (value[0] > 0)
			{
				*lvl_pos += value[0] / 10;
			}
			break;
		case ITEM_SCROLL:
			if (value[1] > 0 && value[1] < MAX_SKILL)
			{
				*lvl_pos += value[0] / 2;
			}
			if (value[2] > 0 && value[1] < MAX_SKILL)
			{
				*lvl_pos += value[0] / 2;
			}
			if (value[3] > 0 && value[1] < MAX_SKILL)
			{
				*lvl_pos += value[0] / 2;
			}
			break;
		case ITEM_POTION:
			if (value[1] > 0 && value[1] < MAX_SKILL)
			{
				*lvl_pos += value[0] / 2;
			}
			if (value[2] > 0 && value[1] < MAX_SKILL)
			{
				*lvl_pos += value[0] / 2;
			}
			if (value[3] > 0 && value[1] < MAX_SKILL)
			{
				*lvl_pos += value[0] / 2;
			}
			break;
		case ITEM_PILL:
			if (value[1] > 0 && value[1] < MAX_SKILL)
			{
				*lvl_pos += value[0] / 2;
			}
			if (value[2] > 0 && value[1] < MAX_SKILL)
			{
				*lvl_pos += value[0] / 2;
			}
			if (value[3] > 0 && value[1] < MAX_SKILL)
			{
				*lvl_pos += value[0] / 2;
			}
			break;
		case ITEM_WAND:
			*lvl_pos += value[0]*value[1] / 4;
			break;
		case ITEM_STAFF:
			*lvl_pos += value[0]*value[1] / 6;
			break;
		case ITEM_WEAPON:
			*lvl_pos += (5*(value[1]*(1+(value[2]-1)/2.0))/2) - 10;
			break;
		case ITEM_ARMOR:
			if (value[0] > 0)
			{
				*lvl_pos += value[0] * value[0] / 5 + 1;
			}
			else
			{
				*lvl_neg += value[0] * value[0] / 5 + 0;
			}
			break;
		case ITEM_LOCKPICK:
			*lvl_pos += value[0] /  2;
			*lvl_pos += value[1] / 10;
			*lvl_pos += IS_SET(value[2], PICK_MAGICAL_LOCK)    ? 30 : 0;
			*lvl_pos += IS_SET(value[2], PICK_TRAPPED_LOCK)    ? 30 : 0;
			*lvl_pos += IS_SET(value[2], PICK_MECHANICAL_LOCK) ? 30 : 0;
			break;
		case ITEM_AMMO:
			*lvl_pos += value[3] * 2 + value[1] * value[2] / 20;
			break;
		case ITEM_CONTAINER:
			*lvl_pos += value[0] / 15;
			break;
		case ITEM_FOOD:
			*lvl_pos += value[0] / 4;
			break;

		case ITEM_MONEY:
		case ITEM_TREASURE:
			*lvl_pos += value[0] / 100;
			break;
		default:
			break;
	}
/*
	*lvl_neg += objIndex->weight / 10;
*/
	if (HAS_BIT(objIndex->extra_flags, ITEM_ANTI_GOOD))
	{
		*lvl_neg += (*lvl_pos / 10);
	}

	if (HAS_BIT(objIndex->extra_flags, ITEM_ANTI_EVIL))
	{
		*lvl_neg += (*lvl_pos / 10);
	}

	if (HAS_BIT(objIndex->extra_flags, ITEM_ANTI_NEUTRAL))
	{
		*lvl_neg += (*lvl_pos / 10);
	}

	if (HAS_BIT(objIndex->extra_flags, ITEM_AUTO_ENGRAVE))
	{
		*lvl_neg += (*lvl_pos / 10);
	}

	if (HAS_BIT(objIndex->extra_flags, ITEM_MYTHICAL))
	{
		*lvl_neg += (*lvl_pos / 10);
	}

	if (HAS_BIT(objIndex->extra_flags, ITEM_EPICAL))
	{
		*lvl_neg += (*lvl_pos / 5);
	}

	level = UMAX(1, *lvl_pos - *lvl_neg);

	pop_call();
	return level;
}

int obj_utility_level(CHAR_DATA *ch, OBJ_DATA *obj)
{
	OBJ_INDEX_DATA *objIndex = obj->pIndexData;
	AFFECT_DATA *aff;
	int level, fighter, mana, rape;
	int value[4];
	int lvl_pos, lvl_neg;

	push_call("obj_utility_level(%p,%p)",ch,objIndex);

	lvl_neg = 0;
	lvl_pos = 1;
	rape    = 0;

	fighter = (ch->pcdata->mclass[0] + ch->pcdata->mclass[1] + ch->pcdata->mclass[2] + ch->pcdata->mclass[3] + ch->pcdata->mclass[6]) >
		(ch->pcdata->mclass[4] + ch->pcdata->mclass[5] + ch->pcdata->mclass[7]) ? 1 : 0;

	mana = ch->pcdata->mclass[CLASS_MONK] >= 45 ? 1 : 0;

	aff = objIndex->first_affect;

	while (aff)
	{
		switch (aff->location)
		{
			case APPLY_STR:
				if (aff->modifier > 0)
				{
					if (get_curr_str(ch) < get_max_str(ch))
					{
						lvl_pos += aff->modifier * aff->modifier * 1.33;
					}
					else
					{
						lvl_pos += aff->modifier * aff->modifier;
					}
				}
				else
				{
					lvl_neg += aff->modifier * aff->modifier * 1.33;
				}
				break;

			case APPLY_WIS:
				if (aff->modifier > 0)
				{
					if (get_curr_wis(ch) < get_max_wis(ch))
					{
						lvl_pos += aff->modifier * aff->modifier * 1.33;
					}
					else
					{
						lvl_pos += aff->modifier * aff->modifier;
					}
				}
				else
				{
					lvl_neg += aff->modifier * aff->modifier * 1.33;
				}
				break;
			case APPLY_CON:
				if (aff->modifier > 0)
				{
					if (get_curr_con(ch) < get_max_con(ch))
					{
						lvl_pos += aff->modifier * aff->modifier * 1.33;
					}
					else
					{
						lvl_pos += aff->modifier * aff->modifier;
					}
				}
				else
				{
					lvl_neg += aff->modifier * aff->modifier * 1.33;
				}
				break;
			case APPLY_DEX:
				if (aff->modifier > 0)
				{
					if (get_curr_dex(ch) < get_max_dex(ch))
					{
						lvl_pos += aff->modifier * aff->modifier * 1.33;
					}
					else
					{
						lvl_pos += aff->modifier * aff->modifier;
					}
				}
				else
				{
					lvl_neg += aff->modifier * aff->modifier * 1.33;
				}
				break;
			case APPLY_INT:
				if (aff->modifier > 0)
				{
					if (get_curr_int(ch) < get_max_int(ch))
					{
						lvl_pos += aff->modifier * aff->modifier * 1.33;
					}
					else
					{
						lvl_pos += aff->modifier * aff->modifier;
					}
				}
				else
				{
					lvl_neg += aff->modifier * aff->modifier * 1.33;
				}
				break;
			case APPLY_MANA:
				if (mana)
				{
					if (aff->modifier > 0)
					{
						lvl_pos += aff->modifier / 3;
					}
					else
					{
						lvl_neg += aff->modifier / 3;
					}
				}
				break;
			case APPLY_HIT:
				if (aff->modifier > 0)
				{
					lvl_pos += aff->modifier / 2;
				}
				else
				{
					lvl_neg += aff->modifier / 2;
				}
				break;
			case APPLY_MOVE:
				if (aff->modifier > 0)
				{
					lvl_pos += aff->modifier / 4;
				}
				else
				{
					lvl_neg += aff->modifier / 4;
				}
				break;
			case APPLY_AC:
				if (fighter)
				{
					if (aff->modifier < 0)
					{
						lvl_pos += aff->modifier * aff->modifier / 8;
					}
					else
					{
						lvl_neg += aff->modifier * aff->modifier / 8;
					}
				}
				else
				{
					if (aff->modifier < 0)
					{
						lvl_pos += aff->modifier * aff->modifier / 4;
					}
					else
					{
						lvl_neg += aff->modifier / 4;
					}
				}
				break;
			case APPLY_HITROLL:
				if (fighter)
				{
					if (aff->modifier > 0)
					{
						lvl_pos += aff->modifier * aff->modifier / 2;
					}
				}
				break;
			case APPLY_DAMROLL:
				if (fighter)
				{
					if (aff->modifier > 0)
					{
						lvl_pos += aff->modifier * aff->modifier;
					}
				}
				break;
			case APPLY_SAVING_PARA:
			case APPLY_SAVING_ROD:
			case APPLY_SAVING_PETRI:
			case APPLY_SAVING_BREATH:
			case APPLY_SAVING_SPELL:
				if (aff->modifier < 0)
				{
					lvl_pos += aff->modifier * aff->modifier / 3;
				}
				else
				{
					lvl_neg += aff->modifier * aff->modifier / 3;
				}
				break;
			default:
				break;
		}
		aff = aff->next;

		if (aff == NULL && rape == 0)
		{
			rape = 1;

			aff = obj->first_affect;
		}
	}

	value[0] = obj->value[0];
	value[1] = obj->value[1];
	value[2] = obj->value[2];
	value[3] = obj->value[3];

	switch(obj->item_type)
	{
		case ITEM_LIGHT:
			if (value[0] > 0)
			{
				lvl_pos += value[0] / 10;
			}
			break;
		case ITEM_SCROLL:
			if (value[1] > 0 && value[1] < MAX_SKILL)
			{
				lvl_pos += value[0] / 4;
			}
			if (value[2] > 0 && value[1] < MAX_SKILL)
			{
				lvl_pos += value[0] / 4;
			}
			if (value[3] > 0 && value[1] < MAX_SKILL)
			{
				lvl_pos += value[0] / 4;
			}
			break;
		case ITEM_POTION:
			if (value[1] > 0 && value[1] < MAX_SKILL)
			{
				lvl_pos += value[0] / 4;
			}
			if (value[2] > 0 && value[1] < MAX_SKILL)
			{
				lvl_pos += value[0] / 4;
			}
			if (value[3] > 0 && value[1] < MAX_SKILL)
			{
				lvl_pos += value[0] / 4;
			}
			break;
		case ITEM_PILL:
			if (value[1] > 0 && value[1] < MAX_SKILL)
			{
				lvl_pos += value[0] / 4;
			}
			if (value[2] > 0 && value[1] < MAX_SKILL)
			{
				lvl_pos += value[0] / 4;
			}
			if (value[3] > 0 && value[1] < MAX_SKILL)
			{
				lvl_pos += value[0] / 4;
			}
			break;
		case ITEM_WAND:
			lvl_pos += value[0] * value[1] / 8;
			break;
		case ITEM_STAFF:
			lvl_pos += value[0] * value[1] / 12;
			break;
		case ITEM_WEAPON:
			if (fighter)
			{
				lvl_pos += (100 + weapon_skill(ch, value[0])) * ((5 * (value[1] * (1 + (value[2] - 1) / 2.0)) / 2) - 10) / 200;
			}
			break;
		case ITEM_ARMOR:
			if (value[0] > 0)
			{
				if (fighter)
				{
					lvl_pos += value[0] * value[0] / 10 + 1;
				}
				else
				{
					lvl_pos += value[0] * value[0] / 5 + 1;
				}
			}
			else
			{
				if (fighter)
				{
					lvl_neg += value[0] * value[0] / 10 + 1;
				}
				else
				{
					lvl_neg += value[0] * value[0] / 5 + 1;
				}
			}
			break;
		case ITEM_LOCKPICK:
			lvl_pos += value[0] /  4;
			lvl_pos += value[1] / 20;
			lvl_pos += IS_SET(value[2], PICK_MAGICAL_LOCK)    ? 15 : 0;
			lvl_pos += IS_SET(value[2], PICK_TRAPPED_LOCK)    ? 15 : 0;
			lvl_pos += IS_SET(value[2], PICK_MECHANICAL_LOCK) ? 15 : 0;
			break;
		case ITEM_AMMO:
			lvl_pos += value[3] * 2 + value[1] * value[2] / 40;
			break;
		case ITEM_CONTAINER:
			lvl_pos += value[0] / 30;
			break;
		case ITEM_FOOD:
			lvl_pos += value[0] / 8;
			break;

		case ITEM_MONEY:
		case ITEM_TREASURE:
			lvl_pos += value[0] / 200;
			break;
		default:
			break;
	}

//	lvl_neg += objIndex->weight / 10;

	level = UMAX(1, lvl_pos - lvl_neg);

	pop_call();
	return level;
}


bool blocking(CHAR_DATA *victim,CHAR_DATA *ch)
{
	push_call("blocking(%p,%p)",victim,ch);

	if (IS_NPC(ch) || IS_NPC(victim))
	{
		pop_call();
		return FALSE;
	}

	if (ch->level >= LEVEL_IMMORTAL)
	{
		pop_call();
		return FALSE;
	}

	if (ch->desc == NULL || victim->desc == NULL)
	{
		pop_call();
		return TRUE;
 	}

	if (IS_SET(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_MUTED))
	{
		pop_call();
		return TRUE;
	}

	if (is_name(ch->name, victim->pcdata->block_list))
	{
		pop_call();
		return TRUE;
	}
	pop_call();
	return FALSE;
}


bool in_camp(CHAR_DATA *ch)
{
	CHAR_DATA *gch;

	push_call("in_camp(%p)",ch);

	if ( ch->in_room == NULL )
	{
		pop_call();
		return( FALSE );
	}

	for (gch=ch->in_room->first_person ; gch != NULL ; gch = gch->next_in_room)
	{
		if (is_same_group(ch, gch) && IS_AFFECTED(gch, AFF2_CAMPING))
		{
			pop_call();
			return TRUE;
		}
	}
	pop_call();
	return FALSE;
}



char * const honorific_table[] =
{
	"Knight ",       "Hunter ",          "Dragoon ",         "Woodsman ",
	"Marshall ",     "Huntress ",        "Guardian ",        "Amazon ",

	"Champion ",     "Sword Master ",    "Hoplite ",         "Quick Blade ",
	"Charioteer ",   "Sword Mistress ",  "Myrmidon ",        "Swift Blade ",

	"Swashbuckler ", "Ruffian ",         "Mercenary ",       "Death Blade ",
	"Rogue ",        "Scoundrell ",      "Vixen ",           "Temptress ",

	"Assassin ",     "Soul Stealer ",    "Sensei ",          "Shinobi ",
	"Assassin ",     "Night Blade ",     "Kaiden ",          "Geisha ",

	"Elementalist ", "Range Caster ",    "Elementalist ",    "Pyromancer ",
	"Enchantress ",  "Rune Caster ",     "Enchantress ",     "Stormbringer ",

	"Wizard ",       "Invoker ",         "Spell Binder ",    "Magus ",
	"Sorceress ",    "Siren ",           "Dream Weaver ",    "Nightmare ",

	"Diviner ",      "Monk ",            "Sacristan ",       "Father ",
	"Oracle ",       "Nun ",             "Acolyte ",         "Mother ",

	"Dread Mage ",   "Iconoclast ",      "Reaper ",          "Warlock ",
	"Lich Queen ",   "Hell Spawn ",      "Succubus ",        "Witch "
};

void do_honorific( CHAR_DATA *ch, char *argument )
{
	int xxx, cnt;
	char buf[MAX_STRING_LENGTH];

	push_call("do_prefix(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	xxx = URANGE(0, ch->sex-1, 1) * 4;

	if (argument[0] == '\0')
	{
		strcpy(buf, "Set what honorific? Choose from the following list:\n\r\n\r");

		for (cnt = 0 ; cnt < MAX_CLASS ; cnt++)
		{
			if (ch->pcdata->mclass[cnt] > 0)
			{
				cat_sprintf(buf, "    %-15s", honorific_table[cnt*8+xxx+0]);
				cat_sprintf(buf, "    %-15s", honorific_table[cnt*8+xxx+1]);
				cat_sprintf(buf, "    %-15s", honorific_table[cnt*8+xxx+2]);
				cat_sprintf(buf, "    %-15s", honorific_table[cnt*8+xxx+3]);
				cat_sprintf(buf, "\n\r");
			}
		}
		send_to_char(buf, ch);
		pop_call();
		return;
	}

	for (cnt = 0 ; cnt < 64 ; cnt++)
	{
		if (!str_prefix(argument, honorific_table[cnt]))
		{
			if (ch->pcdata->mclass[cnt/8] > 0)
			{
				break;
			}
		}
	}

	if (cnt >= 64)
	{
		do_honorific(ch, "");
		pop_call();
		return;
	}

	ch->pcdata->honorific = (cnt/8*8) + (cnt % 4);

	ch_printf(ch, "You will now be known as '%s'.\n\r", get_name(ch));

	pop_call();
	return;
}

char *get_god_name(int god)
{
	return (char *) god_table[god].name;
}

char *get_name( CHAR_DATA *ch )
{
	static char get_name_buffer[2][MAX_INPUT_LENGTH];
	static int cnt;

	push_call("get_name(%p)",ch);

	cnt = (cnt + 1) % 2;

	get_name_buffer[cnt][0] = '\0';

	if (!IS_NPC(ch))
	{
		if (ch->level >= MAX_LEVEL)
		{
			if (IS_GOD(ch))
			{
				if (!strcasecmp(ch->name, "chaos"))
				{
					strcpy(get_name_buffer[cnt], "Lord ");
				}
				if (!strcasecmp(ch->name, "order"))
				{
					strcpy(get_name_buffer[cnt], "Lord ");
				}

				if (!strcasecmp(ch->name, "manwe"))
				{
					switch(number_range(1, 4))
					{
						case 1 : strcpy (get_name_buffer[cnt], "Lord of the Wind ");break;
						case 2 : strcpy (get_name_buffer[cnt], "Cloud Chaser ");	break;
						case 3 : strcpy (get_name_buffer[cnt], "Stormbringer ");	break;
						case 4 : strcpy (get_name_buffer[cnt], "Lord of Birds ");	break;
					}
				}
				if (!strcasecmp(ch->name, "demise"))
				{
					switch(number_range(1, 5))
					{
						case 1 : strcpy(get_name_buffer[cnt],"Fire Starter ");		break;
						case 2 : strcpy(get_name_buffer[cnt],"Pyromaniac ");		break;
						case 3 : strcpy(get_name_buffer[cnt],"Lord of Fire ");		break;
						case 4 : strcpy(get_name_buffer[cnt],"Hot Hot ");			break;
						case 5 : strcpy(get_name_buffer[cnt],"Fireproof ");		break;
					}
				}
				if (!strcasecmp(ch->name, "hypnos"))
				{
					switch(number_range(1, 4))
					{
						case 1 : strcpy(get_name_buffer[cnt], "Mad Scientist ");	break;
						case 2 : strcpy(get_name_buffer[cnt], "Sleepbringer ");	break;
						case 3 : strcpy(get_name_buffer[cnt], "Sandman ");		break;
						case 4 : strcpy(get_name_buffer[cnt], "Dream Stalker ");	break;
					}
				}
				if (!strcasecmp(ch->name, "ulmo"))
				{
					switch(number_range(1, 2))
					{
						case 1 : strcpy(get_name_buffer[cnt], "Cold Bringer ");	break;
						case 2 : strcpy(get_name_buffer[cnt], "Lord of the Seas ");	break;
					}
				}
				if (!strcasecmp(ch->name, "gaia"))
				{
					switch(number_range(1, 3))
					{
						case 1 : strcpy(get_name_buffer[cnt], "Goddess of Earth ");	break;
						case 2 : strcpy(get_name_buffer[cnt], "Green Lady ");		break;
						case 3 : strcpy(get_name_buffer[cnt], "Mother Nature ");	break;
					}
				}
			}
			else
			{
				bug("Illegal GOD: %s (%d)", ch->name, ch->pcdata->pvnum);
				pop_call();
				return(ch->name);
			}
		}
		else if (ch->level > 97)
		{
			strcpy(get_name_buffer[cnt], "Archangel ");
		}
		else if (ch->level > 96)
		{
			strcpy(get_name_buffer[cnt], "Angel ");
		}
		else if (ch->level > 95)
		{
			strcpy(get_name_buffer[cnt], ch->sex == SEX_FEMALE ? "Dutchess " : "Duke ");
		}
		else if (ch->level > 90)
		{
			strcpy(get_name_buffer[cnt], honorific_table[ch->pcdata->honorific + URANGE(0, ch->sex-1, 1) * 4]);
		}
		else if (ch->level > 89)
		{
			strcpy(get_name_buffer[cnt], ch->sex == SEX_FEMALE ? "Lady " : "Sir " );
		}
		strcat(get_name_buffer[cnt], ch->name);

		pop_call();
		return( get_name_buffer[cnt] );
	}

	if (ch->short_descr)
	{
		strcpy( get_name_buffer[cnt], ch->short_descr );

		pop_call();
		return( get_name_buffer[cnt] );
	}
	else
	{
		log_printf("get_name: mob %d has NULL short_descr", ch->pIndexData->vnum);

		pop_call();
		return "(NULL)";
	}
}


int GET_HITROLL( CHAR_DATA *ch )
{
	int val, tmp, max;

	push_call("GET_HITROLL(%p)",ch);

	val = ch->hitroll + str_app[get_curr_str(ch)].tohit;

	if (IS_NPC(ch))
	{
		pop_call();
		return(val);
	}
	tmp = ch->pcdata->eqhitroll;

	max = ch->level * 150 / 95 + 25 + ch->pcdata->reincarnation * 5;

	if (tmp > max)
	{
		val -= tmp - max;
	}
	pop_call();
	return val;
}

int GET_DAMROLL( CHAR_DATA *ch )
{
	int val, tmp, max;

	push_call("GET_DAMROLL(%p)",ch);

	val = ch->damroll + str_app[get_curr_str(ch)].todam;

	if (IS_NPC(ch))
	{
		pop_call();
		return(val);
	}

	tmp = ch->pcdata->eqdamroll;

	max = ch->level * 150 / 95 + 15 + ch->pcdata->reincarnation * 5;

	if (tmp > max)
	{
		val -= tmp - max;
	}
	pop_call();
	return val;
}

int GET_SAVING_THROW( CHAR_DATA *ch )
{
	int val, max;

	push_call("GET_SAVING_THROW(%p)",ch);

	val = ch->saving_throw - get_curr_wis(ch) / 4;

	if (IS_NPC(ch))
	{
		pop_call();
		return(val);
	}

	max  = 0;
	max -= ch->level        / 3;
	max -= get_curr_wis(ch) * 2 / 3;

	if (val < max)
	{
		val = max;
	}

	pop_call();
	return( val );
}

ROOM_INDEX_DATA *get_random_room_index( CHAR_DATA *ch, int lo_room, int hi_room )
{
	ROOM_INDEX_DATA *pRoomIndex;
	int cnt;

	push_call("get_random_room(%p,%d,%d)",ch,lo_room,hi_room);

	for (cnt = 0 ; cnt < 10000 ; cnt++)
	{
		pRoomIndex = get_room_index(number_range(lo_room, hi_room));

		if (pRoomIndex)
		{
			if (hi_room - lo_room > 10000)
			{
				if (IS_SET(pRoomIndex->area->flags, AFLAG_NOTELEPORT))
				{
					continue;
				}
				if (IS_SET(pRoomIndex->area->flags, AFLAG_NORECALL))
				{
					continue;
				}
				if (IS_SET(pRoomIndex->area->flags, AFLAG_NORIP))
				{
					continue;
				}
				if (pRoomIndex->area->average_level*3 > ch->level+15)
				{
					continue;
				}
			}
			if (IS_SET(pRoomIndex->room_flags, ROOM_NO_RECALL))
			{
				continue;
			}
			if (IS_SET(pRoomIndex->room_flags, ROOM_NO_RIP))
			{
				continue;
			}
			if (IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE))
			{
				continue;
			}
			if (IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY))
			{
				continue;
			}
			if (IS_SET(pRoomIndex->room_flags, ROOM_NO_SAVE))
			{
				continue;
			}
			if (IS_SET(pRoomIndex->room_flags, ROOM_IS_CASTLE))
			{
				continue;
			}
			if (IS_SET(pRoomIndex->room_flags, ROOM_RIP))
			{
				continue;
			}
			if (pRoomIndex->sector_type == SECT_ETHEREAL)
			{
				continue;
			}
			if (pRoomIndex->sector_type == SECT_ASTRAL)
			{
				continue;
			}
			if (pRoomIndex->area->low_hard_range != 0 || pRoomIndex->area->hi_hard_range != 0)
			{
				if (!IS_NPC(ch) && ch->level < LEVEL_IMMORTAL)
				{
					if (ch->level < pRoomIndex->area->low_hard_range)
					{
						continue;
					}
					if (ch->level > pRoomIndex->area->hi_hard_range)
					{
						continue;
					}
				}
			}
			break;
		}
	}
	if (cnt >= 10000)
	{
		log_printf("get_random_room(%s) failure", get_name(ch));
		pop_call();
		return NULL;
	}
	pop_call();
	return pRoomIndex;
}


EXIT_DATA *get_exit( int vnum, bool door )
{
	push_call("get_exit(%p,%p)",vnum,door);

	if (room_index[vnum]->exit[door] == NULL)
	{
		pop_call();
		return NULL;
	}

	if (room_index[room_index[vnum]->exit[door]->to_room] == NULL)
	{
		pop_call();
		return NULL;
	}

	pop_call();
	return room_index[vnum]->exit[door];
}


bool can_use_exit( CHAR_DATA *ch, EXIT_DATA *exit )
{
//	int pvnum;

	push_call("can_use_exit(%p,%p)",ch,exit);

//	pvnum = room_index[exit->to_room]->creator_pvnum;

	if (IS_GOD(ch) && IS_SET(ch->act, PLR_HOLYLIGHT))
	{
		pop_call();
		return TRUE;
	}

	if (IS_SET(exit->flags, EX_RIP) && !pvnum_in_group(ch, room_index[exit->to_room]->creator_pvnum))
	{
		pop_call();
		return FALSE;
	}

	if (IS_SET(exit->flags, EX_BACKDOOR) && !pvnum_in_group(ch, ch->in_room->creator_pvnum))
	{
		pop_call();
		return FALSE;
	}

	if (!IS_NPC(ch) && IS_SET(exit->flags, EX_CLAN_BACKDOOR))
	{
		if (get_clan_from_vnum(ch->in_room->creator_pvnum) != ch->pcdata->clan)
		{
			pop_call();
			return FALSE;
		}
	}
	pop_call();
	return TRUE;
}

bool is_valid_exit( CHAR_DATA *ch, ROOM_INDEX_DATA *room, bool dir )
{
	EXIT_DATA *pexit;

	push_call("is_valid_exit(%p,%p,%p)", ch, room, dir);

	if ((pexit = get_exit(room->vnum, dir)) == NULL)
	{
		pop_call();
		return( FALSE );
	}

	if (IS_SET(pexit->flags, EX_CLOSED))
	{
		pop_call();
		return( FALSE );
	}

	if (IS_SET(pexit->flags, EX_HIDDEN) && !can_see_hidden(ch))
	{
		pop_call();
		return FALSE;
	}

	if (!can_use_exit(ch, pexit))
	{
		pop_call();
		return FALSE;
	}

	pop_call();
	return( TRUE );
}
