/***************************************************************************
 * Lola  1.0 by Igor van den Hoven.                                        *
 *                                                                         *
 * Emud  2.2 by Igor van den Hoven, Michiel Lange.                         *
 ***************************************************************************/

#include "mud.h"

/*
	Lets use a table instead - Scandum 28-05-2002
*/

char * const drunk_table[] =
{
	"a",		"A",		"aa",	"ah",	"Ah",	"ao",	"aw",
	"b",		"bu",	"ba",	"B",		"B",		"be",	"buh",
	"c",		"c",		"C",		"cC",	"Cc",	"cc",	"CC",
	"d",		"d",		"D",		"dh",	"dd",	"dD",	"Dd",
	"e",		"e",		"eh",	"E",		"eE",	"Ee",	"he",
	"f",		"f",		"ff",	"fff",	"fFf",	"F",		"Ff",
	"g",		"g",		"G",		"gh",	"Gh",	"gH",	"hG",
	"h",		"h",		"hh",	"hhh",	"Hhh",	"HhH",	"H",
	"i",		"i",		"I",		"i",		"I",		"I",		"I",
	"j",		"j",		"jj",	"Jj",	"jJ",	"J",		"JJ",
	"k",		"k",		"K",		"kk",	"Kk",	"kK",	"ku",
	"l",		"l",		"L",		"lL",	"l",		"L",		"ll",
	"m",		"m",		"mm",	"mmm",	"mMm",	"MmM",	"M",
	"n",		"n",		"nn",	"Nn",	"nn",	"nNn",	"N",
	"o",		"o",		"ooo",	"ao",	"aOoo",	"OooOo",	"ooOo",
	"p",		"p",		"P",		"phh",	"pHh",	"PHh",	"ppP",
	"q",		"q",		"Q",		"qq",	"qu",	"qQ",	"qQ",
	"r",		"r",		"R",		"rr",	"rR",	"Rr",	"rrr",
	"s",		"s",		"ss",	"sss",	"ssZs",	"sSzsZs",	"ssZss",
	"t",		"t",		"tt",	"T",		"tth",	"tT",	"Tt",
	"u",		"u",		"uh",	"Uh",	"Uhuh",	"uhU", 	"uhhu",
	"v",		"v",		"V",		"vff",	"Vvf",	"vuv",	"vuh",
	"w",		"w",		"Ww",	"wwhw",	"wuf",	"WWW",	"W",
	"x",		"xx",	"Xx",	"x",		"xX",	"X",		"x",
	"y",		"y",		"Y",		"yy",	"y",		"yY",	"Yy",
	"z",		"z",		"ZzzZz",	"Zzz",	"Zssz",	"zzz",	"Z"
};

/*
	Manwe 04-01-1999
*/

char *drunkify(const char *text)
{
	static char text2[MAX_STRING_LENGTH];
	int count, len, val;

	push_call("drunkify(%p,%p)",text,text2);

	if (strlen(text) > 200)
	{
		strcpy(text2, "MmmmmmnmM jjJrrGGgh PfffrTttszz...");
		pop_call();
		return text2;
	}

	text2[0] = '\0';
	len      = strlen(text);

	if (!text)
	{
		log_string("drunkify() : const char *text = NULL");
		pop_call();
		return text2;
	}

	for (count = 0 ; count < len ; count++)
	{
		if (isalpha(text[count]))
		{
			val = LOWER(text[count]) - 'a';
			cat_sprintf(text2, drunk_table[val*7+number_range(0,6)]);
		}
		else if (isdigit(text[count]))
		{
			cat_sprintf(text2, "%c", '0' + number_range(0,9));
		}
		else
		{
			cat_sprintf(text2, "%c", text[count]);
		}
	}
	pop_call();
	return text2;
}

/*
	determines wether either a player or a mob is drunk
*/

bool is_drunk(CHAR_DATA *ch)
{
	push_call("is_drunk(%p)",ch);

	if (IS_NPC(ch) && IS_SET(ch->act, ACT_DRUNK))
	{
		pop_call();
		return TRUE;
	}
	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 0)
	{
		pop_call();
		return TRUE;
	}
	pop_call();
	return FALSE;
}
