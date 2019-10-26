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
	Global function prototypes
*/

OBJ_DATA *get_obj_carry_vnum args( ( CHAR_DATA *ch, int vnum) );

/*
	Local function prototypes
*/

char        mudprog_cmd_translate_buf[MAX_INPUT_LENGTH];

char      * mudprog_cmd_translate args( ( char *, CHAR_DATA *, CHAR_DATA *, OBJ_DATA *, void *, CHAR_DATA *) );
MP_TOKEN  * execute_mud_prog      args( ( MP_TOKEN *token, CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, CHAR_DATA *rndm, sh_int level, int vnum) );
int         mudprog_seval         args( ( char *lhs, char* opr, char* rhs ) );
bool        mudprog_veval         args( ( int lhs, char* opr, int rhs ) );
char        mudprog_do_ifchck     args( ( char *ifchck, CHAR_DATA* mob, CHAR_DATA* actor, OBJ_DATA* obj,  void* vo, CHAR_DATA* rndm, int vnum ) );
void        mudprog_translate     args( ( char ch, char* t, CHAR_DATA* mob, CHAR_DATA* actor, OBJ_DATA* obj, void* vo, CHAR_DATA* rndm ) );
void        mudprog_driver        args( ( MUD_PROG *mprog, CHAR_DATA* mob, CHAR_DATA* actor, OBJ_DATA* obj, void* vo, int vnum ) );
long long   mudprog_case_val      args( ( int vnum, char *case_val ) );

/*
	function code and brief comments.
*/

bool         world_index[2000];

OBJ_DATA   * mp_obj     = NULL;
CHAR_DATA  * mp_char    = NULL;
int          lhsvl      = 0;
int          rhsvl      = 0;
sh_int       valid_exit = 0;
CHAR_DATA  * pfind_mob  = NULL;
sh_int       pfind_dir  = 0;

char *mp_types [] =
{
	"unknown",
	"social",
	"command",
	"if",
	"and",
	"or",
	"else",
	"elseif",
	"endif",
	"break",
	"switch",
	"case",
	"default",
	"endswitch"
};

int mprog_name_to_type(char *name)
{
	push_call("mprog_name_to_type(%p)",name);

	switch (*name)
	{
		case 'a':
			if (!strcmp(name, "act_prog"			)) { pop_call(); return ACT_PROG;		}
			if (!strcmp(name, "all_greet_prog"		)) { pop_call(); return ALL_GREET_PROG;	}
			break;
		case 'b':
			if (!strcmp(name, "bribe_prog"		)) { pop_call(); return BRIBE_PROG;	}
			break;
		case 'd':
			if (!strcmp(name, "death_prog"		)) { pop_call(); return DEATH_PROG;	}
			if (!strcmp(name, "delay_prog"		)) { pop_call(); return DELAY_PROG;	}
			if (!strcmp(name, "desc_prog"			)) { pop_call(); return DESC_PROG;		}
			break;
		case 'e':
			if (!strcmp(name, "entry_prog"		)) { pop_call(); return ENTRY_PROG;	}
			if (!strcmp(name, "exit_prog"			)) { pop_call(); return EXIT_PROG;		}
			break;
		case 'f':
			if (!strcmp(name, "fight_prog"		)) { pop_call(); return FIGHT_PROG;	}
			break;
		case 'g':
			if (!strcmp(name, "greet_prog"		)) { pop_call(); return GREET_PROG;	}
			if (!strcmp(name, "group_greet_prog"	)) { pop_call(); return GROUP_GREET_PROG;}
			if (!strcmp(name, "give_prog"			)) { pop_call(); return GIVE_PROG;		}
			break;
		case 'h':
			if (!strcmp(name, "hitprcnt_prog"		)) { pop_call(); return HITPRCNT_PROG;	}
			break;
		case 'k':
			if (!strcmp(name, "kill_prog"			)) { pop_call(); return KILL_PROG;		}
			break;
		case 'r':
			if (!strcmp(name, "rand_prog"			)) {	pop_call(); return RAND_PROG;		}
			if (!strcmp(name, "range_prog"		)) { pop_call(); return RANGE_PROG;	}		
			if (!strcmp(name, "repop_prog"		)) { pop_call(); return REPOP_PROG;	}
			break;
		case 's':
			if (!strcmp(name, "social_prog"		)) { pop_call(); return SOCIAL_PROG;	}
			if (!strcmp(name, "speech_prog"		)) { pop_call(); return SPEECH_PROG;	}
			break;
		case 't':
			if (!strcmp(name, "time_prog"			)) { pop_call(); return TIME_PROG;		}
			if (!strcmp(name, "trigger_prog"		)) { pop_call(); return TRIGGER_PROG;	}
			break;
	}
	pop_call();
	return ERROR_PROG;
}

int oprog_name_to_type(char *name)
{
	push_call("oprog_name_to_type(%p)",name);

	switch (*name)
	{
		case 'c':
			if (!strcmp(name, "command_prog"     )) { pop_call(); return COMMAND_OPROG;     }
			break;

		case 'd':
			if (!strcmp(name, "damage_prog"      )) { pop_call(); return DAMAGE_OPROG;      }
			if (!strcmp(name, "drop_prog"        )) { pop_call(); return DROP_OPROG;        }
			break;

		case 'g':
			if (!strcmp(name, "get_prog"         )) { pop_call(); return GET_OPROG;         }
			break;

		case 'h':
			if (!strcmp(name, "hit_prog"         )) { pop_call(); return HIT_OPROG;         }
			break;

		case 'r':
			if (!strcmp(name, "rand_prog"        )) { pop_call(); return RAND_OPROG;        }
			if (!strcmp(name, "remove_prog"      )) { pop_call(); return REMOVE_OPROG;      }
			break;

		case 's':
			if (!strcmp(name, "sacrifice_prog"   )) { pop_call(); return SACRIFICE_OPROG;   }
			break;

		case 'u':
			if (!strcmp(name, "unknown_prog"     )) { pop_call(); return UNKNOWN_OPROG;     }
			break;

		case 'w':
			if (!strcmp(name, "wear_prog"        )) { pop_call(); return WEAR_OPROG;        }
			break;
	}
	pop_call();
	return ERROR_PROG;
}

int rprog_name_to_type(char *name)
{
	push_call("rprog_name_to_type(%p)",name);

	switch (*name)
	{
		case 'c':
			if (!strcmp(name, "command_prog"     )) { pop_call(); return COMMAND_RPROG;     }
			break;

		case 'u':
			if (!strcmp(name, "unknown_prog"     )) { pop_call(); return UNKNOWN_RPROG;     }
			break;
	}
	pop_call();
	return ERROR_PROG;
}

char *mprog_type_to_name(int type)
{
	switch (type)
	{
		case ACT_PROG:			return "act_prog";
		case SOCIAL_PROG:		return "social_prog";
		case SPEECH_PROG:		return "speech_prog";
		case RAND_PROG:		return "rand_prog";
		case FIGHT_PROG:		return "fight_prog";
		case HITPRCNT_PROG:		return "hitprcnt_prog";
		case DEATH_PROG:		return "death_prog";
		case KILL_PROG:		return "kill_prog";
		case ENTRY_PROG:		return "entry_prog";
		case GREET_PROG:		return "greet_prog";
		case ALL_GREET_PROG:	return "all_greet_prog";
		case GROUP_GREET_PROG:	return "group_greet_prog";
		case GIVE_PROG:		return "give_prog";
		case BRIBE_PROG:		return "bribe_prog";
		case RANGE_PROG:		return "range_prog";
		case TIME_PROG:		return "time_prog";
		case REPOP_PROG:		return "repop_prog";
		case DELAY_PROG:		return "delay_prog";
		case EXIT_PROG:		return "exit_prog";
		case TRIGGER_PROG:		return "trigger_prog";
		case DESC_PROG:		return "desc_prog";
		default:				return "ERROR_PROG";
	}
}


char *oprog_type_to_name(int type)
{
	switch (type)
	{
		case COMMAND_OPROG:      return "command_prog";
		case DAMAGE_OPROG:       return "damage_prog";
		case DROP_OPROG:         return "drop_prog";
		case GET_OPROG:          return "get_prog";
		case HIT_OPROG:          return "hit_prog";
		case RAND_OPROG:         return "rand_prog";
		case REMOVE_OPROG:       return "remove_prog";
		case SACRIFICE_OPROG:    return "sacrifice_prog";
		case UNKNOWN_OPROG:      return "unknown_prog";
		case WEAR_OPROG:         return "wear_prog";
		default:                 return "ERROR_PROG";
	}
}

char *rprog_type_to_name(int type)
{
	switch (type)
	{
		case COMMAND_RPROG:      return "command_prog";
		case UNKNOWN_RPROG:      return "unknown_prog";
		default:                 return "ERROR_PROG";
	}
}

/*
	These two functions do the basic evaluation of ifcheck operators.
*/

int mudprog_seval( char *lhs, char *opr, char *rhs )
{
	push_call("mudprog_seval(%p,%p,%p)",lhs,opr,rhs);

	switch (opr[0])
	{
		case '=':
			pop_call();
			return !strcasecmp(lhs, rhs);

		case '!':
			switch (opr[1])
			{
				case '=':
					pop_call();
					return strcasecmp(lhs, rhs);
				case '/':
					pop_call();
					return str_infix(rhs, lhs);
			}
			break;

		case '/':
			pop_call();
			return !str_infix(rhs, lhs);
	}

	log_printf("mudprog_seval: invalid operator, dumping stack:\n\r");
	dump_stack();

	pop_call();
	return FALSE;
}

bool mudprog_veval( int lhs, char *opr, int rhs)
{
	push_call("mudprog_veval(%p,%p,%p)",lhs,opr,rhs);

	switch (opr[0])
	{
		case '=':
			pop_call();
			return ( lhs == rhs);

		case '!':
			pop_call();
			return ( lhs != rhs);

		case '>':
			switch (opr[1])
			{
				case '\0':
					pop_call();
					return ( lhs > rhs);

				case '=':
					pop_call();
					return ( lhs >= rhs);
			}
			break;
		case '<':
			switch (opr[1])
			{
				case '\0':
					pop_call();
					return ( lhs < rhs);

				case '=':
					pop_call();
					return ( lhs <= rhs);
			}
			break;

		case '&':
			pop_call();
			return ( lhs & rhs);

		case '|':
			pop_call();
			return ( lhs | rhs);
	}
	log_printf("mudprog_veval: invalid operator, dumping stack\n\r");
	dump_stack();

	pop_call();
	return FALSE;
}


long long mudprog_case_val(int vnum, char *case_val)
{
	int cnt;

	push_call("mudprog_case_val(%d,%p",vnum,case_val);

	if (isalpha(case_val[0]))
	{
		for (cnt = mud->bitvector_ref[toupper(case_val[0]) - 'A'] ; cnt < MAX_BITVECTOR ; cnt++)
		{
			if (!strcasecmp(bitvector_table[cnt].name, case_val))
			{
				pop_call();
				return bitvector_table[cnt].value;
			}
		}
	}
	else if (isdigit(case_val[0]))
	{
		pop_call();
		return atoll(case_val);
	}
	log_build_printf(vnum, "bad case value: '%s'", case_val);

	pop_call();
	return -1;
}


bool valid_mp_exit( EXIT_DATA *pexit)
{
	if (room_index[pexit->to_room] == NULL)
	{
		return FALSE;
	}

	if (HAS_BIT(pfind_mob->act, ACT_STAY_SECTOR) && pfind_mob->reset && room_index[pexit->to_room]->sector_type != room_index[pfind_mob->reset->arg3]->sector_type)
	{
		return FALSE;
	}

	if (room_index[pexit->to_room]->area != pfind_mob->in_room->area)
	{
		return FALSE;
	}

	if (HAS_BIT(pexit->flags, EX_CLOSED))
	{
		if (!IS_AFFECTED(pfind_mob, AFF_PASS_DOOR)
		&&  !rspec_req(pfind_mob, RSPEC_PASS_DOOR))
		{
			return FALSE;
		}

		if (HAS_BIT(pexit->flags, EX_PASSPROOF))
		{
			return FALSE;
		}
	}

	if (HAS_BIT(room_index[pexit->to_room]->room_flags, ROOM_BLOCK) && !IS_AFFECTED(pfind_mob, AFF_CLEAR))
	{
		return FALSE;
	}
	return TRUE;
}


/*
	Fast short distance node weighted algorithm, can be used to backtrack the
	entire shortest path - Scandum
*/

void mprog_findpath(ROOM_INDEX_DATA *room, bool cnt)
{
	bool door;

	world_index[room->vnum - room->area->low_r_vnum] = cnt;

	if (cnt < lhsvl)
	{
		if (room->vnum == rhsvl)
		{
			lhsvl      = cnt;
			valid_exit = pfind_dir;
		}
		else
		{
			cnt++;

			for (door = 0 ; door < 6 ; door++)
			{
				if (room->exit[door] && valid_mp_exit(room->exit[door]))
				{
					if (world_index[room->exit[door]->to_room - room->area->low_r_vnum] > cnt)
					{
						mprog_findpath(room_index[room->exit[door]->to_room], cnt);
					}
				}
			}
		}
	}
}

/*
	This function performs the evaluation of the if checks.
*/

char mudprog_do_ifchck(char *ifchck, CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo, CHAR_DATA *rndm, int vnum)
{
	char buf[ MAX_INPUT_LENGTH ];
	char arg[ MAX_INPUT_LENGTH ];
	char opr[ MAX_INPUT_LENGTH ];
	char val[ MAX_INPUT_LENGTH ];

	CHAR_DATA *ifchk_char = NULL;
	OBJ_DATA  *ifchk_obj  = NULL;

	char		*bufpt = buf;
	char		*argpt = arg;
	char		*oprpt = opr;
	char		*valpt = val;
	char		*point = ifchck;

	int		cnt;
	bool		retval;

	push_call("mudprog_do_ifchck(%p,%p,%p,%p,%p,%p)",ifchck,mob,actor,obj,vo,rndm);

	retval = -1;

	if (*point == '\0')
	{
		log_build_printf(vnum, "null ifchck");
		pop_call();
		return -1;
	}

	/*
		store the if check name in 'buf'
	*/

	while (*point != '(')
	{
		if (*point == ' ')
		{
			point++;
		}
		else if (*point == '!')
		{
			point++;
		}
		else
		{
			*bufpt++ = *point++;
		}
	}

	*bufpt = '\0';
	point++;

	/*
		store whatever is between the () in 'arg'
	*/

	while (*point != ')')
	{
		*argpt++ = *point++;
	}

	*argpt = '\0';
	point++;

	/*
		store the operator in 'opr'
	*/

	if (*point == '\0')
	{
		*opr = '\0';
		*val = '\0';
	}
	else
	{
		point++;

		while (*point != ' ')
		{
			*oprpt++ = *point++;
		}
		*oprpt = '\0';
		point++;

		/*
			store the value in 'val'
		*/

		while (*point != '\0')
		{
			*valpt++ = *point++;
		}
		*valpt = '\0';
	}

	/*
		Translate the value if needed
	*/

	if (isalpha(val[0]))
	{
		for (cnt = mud->bitvector_ref[toupper(val[0]) - 'A'] ; *bitvector_table[cnt].name ; cnt++)
		{
			if (toupper(val[0]) == bitvector_table[cnt].name[0])
			{
				if (!strcasecmp(bitvector_table[cnt].name, val))
				{
					sprintf(val, "%lld", bitvector_table[cnt].value);
					break;
				}
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		if (val[0] == '$')
		{
			strcpy(val, mudprog_cmd_translate(val, mob, actor, obj, vo, rndm));
		}
	}


	if (arg[0] == '$')
	{
		switch (arg[1])
		{
			case 'i':
				ifchk_char = mob;
				break;
			case 'n':
				ifchk_char = actor;
				break;
			case 't':
				ifchk_char = mp_char;
				break;
			case 'r':
				ifchk_char = rndm;
				break;
			case 'o':
				ifchk_obj  = obj;
				break;
			case 'c':
				ifchk_obj  = mp_obj;
				break;
			case 'x':
				sprintf(arg, "%d", lhsvl);
				break;
			case 'X':
				sprintf(arg, "%s", mob->npcdata->remember);
				break;
		}
	}

	switch (buf[0])
	{

		case 'a':

			if (!strcmp(buf, "actorhasobjnum"))
			{
				if (actor == NULL)
				{
					log_build_printf(vnum, "No actor defined for 'actorhasobjnum'");
					pop_call();
					return FALSE;
				}
				retval = ((mp_obj = get_obj_carry_vnum(actor, atol(arg))) != NULL);
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "actorwearsobjnum"))
			{
				if (actor == NULL)
				{
					log_build_printf(vnum, "No actor defined for 'actorwearsobjnum'");
					pop_call();
					return FALSE;
				}
				retval = ((mp_obj = get_obj_wear_vnum(actor, atol(arg))) != NULL);
				lhsvl  = mp_obj ? mp_obj->wear_loc : -1;
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "age"))
			{
				if (ifchk_char)
				{
					lhsvl  = get_age(ifchk_char);
					rhsvl  = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);
				}
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "areaquest"))
			{
				int firstBit, len;
				AREA_DATA *area;
				char name[MAX_INPUT_LENGTH];

				if (sscanf(arg, "%d, %d, %s", &firstBit, &len, name) != 3)
				{
					log_build_printf(vnum, "Bad ifcheck: <areaquest (%s)>", arg);
					pop_call();
					return FALSE;
				}

				switch (name[1])
				{
					case 'i':
						area = mob->in_room->area;
						break;

					case 'o':
						if (obj)
						{
							area = obj->pIndexData->area;
						}
						else
						{
							pop_call();
							return -1;
						}
						break;

					case 'c':
						if (mp_obj)
						{
							area = mp_obj->pIndexData->area;
						}
						else
						{
							pop_call();
							return -1;
						}
						break;

					default:
						log_build_printf(vnum, "Bad ifcheck: <areaquest (%s)>", arg);
						pop_call();
						return FALSE;
				}

				lhsvl  = get_quest_bits( area->area_quest, firstBit, len);
				rhsvl  = atol(val);
				retval = mudprog_veval(lhsvl,opr,rhsvl);

				pop_call();
				return retval;
			}
			break;

		case 'c':

			if (!strcmp(buf, "cansee"))
			{
				if (ifchk_char)
				{
					retval = can_see(mob, ifchk_char);
				}
				else if (ifchk_obj)
				{
					retval = can_see_obj(mob, ifchk_obj);
				}
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "cancarry"))
			{
				if (ifchk_char)
				{
					retval = (ifchk_char->carry_number < can_carry_n(ifchk_char) && ifchk_char->carry_weight < can_carry_w(ifchk_char));
				}
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "class"))
			{
				if (ifchk_char)
				{
					lhsvl  = ifchk_char->class;
					rhsvl  = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);
				}
				pop_call();
				return retval;
			}
			break;

		case 'd':
			if (!strcmp(buf, "delayed"))
			{
				if (ifchk_char)
				{
					if (!IS_NPC(ifchk_char))
					{
						lhsvl = ifchk_char->wait;
					}
					else
					{
						lhsvl = ifchk_char->wait / 2 + ifchk_char->timer;
					}
					retval = (lhsvl > 0);
				}
				pop_call();
				return retval;
			}
			break;

		case 'e':
			if (!strcmp(buf, "existsroom"))
			{
				OBJ_DATA  *robj;
				CHAR_DATA *rch;

				if (arg[0] == '$')
				{
					if (ifchk_char)
					{
						pop_call();
						return (mob->in_room == ifchk_char->in_room);
					}
					else if (ifchk_obj)
					{
						pop_call();
						return (mob->in_room == obj->in_room);
					}
					else
					{
						log_build_printf(vnum, "Bad ifcheck: <existsroom (%s)>", arg);
					}
				}

				if (!strcmp(arg, "mob"))
				{
					for (mp_char = NULL, cnt = 0, rch = mob->in_room->first_person ; rch ; rch = rch->next_in_room)
					{
						if (IS_NPC(rch) && mob != rch)
						{
							if (number_range(0, cnt) == 0)
							{
								mp_char = rch;
							}
							cnt++;
						}
					}
					pop_call();
					return (mp_char != NULL);
				}

				if (!strcmp(arg, "plr"))
				{
					for (mp_char = NULL, cnt = 0, rch = mob->in_room->first_person ; rch ; rch = rch->next_in_room)
					{
						if (!IS_NPC(rch) && mob != rch)
						{
							if (number_range(0, cnt) == 0)
							{
								mp_char = rch;
							}
							cnt++;
						}
					}
					pop_call();
					return (mp_char != NULL);
				}

				if (!strcmp(arg, "obj"))
				{
					for (mp_obj = NULL, cnt = 0, robj = mob->in_room->first_content ; robj ; robj = robj->next_content)
					{
						if (number_range(0, cnt) == 0)
						{
							mp_obj = robj;
						}
						cnt++;
					}
					pop_call();
					return (mp_obj != NULL);
				}

				if ((mp_char = get_char_room_even_blinded(mob, arg)) != NULL)
				{
					pop_call();
					return TRUE;
				}

				if ((mp_obj = get_obj_list_even_blinded(mob, arg, mob->in_room->first_content)) != NULL)
				{
					pop_call();
					return TRUE;
				}

				pop_call();
				return FALSE;
			}

			if (!strcmp(buf, "existsarea"))
			{
				if (arg[0] == '$')
				{
					switch (arg[1])
					{
						case 'n':
						case 'r':
						case 't':
							if (ifchk_char)
							{
								pop_call();
								return (mob->in_room->area == ifchk_char->in_room->area);
							}
							else
							{
								pop_call();
								return -1;
							}
							break;
						case 'c':
						case 'o':
							if (ifchk_obj)
							{
								pop_call();
								return (ifchk_obj->in_room && mob->in_room->area == ifchk_obj->in_room->area);
							}
							else
							{
								pop_call();
								return -1;
							}
							break;
						default:
							log_build_printf(vnum, "Bad ifcheck: <existsarea (%s)>", arg);
							break;
					}
				}

				if ((mp_char = get_char_area_even_blinded(mob, arg)) != NULL)
				{
					pop_call();
					return TRUE;
				}

				if ((mp_obj = get_obj_area_even_blinded(mob, arg)) != NULL)
				{
					pop_call();
					return TRUE;
				}

				pop_call();
				return FALSE;
			}
			break;

		case 'f':
			if (!strcmp(buf, "findpath"))
			{
				ROOM_INDEX_DATA *room;
				int area_size;

				if (sscanf(arg, "%d, %d", &rhsvl, &lhsvl) != 2)
				{
					log_build_printf(vnum, "Bad ifcheck: <findpath (%s)>", arg);
					pop_call();
					return FALSE;
				}

				if (lhsvl < 1 || lhsvl > 100)
				{
					log_build_printf(vnum, "Bad Path length: <findpath (%s)>", arg);
					pop_call();
					return FALSE;
				}

				if ((room = get_room_index(rhsvl)) == NULL)
				{
					log_build_printf(vnum, "Room not found: <findpath (%s)>", arg);
					pop_call();
					return FALSE;
				}

				if (room == mob->in_room)
				{
					log_build_printf(vnum, "Already there: <findpath (%s)>", arg);
					pop_call();
					return FALSE;
				}

				if (room->area != mob->in_room->area)
				{
					log_build_printf(vnum, "Destination not in mob's area <findpath (%s)>", arg);
					pop_call();
					return FALSE;
				}

				if ((area_size = room->area->hi_r_vnum - room->area->low_r_vnum) >= 2000)
				{
					log_build_printf(vnum, "Area too big <findpath (%s)>", arg);
					pop_call();
					return FALSE;
				}

				pfind_mob  = mob;
				valid_exit = -1;

				memset(world_index, 128, area_size);
				world_index[mob->in_room->vnum - room->area->low_r_vnum] = 0;

				for (pfind_dir = 0 ; pfind_dir < 6 ; pfind_dir++)
				{
					if (mob->in_room->exit[pfind_dir] && valid_mp_exit(mob->in_room->exit[pfind_dir]))
					{
						mprog_findpath(room_index[mob->in_room->exit[pfind_dir]->to_room], 1);
					}
				}

				pop_call();
				return (valid_exit != -1);
			}
			break;

		case 'g':
			if (!strcmp(buf, "gold"))
			{
				if (ifchk_char)
				{
					lhsvl  = ifchk_char->gold;
					rhsvl  = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);
				}
				else if (ifchk_obj)
				{
					lhsvl  = ifchk_obj->cost;
					rhsvl  = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);
				}
				pop_call();
				return retval;
			}
			break;

		case 'h':
			if (!strcmp(buf, "hasobj"))
			{
				if (ifchk_char)
				{
					rhsvl  = ((mp_obj = get_obj_list_even_blinded(ifchk_char, arg, ifchk_char->first_carrying)) != NULL);
					retval = mudprog_veval(1, opr, rhsvl);
				}
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "hasobjnum"))
			{
				if (ifchk_char)
				{
					rhsvl  = ((mp_obj = get_obj_carry_vnum(ifchk_char, atol(val))) != NULL);
					retval = mudprog_veval(1, opr, rhsvl);
				}
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "hitprcnt"))
			{
				if (ifchk_char)
				{
					lhsvl  = 100 * ifchk_char->hit / UMAX(1, ifchk_char->max_hit);
					rhsvl  = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);
				}
				pop_call();
				return retval;
			}
			break;

		case 'i':
			if (!strcmp(buf, "isnight"))
			{
				if (mud->time_info->hour >= 5 && mud->time_info->hour <= 19)
				{
					pop_call();
					return FALSE;
				}
				else
				{
					pop_call();
					return TRUE;
				}
			}

			if (!strcmp(buf, "isday"))
			{
				if (mud->time_info->hour >= 5 && mud->time_info->hour <=19)
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

			if (!strcmp(buf, "ispc"))
			{
				if (ifchk_char)
				{
					retval = (!IS_NPC(ifchk_char));
				}
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "isnpc"))
			{
				if (ifchk_char)
				{
					retval = (IS_NPC(ifchk_char));
				}
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "isgood"))
			{
				if (ifchk_char)
				{
					retval = (IS_GOOD(ifchk_char));
				}
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "isevil"))
			{
				if (ifchk_char)
				{
					retval = (IS_EVIL(ifchk_char));
				}
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "isneutral"))
			{
				if (ifchk_char)
				{
					retval = (IS_NEUTRAL(ifchk_char));
				}
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "iskiller"))
			{
				if (ifchk_char)
				{
					retval = (!IS_NPC(ifchk_char) && HAS_BIT(ifchk_char->act, PLR_KILLER));
				}
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "isthief"))
			{
				if (ifchk_char)
				{
					retval = (!IS_NPC(ifchk_char) && HAS_BIT(ifchk_char->act, PLR_THIEF));
				}
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "isfight"))
			{
				if (ifchk_char)
				{
					mp_char = who_fighting(ifchk_char);
					retval  = (mp_char != NULL);
				}
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "isfollow"))
			{
				if (ifchk_char)
				{
					mp_char = ifchk_char->master;
					retval  = (mp_char != NULL);
				}
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "isaffected"))
			{
				if (ifchk_char)
				{
					rhsvl  = is_mp_affected(ifchk_char, val);
					retval = mudprog_veval(1, opr, rhsvl);
				}
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "inarea"))
			{
				if (ifchk_char)
				{
					lhsvl  = ifchk_char->in_room->area->low_r_vnum;
					rhsvl  = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);
				}
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "inroom"))
			{
				if (ifchk_char)
				{
					lhsvl  = ifchk_char->in_room->vnum;
					rhsvl  = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);
				}
				pop_call();
				return retval;
			}
			break;

		case 'l':
			if (!strcmp(buf, "level"))
			{
				if (ifchk_char)
				{
					lhsvl  = ifchk_char->level;
					rhsvl  = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);
				}
				else if (ifchk_obj)
				{
					lhsvl  = ifchk_obj->level;
					rhsvl  = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);
				}
				pop_call();
				return retval;
			}
			break;

		case 'm':
			if (!strcmp(buf, "moveprcnt"))
			{
				if (ifchk_char)
				{
					lhsvl  = 100 * ifchk_char->move / UMAX(1, ifchk_char->max_move);
					rhsvl  = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);
				}
				pop_call();
				return retval;
			}
			if (!strcmp(buf, "multiran"))
			{
				if (ifchk_char)
				{
					lhsvl  = IS_NPC(ifchk_char) ? -1 : ifchk_char->pcdata->mclass[CLASS_RANGER];
					rhsvl  = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);
				}
				pop_call();
				return retval;
			}
			if (!strcmp(buf, "multigla"))
			{
				if (ifchk_char)
				{
					lhsvl  = IS_NPC(ifchk_char) ? -1 : ifchk_char->pcdata->mclass[CLASS_GLADIATOR];
					rhsvl  = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);
				}
				pop_call();
				return retval;
			}
			if (!strcmp(buf, "multimar"))
			{
				if (ifchk_char)
				{
					lhsvl  = IS_NPC(ifchk_char) ? -1 : ifchk_char->pcdata->mclass[CLASS_MARAUDER];
					rhsvl  = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);
				}
				pop_call();
				return retval;
			}
			if (!strcmp(buf, "multinin"))
			{
				if (ifchk_char)
				{
					lhsvl  = IS_NPC(ifchk_char) ? -1 : ifchk_char->pcdata->mclass[CLASS_NINJA];
					rhsvl  = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);
				}
				pop_call();
				return retval;
			}
			if (!strcmp(buf, "multiele"))
			{
				if (ifchk_char)
				{
					lhsvl  = IS_NPC(ifchk_char) ? -1 : ifchk_char->pcdata->mclass[CLASS_ELEMENTALIST];
					rhsvl  = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);
				}
				pop_call();
				return retval;
			}
			if (!strcmp(buf, "multiill"))
			{
				if (ifchk_char)
				{
					lhsvl  = IS_NPC(ifchk_char) ? -1 : ifchk_char->pcdata->mclass[CLASS_ILLUSIONIST];
					rhsvl  = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);
				}
				pop_call();
				return retval;
			}
			if (!strcmp(buf, "multimon"))
			{
				if (ifchk_char)
				{
					lhsvl  = IS_NPC(ifchk_char) ? -1 : ifchk_char->pcdata->mclass[CLASS_MONK];
					rhsvl  = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);
				}
				pop_call();
				return retval;
			}
			if (!strcmp(buf, "multinec"))
			{
				if (ifchk_char)
				{
					lhsvl  = IS_NPC(ifchk_char) ? -1 : ifchk_char->pcdata->mclass[CLASS_NECROMANCER];
					rhsvl  = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);
				}
				pop_call();
				return retval;
			}
			break;

		case 'n':
			if (!strcmp(buf, "numberrange"))
			{
				int lo_range, hi_range;

				if (sscanf(arg, "%d,%d", &lo_range, &hi_range) != 2)
				{
					log_build_printf(vnum, "Bad ifcheck: <numberrange (%s)>", arg);
					pop_call();
					return FALSE;
				}

				if (lo_range < 0)
				{
					log_build_printf(vnum, "Negative range <numberrange (%s)>", arg);
					pop_call();
					return FALSE;
				}

				if (lo_range >= hi_range)
				{
					log_build_printf(vnum, "Bad range: <numberrange (%s)>", arg);
					pop_call();
					return FALSE;
				}
				lhsvl = number_range(lo_range, hi_range);
				rhsvl = atol(val);
				retval = mudprog_veval(lhsvl, opr, rhsvl);

				pop_call();
				return retval;
			}

			if (!strcmp(buf, "number"))
			{
				switch (arg[1])
				{
					case 'i':
					case 'n':
					case 't':
					case 'r':
						if (ifchk_char)
						{
							if (IS_NPC(ifchk_char))
							{
								lhsvl  = ifchk_char->pIndexData->vnum;
								rhsvl  = atol(val);
								retval = mudprog_veval(lhsvl, opr, rhsvl);
							}
							else
							{
								lhsvl  = ifchk_char->pcdata->pvnum;
								rhsvl  = atol(val);
								retval = mudprog_veval(lhsvl, opr, rhsvl);
							}
						}
						break;
					case 'o':
					case 'c':
						if (ifchk_obj)
						{
							lhsvl  = ifchk_obj->pIndexData->vnum;
							rhsvl  = atol(val);
							retval = mudprog_veval(lhsvl, opr, rhsvl);
						}
						break;
					default:
						lhsvl  = atoll(arg);
						rhsvl  = atoll(val);
						retval = mudprog_veval(lhsvl, opr, rhsvl);
						break;
				}
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "name"))
			{
				retval = mudprog_seval(mudprog_cmd_translate(arg, mob, actor, obj, vo, rndm), opr, val);

				pop_call();
				return retval;
			}
			break;

		case 'o':
			if (!strcmp(buf, "objquest"))
			{
				int firstBit,len;
				OBJ_DATA *item;
				char name[MAX_INPUT_LENGTH];

				if (sscanf(arg, "%d, %d, %s", &firstBit, &len, name) != 3)
				{
					log_build_printf(vnum, "Bad ifcheck: <objquest (%s)>", arg);
					pop_call();
					return FALSE;
				}

				if (name[0] == '$')
				{
					switch (name[1])
					{
						case 'o':
							if (obj)
							{
								item = obj;
							}
							else
							{
								pop_call();
								return -1;
							}
							break;
						case 'c':
							if (mp_obj)
							{
								item = mp_obj;
							}
							else
							{
								pop_call();
								return -1;
							}
							break;
						default:
							log_build_printf(vnum, "Bad ifcheck: <objquest (%s)>", arg);
							pop_call();
							return -1;
					}
				}
				else
				{
					item = NULL;
				}
				if ((item == NULL) && (item = get_obj_carry_even_blinded(mob,name)) == NULL)
				{
					pop_call();
					return FALSE;
				}
				lhsvl = get_quest_bits( item->obj_quest, firstBit, len);
				rhsvl = atol(val);
				retval = mudprog_veval(lhsvl,opr,rhsvl);
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "objtype"))
			{
				if (ifchk_obj)
				{
					lhsvl  = ifchk_obj->item_type;
					rhsvl  = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);
				}
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "objval0"))
			{
				switch (arg[1])
				{
					case 'o':
						if ( obj)
						{
							lhsvl = obj->value[0];
							rhsvl = atol( val);
							retval = mudprog_veval( lhsvl, opr, rhsvl);
							pop_call();
							return retval;
						}
						else
						{
							pop_call();
							return -1;
						}
					case 'c':
						if ( mp_obj)
						{
							lhsvl = mp_obj->value[0];
							rhsvl = atol( val);
							retval = mudprog_veval( lhsvl, opr, rhsvl);
							pop_call();
							return retval;
						}
						else
						{
							pop_call();
							return -1;
						}
					default:
						log_build_printf(vnum, "Bad argument to 'objval0'");
						pop_call();
						return -1;
				}
			}

			if (strstr(buf, "objval"))
			{
				cnt = buf[6] - '0';

				if (cnt >= 0 && cnt <= 3)
				{
					if (ifchk_obj)
					{
						lhsvl  = ifchk_obj->value[cnt];
						rhsvl  = atol(val);
						retval = mudprog_veval(lhsvl, opr, rhsvl);
					}
				}
				pop_call();
				return retval;
			}
			break;

		case 'p':
			if (!strcmp(buf, "pcsinarea"))
			{
				lhsvl = atol(mudprog_cmd_translate(arg, mob, actor, obj, vo, rndm));

				if (get_room_index(lhsvl) == NULL)
				{
					lhsvl = mob->in_room->area->nplayer;
				}
				else
				{
					lhsvl = room_index[lhsvl]->area->nplayer;
				}
				rhsvl = atol(val);
				retval = mudprog_veval(lhsvl,opr,rhsvl);

				pop_call();
				return retval;
			}

			if (!strcmp(buf, "pcsinroom"))
			{
				CHAR_DATA *rch;

				lhsvl = atol(mudprog_cmd_translate(arg, mob, actor, obj, vo, rndm));

				if (get_room_index(lhsvl) == NULL)
				{
					rch = mob->in_room->first_person;
				}
				else
				{
					rch = room_index[lhsvl]->first_person;
				}

				for (lhsvl = 0 ; rch ; rch = rch->next_in_room)
				{
					if (!IS_NPC(rch))
					{
						lhsvl++;
					}
				}
				rhsvl  = atol(val);
				retval = mudprog_veval(lhsvl, opr, rhsvl);
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "position"))
			{
				if (ifchk_char)
				{
					lhsvl  = ifchk_char->position;
					rhsvl  = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);
				}
				pop_call();
				return retval;
			}
			break;

		case 'q':

			if (!strcmp(buf, "quest"))
			{
				int firstBit, len;
				char name[MAX_INPUT_LENGTH];

				if (sscanf(arg, "%d, %d, %s", &firstBit, &len, name) != 3)
				{
					log_build_printf(vnum, "Bad ifcheck: <quest (%s)>", arg);
					pop_call();
					return FALSE;
				}

				if (name[0] == '$')
				{
					switch (name[1])
					{
						case 'i':
							ifchk_char = mob;
							break;
						case 'n':
							ifchk_char = actor;
							break;
						case 't':
							ifchk_char = mp_char;
							break;
						case 'r':
							ifchk_char = rndm;
							break;
						case 'o':
							ifchk_obj  = obj;
							break;
						case 'c':
							ifchk_obj  = mp_obj;
							break;
						case 'X':
							sprintf(name, "%s", mob->npcdata->remember);
							break;
						default:
							log_build_printf(vnum, "Bad ifcheck: <quest (%s)>", arg);

							pop_call();
							return -1;
					}
				}

				if (ifchk_char != NULL || (ifchk_char = get_char_room_even_blinded(mob, name)) != NULL)
				{
					if (IS_NPC(ifchk_char))
					{
						lhsvl = get_quest_bits(ifchk_char->npcdata->mob_quest, firstBit, len);
					}
					else if (ifchk_char == mob)
					{
						if (obj)
						{
							lhsvl = get_quest_bits(ifchk_char->pcdata->quest[obj->pIndexData->area->low_r_vnum/100], firstBit, len);
						}
						else
						{
							lhsvl = get_quest_bits(ifchk_char->pcdata->quest[ifchk_char->in_room->area->low_r_vnum/100], firstBit, len);
						}
					}
					else
					{
						lhsvl = get_quest_bits(ifchk_char->pcdata->quest[mob->pIndexData->area->low_r_vnum/100], firstBit, len);
					}
					rhsvl = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);

					pop_call();
					return retval;
				}

				if (ifchk_obj != NULL || (ifchk_obj = get_obj_here_even_blinded(mob, name)) != NULL)
				{
					lhsvl = get_quest_bits(ifchk_obj->obj_quest, firstBit, len);

					rhsvl = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);

					pop_call();
					return retval;
				}
				pop_call();
				return FALSE;
			}

			if (!strcmp(buf, "questr"))
			{
				int firstBit,len, vnum;
				CHAR_DATA *victim;
				char name[MAX_INPUT_LENGTH];

				if (sscanf(arg, "%d, %d, %d, %s", &vnum, &firstBit, &len, name) != 4)
				{
					log_build_printf(vnum, "Bad ifcheck: <questr (%s)", arg);
					pop_call();
					return FALSE;
				}

				if (name[0] == '$')
				{
					switch (name[1])
					{
						case 'i':
							ifchk_char = mob;
							break;
						case 'n':
							ifchk_char = actor;
							break;
						case 't':
							ifchk_char = mp_char;
							break;
						case 'r':
							ifchk_char = rndm;
							break;
						case 'o':
							ifchk_obj  = obj;
							break;
						case 'c':
							ifchk_obj  = mp_obj;
							break;
						case 'X':
							sprintf(name, "%s", mob->npcdata->remember);
							break;

						default:
							log_build_printf(vnum, "Bad ifcheck: <questr (%s)>", arg);
							pop_call();
							return -1;
					}
				}

				if (vnum < 0 || vnum >= MAX_VNUM || room_index[vnum] == NULL)
				{
					pop_call();
					return FALSE;
				}

				if (ifchk_char == NULL)
				{
					if ((victim = get_char_room_even_blinded(mob,name)) == NULL)
					{
						pop_call();
						return FALSE;
					}

					if (IS_NPC(victim))
					{
						log_build_printf(vnum, "Bad target: <questr (%s)>", arg);
						pop_call();
						return FALSE;
					}
				}
				else
				{
					victim = ifchk_char;
				}

				lhsvl  = get_quest_bits(victim->pcdata->quest[room_index[vnum]->area->low_r_vnum/100], firstBit, len);
				rhsvl  = atol(val);
				retval = mudprog_veval(lhsvl, opr, rhsvl);

				pop_call();
				return retval;
			}
			break;

		case 'r':

			if (!strcmp(buf, "rand"))
			{
				lhsvl  = number_percent();
				retval = (lhsvl <= atol(arg));
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "race"))
			{
				if (ifchk_char)
				{
					lhsvl  = ifchk_char->race;
					rhsvl  = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);
				}
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "roomsector"))
			{
				lhsvl = atol(arg);

				if (get_room_index(lhsvl) == NULL)
				{
					lhsvl = mob->in_room->sector_type;
				}
				else
				{
					lhsvl = room_index[lhsvl]->sector_type;
				}
				rhsvl  = atol(val);
				retval = mudprog_veval(lhsvl, opr, rhsvl);
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "reinc"))
			{
				if (ifchk_char)
				{
					lhsvl  = IS_NPC(ifchk_char) ? 0 : actor->pcdata->reincarnation;
					rhsvl  = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);
				}
				pop_call();
				return retval;
			}
			break;

		case 's':
			if (!strcmp(buf, "sex"))
			{
				if (ifchk_char)
				{
					lhsvl  = URANGE(0, ifchk_char->sex, 2);
					rhsvl  = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);
				}
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "shotfrom"))
			{
				switch(arg[1])
				{
					case 'i':
						retval = mudprog_seval(dir_name[valid_exit], opr, val);
						break;
				}
				pop_call();
				return retval;
			}
			break;

		case 't':

			if (!strcmp(buf, "time"))
			{
				lhsvl  = mud->time_info->hour;
				rhsvl  = atol(val);
				retval = mudprog_veval(lhsvl,opr,rhsvl);
				pop_call();
				return retval;
			}
			break;

		case 'v':

			if (!strcmp( buf, "validexit"))
			{
				sh_int door;

				if (mob->in_room == NULL)
				{
					pop_call();
					return FALSE;
				}

				pfind_mob = mob;

				if (arg[0] == '\0')
				{
					for (lhsvl = cnt = door = 0 ; door < 6 ; door++)
					{
						if (get_exit(mob->in_room->vnum, door) && valid_mp_exit(mob->in_room->exit[door]))
						{
							if (number_range(0, cnt) == 0)
							{
								valid_exit = door;
								lhsvl      = mob->in_room->exit[door]->to_room;
							}
							cnt++;
						}
					}
					pop_call();
					return cnt;
				}
				door = direction_door(arg);

				if (door < 0 || door > 5)
				{
					log_build_printf(vnum, "Bad ifcheck: <validexit (%s)>", arg);
					pop_call();
					return FALSE;
				}

				if (mob->in_room->exit[door] && valid_mp_exit(mob->in_room->exit[door]))
				{
					valid_exit = door;
					lhsvl      = mob->in_room->exit[door]->to_room;
					pop_call();
					return TRUE;
				}
				pop_call();
				return FALSE;
			}
			break;

		case 'w':

			if (!strcmp(buf, "wearloc"))
			{
				if (ifchk_obj)
				{
					lhsvl  = ifchk_obj->wear_loc;
					rhsvl  = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);
				}
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "wearsobjnum"))
			{
				if (ifchk_char)
				{
					rhsvl  = ((mp_obj = get_obj_wear_vnum(ifchk_char, atol(val))) != NULL);
					retval = mudprog_veval(1, opr, rhsvl);
				}
				pop_call();
				return retval;
			}

			if (!strcmp(buf, "whichgod"))
			{
				if (ifchk_char)
				{
					lhsvl  = which_god(ifchk_char);
					rhsvl  = atol(val);
					retval = mudprog_veval(lhsvl, opr, rhsvl);
				}
				pop_call();
				return retval;
			}
			break;
	}
	log_build_printf(vnum, "Unknown ifcheck <%s>", buf);

	pop_call();
	return FALSE;
}


/*
	This routine handles the variables for command expansion.
*/

char arg_save[MAX_INPUT_LENGTH];

void mudprog_translate(char ch, char *t, CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo, CHAR_DATA *rndm)
{
	static char *he_she		[] = { "it",  "he",  "she" };
	static char *him_her	[] = { "it",  "him", "her" };
	static char *his_her	[] = { "its", "his", "her" };

	push_call("mudprog_translate(%p,%p,%p,%p,%p,%p,%p)",ch,t,mob,actor,obj,vo,rndm);

	*t = '\0';

	switch ( ch)
	{
		case 'a':
			sprintf(t, "%d", mob->pIndexData->area->low_r_vnum);
			break;

		case 'A':
			strcpy(t, mob->pIndexData->area->name);
			break;

		case 'd':
			sprintf(t, "%d", valid_exit);
			break;
		case 'D':
			if (valid_exit < 0 || valid_exit > 5)
			{
				log_build_printf(mob->pIndexData->vnum, "bad exit direction in mprog");
			}
			else
			{
				strcpy(t, dir_name[valid_exit]);
			}
			break;

		case 'i':
			one_argument_nolower(mob->name, t);
			break;
		case 'I':
			strcpy(t, get_name(mob));
			break;

		case 'n':
			if (!actor || !actor->name)
			{
				log_build_printf(mob->pIndexData->vnum, "null target in mprog");
				if (!rndm || !rndm->name)
				{
					break;
				}
				else
				{
					actor = rndm;
				}
			}
			one_argument_nolower(actor->name, t);
			break;

		case 'N':
			if (!actor)
			{
				if (!rndm)
				{
					break;
				}
				else
				{
					actor = rndm;
				}
			}
			strcpy(t, get_name(actor));
			break;

		case 't':
			if (mp_char)
			{
				one_argument_nolower(mp_char->name, t);
			}
			break;
		case 'T':
			if (mp_char)
			{
				strcpy(t, get_name(mp_char));
			}
			break;

		case 'r':
			if (rndm && rndm->name)
			{
				one_argument_nolower(rndm->name, t);
			}
			break;
		case 'R':
			if (rndm)
			{
				strcpy(t, get_name(rndm));
			}
			break;

		case 'e':
			if (actor)
			{
				strcpy(t, he_she[ URANGE(0, actor->sex, 2) ]);
			}
			break;
		case 'm':
			if (actor)
			{
				strcpy(t, him_her[ URANGE(0, actor->sex, 2) ]);
			}
			break;
		case 's':
			if (actor)
			{
				strcpy(t, his_her[ URANGE(0, actor->sex, 2) ]);
			}
			break;

 		case 'E':
			if (mp_char)
			{
				strcpy(t, he_she[ URANGE(0, mp_char->sex, 2) ]);
			}
			break;
		case 'M':
			if (mp_char)
			{
				strcpy(t, him_her[ URANGE(0, mp_char->sex, 2) ]);
			}
			break;
 		case 'S':
 			if (mp_char)
 			{
				strcpy(t, his_her[ URANGE(0, mp_char->sex, 2) ]);
			}
			break;

		case 'j':
			strcpy(t, he_she[ URANGE(0, mob->sex, 2) ]);
			break;
		case 'k':
			strcpy(t, him_her[ URANGE(0, mob->sex, 2) ]);
			break;
		case 'l':
			strcpy(t, his_her[ URANGE(0, mob->sex, 2) ]);
			break;

		case 'J':
			if (rndm)
			{
				strcpy(t, he_she[ URANGE(0, rndm->sex, 2) ]);
			}
			break;
		case 'K':
			if (rndm)
			{
				strcpy(t, him_her[ URANGE(0, rndm->sex, 2) ]);
			}
			break;
		case 'L':
			if (rndm)
			{
				strcpy(t, his_her[ URANGE(0, rndm->sex, 2) ]);
			}
			break;

		case 'o':
			if (obj)
			{
				sprintf(t, "%lld", obj->obj_ref_key);
			}
			break;
		case 'O':
			if (obj && obj->short_descr)
			{
				strcpy(t, obj->short_descr);
			}
			break;

		case 'c':
			if (mp_obj && mp_obj->name)
			{
				one_argument_nolower(mp_obj->name, t);
			}
			break;
		case 'C':
			if (mp_obj && mp_obj->short_descr)
			{
				strcpy(t, mp_obj->short_descr);
			}
			break;

		case 'x':
			sprintf(t, "%d", lhsvl);
			break;

		case 'X':
			sprintf(t, "%s", mob->npcdata->remember);
			break;

		case '$':
			strcpy(t, "$");
			break;

		case '/':
			strcpy(t, "\n\r");
			break;

		case '0':
			strcpy(t, arg_save);
			break;

		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			{
				int cnt, n;
				char *tmp;

				tmp = arg_save;
				n   = (int) ch - (int) '0';

				for (cnt = 0 ; cnt < n ; cnt++)
				{
					tmp = one_argument_nolower(tmp, t);
				}
				one_argument_nolower(t, t);
				break;
			}
		default:
			log_build_printf(mob->pIndexData->vnum, "Bad $var");
			break;
	}
	pop_call();
	return;
}

/*
	This procedure simply copies the cmnd to a buffer while expanding
	any variables by calling the translate procedure.
*/

char *mudprog_cmd_translate( char *cmnd, CHAR_DATA *mob, CHAR_DATA *actor,OBJ_DATA *obj, void *vo, CHAR_DATA *rndm)
{
	char tmp[ MAX_INPUT_LENGTH ];
	char *str;
	char *i;
	char *point;
	point	= mudprog_cmd_translate_buf;
	str		= cmnd;

	push_call("mudprog_cmd_translate(%p,%p,%p,%p,%p,%p)",cmnd,mob,actor,obj,vo,rndm);

	while ( *str != '\0')
	{
		if ( *str != '$')
		{
			*point++ = *str++;
			continue;
		}
		str++;
		mudprog_translate( *str, tmp, mob, actor, obj, vo, rndm);
		i = tmp;
		++str;
		while ( ( *point = *i) != '\0')
		{
			++point, ++i;
		}
	}
	*point = '\0';
	pop_call();
	return( mudprog_cmd_translate_buf);
}

/*
	This routine is called whenever a trigger is successful.
*/

void mudprog_driver(MUD_PROG *mprog, CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo, int vnum)
{
	CHAR_DATA *rndm = NULL;
	CHAR_DATA *vch  = NULL;
	MP_TOKEN *tok, *toktmp;
	int cnt = 0;
	long long mp_start, mp_end;

	push_call_format("mudprog_driver(%p,%p,%p,%p,%d)",mprog,mob,actor,obj,vo,vnum);

	mp_start = get_game_usec();


	/*
		Only execute progs containing $r with a player in the room
	*/

	if (HAS_BIT(mprog->flags, MPTRIGGER_RAND_PLR))
	{
		if (mob->in_room->area->nplayer > 0)
		{
			for (vch = mob->in_room->first_person ; vch ; vch = vch->next_in_room)
			{
				if (!IS_NPC(vch) && can_see(mob, vch))
				{
					if (number_range(0, cnt) == 0)
					{
						rndm = vch;
					}
					cnt++;
				}
			}
		}
		if (rndm == NULL)
		{
			pop_call();
			return;
		}
	}


	if (!IS_NPC(mob))
	{
		if (mob->desc == NULL || mob->desc->character != mob)
		{
			pop_call();
			return;
		}

		if (HAS_BIT(mob->pcdata->interp, INTERP_PROG))
		{
			pop_call();
			return;
		}

		SET_BIT(mob->pcdata->interp, INTERP_PROG);

		mob->trust = MAX_LEVEL - 1;
	}

	if (mud->mudprog_nest > MAX_MUDPROG_NEST)
	{
		log_build_printf(vnum, "mud_prog_nest exceeded maximum");

		build_dump_stack(vnum);

		pop_call();
		return;
	}

	mud->mudprog_nest++;

	tok = mprog->first_token;

	for (tok = mprog->first_token ; tok ; tok = toktmp)
	{
		toktmp = execute_mud_prog(tok, mob, actor, obj, rndm, tok->level, vnum);
	}

	if (!IS_NPC(mob))
	{
		DEL_BIT(mob->pcdata->interp, INTERP_PROG);

		mob->trust = mob->level;
	}

	mud->mudprog_nest--;

	mp_end = get_game_usec();

	if (mp_end - mp_start > 100000)
	{
		log_build_printf(vnum, "mprog driver: %lld usec to execute mob prog", mp_end - mp_start);
		build_dump_stack(vnum);
	}

	if (HAS_BIT(mud->flags, MUD_SKIPOUTPUT))
	{
		DEL_BIT(mud->flags, MUD_SKIPOUTPUT);
	}

	pop_call();
	return;
}


char mp_switch_buf[MAX_INPUT_LENGTH];

MP_TOKEN *execute_mud_prog(MP_TOKEN *token, CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, CHAR_DATA *rndm, sh_int level, int vnum)
{
	long long if_val;

	while (token)
	{
		if (mob == NULL || mob->name == NULL)
		{
			log_build_printf(vnum, "NULL mob in execute_mud_prog, dumping stack:");
			build_dump_stack(vnum);

			return NULL;
		}

		if (token->level < level)
		{
			return token;
		}

		if (actor && !IS_NPC(actor) && HAS_BIT(actor->act, PLR_WIZTIME))
		{
			mpdebug(actor, token);
		}

		if (mob && !IS_NPC(mob) && HAS_BIT(mob->act, PLR_WIZTIME))
		{
			mpdebug(mob, token);
		}

		switch (token->type)
		{
			case MPTOKEN_SOCIAL:
				interp_social(mob, token->value, mudprog_cmd_translate(token->string, mob, actor, obj, NULL, rndm));
				break;

			case MPTOKEN_COMMAND:
				(*cmd_table[token->value].do_fun) (mob, mudprog_cmd_translate(token->string, mob, actor, obj, NULL, rndm));
				break;

			case MPTOKEN_IF:
			case MPTOKEN_AND:
			case MPTOKEN_OR:
			case MPTOKEN_ELSEIF:
				if_val = mudprog_do_ifchck(token->string, mob, actor, obj, NULL, rndm, vnum);

				if (if_val == -1)
				{
					log_build_printf(vnum, "Bad mudprog ifcheck: %s", token->string);
				}

				if (HAS_BIT(token->value, TOKENFLAG_NOT))
				{
					if_val = !if_val;
				}

				if (if_val)
				{
					token = token->next;

					while (token)
					{
						if (token->type != MPTOKEN_OR)
						{
							break;
						}
						token = token->next;
					}

					if (token)
					{
						switch (token->type)
						{
							case MPTOKEN_ELSE:
							case MPTOKEN_ELSEIF:
							case MPTOKEN_ENDIF:
								break;

							case MPTOKEN_AND:
								continue;

							default:
								token = execute_mud_prog(token, mob, actor, obj, rndm, level + 1, vnum);
								break;
						}
					}

					while (token)
					{
						if (token->type == MPTOKEN_ELSE || token->type == MPTOKEN_ELSEIF)
						{
							token = token->next;

							while (token && token->level > level)
							{
								token = token->next;
							}
						}
						break;
					}
					continue;
				}

				if (if_val == 0)
				{
					token = token->next;

					while (token)
					{
						if (token->type == MPTOKEN_AND)
						{
							token = token->next;
						}
						else
						{
							break;
						}
					}

					while (token && token->level > level)
					{
						token = token->next;
					}

					if (token)
					{
						switch (token->type)
						{
							case MPTOKEN_OR:
							case MPTOKEN_ELSE:
							case MPTOKEN_ELSEIF:
								continue;
						}
					}
				}
				break;

			case MPTOKEN_ELSE:
				token = token->next;

				if (token)
				{
					token = execute_mud_prog(token, mob, actor, obj, rndm, level + 1, vnum);
				}
				continue;

			case MPTOKEN_ENDIF:
			case MPTOKEN_ENDSWITCH:
				break;

			case MPTOKEN_BREAK:
				return NULL;
				break;

			case MPTOKEN_SWITCH:
				sprintf(mp_switch_buf, "%s == 0", token->string);
				mudprog_do_ifchck(mp_switch_buf, mob, actor, obj, NULL, rndm, vnum);

				if_val = lhsvl;

				token = token->next;

				while (token && token->level > level)
				{
					if (actor && !IS_NPC(actor) && HAS_BIT(actor->act, PLR_WIZTIME))
					{
						mpdebug(actor, token);
					}

					if (token->type == MPTOKEN_CASE)
					{
						if (if_val == mudprog_case_val(vnum, token->string))
						{
							token = token->next;

							while (token && token->level < level + 2)
							{
								token = token->next;
							}

							if (token)
							{
								token = execute_mud_prog(token, mob, actor, obj, rndm, token->level, vnum);
							}

							while (token && token->level > level)
							{
								token = token->next;
							}
							break;
						}
					}
					else if (token->type == MPTOKEN_DEFAULT)
					{
						token = token->next;

						while (token && token->level < level + 2)
						{
							token = token->next;
						}

						if (token)
						{
							token = execute_mud_prog(token, mob, actor, obj, rndm, level + 2, vnum);
						}

						while (token && token->level > level)
						{
							token = token->next;
						}
						break;
					}

					if (token)
					{
						token = token->next;

						while (token && token->level > level + 1)
						{
							token = token->next;
						}
						continue;
					}
				}
				continue;

			default:
				log_build_printf(vnum, "Unknown mob_prog token type: %d", token->type);
				break;
		}

		if (token)
		{
			token = token->next;
		}
	}
	return token;
}

/*
	The next two routines are the basic trigger types. Either trigger
	on a certain percent, or trigger on a keyword or word phrase.
	To see how this works, look at the various trigger routines..
*/

void mprog_wordlist_check(char *arg, CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo, int type)
{
	MUD_PROG *mprg;
	int vnum;

	push_call("mprog_wordlist_check(%p,%p,%p,%p,%p,%p)",arg,mob,actor,obj,vo,type);

	mprg = mob->pIndexData->first_prog;
	vnum = mob->pIndexData->vnum;

	strcpy(arg_save, arg);

	while (mprg)
	{
		if (HAS_BIT(mprg->type, type))
		{
			if (mprg->arglist[0] == '\0')
			{
				mudprog_driver(mprg, mob, actor, obj, vo, vnum);
			}
			else if (mprg->arglist[0] == 'p' && mprg->arglist[1] == ' ')
			{
				if (!str_infix(&mprg->arglist[2], arg))
				{
					mudprog_driver(mprg, mob, actor, obj, vo, vnum);
				}
			}
			else if (mprg->arglist[0] == 'm' && mprg->arglist[1] == ' ')
			{
				if (is_multi_name_list(arg, &mprg->arglist[2]))
				{
					mudprog_driver(mprg, mob, actor, obj, vo, vnum);
				}
			}
			else
			{
				if (is_name_list(arg, mprg->arglist))
				{
					mudprog_driver(mprg, mob, actor, obj, vo, vnum);
				}
			}
		}
		mprg = mprg->next;
	}
	pop_call();
	return;
}

bool oprog_wordlist_check(char *cmd, char *arg, CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo, int type)
{
	MUD_PROG *oprg;
	int vnum;

	push_call("oprog_wordlist_check(%p,%p,%p,%p,%p,%p)",arg,mob,actor,obj,vo,type);

	oprg = obj->pIndexData->first_prog;
	vnum = obj->pIndexData->vnum;

	strcpy(arg_save, arg);

	while (oprg)
	{
		if (HAS_BIT(oprg->type, type))
		{
			if (is_name_list(cmd, oprg->arglist))
			{
				mudprog_driver(oprg, mob, actor, obj, vo, vnum);

				pop_call();
				return TRUE;
			}
		}
		oprg = oprg->next;
	}
	pop_call();
	return FALSE;
}

bool rprog_wordlist_check(char *cmd, char *arg, CHAR_DATA *mob, CHAR_DATA *actor, ROOM_INDEX_DATA *room, void *vo, int type)
{
	MUD_PROG *rprg;
	int vnum;

	push_call("rprog_wordlist_check(%p,%p,%p,%p,%p,%p)",arg,mob,actor,room,vo,type);

	rprg = room->first_prog;
	vnum = room->vnum;

	strcpy(arg_save, arg);

	while (rprg)
	{
		if (HAS_BIT(rprg->type, type))
		{
			if (!strcasecmp(cmd, rprg->arglist))
			{
				mudprog_driver(rprg, mob, actor, NULL, vo, vnum);

				pop_call();
				return TRUE;
			}
		}
		rprg = rprg->next;
	}
	pop_call();
	return FALSE;
}

void mudprog_percent_check(MUD_PROG *mprg, CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo, int type, int vnum)
{
	push_call_format("mudprog_percent_check(%p,%p,%p,%p,%p,%d)",mob,actor,obj,vo,type,vnum);

	while (mprg)
	{
		if (HAS_BIT(mprg->type, type) && number_percent() <= atol(mprg->arglist))
		{
			mudprog_driver(mprg, mob, actor, obj, vo, vnum);

			switch (type)
			{
				case GREET_PROG:
				case ALL_GREET_PROG:
				case GROUP_GREET_PROG:
					break;

				default:
					pop_call();
					return;
			}
		}
		mprg = mprg->next;
	}
	pop_call();
	return;
}

/*
	The triggers
*/

void mprog_time_check( CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo, int type)
{
	MUD_PROG * mprg;

	push_call("mprog_time_check(%p,%p,%p,%p,%p)",mob,actor,obj,vo,type);

	for (mprg = mob->pIndexData->first_prog ; mprg ; mprg = mprg->next)
	{
		if (mprg->type != type)
		{
			continue;
		}

		if (atol(mprg->arglist) == 24 || mud->time_info->hour == atol(mprg->arglist))
		{
			mudprog_driver(mprg, mob, actor, obj, vo, mob->pIndexData->vnum);
		}
	}
	pop_call();
	return;
}


void mprog_act_trigger( char *buf, CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj, void *vo)
{
	push_call("mprog_act_trigger(%p,%p,%p,%p,%p)",buf,mob,ch,obj,vo);

	if (HAS_BIT(mob->pIndexData->progtypes, ACT_PROG))
	{
		mprog_wordlist_check(buf, mob, ch, obj, vo, ACT_PROG);
	}
	pop_call();
	return;
}



bool mprog_social_trigger( char *social, CHAR_DATA *mob, CHAR_DATA *ch)
{
	MUD_PROG *mprg;

	push_call("mprog_social_trigger(%p,%p,%p)",social,mob,ch);

	if (!MP_VALID_MOB(mob) || !IS_AWAKE(mob))
	{
		pop_call();
		return TRUE;
	}

	if (HAS_BIT(mob->pIndexData->progtypes, SOCIAL_PROG))
	{
		for (mprg = mob->pIndexData->first_prog ; mprg ; mprg = mprg->next)
		{
			if (HAS_BIT(mprg->type, SOCIAL_PROG) && !strcasecmp(social, mprg->arglist))
			{
				mudprog_driver(mprg, mob, ch, NULL, NULL, mob->pIndexData->vnum);

				pop_call();
				return TRUE;
			}
		}
	}
	pop_call();
	return FALSE;
}

void mprog_trigger_trigger( char *txt, CHAR_DATA *mob, CHAR_DATA *ch)
{
	push_call("mprog_trigger_trigger(%p,%p,%p)",txt,mob,ch);
	
	if (HAS_BIT(mob->pIndexData->progtypes, TRIGGER_PROG))
	{
		mprog_wordlist_check(txt, mob, ch, NULL, NULL, TRIGGER_PROG);
	}
	pop_call();
	return;
}


void mprog_bribe_trigger( CHAR_DATA *mob, CHAR_DATA *ch, int amount)
{
	char			buf[ MAX_STRING_LENGTH ];
	MUD_PROG	*mprg;
	OBJ_DATA		*obj;

	push_call("mprog_bribe_trigger(%p,%p,%p)",mob,ch,amount);

	if (!MP_VALID_MOB(mob) || IS_NPC(ch) || mob->position <= POS_SLEEPING)
	{
		pop_call();
		return;
	}

	if (HAS_BIT(mob->pIndexData->progtypes, BRIBE_PROG))
	{
		for (mprg = mob->pIndexData->first_prog ; mprg ; mprg = mprg->next)
		{
			if (HAS_BIT(mprg->type, BRIBE_PROG) && amount >= atol(mprg->arglist))
			{
				obj = create_object( get_obj_index( OBJ_VNUM_MONEY_SOME), 0);
				sprintf( buf, obj->short_descr, amount);
				RESTRING(obj->short_descr, buf);
				obj->value[0]		= amount;
				obj_to_char( obj, mob);
				mob->gold -= amount;

				mudprog_driver(mprg, mob, ch, obj, NULL, mob->pIndexData->vnum);
				break;
			}
		}
	}
	pop_call();
	return;
}

void mprog_death_trigger( CHAR_DATA *mob)
{
	CHAR_DATA *who;

	push_call("mprog_death_trigger(%p)",mob);

	if (!MP_VALID_MOB(mob))
	{
		pop_call();
		return;
	}

	who = who_fighting( mob);

	if (who && who != mob)
	{
		if (HAS_BIT(mob->pIndexData->progtypes, DEATH_PROG))
		{
			mob->position = POS_STANDING;
			SET_BIT(mob->act, ACT_WILL_DIE); /* avoid death prog triggering twice - Scandum */

			mudprog_percent_check(mob->pIndexData->first_prog, mob, who, NULL, NULL, DEATH_PROG, mob->pIndexData->vnum);

			mob->position = POS_DEAD;
			DEL_BIT(mob->act, ACT_WILL_DIE);
		}
	}
	death_cry( mob);
	pop_call();
	return;
}

void mprog_kill_trigger( CHAR_DATA *mob)
{
	push_call("mprog_kill_trigger(%p)",mob);

	if (!MP_VALID_MOB(mob) || mob->fighting == NULL)
	{
		pop_call();
		return;
	}

	if (!IS_NPC(mob->fighting->who) && HAS_BIT(mob->pIndexData->progtypes, KILL_PROG))
	{
		mudprog_percent_check(mob->pIndexData->first_prog, mob, mob->fighting->who, NULL, NULL, KILL_PROG, mob->pIndexData->vnum);
	}
	pop_call();
	return;
}

void mprog_entry_trigger( CHAR_DATA *mob)
{
	push_call("mprog_entry_trigger(%p)",mob);

	if (MP_VALID_MOB(mob) && HAS_BIT(mob->pIndexData->progtypes, ENTRY_PROG))
	{
		mudprog_percent_check(mob->pIndexData->first_prog, mob, NULL, NULL, NULL, ENTRY_PROG, mob->pIndexData->vnum);
	}

	pop_call();
	return;
}

void mprog_fight_trigger( CHAR_DATA *mob, CHAR_DATA *ch)
{
	push_call("mprog_fight_trigger(%p,%p)",mob,ch);

	if (MP_VALID_MOB(mob) && HAS_BIT(mob->pIndexData->progtypes, FIGHT_PROG))
	{
		mudprog_percent_check(mob->pIndexData->first_prog, mob, ch, NULL, NULL, FIGHT_PROG, mob->pIndexData->vnum);
	}
	pop_call();
	return;
}

void mprog_give_trigger( CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj)
{
	MUD_PROG	*mprg;

	push_call("mprog_give_trigger(%p,%p,%p)",mob,ch,obj);

	if (!MP_VALID_MOB(mob) || mob->position <= POS_SLEEPING)
	{
		pop_call();
		return;
	}

	if (IS_OBJ_STAT(obj, ITEM_FORGERY))
	{
		pop_call();
		return;
	}

	if (HAS_BIT(mob->pIndexData->progtypes, GIVE_PROG))
	{
		for (mprg = mob->pIndexData->first_prog ; mprg ; mprg = mprg->next)
		{
			if (HAS_BIT(mprg->type, GIVE_PROG) && is_name(mprg->arglist, obj->name))
			{
				mudprog_driver(mprg, mob, ch, obj, NULL, mob->pIndexData->vnum);
				break;
			}
		}
	}
	pop_call();
	return;
}

void mprog_greet_trigger( CHAR_DATA *ch)
{
	CHAR_DATA *vmob;

	push_call("mprog_greet_trigger(%p)",ch);

	if (!IS_NPC(ch))
	{
		for (vmob = ch->in_room->first_person ; vmob ; vmob = vmob->next_in_room)
		{
			if (!MP_VALID_MOB(vmob) || ch == vmob || vmob->fighting || !IS_AWAKE(vmob) || vmob->in_room != ch->in_room)
			{
				continue;
			}
			if (can_see(vmob, ch) && HAS_BIT(vmob->pIndexData->progtypes, GREET_PROG))
			{
				mudprog_percent_check(vmob->pIndexData->first_prog, vmob, ch, NULL, NULL, GREET_PROG, vmob->pIndexData->vnum);
			}
			else if (HAS_BIT(vmob->pIndexData->progtypes, ALL_GREET_PROG))
			{
				mudprog_percent_check(vmob->pIndexData->first_prog, vmob, ch, NULL, NULL, ALL_GREET_PROG, vmob->pIndexData->vnum);
			}
			else if (can_see(vmob, ch) && HAS_BIT(vmob->pIndexData->progtypes, GROUP_GREET_PROG))
			{
				if (mud->mp_group_greeted == NULL)
				{
					mud->mp_group_greeter = vmob;
					mud->mp_group_greeted = ch;
				}
				else if (is_same_group(ch, mud->mp_group_greeted))
				{
					mud->mp_group_greeter = vmob;
					mud->mp_group_greeted = ch;
				}
			}
		}
	}
	pop_call();
	return;
}

void mprog_hitprcnt_trigger(CHAR_DATA *mob, CHAR_DATA *ch)
{
	MUD_PROG *mprg;

	push_call("mprog_hitprcnt_trigger(%p,%p)",mob,ch);

	if (MP_VALID_MOB(mob) && HAS_BIT(mob->pIndexData->progtypes, HITPRCNT_PROG))
	{
		for (mprg = mob->pIndexData->first_prog ; mprg ; mprg = mprg->next)
		{
			if (HAS_BIT(mprg->type, HITPRCNT_PROG) && 100 * mob->hit / UMAX(1, mob->max_hit) <= atol(mprg->arglist))
			{
				mudprog_driver(mprg, mob, ch, NULL, NULL, mob->pIndexData->vnum);
				break;
			}
		}
	}
	pop_call();
	return;

}

void mprog_repop_trigger( CHAR_DATA *mob)
{
	push_call("mprog_repop_trigger(%p)",mob);

	if (HAS_BIT(mob->pIndexData->progtypes, REPOP_PROG))
	{
		mudprog_percent_check(mob->pIndexData->first_prog, mob, NULL, NULL, NULL, REPOP_PROG, mob->pIndexData->vnum);
	}
	pop_call();
	return;
}

void mprog_delay_trigger(CHAR_DATA *mob, bool index)
{
	MUD_PROG *mprg;

	push_call("mprog_delay_trigger(%p,%p)",mob,index);

	if (mob->position == POS_SLEEPING)
	{
		pop_call();
		return;
	}

	if (HAS_BIT(mob->pIndexData->progtypes, DELAY_PROG))
	{
		for (mprg = mob->pIndexData->first_prog ; mprg ; mprg = mprg->next)
		{
			if (HAS_BIT(mprg->type, DELAY_PROG) && index == atol(mprg->arglist))
			{
				mob->npcdata->delay_index = 0;
				mudprog_driver(mprg, mob, NULL, NULL, NULL, mob->pIndexData->vnum);
				break;
			}
		}
	}
	pop_call();
	return;
}

void mprog_speech_trigger( char *txt, CHAR_DATA *mob)
{
	CHAR_DATA *vmob;

	push_call("mprog_speech_trigger(%p,%p)",txt,mob);

	for (vmob = mob->in_room->first_person ; vmob ; vmob = vmob->next_in_room)
	{
		if (MP_VALID_MOB(vmob) && vmob != mob && HAS_BIT(vmob->pIndexData->progtypes, SPEECH_PROG) && IS_AWAKE(vmob))
		{
			mprog_wordlist_check( txt, vmob, mob, NULL, NULL, SPEECH_PROG);
		}
	}
	pop_call();
	return;
}

bool mprog_desc_trigger(CHAR_DATA *mob, CHAR_DATA *ch, char *txt)
{
	MUD_PROG *mprg;

	push_call("mprog_desc_trigger(%p,%p,%p)",mob,ch,txt);

	if (MP_VALID_MOB(mob) && HAS_BIT(mob->pIndexData->progtypes, DESC_PROG))
	{
		for (mprg = mob->pIndexData->first_prog ; mprg ; mprg = mprg->next)
		{
			if (HAS_BIT(mprg->type, DESC_PROG))
			{
				if (is_name(txt, mprg->arglist) || !strcmp(txt, mprg->arglist))
				{
					mudprog_driver(mprg, mob, ch, NULL, NULL, mob->pIndexData->vnum);

					pop_call();
					return TRUE;
				}
			}
		}
	}
	pop_call();
	return FALSE;
}

void mprog_range_trigger( CHAR_DATA *mob, CHAR_DATA *ch, bool shot_from)
{
	MUD_PROG *mprg;

	push_call("mprog_range_trigger(%p,%p)",mob,ch);

	valid_exit = shot_from;

	if (HAS_BIT(mob->pIndexData->progtypes, RANGE_PROG))
	{
		for (mprg = mob->pIndexData->first_prog ; mprg ; mprg = mprg->next)
		{
			if (HAS_BIT(mprg->type, RANGE_PROG))
			{
				mudprog_driver(mprg, mob, ch, NULL, NULL, mob->pIndexData->vnum);
				break;
			}
		}
	}
	pop_call();
	return;
}

void mprog_exit_trigger(CHAR_DATA *ch, int door)
{
	CHAR_DATA *vmob;
	MUD_PROG *mprg;

	push_call("mprog_exit_trigger(%p)",ch);

	if (!IS_NPC(ch))
	{
		for (vmob = ch->in_room->first_person ; vmob ; vmob = vmob->next_in_room)
		{
			if (!MP_VALID_MOB(vmob) || vmob->fighting || !IS_AWAKE(vmob) || vmob->in_room != ch->in_room)
			{
				continue;
			}
			if (!HAS_BIT(vmob->pIndexData->progtypes, EXIT_PROG) || !can_see(vmob, ch))
			{
				continue;
			}
			for (mprg = vmob->pIndexData->first_prog ; mprg ; mprg = mprg->next)
			{
				if (HAS_BIT(mprg->type, EXIT_PROG) && (*mprg->arglist == 0 || direction_door(mprg->arglist) == door))
				{
					mudprog_driver(mprg, vmob, ch, NULL, NULL, vmob->pIndexData->vnum);
					break;
				}
			}
		}
	}
	pop_call();
	return;
}

/*
	Room triggers
*/

bool rprog_command_trigger(CHAR_DATA *ch, char *cmd, char *arg)
{
	ROOM_INDEX_DATA *room;

	push_call("rprog_command_trigger(%p,%p,%p)",ch,cmd,arg);

	if (IS_NPC(ch))
	{
		pop_call();
		return FALSE;
	}

	if (HAS_BIT(ch->pcdata->interp, INTERP_PROG))
	{
		pop_call();
		return FALSE;
	}

	room = ch->in_room;

	if (HAS_BIT(room->progtypes, COMMAND_RPROG))
	{
		if (rprog_wordlist_check(cmd, arg, ch, NULL, room, NULL, COMMAND_RPROG))
		{
			pop_call();
			return TRUE;
		}
	}
	pop_call();
	return FALSE;
}

bool rprog_unknown_trigger(CHAR_DATA *ch, char *cmd, char *arg)
{
	ROOM_INDEX_DATA *room;

	push_call("rprog_unknown_trigger(%p,%p,%p)",ch,cmd,arg);

	room = ch->in_room;

	if (HAS_BIT(room->progtypes, UNKNOWN_RPROG))
	{
		if (rprog_wordlist_check(cmd, arg, ch, NULL, room, NULL, UNKNOWN_RPROG))
		{
			pop_call();
			return TRUE;
		}
	}
	pop_call();
	return FALSE;
}

/*
	Object programmer trigger checks
*/

bool oprog_command_trigger(CHAR_DATA *ch, char *cmd, char *arg)
{
	OBJ_DATA *obj, *obj_next;

	push_call("oprog_command_trigger(%p,%p,%p)",ch,cmd,arg);

	if (!IS_NPC(ch) && HAS_BIT(ch->pcdata->interp, INTERP_PROG))
	{
		pop_call();
		return FALSE;
	}

	for (obj = ch->first_carrying ; obj ; obj = obj_next)
	{
		obj_next = obj->next_content;

		if (HAS_BIT(obj->pIndexData->progtypes, COMMAND_OPROG))
		{
			if (oprog_wordlist_check(cmd, arg, ch, NULL, obj, NULL, COMMAND_OPROG))
			{
				pop_call();
				return TRUE;
			}
		}
	}
	pop_call();
	return FALSE;
}

void oprog_damage_trigger(CHAR_DATA *ch, OBJ_DATA *obj, CHAR_DATA *victim)
{
	push_call("oprog_damage_trigger(%p,%p,%p)",ch,obj,victim);

	if (HAS_BIT(obj->pIndexData->progtypes, DAMAGE_OPROG))
	{
		mudprog_percent_check(obj->pIndexData->first_prog, ch, victim, obj, NULL, DAMAGE_OPROG, obj->pIndexData->vnum);
	}
	pop_call();
	return;
}

void oprog_drop_trigger(CHAR_DATA *ch, OBJ_DATA *obj)
{
	push_call("oprog_drop_trigger(%p,%p)",ch,obj);

	if (HAS_BIT(obj->pIndexData->progtypes, DROP_OPROG))
	{
		mudprog_percent_check(obj->pIndexData->first_prog, ch, NULL, obj, NULL, DROP_OPROG, obj->pIndexData->vnum);
	}
	pop_call();
	return;
}

void oprog_get_trigger(CHAR_DATA *ch, OBJ_DATA *obj)
{
	push_call("oprog_get_trigger(%p,%p)",ch,obj);

	if (HAS_BIT(obj->pIndexData->progtypes, GET_OPROG))
	{
		mudprog_percent_check(obj->pIndexData->first_prog, ch, NULL, obj, NULL, GET_OPROG, obj->pIndexData->vnum);
	}
	pop_call();
	return;
}

void oprog_hit_trigger(CHAR_DATA *ch, CHAR_DATA *victim)
{
	OBJ_DATA *obj;

	push_call("oprog_hit_trigger(%p,%p)",ch,victim);

	for (obj = ch->first_carrying ; obj ; obj = obj->next)
	{
		if (HAS_BIT(obj->pIndexData->progtypes, HIT_OPROG))
		{
			mudprog_percent_check(obj->pIndexData->first_prog, ch, victim, obj, NULL, HIT_OPROG, obj->pIndexData->vnum);
		}
	}
	pop_call();
	return;
}

void oprog_rand_trigger(CHAR_DATA *ch)
{
	OBJ_DATA *obj, *obj_next;

	push_call("oprog_rand_trigger(%p)",ch);

	for (obj = ch->first_carrying ; obj ; obj = obj_next)
	{
		obj_next = obj->next_content;

		if (HAS_BIT(obj->pIndexData->progtypes, RAND_OPROG))
		{
			mudprog_percent_check(obj->pIndexData->first_prog, ch, NULL, obj, NULL, RAND_OPROG, obj->pIndexData->vnum);
		}
	}
	pop_call();
	return;
}

void oprog_remove_trigger(CHAR_DATA *ch, OBJ_DATA *obj)
{
	push_call("oprog_remove_trigger(%p,%p)",ch,obj);

	if (HAS_BIT(obj->pIndexData->progtypes, REMOVE_OPROG))
	{
		mudprog_percent_check(obj->pIndexData->first_prog, ch, NULL, obj, NULL, REMOVE_OPROG, obj->pIndexData->vnum);
	}
	pop_call();
	return;
}

void oprog_sacrifice_trigger(CHAR_DATA *ch, OBJ_DATA *obj)
{
	push_call("oprog_sacrifice_trigger(%p,%p)",ch,obj);

	if (HAS_BIT(obj->pIndexData->progtypes, SACRIFICE_OPROG))
	{
		mudprog_percent_check(obj->pIndexData->first_prog, ch, NULL, obj, NULL, SACRIFICE_OPROG, obj->pIndexData->vnum);
	}
	pop_call();
	return;
}

bool oprog_unknown_trigger(CHAR_DATA *ch, char *cmd, char *arg)
{
	OBJ_DATA *obj, *obj_next;

	push_call("oprog_unknown_trigger(%p,%p,%p)",ch,cmd,arg);

	for (obj = ch->first_carrying ; obj ; obj = obj_next)
	{
		obj_next = obj->next_content;

		if (HAS_BIT(obj->pIndexData->progtypes, UNKNOWN_OPROG))
		{
			if (oprog_wordlist_check(cmd, arg, ch, NULL, obj, NULL, UNKNOWN_OPROG))
			{
				pop_call();
				return TRUE;
			}
		}
	}
	pop_call();
	return FALSE;
}

void oprog_wear_trigger(CHAR_DATA *ch, OBJ_DATA *obj)
{
	push_call("oprog_wear_trigger(%p,%p)",ch,obj);

	if (HAS_BIT(obj->pIndexData->progtypes, WEAR_OPROG))
	{
		mudprog_percent_check(obj->pIndexData->first_prog, ch, who_fighting(ch), obj, NULL, WEAR_OPROG, obj->pIndexData->vnum);
	}
	pop_call();
	return;
}

