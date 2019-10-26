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
	Wiped clean from old stuff, asuming it will be stored somewhere in
	a backup file or something - Scandum
*/

/*
	predeclarations
*/

bool    cast_spell      args( (CHAR_DATA *ch,char *spell) );
void    set_fighting    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    do_kill         args( ( CHAR_DATA *ch, char *argument ) );

/*
	The following special functions are available for mobiles.
*/

DECLARE_SPEC_FUN(	spec_breath_any		);
DECLARE_SPEC_FUN(	spec_breath_acid		);
DECLARE_SPEC_FUN(	spec_breath_fire		);
DECLARE_SPEC_FUN(	spec_breath_frost		);
DECLARE_SPEC_FUN(	spec_breath_gas		);
DECLARE_SPEC_FUN(	spec_breath_lightning	);
DECLARE_SPEC_FUN(	spec_cast_adept		);
DECLARE_SPEC_FUN(	spec_cast_cleric		);
DECLARE_SPEC_FUN(	spec_cast_judge		);
DECLARE_SPEC_FUN(	spec_cast_mage			);
DECLARE_SPEC_FUN(	spec_cast_undead		);
DECLARE_SPEC_FUN(	spec_executioner		);
DECLARE_SPEC_FUN(	spec_fido				);
DECLARE_SPEC_FUN(	spec_guard			);
DECLARE_SPEC_FUN(	spec_janitor			);
DECLARE_SPEC_FUN(	spec_poison			);
DECLARE_SPEC_FUN(	spec_thief			);

struct spec_fun_type
{
	char *name;
	SPEC_FUN *function;
};

const struct spec_fun_type spec_fun[]=
{
	{"spec_guard",			spec_guard},
	{"",					NULL}
};

/*
	Given a name, Return the appropriate spec fun.
*/

SPEC_FUN *spec_lookup( const char *name )
{
	int i;

	push_call("spec_lookup(%p)",name);

	for (i = 0 ; *spec_fun[i].name != 0 ; i++)
	{
		if (strcmp(name, spec_fun[i].name))
		{
			pop_call();
			return spec_fun[i].function;
		}
	}
	pop_call();
	return NULL;
}

/*
	Given a spec function, Return the appropriate name.
*/

char *spec_name_lookup( SPEC_FUN *fun )
{
	int i;

	push_call("spec_name(%p)",fun);

	for (i = 0 ; *spec_fun[i].name != 0 ; i++)
	{
		if (spec_fun[i].function == fun)
		{
			pop_call();
			return spec_fun[i].name;
		}
	}
	pop_call();
	return 0;
}

/*
	Core procedure for dragons.
*/

bool dragon( CHAR_DATA *ch, char *spell_name )
{
	CHAR_DATA *victim;
	CHAR_DATA *v_next;
	int sn;

	push_call("dragon(%p,%p)",ch,spell_name);

	if (ch->position != POS_FIGHTING || (number_bits(3) != 0))
	{
		pop_call();
		return FALSE;
	}
	for (victim = ch->in_room->first_person ; victim != NULL ; victim = v_next)
	{
		v_next = victim->next_in_room;
		if (victim->fighting && victim->fighting->who == ch && number_bits(2) == 0)
		{
			break;
		}
	}
	if (victim == NULL)
	{
		pop_call();
		return FALSE;
	}
	if ((sn = skill_lookup(spell_name)) < 0)
	{
		pop_call();
		return FALSE;
	}
	(*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim, 0 );

	pop_call();
	return TRUE;
}

/*
	Special procedures for mobiles.
*/

bool spec_breath_any( CHAR_DATA *ch )
{
	push_call("spec_breath_any(%p)",ch);

	if (ch->position != POS_FIGHTING || (number_bits(2) != 0))
	{
		pop_call();
		return FALSE;
	}
	switch (number_range(1, 5))
	{
		case 1:
			pop_call();
			return spec_breath_fire(ch);
		case 2:
			pop_call();
			return spec_breath_lightning(ch);
		case 3:
			pop_call();
			return spec_breath_gas(ch);
		case 4:
			pop_call();
			return spec_breath_acid(ch);
		case 5:
			pop_call();
			return spec_breath_frost(ch);
	}
	pop_call();
	return FALSE;
}

bool spec_breath_acid( CHAR_DATA *ch )
{
	push_call("spec_breath_acid)",ch);

	pop_call();
	return dragon( ch, "acid breath" );
}



bool spec_breath_fire( CHAR_DATA *ch )
{
	push_call("spec_breath_fire(%p)",ch);

	pop_call();
	return dragon( ch, "fire breath" );
}

bool spec_breath_frost( CHAR_DATA *ch )
{
	push_call("spec_breath_frost(%p)",ch);

	pop_call();
	return dragon( ch, "frost breath" );
}



bool spec_breath_gas( CHAR_DATA *ch )
{
	push_call("spec_breath_gas(%p)",ch);

	pop_call();
	return dragon( ch, "gas breath" );
}

bool spec_breath_lightning( CHAR_DATA *ch )
{
	push_call("spec_breath_lightning(%p)",ch);

	pop_call();
	return dragon( ch, "lightning breath" );
}

bool spec_cast_healer( CHAR_DATA *ch )
{
	CHAR_DATA *victim;
	CHAR_DATA *v_next;

	push_call("spec_cast_healer(%p)",ch);

	if ( !IS_AWAKE(ch) )
	{
		pop_call();
		return FALSE;
	}
	for ( victim = ch->in_room->first_person; victim != NULL; victim = v_next )
	{
		v_next = victim->next_in_room;
		if ( !IS_NPC(victim) &&	victim != ch
			&& can_see( ch, victim ) && number_bits( 1 ) == 0 )
		{
			break;
		}
	}
	if ( victim == NULL )
	{
		pop_call();
		return FALSE;
	}
	switch ( number_bits( 3 ) )
	{
		case 0:
			act( "$n utters the word 'romra'.", ch, NULL, NULL, TO_ROOM );
			spell_armor( skill_lookup( "armor" ), ch->level, ch, victim, 0 );

			pop_call();
			return TRUE;

		case 1:
			act( "$n utters the word 'ewnam'.", ch, NULL, NULL, TO_ROOM );
			spell_bless( skill_lookup( "bless" ), ch->level, ch, victim, 0 );

			pop_call();
			return TRUE;

		case 2:
			act( "$n utters the word 'tolecnal'.", ch, NULL, NULL, TO_ROOM );
			spell_cure_blindness( skill_lookup( "cure blindness" ), ch->level, ch, victim, 0 );

			pop_call();
			return TRUE;

		case 3:
			act( "$n utters the word 'lydnelle'.", ch, NULL, NULL, TO_ROOM );
			spell_cure_light( skill_lookup( "cure light" ), ch->level, ch, victim, 0 );

			pop_call();
			return TRUE;

		case 4:
			act( "$n utters the word 'aiag'.", ch, NULL, NULL, TO_ROOM );
			spell_cure_poison( skill_lookup( "cure poison" ), ch->level, ch, victim, 0 );

			pop_call();
			return TRUE;

		case 5:
			act( "$n utters the word 'sonpyh'.", ch, NULL, NULL, TO_ROOM );
			spell_refresh( skill_lookup( "refresh" ), ch->level, ch, victim, 0 );

			pop_call();
			return TRUE;

		case 6:
			act( "$n utters the word 'esimed'.", ch, NULL, NULL, TO_ROOM);
			spell_refresh( skill_lookup( "refresh" ), ch->level, ch, victim, 0 );

			pop_call();
			return TRUE;

	}
	pop_call();
	return FALSE;
}

bool spec_cast_adept( CHAR_DATA *ch )
{
	CHAR_DATA *victim;
	CHAR_DATA *v_next;

	push_call("spec_cast_adept(%p)",ch);

	if ( !IS_AWAKE(ch) )
	{
		pop_call();
		return FALSE;
	}
	for ( victim = ch->in_room->first_person; victim != NULL; victim = v_next )
	{
		v_next = victim->next_in_room;
		if ( victim != ch && can_see( ch, victim ) && number_bits( 1 ) == 0 )
		{
			break;
		}
	}
	if ( victim == NULL )
	{
		pop_call();
		return FALSE;
	}
	switch ( number_bits( 3 ) )
	{
		case 0:
			act( "$n utters the word 'aiag'.", ch, NULL, NULL, TO_ROOM );
			spell_armor( skill_lookup( "armor" ), ch->level, ch, victim, 0 );

			pop_call();
			return TRUE;

		case 1:
			act( "$n utters the word 'sopnyh'.", ch, NULL, NULL, TO_ROOM );
			spell_bless( skill_lookup( "bless" ), ch->level, ch, victim, 0 );

			pop_call();
			return TRUE;

		case 2:
			act( "$n utters the word 'suirauqa'.", ch, NULL, NULL, TO_ROOM );
			spell_cure_blindness( skill_lookup( "cure blindness" ), ch->level, ch, victim, 0 );

			pop_call();
			return TRUE;

		case 3:
			act( "$n utters the word 'esimed'.", ch, NULL, NULL, TO_ROOM );
			spell_cure_light( skill_lookup( "cure light" ), ch->level, ch, victim, 0 );

			pop_call();
			return TRUE;

		case 4:
			act( "$n utters the word 'faldnag'.", ch, NULL, NULL, TO_ROOM );
			spell_cure_poison( skill_lookup( "cure poison" ), ch->level, ch, victim, 0 );

			pop_call();
			return TRUE;

		case 5:
			act( "$n utters the word 'ewnam'.", ch, NULL, NULL, TO_ROOM );
			spell_refresh( skill_lookup( "refresh" ), ch->level, ch, victim, 0 );

			pop_call();
			return TRUE;

		case 6:
			act( "$n utters the word 'madrettor'.", ch, NULL, NULL, TO_ROOM);
			spell_giant_strength( skill_lookup( "giant strength" ), ch->level, ch, victim, 0 );

			pop_call();
			return TRUE;

	}
	pop_call();
	return FALSE;
}

bool spec_cast_cleric( CHAR_DATA *ch )
{
	CHAR_DATA *victim;
	CHAR_DATA *v_next;
	char *spell;
	int sn;

	push_call("spec_cast_cleric(%p)",ch);

	if ( ch->position != POS_FIGHTING ||(number_bits(2)!=0))
	{
		pop_call();
		return FALSE;
	}

	for ( victim = ch->in_room->first_person; victim != NULL; victim = v_next )
	{
		v_next = victim->next_in_room;
		if ( victim->fighting && victim->fighting->who == ch && number_bits( 2 ) == 0 )
		{
			break;
		}
	}
	if ( victim == NULL )
	{
		pop_call();
		return FALSE;
	}
	for ( ;; )
	{
		int min_level;

		switch ( number_bits( 4 ) )
		{
			case  0: min_level =  0; spell = "blindness";      break;
			case  1: min_level =  3; spell = "cause serious";  break;
			case  2: min_level =  7; spell = "earthquake";     break;
			case  3: min_level =  9; spell = "cause critical"; break;
			case  4: min_level = 10; spell = "dispel evil";    break;
			case  5: min_level = 12; spell = "curse";          break;
			case  6: min_level = 12; spell = "change sex";     break;
			case  7: min_level = 13; spell = "flamestrike";    break;
			case  8:
			case  9:
			case 10: min_level = 15; spell = "harm";           break;
			default: min_level = 16; spell = "dispel magic";   break;
		}

		if ( ch->level >= min_level )
		{
			break;
		}
	}
	if ( ( sn = skill_lookup( spell ) ) < 0 )
	{
		pop_call();
		return FALSE;
	}

	(*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim, 0 );

	pop_call();
	return TRUE;
}

bool spec_cast_judge( CHAR_DATA *ch )
{
	CHAR_DATA *victim;
	CHAR_DATA *v_next;
	char *spell;
	int sn;

	push_call("spec_cast_judge(%p)",ch);

	if ( ch->position != POS_FIGHTING ||(number_bits(2)!=0))
	{
		pop_call();
		return FALSE;
	}
	for ( victim = ch->in_room->first_person; victim != NULL; victim = v_next )
	{
		v_next = victim->next_in_room;
		if ( victim->fighting && victim->fighting->who == ch && number_bits( 2 ) == 0 )
		{
			break;
		}
	}
	if ( victim == NULL )
	{
		pop_call();
		return FALSE;
	}

	spell = "high explosive";

	if ( ( sn = skill_lookup( spell ) ) < 0 )
	{
		pop_call();
		return FALSE;
	}

	(*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim, 0 );

	pop_call();
	return TRUE;
}

bool spec_cast_mage( CHAR_DATA *ch )
{
	CHAR_DATA *victim;
	CHAR_DATA *v_next;
	char *spell;
	int sn;

	push_call("spec_cast_mage(%p)",ch);

	if ( ch->position != POS_FIGHTING ||(number_bits(2)!=0))
	{
		pop_call();
		return FALSE;
	}
	for ( victim = ch->in_room->first_person; victim != NULL; victim = v_next )
	{
		v_next = victim->next_in_room;
		if ( victim->fighting && victim->fighting->who == ch && number_bits( 2 ) == 0 )
		{
			break;
		}
	}
	if ( victim == NULL )
	{
		pop_call();
		return FALSE;
	}
	for ( ;; )
	{
		int min_level;

		switch ( number_bits( 4 ) )
		{
			case  0: min_level =  0; spell = "blindness";      break;
			case  1: min_level =  3; spell = "chill touch";    break;
			case  2: min_level =  7; spell = "weaken";         break;
			case  3: min_level =  8; spell = "teleport";       break;
			case  4: min_level = 11; spell = "color spray";    break;
			case  5: min_level = 12; spell = "change sex";     break;
			case  6: min_level = 13; spell = "energy drain";   break;
			case  7:
			case  8:
			case  9: min_level = 15; spell = "fireball";       break;
			default: min_level = 20; spell = "acid blast";     break;
		}
		if ( ch->level >= min_level )
		{
			break;
		}
	}
	if ( ( sn = skill_lookup( spell ) ) < 0 )
	{
		pop_call();
		return FALSE;
	}

	(*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim, 0 );

	pop_call();
	return TRUE;
}

bool spec_cast_undead( CHAR_DATA *ch )
{
	CHAR_DATA *victim;
	CHAR_DATA *v_next;
	char *spell;
	int sn;

	push_call("spec_cast_undead(%p)",ch);

	if ( ch->position != POS_FIGHTING ||(number_bits(2)!=0))
	{
		pop_call();
		return FALSE;
	}
	for ( victim = ch->in_room->first_person; victim != NULL; victim = v_next )
	{
		v_next = victim->next_in_room;
		if ( victim->fighting && victim->fighting->who == ch && number_bits( 2 ) == 0 )
		{
			break;
		}
	}
	if ( victim == NULL )
	{
		pop_call();
		return FALSE;
	}
	for ( ;; )
	{
		int min_level;

		switch ( number_bits( 4 ) )
		{
			case  0: min_level =  0; spell = "curse";          break;
			case  1: min_level =  3; spell = "weaken";         break;
			case  2: min_level =  6; spell = "burning hands";  break;
			case  3: min_level =  9; spell = "blindness";      break;
			case  4: min_level = 12; spell = "poison";         break;
			case  5: min_level = 15; spell = "energy drain";   break;
			case  6: min_level = 18; spell = "harm";           break;
			case  7: min_level = 31; spell = "mage blast";     break;
			default: min_level = 54; spell = "vampiric touch"; break;
		}

		if ( ch->level >= min_level )
		{
			break;
		}
	}
	if ( ( sn = skill_lookup( spell ) ) < 0 )
	{
		pop_call();
		return FALSE;
	}

	(*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim, 0 );

	pop_call();
	return TRUE;
}

bool spec_executioner( CHAR_DATA *ch )
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *v_next;
	char *crime;

	push_call("spec_executioner(%p)",ch);

	if ( !IS_AWAKE(ch) || ch->fighting != NULL
		|| IS_SET( ch->in_room->room_flags, ROOM_SAFE))
	{
		pop_call();
		return FALSE;
	}
	crime = "";
	for ( victim = ch->in_room->first_person; victim != NULL; victim = v_next )
	{
		v_next = victim->next_in_room;

		if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER) )
		{
			crime = "MURDERER"; break;
		}
		if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF) )
		{
			crime = "THIEF"; break;
		}
	}
	if ( victim == NULL )
	{
		pop_call();
		return FALSE;
	}
	sprintf( buf, "%s is a %s!  PROTECT THE INNOCENT!  MORE BLOOOOD!!!", victim->name, crime );
	do_shout( ch, buf );
	multi_hit( ch, victim, TYPE_UNDEFINED );

	pop_call();
	return TRUE;
}


bool spec_fido( CHAR_DATA *ch )
{
	OBJ_DATA *corpse;

	push_call("spec_fido(%p)",ch);

	if (!IS_AWAKE(ch))
	{
		pop_call();
		return FALSE;
	}

	for (corpse = ch->in_room->first_content ; corpse ; corpse = corpse->next_content)
	{
		if (corpse->item_type != ITEM_CORPSE_NPC)
		{
			continue;
		}
		act("$n savagely devours a corpse.", ch, NULL, NULL, TO_ROOM);

		while (corpse->first_content)
		{
			corpse->first_content->sac_timer = OBJ_SAC_TIME;

			obj_to_room(corpse->first_content, ch->in_room->vnum);
		}
		junk_obj( corpse );

		pop_call();
		return TRUE;
	}
	pop_call();
	return FALSE;
}

bool spec_guard( CHAR_DATA *ch )
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *v_next;
	CHAR_DATA *ech;
	char *crime;
	int max_evil;

	push_call("spec_guard(%p)",ch);

	if (!IS_AWAKE(ch) || ch->fighting != NULL || IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		pop_call();
		return FALSE;
	}
	max_evil = 300;
	ech      = NULL;
	crime    = "";

	for (victim = ch->in_room->first_person ; victim ; victim = v_next)
	{
		v_next = victim->next_in_room;

		if (!IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER) )
		{
			if (!IS_AFFECTED(victim, AFF_SNEAK))
			{
				if (!IS_AFFECTED(victim, AFF_STEALTH))
				{
					if (number_percent() < 20)
					{
						crime = "MURDERER";
						if (victim->level > (10*(int)ch->level)/6)
						{
							int scale, divisor, level;

							/* scale the guard UP to 60% of the victim's level */

							level=victim->level;
							scale=6 * level;

							level=ch->pIndexData->level;
							divisor = 10 * level;
							level=level*scale/divisor;
							ch->level= (char)level;
							ch->hit= 1+dice(ch->pIndexData->hitnodice*scale/divisor,
								ch->pIndexData->hitsizedice*scale/divisor)+
								ch->pIndexData->hitplus*scale/divisor;
							ch->max_hit= ch->hit;
							ch->npcdata->damnodice=
							ch->pIndexData->damnodice*scale/divisor;
							ch->npcdata->damsizedice=
							ch->pIndexData->damsizedice*scale/divisor;
							ch->npcdata->damplus= ch->pIndexData->damplus*scale/divisor+1;
						}
						break;
					}
				}
			}
		}
		if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF) )
		{
			if(!IS_AFFECTED(victim,AFF_SNEAK))
			{
				if(!IS_AFFECTED(victim,AFF_STEALTH))
				{
					if(number_percent()<20)
					{
						crime = "THIEF";
						break;
					}
				}
			}
		}
		if ( victim->fighting != NULL
			&& victim->fighting->who != ch
			&& victim->alignment < max_evil )
		{
			max_evil = victim->alignment;
			ech      = victim;
		}
	}
	if ( victim != NULL )
	{
		sprintf( buf, "%s is a %s!  PROTECT THE INNOCENT!!  BANZAI!!",
		victim->name, crime );
		act("$n screams '$T'",ch,NULL,buf,TO_ROOM);
		multi_hit( ch, victim, TYPE_UNDEFINED );

		pop_call();
		return TRUE;
	}

	if ( ech != NULL )
	{
		if((number_percent()<75)&&(max_evil>-300)&&ech->fighting && (ech->fighting->who->alignment<300))
		{
			act("$n says 'Alright you guys, break it up!  NOW!'",
			ch,NULL,NULL,TO_ROOM);
			for(victim=ch->in_room->first_person;victim!=NULL;victim=v_next)
			{
				v_next = victim->next_in_room;
				if(!(IS_NPC(victim) && (ch->pIndexData->vnum==victim->pIndexData->vnum)))
				{
					stop_fighting(victim,FALSE);
				}
			}
			pop_call();
			return TRUE;
		}

		act( "$n screams 'PROTECT THE INNOCENT!!  BANZAI!!", ch, NULL, NULL, TO_ROOM );
		multi_hit( ch, ech, TYPE_UNDEFINED );

		pop_call();
		return TRUE;
	}
	pop_call();
	return FALSE;
}

bool spec_janitor( CHAR_DATA *ch )
{
	OBJ_DATA *trash;
	OBJ_DATA *trash_next;

	push_call("spec_janitor(%p)",ch);

	if ( !IS_AWAKE(ch) )
	{
		pop_call();
		return FALSE;
	}
	for ( trash = ch->in_room->first_content; trash != NULL; trash = trash_next )
	{
		trash_next = trash->next_content;
		if ( !IS_SET( trash->wear_flags, ITEM_TAKE ) )
		{
			continue;
		}
		if ( trash->item_type == ITEM_DRINK_CON
			|| trash->item_type == ITEM_TRASH
			|| trash->cost < 101 )
		{
			act( "$n picks up some trash.", ch, NULL, NULL, TO_ROOM );
			obj_from_room( trash );
			obj_to_char( trash, ch );

			pop_call();
			return TRUE;
		}
	}
	pop_call();
	return FALSE;
}

bool spec_poison( CHAR_DATA *ch )
{
	CHAR_DATA *victim;

	push_call("spec_poison(%p)",ch);

	if ( ch->position != POS_FIGHTING
		|| ( victim = who_fighting(ch) ) == NULL
		|| number_percent( ) > 2 * ch->level
		|| victim->in_room != ch->in_room )
	{
		pop_call();
		return FALSE;
	}
	act( "You bite $N!",  ch, NULL, victim, TO_CHAR    );
	act( "$n bites $N!",  ch, NULL, victim, TO_NOTVICT );
	act( "$n bites you!", ch, NULL, victim, TO_VICT    );
	spell_poison( gsn_poison, ch->level, ch, victim, skill_table[gsn_poison].target );

	pop_call();
	return TRUE;
}


bool spec_thief( CHAR_DATA *ch )
{
	CHAR_DATA *victim;
	CHAR_DATA *v_next;
	int gold;

	push_call("spec_thief(%p)",ch);

	if ( ch->position != POS_STANDING )
	{
		pop_call();
		return FALSE;
	}
	for ( victim = ch->in_room->first_person; victim != NULL; victim = v_next )
	{
		v_next = victim->next_in_room;

		if ( IS_NPC(victim) || victim->level >= LEVEL_IMMORTAL
			|| number_bits( 2 ) != 0
			|| !can_see( ch, victim ) )
		{
			continue;
		}
		if ( IS_AWAKE(victim) && number_range( 0, ch->level ) == 0 )
		{
			act( "You discover $n's hands in your wallet!",
			ch, NULL, victim, TO_VICT );
			act( "$N discovers $n's hands in $S wallet!",
			ch, NULL, victim, TO_NOTVICT );

			pop_call();
			return TRUE;
		}
		else
		{
			gold = victim->gold * number_range( 1, 5 ) / 100;
			gold = UMIN( gold , 500 );  /* Chaos 10/5/93 */
			ch->gold += 7 * gold / 8;
			victim->gold -= gold;

			pop_call();
			return TRUE;
		}
	}
	pop_call();
	return FALSE;
}
