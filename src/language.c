/***************************************************************************
 * Lola  1.0 by Igor van den Hoven.                                        *
 *                                                                         *
 * Emud  2.2 by Igor van den Hoven, Michiel Lange.                         *
 *                                                                         *
 * MrMud 1.4 by David Bills and Dug Michael.                               *
 ***************************************************************************/

#include "mud.h"

void do_speak( CHAR_DATA *ch, char *argument )
{
	int cnt;

	push_call("do_speak(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (argument[0] == '\0')
	{
		send_to_char("Speak what language?\n\r", ch);

		pop_call();
		return;
	}
	argument[2] = '\0';

	for (cnt = 0 ; cnt < MAX_LANGUAGE ; cnt++)
	{
		if (!str_prefix(argument, language_table[cnt].name))
		{
			if (IS_SET(ch->pcdata->language, 1 << cnt))
			{
				ch_printf(ch, "You'll now speak %s.\n\r", language_table[cnt].name);
				ch->pcdata->speak = 1 << cnt;

				pop_call();
				return;
			}
		}
	}
	send_to_char("You cannot speak that language.\n\r", ch);

	pop_call();
	return;
}

void add_language( CHAR_DATA *ch )
{
	int cnt, end;

	push_call("add_language(%p)",ch);

	end = cnt = number_range(0, MAX_RACE_LANGUAGE-1);

	while (TRUE)
	{
		cnt++;

		if (cnt == MAX_RACE_LANGUAGE)
		{
			cnt = 0;
		}

		if (cnt == end)
		{
			pop_call();
			return;
		}

		if (!HAS_BIT(ch->pcdata->language, 1 << cnt))
		{
			break;
		}
	}

	SET_BIT(ch->pcdata->language, 1 << cnt);

	/*
		preventing from showing to new users and reincarnators, Manwe, 15/10/1999
	*/

	if (ch->level > 2)
	{
		ch_printf(ch, "You just learned a new language : %s\n\r", language_table[cnt].name);
	}
	pop_call();
	return;
}

int total_language( CHAR_DATA *ch )
{
	int total, cnt;

	push_call("total_language(%p)",ch);

	for (total = cnt = 0 ; cnt < MAX_RACE_LANGUAGE ; cnt++)
	{
		if (IS_SET(ch->pcdata->language, 1 << cnt))
		{
			total++;
		}
	}
	pop_call();
	return total;
}

void fix_language(CHAR_DATA *ch)
{
	int lan, tot;

	tot = total_language(ch);

	for (lan = 3 + ch->level / 15 ; lan > tot ; tot++)
	{
		add_language(ch);
	}
}

/*
	Utter mystical words for an sn.
*/

void say_spell( CHAR_DATA *ch, int sn )
{
	char buf  [MAX_STRING_LENGTH];
	char buf2 [MAX_STRING_LENGTH];
	CHAR_DATA *rch;
	char *pName;
	int iSyl;
	int length;

	struct syl_type
	{
		char *old;
		char *new;
	};

	static const struct syl_type syl_table[] =
	{
		{ " ",      " "      },
		{ "ar",     "abra"   },
		{ "au",     "kada"   },
		{ "bless",  "fido"   },
		{ "blind",  "nose"   },
		{ "bur",    "mosa"   },
		{ "cu",     "judi"   },
		{ "de",     "oculo"  },
		{ "en",     "unso"   },
		{ "hance",  "sch"    },
		{ "light",  "dies"   },
		{ "lo",     "hi"     },
		{ "mor",    "zak"    },
		{ "move",   "sido"   },
		{ "ob",     "kn"     },
		{ "ness",   "lacri"  },
		{ "ning",   "illa"   },
		{ "per",    "duda"   },
		{ "ra",     "gru"    },
		{ "re",     "candus" },
		{ "son",    "sabru"  },
		{ "tect",   "infra"  },
		{ "tri",    "cula"   },
		{ "ven",    "nofo"   },
		{ "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
		{ "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
		{ "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
		{ "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
		{ "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
		{ "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
		{ "y", "l" }, { "z", "k" }, {  "", ""  }
	};

	push_call("say_spell(%p,%p)",ch,sn);

	buf[0]	= '\0';

	for (pName = skill_table[sn].name ; *pName != '\0' ; pName += length)
	{
		for (iSyl = 0 ; (length = strlen(syl_table[iSyl].old)) != 0 ; iSyl++)
		{
			if (!str_prefix(syl_table[iSyl].old, pName))
			{
				strcat(buf, syl_table[iSyl].new);
				break;
			}
		}

		if ( length == 0 )
		{
			length = 1;
		}
	}

	sprintf( buf2, "$n utters the words, '%s'.", buf );
	sprintf( buf,  "$n utters the words, '%s'.", skill_table[sn].name );

	/*
	   make wizcloaked gods even more invis,
	   their uttering cannot be heard...
	*/

	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_WIZCLOAK))
	{
		pop_call();
		return;
	}

	for (rch = ch->in_room->first_person ; rch ; rch = rch->next_in_room)
	{
		if (rch != ch)
		{
			if (IS_NPC(rch) || !IS_SET(rch->pcdata->spam, 4096))
			{
				if (IS_NPC(rch) || learned(rch, sn) == 0
				|| (IS_AFFECTED(ch, AFF2_QUICKEN) && !IS_AFFECTED(rch, AFF2_QUICKEN))
				|| (skill_table[sn].follower && which_god(ch) != which_god(rch))
				|| (skill_table[sn].race && ch->race != rch->race))
				{
					act(buf2, ch, NULL, rch, TO_VICT);
				}
				else
				{
					act(buf,  ch, NULL, rch, TO_VICT);
				}
			}
		}
	}

	pop_call();
	return;
}

bool can_understand(CHAR_DATA * victim, CHAR_DATA * ch, bool fDisplay)
{
	int language, speak;

	push_call("can_understand(%p,%p)",victim,ch);

	if (IS_NPC(victim) && !IS_SET(victim->act, ACT_RACE))
	{
		pop_call();
		return TRUE;
	}

	if (IS_NPC(ch) && !IS_SET(ch->act, ACT_RACE))
	{
		pop_call();
		return TRUE;
	}

	if (IS_AFFECTED(ch, AFF_TONGUES) || IS_AFFECTED(victim, AFF_UNDERSTAND))
	{
		pop_call();
		return TRUE;
	}

	if (ch->level >= LEVEL_IMMORTAL || victim->level >= LEVEL_IMMORTAL)
	{
		pop_call();
		return TRUE;
	}

	language = IS_NPC(victim) ? race_table[victim->race].language : victim->pcdata->language;
	speak    = IS_NPC(ch)     ? race_table[ch->race].language     : ch->pcdata->speak;

	if (!IS_SET(language, speak))
	{
		if (fDisplay)
		{
			ch_printf(ch, "%s does not understand %s.\n\r", capitalize(get_name(victim)), language_table[UNSHIFT(speak)].name);
		}
		pop_call();
		return FALSE;
	}
	pop_call();
	return TRUE;
}

char *translate (CHAR_DATA *ch, char *text)
{
	static char text2[MAX_STRING_LENGTH];
	char *pName;
	int iSyl, length, speak;

	struct syl_type
	{
		char *old;
		char *new;
	};

	static const struct syl_type syl_table[] =
	{
		{ " ",  " "  },
		{ "ar",	"ug" },
		{ "au",	"ja" },
		{ "le",	"fi" },
		{ "li",	"ni" },
		{ "ur",	"ir" },
		{ "cu",	"je" },
		{ "de",	"ca" },
		{ "en",	"un" },
		{ "li",	"fu" },
		{ "lo",	"hi" },
		{ "mo",	"za" },
		{ "ma",	"do" },
		{ "ne",	"la" },
		{ "ni",	"ta" },
		{ "pe",	"da" },
		{ "ra",	"ru" },
		{ "re",	"ca" },
		{ "so",	"sa" },
		{ "ec",	"in" },
		{ "ri",	"lu" },
		{ "en",	"of" },
		{ "a",  "i"  },
		{ "b",  "t"  },
		{ "c",  "f"  },
		{ "d",  "p"  },
		{ "e",  "u"  },
		{ "f",  "l"  },
		{ "g",  "j"  },
		{ "h",  "s"  },
		{ "i",  "e"  },
		{ "j",  "n"  },
		{ "k",  "b"  },
		{ "l",  "k"  },
		{ "m",  "g"  },
		{ "n",  "r"  },
		{ "o",  "a"  },
		{ "p",  "y"  },
		{ "q",  "d"  },
		{ "r",  "m"  },
		{ "s",  "h"  },
		{ "t",  "w"  },
		{ "u",  "o"  },
		{ "v",  "x"  },
		{ "w",  "q"  },
		{ "x",  "z"  },
		{ "y",  "c"  },
		{ "z",  "v"  },
		{ "?",  "?"  },
		{ "!",  "!"  },
		{ ".",  "."  },
		{ ")",  ")"  },
		{ "(",  "("  },
		{ ":",  ":"  },
		{ "'",  "'"  },
		{ "-",  "-"  },
		{ "=",  "="  },
		{ "*",  "*"  },
		{ "%",  "%"  },
		{ ",",  ","  },
		{ "<",  "<"  },
		{ ">",  ">"  },
		{ "",   ""   }
	};

	static const struct syl_type for_table[] =
	{
		{ " ",  " "  },
		{ "ar",	"ox" },
		{ "au",	"uu" },
		{ "le",	"ki" },
		{ "li",	"ko" },
		{ "ur",	"ak" },
		{ "cu",	"xi" },
		{ "de",	"ka" },
		{ "en",	"yx" },
		{ "li",	"qa" },
		{ "lo",	"qo" },
		{ "mo",	"zi" },
		{ "ma",	"ci" },
		{ "ne",	"ki" },
		{ "ni",	"xy" },
		{ "pe",	"po" },
		{ "ra",	"ru" },
		{ "re",	"ca" },
		{ "so",	"xi" },
		{ "ec",	"ic" },
		{ "ri",	"ba" },
		{ "en",	"ox" },
		{ "a",  "i"  },
		{ "b",  "t"  },
		{ "c",  "g"  },
		{ "d",  "q"  },
		{ "e",  "u"  },
		{ "f",  "m"  },
		{ "g",  "k"  },
		{ "h",  "t"  },
		{ "i",  "u"  },
		{ "j",  "p"  },
		{ "k",  "c"  },
		{ "l",  "k"  },
		{ "m",  "k"  },
		{ "n",  "x"  },
		{ "o",  "u"  },
		{ "p",  "y"  },
		{ "q",  "f"  },
		{ "r",  "n"  },
		{ "s",  "k"  },
		{ "t",  "z"  },
		{ "u",  "o"  },
		{ "v",  "x"  },
		{ "w",  "q"  },
		{ "x",  "z"  },
		{ "y",  "c"  },
		{ "z",  "x"  },
		{ "?",  "?"  },
		{ "!",  "!"  },
		{ ".",  "."  },
		{ ")",  ")"  },
		{ "(",  "("  },
		{ ":",  ":"  },
		{ "'",  "'"  },
		{ "-",  "-"  },
		{ "=",  "="  },
		{ "*",  "*"  },
		{ "%",  "%"  },
		{ ",",  ","  },
		{ "<",  "<"  },
		{ ">",  ">"  },
		{ "",   ""   }
	};

	push_call("translate(%p)",text);

	speak = IS_NPC(ch) ? race_table[ch->race].language : ch->pcdata->speak;

	text2[0] = '\0';

	for (pName = text ; *pName != '\0' ; pName += length)
	{
		for (iSyl = 0 ; (length = strlen (syl_table[iSyl].old)) != 0 ; iSyl++)
		{
			if (speak == LANG_EQUILIBRIUM)
			{
				if (!str_prefix(for_table[iSyl].old, pName))
				{
					strcat(text2, syl_table[iSyl].new);
					break;
				}
			}
			else
			{
				if (!str_prefix(syl_table[iSyl].old, pName))
				{
					strcat(text2, syl_table[iSyl].new);
					break;
				}
			}
		}
		if (length == 0)
		{
			length = 1;
		}
	}
	pop_call();
	return text2;
}

