/***************************************************************************
 * Lola  1.0 by Igor van den Hoven.                                        *
 *                                                                         *
 * Emud  2.2 by Igor van den Hoven, Michiel Lange.                         *
 *                                                                         *
 * MrMud 1.4 by David Bills and Dug Michael.                               *
 ***************************************************************************/

#include <stdarg.h>
#include "mud.h"

void do_vt100( CHAR_DATA *ch, char *arg)
{
	int cnt;
	char buf0[MAX_INPUT_LENGTH];
	char buf1[90], buf2[90], buf4[90], skp1[80], skp2[80];
	char colg[10], colw[10], colW[10];

	push_call("do_vt100(%p,%p)",ch,arg);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	arg = one_argument(arg, buf0);

	if (buf0[0] == '\0')
	{
		strcpy(colg, ansi_translate_text(ch, "{028}"));
		strcpy(colw, get_color_string(ch, COLOR_SCORE, VT102_DIM));
		strcpy(colW, get_color_string(ch, COLOR_SCORE, VT102_BOLD));

		sprintf(buf4, "%s +----------------------------------------------------------------------------+\n\r", colg);

		strcpy(buf0, buf4);

		sprintf(buf1, "%s", HAS_BIT(ch->pcdata->vt100_flags, VT100_INTERFACE) ? "ON" : "OFF");
		sprintf(buf2, "%s", IS_SET(ch->pcdata->vt100_flags, VT100_FAST) ? "FAST" : "SLOW");
		cat_sprintf(buf0, "%s |%s         VT100 Mode%s:%s %-11s %s             VT100 Speed%s:%s %-11s      %s|\n\r",
		colg,
		colw, colW, colw, str_resize(buf1, skp1, -11),
		colw, colW, colw, str_resize(buf2, skp2, -11),
		colg);


		sprintf(buf1, "%d", ch->pcdata->screensize_v);
		sprintf(buf2, "%d", ch->pcdata->screensize_h);
		cat_sprintf(buf0, "%s |%s      Terminal Rows%s:%s %-11s %s        Terminal Columns%s:%s %-11s      %s|\n\r",
			colg,
			colw, colW, colw, str_resize(buf1, skp1, -11),
			colw, colW, colw, str_resize(buf2, skp2, -11),
			colg);

		strcat(buf0, buf4);

		send_to_char_color(buf0, ch);
		pop_call();
		return;
	}

	if (!strcasecmp(buf0,  "on") && !HAS_BIT(ch->pcdata->vt100_flags, VT100_INTERFACE))
	{
		vt100on(ch);
		pop_call();
		return;
	}

	if (!strcasecmp(buf0, "off") && HAS_BIT(ch->pcdata->vt100_flags, VT100_INTERFACE))
	{
		vt100off(ch);
		pop_call();
		return;
	}

	if (!strcasecmp(buf0, "rows"))
	{
		strcpy(buf0, arg);
	}

	if ((cnt = atol(buf0)) != 0)
	{
		if (cnt < 15 || cnt > 200)
		{
			send_to_char("Rows must be between 15 and 200.\n\r", ch );
			pop_call();
			return;
		}
		ch->pcdata->screensize_v = cnt;

		cnt = URANGE(1, ch->pcdata->screensize_v / 2, MAX_TACTICAL_ROW - 1);

		if (ch->pcdata->tactical_size_v > cnt)
		{
			ch->pcdata->tactical_size_v = cnt;
		}
		do_refresh(ch, "");
		pop_call();
		return;
	}

	if (!strcasecmp(buf0, "columns"))
	{
		cnt = atol(arg);

		if (cnt < 80 || cnt > MAX_TACTICAL_COL)
		{
			ch_printf(ch, "Columns must be between 80 and %d.\n\r", MAX_TACTICAL_COL );
			pop_call();
			return;
		}
		ch->pcdata->screensize_h = cnt;

		do_refresh(ch, "");
		pop_call();
		return;
	}

	if (!strcasecmp(buf0, "fast"))
	{
		send_to_char("VT100 mode set to FAST.\n\r", ch);
		SET_BIT(ch->pcdata->vt100_flags, VT100_FAST);
		pop_call();
		return;
	}

	if (!strcasecmp(buf0, "slow"))
	{
		send_to_char("VT100 mode set to SLOW.\n\r", ch);
		REMOVE_BIT(ch->pcdata->vt100_flags, VT100_FAST);
		pop_call();
		return;
	}

	send_to_char("Syntax: VT100 [ON/OFF|FAST/SLOW|ROWS|COLUMNS] [VALUE]\n\r", ch);
	pop_call();
	return;
}


void do_color (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_INPUT_LENGTH];
	int col1, col2, col3, cnt;

	push_call("do_color(%p,%p)",ch,argument);

	if (IS_NPC (ch) || !is_desc_valid (ch))
	{
		pop_call();
		return;
	}

	if (!strcasecmp ("on", argument))
	{
		if (HAS_BIT(ch->pcdata->vt100_flags, VT100_ANSI))
		{
			send_to_char ("You already have color on.\n\r", ch);
			pop_call();
			return;
		}
		SET_BIT(ch->pcdata->vt100_flags, VT100_ANSI);
		do_refresh(ch, "");
		ch_printf_color(ch, "{300}Ansi color is ON!\n\r");
		pop_call();
		return;
	}

	if (!strcasecmp ("reset", argument))
	{
		/*
			Scandum's favorite colors
		*/
		reset_color (ch);
		do_refresh(ch, "");
		send_to_char ("Colors reset to defaults.\n\r", ch);
		pop_call();
		return;
	}

	if (!strcasecmp ("off", argument))
	{
		DEL_BIT(ch->pcdata->vt100_flags, VT100_ANSI);
		do_refresh(ch, "");
		send_to_char ("Ansi color is OFF.\n\r", ch);
		pop_call();
		return;
	}

	if (!strcasecmp ("", argument))
	{
		char bufx[40];
		char text[20];
		sprintf(text, "%s%s", ansi_translate_text(ch, "{088}"), get_color_string (ch, COLOR_TEXT, VT102_DIM));

		for (buf[0] = '\0', cnt = 0 ; cnt < COLOR_MAX ; cnt++)
		{
			switch (cnt)
			{
				case 0:  strcpy (bufx, "Standard Text             "); break;
				case 1:  strcpy (bufx, "Top Status Bar            "); break;
				case 2:  strcpy (bufx, "Bottom Status Bar         "); break;
				case 3:  strcpy (bufx, "Score, Status, and Affects"); break;
				case 4:  strcpy (bufx, "You are hit               "); break;
				case 5:  strcpy (bufx, "You hit others            "); break;
				case 6:  strcpy (bufx, "Prompt line               "); break;
				case 7:  strcpy (bufx, "Exits                     "); break;
				case 8:  strcpy (bufx, "Party member is hit       "); break;
				case 9:  strcpy (bufx, "Speech commands           "); break;
				case 10: strcpy (bufx, "Objects                   "); break;
				case 11: strcpy (bufx, "Living things             "); break;
				case 12: strcpy (bufx, "General Tactical Map      "); break;
				case 13: strcpy (bufx, "Party members in Tactical "); break;
				case 14: strcpy (bufx, "Enemies in Tactical       "); break;
				case 15: strcpy (bufx, "Neutrals in Tactical      "); break;
				case 16: strcpy (bufx, "Chat Channel              "); break;
				case 17: strcpy (bufx, "Talk Channel              "); break;
				case 18: strcpy (bufx, "Plan Channel              "); break;
			}
			cat_sprintf(buf, "%s%2d - %s%s%s  foreground %2d, background %2d\n\r", text, cnt, get_color_string(ch, cnt, VT102_DIM), bufx, text, ch->pcdata->color[cnt] % 10, ch->pcdata->color[cnt] / 10);
		}
		cat_sprintf(buf, "%s\n\r%sSyntax:  COLOR <field number> <forground number> <background number>\n\r", text, text);
		send_to_char(buf, ch);
		pop_call();
		return;
	}

	argument = one_argument (argument, buf);
	argument = one_argument (argument, buf2);
	if (buf[0] < '0' || buf[0] > '9')
	{
		send_to_char ("Try using HELP COLOR.\n\r", ch);
		pop_call();
		return;
	}
	col1 = atol (buf) % COLOR_MAX;
	col2 = atol (buf2) % 10;
	col3 = atol (argument) % 10;
	ch->pcdata->color[col1] = col2 + col3 * 10;
	do_refresh(ch, "");
	ch_printf_color(ch, "{1%d%d}Color field #%d set to color forground %d, background %d.{088}\n\r", ch->pcdata->color[col1] % 10, ch->pcdata->color[col1] / 10, col1, col2, col3);

	pop_call();
	return;
}

/*
	gives back string length not including color codes - Scandum 10-05-2002
*/

int ansi_strlen(char *text)
{
	int length = 0;

	push_call("ansi_strlen(%p)",text);

	while (*text != '\0')
	{
		if (*text == '<')
		{
			if (*(text+1) >= 'a' && *(text+1) <= 'f'
			&&  *(text+2) >= 'a' && *(text+2) <= 'f'
			&&  *(text+3) >= 'a' && *(text+3) <= 'f')
			{
				if (*(text+4) == '>')
				{
					text += 5;
					continue;
				}
			}
		}
		if (*text == '^')
		{
			if (isalpha(text[1]))
			{
				text += 2;
				continue;
			}
			if (text[1] == '^')
			{
				text += 2;
				length += 1;
				continue;
			}
		}
		if (*text == '{')
		{
			if (*(text+1) >= '0' && *(text+1) <= '8'
			&&  *(text+2) >= '0' && *(text+2) <= '8'
			&&  *(text+3) >= '0' && *(text+3) <= '8')
			{
				if (*(text+4) == '}')
				{
					text += 5;
					continue;
				}
			}
		}
		text++;
		length++;
	}
	pop_call();
	return length;
}

/*
	rewritten, will mainly be replaced by _def version - Scandum 10-05-2002
*/

char *ansi_translate_text( CHAR_DATA *ch, char *text_in )
{
	char ansi_translate_buffer[MAX_STRING_LENGTH];
	static char xterm_translate_buffer[MAX_STRING_LENGTH];

	const char *pti;
	char *pto, *pt2;
	int val1, val2, val3;
	int color;

	push_call("ansi_translate_text(%p,%p)",ch,text_in);

	pti  = text_in;
	pto  = ansi_translate_buffer;
	val1 = val2 = val3 = 0;

	if (ch->desc == NULL)
	{
		pop_call();
		return "";
	}

	while (*pti != '\0')
	{
		if (*(pti+0) == '{'
		&&  *(pti+1) >= '0' && *(pti+1) <= '8'
		&&  *(pti+2) >= '0' && *(pti+2) <= '8'
		&&  *(pti+3) >= '0' && *(pti+3) <= '8'
		&&  *(pti+4) == '}')
		{
			val1 = *(pti+1) - '0';
			val2 = *(pti+2) - '0';
			val3 = *(pti+3) - '0';

			pti += 5;

			if (val1 == 3)
			{
				val1 = VT102_DIM;
				val2 = CH(ch->desc)->pcdata->color[COLOR_TEXT] % 10;
				val3 = CH(ch->desc)->pcdata->color[COLOR_TEXT] / 10;
			}

			pt2 = get_color_diff(ch, 8, 8, 8, val2, val3, val1);

			for ( ; *pt2 != '\0' ; pt2++, pto++)
			{
				*pto = *pt2;
			}
			continue;
		}
		*pto = *pti;
		pti++;
		pto++;
	}
	*pto = '\0';

	color = 0;

	if (HAS_BIT(CH(ch->desc)->pcdata->vt100_flags, VT100_ANSI))
	{
		color = 16;

		if (HAS_BIT(ch->desc->mth->comm_flags, COMM_FLAG_256COLORS))
		{
			color = 256;
		}
	}

	substitute_color(ansi_translate_buffer, xterm_translate_buffer, color);

	pop_call();
	return( xterm_translate_buffer );
}

/*
	Uses minimal amount of color codes - Scandum
*/

char *ansi_compress( CHAR_DATA *ch, char *txt, int color, int code )
{
	char ansi_buf[MAX_STRING_LENGTH];
	static char xterm_buf[MAX_STRING_LENGTH];

	char *pti, *pto, *ptt;
	int val[4], old[4];
	int mode;

	push_call("ansi_compress(%p,%p,%p,%p)",ch,txt,color,code);

	if (ch->desc == NULL)
	{
		pop_call();
		return "";
	}

	pti  = txt;
	pto  = ansi_buf;

	memset(val, 0, 3);
	memset(old, 8, 3);

	old[0] = old[1] = old[2] = 0;

	while (*pti != '\0')
	{
		if (*(pti+0) == '{'
		&&  *(pti+1) >= '0' && *(pti+1) <= '8'
		&&  *(pti+2) >= '0' && *(pti+2) <= '8'
		&&  *(pti+3) >= '0' && *(pti+3) <= '8'
		&&  *(pti+4) == '}')
		{
			val[0] = *(pti+1) - '0';
			val[1] = *(pti+2) - '0';
			val[2] = *(pti+3) - '0';

			pti += 5;

			if (val[0] == 3)
			{
				val[0] = code;
				val[1] = CH(ch->desc)->pcdata->color[color] % 10;
				val[2] = CH(ch->desc)->pcdata->color[color] / 10;
			}

			ptt = get_color_diff(ch, old[1], old[2], old[0], val[1], val[2], val[0]);

			while (*ptt)
			{
				*pto++ = *ptt++;
			}
			memcpy(old, val, 3);

			continue;
		}
		else if (*pti == '\n')
		{
			memset(old, 8, 3);
		}
		*pto = *pti;
		pti++;
		pto++;
	}
	*pto = '\0';

	mode = 0;

	if (HAS_BIT(CH(ch->desc)->pcdata->vt100_flags, VT100_ANSI))
	{
		mode = 16;

		if (HAS_BIT(ch->desc->mth->comm_flags, COMM_FLAG_256COLORS))
		{
			mode = 256;
		}
	}

	substitute_color(ansi_buf, xterm_buf, mode);

	pop_call();
	return( xterm_buf );
}

/*
	Basic color translator for internal usage - Scandum 21-05-2002
*/

char *ansi_translate(char *text_in)
{
	char ansi_translate_buffer[MAX_STRING_LENGTH];
	static char xterm_translate_buffer[MAX_STRING_LENGTH];

	char *pti, *pto;
	char pt2[11];
	int val1, val2, val3, cnt;

	push_call("ansi_translate(%p)",text_in);

	pti  = text_in;
	pto  = ansi_translate_buffer;
	val1 = val2 = val3 = 0;

	while (*pti != '\0')
	{
		if (*(pti+0) == '{'
		&&  *(pti+1) >= '0' && *(pti+1) <= '8'
		&&  *(pti+2) >= '0' && *(pti+2) <= '8'
		&&  *(pti+3) >= '0' && *(pti+3) <= '8'
		&&  *(pti+4) == '}')
		{
			val1 = *(pti+1) - '0';
			val2 = *(pti+2) - '0';
			val3 = *(pti+3) - '0';

			pti += 5;

			if (val1 == 8 && val2 == 8 && val3 == 8)
			{
				continue;
			}
			sprintf(pt2, "\033[");

			if (val1 < 8)
			{
				cat_sprintf(pt2, "%d", val1);
			}

			if (val2 < 8)
			{
				cat_sprintf(pt2, "%s%d", (val1 < 8) ? ";" : "", 30 + val2);
			}

			if (val3 < 8)
			{
				cat_sprintf(pt2, "%s%d", (val1 < 8 || val2 < 8) ? ";" : "", 40 + val3);
			}
			strcat(pt2, "m");

			for (cnt = 0 ; pt2[cnt] != '\0' ; cnt++, pto++)
			{
				*pto = pt2[cnt];
			}
			continue;
		}
		*pto = *pti;
		pti++;
		pto++;
	}
	*pto = '\0';

	substitute_color(ansi_translate_buffer, xterm_translate_buffer, 16);

	pop_call();
	return( xterm_translate_buffer );
}

/*
	Translates color codes setting {300} to given def - Scandum 14-05-2002
*/

char *ansi_translate_def( CHAR_DATA *ch, char *text_in, char *def )
{
	char ansi_translate_buffer[MAX_STRING_LENGTH];
	static char xterm_translate_buffer[MAX_STRING_LENGTH];
	char *pti, *pto, *pt2;
	int val1, val2, val3;
	int color;

	push_call("ansi_translate_def(%p,%p,%p)",ch,text_in,def);

	pti  = text_in;
	pto  = ansi_translate_buffer;
	val1 = val2 = val3 = 0;

	while (*pti != '\0')
	{
		if (*(pti+0) == '{'
		&&  *(pti+1) >= '0' && *(pti+1) <= '8'
		&&  *(pti+2) >= '0' && *(pti+2) <= '8'
		&&  *(pti+3) >= '0' && *(pti+3) <= '8'
		&&  *(pti+4) == '}')
		{
			val1 = *(pti+1) - '0';
			val2 = *(pti+2) - '0';
			val3 = *(pti+3) - '0';

			pti += 5;

			if (val1 == 3)
			{
				pt2 = def;
				for ( ; *pt2 != '\0' ; pt2++, pto++)
				{
					*pto = *pt2;
				}
			}
			else
			{
				pt2 = get_color_diff(ch, 8, 8, 8, val2, val3, val1);
				for ( ; *pt2 != '\0' ; pt2++, pto++)
				{
					*pto = *pt2;
				}
			}
			continue;
		}
		*pto = *pti;
		pti++;
		pto++;
	}
	*pto = '\0';

	color = 0;

	if (HAS_BIT(CH(ch->desc)->pcdata->vt100_flags, VT100_ANSI))
	{
		color = 16;

		if (HAS_BIT(ch->desc->mth->comm_flags, COMM_FLAG_256COLORS))
		{
			color = 256;
		}
	}

	substitute_color(ansi_translate_buffer, xterm_translate_buffer, color);

	pop_call();
	return( xterm_translate_buffer );
}

void reset_color (CHAR_DATA * ch)
{
	push_call("reset_color(%p)",ch);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	ch->pcdata->color[ 0] = 6 + 8 * 10;
	ch->pcdata->color[ 1] = 7 + 1 * 10;
	ch->pcdata->color[ 2] = 3 + 4 * 10;
	ch->pcdata->color[ 3] = 3 + 8 * 10;
	ch->pcdata->color[ 4] = 1 + 8 * 10;
	ch->pcdata->color[ 5] = 2 + 8 * 10;
	ch->pcdata->color[ 6] = 7 + 8 * 10;
	ch->pcdata->color[ 7] = 3 + 8 * 10;
	ch->pcdata->color[ 8] = 4 + 8 * 10;
	ch->pcdata->color[ 9] = 3 + 8 * 10;
	ch->pcdata->color[10] = 2 + 8 * 10;
	ch->pcdata->color[11] = 5 + 8 * 10;
	ch->pcdata->color[12] = 7 + 4 * 10;
	ch->pcdata->color[13] = 3 + 4 * 10;
	ch->pcdata->color[14] = 1 + 4 * 10;
	ch->pcdata->color[15] = 5 + 4 * 10;
	ch->pcdata->color[16] = 3 + 8 * 10;
	ch->pcdata->color[17] = 5 + 8 * 10;
	ch->pcdata->color[18] = 7 + 8 * 10;

	pop_call();
	return;
}

char *get_color_diff (CHAR_DATA * ch, int old_for, int old_bak, int old_bold, int new_for, int new_bak, int new_bold)
{
	static char buf[20];

	push_call("get_color_diff(%p,%p,%p,%p,%p,%p,%p)",ch,old_for,old_bak,old_bold,new_for,new_bak,new_bold);

	buf[0] = '\0';

	if (ch->desc == NULL)
	{
		pop_call();
		return buf;
	}

	if (!HAS_BIT(CH(ch->desc)->pcdata->vt100_flags, VT100_ANSI) && !IS_SET(CH(ch->desc)->pcdata->vt100_flags, VT100_BOLD) && !IS_SET(CH(ch->desc)->pcdata->vt100_flags, VT100_REVERSE))
	{
		pop_call();
		return buf;
	}

	/* supporting bold and background color */

	if (!HAS_BIT(CH(ch->desc)->pcdata->vt100_flags, VT100_ANSI))
	{
		if (new_bak == old_bak && new_bold == old_bold)
		{
			pop_call();
			return buf;
		}

		if (new_bak > 0 && new_bak != 8 && IS_SET(CH(ch->desc)->pcdata->vt100_flags, VT100_REVERSE))
		{
			if (new_bak != old_bak)
			{
				strcpy(buf, "\033[7m");
			}
		}
		else
		{
			switch (new_bold)
			{
				case 0:
					if (IS_SET(CH(ch->desc)->pcdata->vt100_flags, VT100_ECMA48))
					{
						strcpy(buf, "\033[m");
					}
					else
					{
						strcpy(buf, "\033[0m");
					}
					break;
				case 1:
					if (IS_SET(CH(ch->desc)->pcdata->vt100_flags, VT100_BOLD))
					{
						strcpy(buf, "\033[1m");
					}
					break;
				case 4:
					if (IS_SET(CH(ch->desc)->pcdata->vt100_flags, VT100_UNDERLINE))
					{
						strcpy(buf, "\033[4m");
					}
					break;
				case 5:
					if (IS_SET(CH(ch->desc)->pcdata->vt100_flags, VT100_FLASH))
					{
						strcpy(buf, "\033[5m");
					}
					break;
				case 7:
					if (IS_SET(CH(ch->desc)->pcdata->vt100_flags, VT100_REVERSE))
					{
						strcpy(buf, "\033[7m");
					}
					break;
				case 22:
					if (IS_SET(CH(ch->desc)->pcdata->vt100_flags, VT100_ECMA48))
					{
						strcpy(buf, "\033[22m");
					}
					break;
				default:
					pop_call();
					return buf;
			}
		}
		pop_call();
		return buf;
	}

	if (old_for == new_for && old_bak == new_bak && old_bold == new_bold)
	{
		pop_call();
		return buf;
	}

	/*
		VT100 and minor ECMA-48 support - Scandum 27-02-2002
	*/

	strcpy(buf, "\033[");

	if (new_bold != old_bold)
	{
		switch (new_bold)
		{
			case 0:
				strcat(buf, "0");
				break;
			case 1:
				if (IS_SET(CH(ch->desc)->pcdata->vt100_flags, VT100_BOLD))
				{
					strcat(buf, "1");
				}
				break;
			case 4:
				if (IS_SET(CH(ch->desc)->pcdata->vt100_flags, VT100_UNDERLINE))
				{
					strcat(buf, "4");
				}
				break;
			case 5:
				if (IS_SET(CH(ch->desc)->pcdata->vt100_flags, VT100_FLASH))
				{
					strcat(buf, "5");
				}
				break;
			case 7:
				if (IS_SET(CH(ch->desc)->pcdata->vt100_flags, VT100_REVERSE))
				{
					strcat(buf, "7");
				}
				break;
			case 22:
				if (IS_SET(CH(ch->desc)->pcdata->vt100_flags, VT100_ECMA48))
				{
					strcat(buf, "22");
				}
				break;
		}
	}
	if (new_for != 8 && ((new_bold != old_bold && new_bold == 0) || new_for != old_for))
	{
		cat_sprintf(buf, "%s%d", (buf[2] != '\0') ? ";" : "", 30 + new_for);
	}

	if (new_bak != 8 && ((new_bold != old_bold && new_bold == 0) || new_bak != old_bak))
	{
		cat_sprintf(buf, "%s%d", (buf[2] != '\0') ? ";" : "", 40 + new_bak);
	}
	strcat(buf, "m");

	pop_call();
	return buf;
}


void do_refresh (CHAR_DATA * ch, char *argument)
{
	push_call("do_refresh(%p, %p)",ch,argument);

	if (!is_desc_valid(ch))
	{
		pop_call();
		return;
	}

	if (!HAS_BIT(CH(ch->desc)->pcdata->vt100_flags, VT100_INTERFACE))
	{
		pop_call();
		return;
	}

	if (CH(ch->desc)->pcdata->tactical != NULL)
	{
		FREEMEM(CH(ch->desc)->pcdata->tactical);
		CH(ch->desc)->pcdata->tactical = NULL;
	}

	vt100on(ch);

	pop_call();
	return;
}

void do_tactical (CHAR_DATA * ch, char *arg)
{
	int val;
	int max;
	char buf[MAX_STRING_LENGTH];

	push_call("do_tactical(%p,%p)",ch,arg);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (!strcasecmp (arg, "dump"))
	{
		int cnt, size_v, size_h, old_bld, old_for, old_bak, new_bld, new_for, new_bak;

		old_bld = old_for = old_bak = -1;

		get_tactical_map(ch);

		size_v = ch->pcdata->tactical_size_v + 1;
		size_h = ch->pcdata->screensize_h;

		for (buf[0] = cnt = 0 ; cnt < size_v * size_h ; cnt++)
		{
			if (cnt % size_h == 0)
			{
				old_bld = old_for = old_bak = -1;
				strcat(buf, "\033[0m\n\r");
			}
			new_bld = (mud->tactical->map[cnt] / 128 || mud->tactical->color[cnt] / 128) ? 1 : 22;
			new_for = mud->tactical->color[cnt] % 8;
			new_bak = mud->tactical->color[cnt] % 64 / 8;

			cat_sprintf(buf, "%s%c", get_color_diff(ch, old_for, old_bak, old_bld, new_for, new_bak, new_bld), mud->tactical->map[cnt] % 128);

			old_bld = new_bld;
			old_for = new_for;
			old_bak = new_bak;
		}

		if (IS_SET(ch->act, PLR_WIZTIME))
		{
			cat_sprintf(buf, "Size: %d bytes.\n\r", strlen(buf));
		}
		ch_printf(ch, "%s", buf);
		pop_call();
		return;
	}

	if (!strcasecmp (arg, "top"))
	{
		SET_BIT(ch->pcdata->tactical_flags, TACTICAL_TOP);
		send_to_char ("Your tactical is set to the top.\n\r", ch);
		pop_call();
		return;
	}

	if (!strcasecmp (arg, "bottom"))
	{
		DEL_BIT(ch->pcdata->tactical_flags, TACTICAL_TOP);

		send_to_char ("Your tactical is set to the bottom.\n\r", ch);
		pop_call();
		return;
	}

	if (arg[0] == 'n' || arg[0] == 'N')
	{
		if (HAS_BIT(ch->pcdata->tactical_flags, TACTICAL_INDEX))
		{
			send_to_char ("You turn the tactical index off.\n\r", ch);
		}
		else
		{
			send_to_char ("You turn the tactical index on.\n\r", ch);
		}
		TOG_BIT(ch->pcdata->tactical_flags, TACTICAL_INDEX);
		pop_call();
		return;
	}

	if (arg[0] == 'i' || arg[0] == 'I')
	{
		char bufs[10];

		arg = one_argument (arg, buf);

		if (arg[0] <= ' ' || arg[0] > 'z' || arg[1] <= ' ' || arg[1] > 'z')
		{
			send_to_char ("You must specify a two character string for an index.\n\r", ch);
			pop_call();
			return;
		}

		if (arg[0] == '~')
		{
			arg[0] = '-';
		}

		if (arg[1] == '~')
		{
			arg[1] = '-';
		}

		bufs[0] = arg[0];
		bufs[1] = arg[1];
		bufs[2] = '\0';

		RESTRING(ch->pcdata->tactical_index, bufs);

		send_to_char ("Index set.\n\r", ch);

		pop_call();
		return;
	}

	if (arg[0] == 'c' || arg[0] == 'C')
	{
		arg = one_argument (arg, buf);

		if (arg[0] == '\0')
		{
			char buf1[MAX_STRING_LENGTH];
			char buf_sect[20];
			char col_tact[10];
			char col_tops[10];
			int cnt, col;

			strcpy(col_tact, get_color_code(ch, COLOR_TACTICAL, VT102_DIM));
			strcpy(col_tops, get_color_code(ch, COLOR_TOP_STAT, VT102_DIM));

			sprintf(buf1, "%s%s\n\r", col_tops, "Sector Types and their Compass colors.                                          {088}");

			for (cnt = 0 ; cnt < SECT_MAX ; cnt++)
			{
				col = sector_table[cnt].color;

				sprintf(buf_sect, "%-14s", capitalize(sector_table[cnt].sector_name));

				cat_sprintf(buf1, "%s {%d%d%d}%s%s ", col_tact, col / 128, col % 8, (col % 128) / 8, buf_sect, col_tact);
				if ((cnt+1) % 5 == 0)
				{
					strcat(buf1, "{088}\n\r");
				}
			}
			while (cnt % 5 != 0)
			{
				strcat(buf1, "                ");
				cnt++;
				if (cnt % 5 == 0)
				{
					strcat(buf1, "{088}\n\r");
				}
			}
			cat_sprintf(buf1, "%s                                                                                {088}\n\r", col_tops);
			send_to_char_color(buf1, ch);

			pop_call();
			return;
		}

		val = atol (arg);
		if (val == 0)
		{
			send_to_char ("The tactical compass is turned off.\n\r", ch);
			ch->pcdata->tactical_size_c = 0;
			pop_call();
			return;
		}

		if (val > 15 || val < 5)
		{
			send_to_char ("The tactical compass size must be from 5 to 15, or 0 to turn off.\n\r", ch);
			pop_call();
			return;
		}

		ch->pcdata->tactical_size_c = val;
		send_to_char ("The tactical compass size is set.\n\r", ch);
		pop_call();
		return;
	}

	max = URANGE(1, ch->pcdata->screensize_v / 2, MAX_TACTICAL_ROW - 1);

	val = atol (arg);

	if (val < 1 || val > max)
	{
		ch_printf(ch, "The tactical size must be from 1 to %d.\n\r", max);
		pop_call();
		return;
	}

	ch->pcdata->tactical_size_v = val;
	do_refresh (ch, "");
	pop_call();
	return;
}

void clear_tactical_map (TACTICAL_MAP *tact)
{
	static const TACTICAL_MAP clear_tact;

	*tact = clear_tact;
/*
	memset(tact->map,   0, MAX_TACTICAL_ROW * MAX_TACTICAL_COL);
	memset(tact->color, 0, MAX_TACTICAL_ROW * MAX_TACTICAL_COL);
*/
	return;
}

/*
	Chaos - 3/1/95
*/

char *get_tactical_string (CHAR_DATA * ch, TACTICAL_MAP * tact)
{
	DESCRIPTOR_DATA *d;

	static char buf[MAX_STRING_LENGTH];

	int cnt, lcnt;
	int new_bld, old_bld;
	int old_for,  old_bak;		/* last color selected */
	int new_for,  new_bak;
	int prev_h,   prev_v;		/* last known change in text */
	int size_h,   size_v;

	int ccnt, stbar;
	unsigned char *tm, *tc, *cm, *cc;	/* Pointers to tacticals */

	push_call("get_tactical_string(%p,%p)",ch,tact);

	d = ch->desc;

	if (!is_desc_valid(ch))
	{
		pop_call();
		return NULL;
	}

	prev_h   = -10;
	prev_v   = -10;
	old_bld = -1;
	old_for  = -1;
	old_bak  = -1;

	buf[0]   = '\0';

	stbar  = CH(d)->pcdata->screensize_v - 1;

	size_v = CH(d)->pcdata->tactical_size_v + 1;
	size_h = CH(d)->pcdata->screensize_h;

	tm = (unsigned char *) tact->map;
	tc = (unsigned char *) tact->color;

	cm = (unsigned char *) CH(d)->pcdata->tactical->map;
	cc = (unsigned char *) CH(d)->pcdata->tactical->color;

	for (lcnt = 0 ; lcnt < size_v ; lcnt++)
	{
		for (cnt = 0 ; cnt < size_h ; cnt++, tm++, tc++, cm++, cc++)
		{
			if (*tm != *cm || *tc != *cc)
			{
				if (IS_SET(CH(d)->pcdata->vt100_flags, VT100_BOLD))
				{
					if (*tc >= 128 || *tm >= 128)
					{
						new_bld = 1;
					}
					else if (IS_SET(CH(d)->pcdata->vt100_flags, VT100_ECMA48))
					{
						new_bld = 22;
					}
					else
					{
						new_bld = 0;
					}
				}
				else
				{
					new_bld = 0;
				}

				if (!HAS_BIT(CH(d)->pcdata->vt100_flags, VT100_ANSI))
				{
					new_for = 0;
					new_bak = 1;
				}
				else
				{
					new_for = *tc % 8;
					new_bak = (*tc % 128) / 8;
				}
				/*
					Get color shifting
				*/

				strcat(buf, get_color_diff(ch, old_for, old_bak, old_bld, new_for, new_bak, new_bld));

				if (lcnt == CH(d)->pcdata->tactical_size_v)
				{
					ccnt = stbar - 1;
				}
				else
				{
					ccnt = lcnt;
				}

				if (prev_v == ccnt + 1 && prev_h == cnt)
				{
					cat_sprintf(buf, "%c", *tm % 128);
				}
				else
				{
					/*
						Check for slow VT100 processing
					*/

					if (!IS_SET(CH(d)->pcdata->vt100_flags, VT100_FAST))
					{
						cat_sprintf(buf, "\033[%d;%dH%c", ccnt + 1, cnt + 1, *tm % 128);
					}
					else
					{
						if (ccnt + 1 == prev_v)	/* Same row optimize */
						{
							cat_sprintf(buf, "\033[%dC%c", cnt - prev_h, *tm % 128);
						}
						else	/* Direct cursor positioning */
						{
							cat_sprintf(buf, "\033[%d;%dH%c", ccnt + 1, cnt + 1, *tm % 128);
						}
					}
				}
				prev_v = ccnt + 1;
				prev_h = cnt + 1;
				old_for = new_for;
				old_bak = new_bak;
				old_bld = new_bld;
			}
		}
	}

	/* Update the new map for player */

	memcpy(CH(d)->pcdata->tactical->map,	tact->map,	lcnt * size_h);
	memcpy(CH(d)->pcdata->tactical->color,	tact->color,	lcnt * size_h);

	pop_call();
	return(buf);
}

TACTICAL_MAP *get_tactical_map (CHAR_DATA * ch)
{
	int val, cnt, lcnt, size_v, size_h, cuc, door;
	bool *pti, *pto;
	bool *ptoc;
	AFFECT_DATA *paf;
	PC_DATA *pcd;			/* Shortcutt one reference */
	char buf[MAX_STRING_LENGTH], buf2[MAX_INPUT_LENGTH], buf3[MAX_INPUT_LENGTH];
	bool *ptb;
	int color1, color2, color3;
	int hr, mn, col, row, bld, colors, max_width, offset, vo, hour, cw;
	struct tm clk;
	bool *tm, *tc;
	EXIT_DATA *pexit;
	int found;
	CHAR_DATA *fch;

	push_call("get_tactical_map(%p)",ch);

	pcd  = CH(ch->desc)->pcdata;

	clk = *localtime (&mud->current_time);

	clear_tactical_map(mud->tactical);

	tm = (unsigned char *) mud->tactical->map;
	tc = (unsigned char *) mud->tactical->color;

	color1 = pcd->color[COLOR_TOP_STAT] % 10 + pcd->color[COLOR_TOP_STAT] / 10 * 8;
	color2 = pcd->color[COLOR_TACTICAL] % 10 + pcd->color[COLOR_TACTICAL] / 10 * 8;
	color3 = pcd->color[COLOR_BOT_STAT] % 10 + pcd->color[COLOR_BOT_STAT] / 10 * 8;

	size_v = pcd->tactical_size_v + 1;
	size_h = pcd->screensize_h;

	for (lcnt = 0 ; lcnt < size_v ; lcnt++)
	{
		for (cnt = 0 ; cnt < size_h ; cnt++, tm++, tc++)
		{
			*tm = 128 + ' ';

			if (HAS_BIT(pcd->tactical_flags, TACTICAL_TOP) && lcnt == 0)
			{
				*tc = color1;
			}
			else if (!HAS_BIT(pcd->tactical_flags, TACTICAL_TOP) && lcnt == pcd->tactical_size_v -1)
			{
				*tc = color1;
			}
			else if (lcnt < pcd->tactical_size_v)
			{
				*tc = color2;
			}
			else
			{
				*tc = color3;
			}
		}
	}

	/* Let's use these as a index pointer for speed */

	tm = (unsigned char *) mud->tactical->map;
	tc = (unsigned char *) mud->tactical->color;

	if (HAS_BIT(pcd->tactical_flags, TACTICAL_TOP))
	{
		pto = tm;
	}
	else
	{
		pto = tm + size_h * (size_v - 2);
	}

	if (!IS_NPC(ch))
	{
		for (cnt = 0, pti = (bool *) ch->name ; *pti != '\0' ; pti++, pto++, cnt++)
		{
			*pto = *pti;
		}
	}
	else
	{
		for (cnt = 0, pti = (bool *) ch->short_descr ; *pti != '\0' && cnt < 73 ; pti++, pto++, cnt++)
		{
			*pto = *pti;
		}
	}

	for ( ; cnt < size_h ; cnt++, pto++)
	{
		*pto = ' ';
	}

	pto--;

	*pto-- = ch->level % 10 + 128 + '0';
	if (ch->level >= 10)
	{
		*pto-- = ch->level / 10 + 128 + '0';
	}
	*pto-- = ':';
	*pto-- = 'v';
	*pto-- = 'e';
	*pto-- = 'L';
	*pto-- = ' ';

	sprintf(buf, "%d", ch->alignment);

	for (cnt = strlen(buf) - 1 ; cnt >= 0 ; cnt--)
	{
		*pto-- = 128 + buf[cnt];
	}

	*pto-- = ':';
	*pto-- = 'l';
	*pto-- = 'A';
	*pto-- = ' ';

	switch (ch->speed)
	{
		case 0:	*pto-- = 128 + 'W'; break;
		case 1:	*pto-- = 128 + 'N'; break;
		case 2:	*pto-- = 128 + 'J'; break;
		case 3:	*pto-- = 128 + 'R'; break;
		case 4:	*pto-- = 128 + 'H'; break;
		case 5:	*pto-- = 128 + 'B'; break;
	}

	*pto-- = ' ';

	col = 0;
	cnt = 0;

	if (!IS_NPC(ch))
	{
		if (IS_SET(ch->act, PLR_HOLYLIGHT))
		{
			cnt++;
			*pto-- = 'L' + 128;
			*pto-- = 'H' + 128;
		}
		if (IS_SET(ch->act, PLR_WIZINVIS))
		{
			cnt++;
			*pto-- = 'I' + 128;
			*pto-- = 'W' + 128;
		}
		if (IS_SET(ch->act, PLR_WIZCLOAK))
		{
			cnt++;
			*pto-- = 'C' + 128;
			*pto-- = 'W' + 128;
		}
	}

	for (paf = ch->first_affect ; paf ; paf = paf->next)
	{
		if (cnt++ < (size_h - 40) / 2)
		{
			*pto-- = skill_table[paf->type].name[1] + 128;
			*pto-- = skill_table[paf->type].name[0] + 128 - 32;
		}
	}

	if (cnt == 0)
	{
		*pto-- = 128 + 'e';
		*pto-- = 128 + 'n';
		*pto-- = 128 + 'o';
		*pto-- = 128 + 'N';
	}
	*pto-- = ':';
	*pto-- = 'X';
	*pto-- = 'F';
	*pto-- = ' ';

	*pto-- = 128 + race_table[UNSHIFT(CH(ch->desc)->pcdata->speak)].race_name[1];
	*pto-- = 128 + race_table[UNSHIFT(CH(ch->desc)->pcdata->speak)].race_name[0];

	*pto-- = ' ';

	*pto-- = ':';
	*pto-- = 'p';
	*pto-- = 'S';
	*pto-- = ' ';

	hour = (mud->time_info->hour % 12) ? mud->time_info->hour % 12 : 12;

	*pto-- = 'm' + 128;

	*pto-- = (mud->time_info->hour >= 12) ? 'p' + 128 : 'a' + 128;

	*pto-- = '0' + (hour % 10) + 128;

	if (hour >= 10)
	{
		*pto-- = '1' + 128;
	}
	*pto-- = ' ';

	/*
		Add the bottom stat bar
	*/

	pto  = tm + (size_h * (size_v - 1));
	ptoc = tc + (size_h * (size_v - 1));

	cuc = color3;

	if (!IS_NPC(ch) && IS_IMMORTAL(ch) && ch->pcdata->editmode != MODE_NONE && pcd->subprompt && *pcd->subprompt)
	{
		sprintf(buf, "%-61s", ch->pcdata->subprompt);

		for (ptb = (bool *) buf ; *ptb != '\0' ; ptb++)
		{
			*pto++  = *ptb + 128;
			*ptoc++ = cuc;
		}

		for (cnt = 61 ; cnt < size_h - 19 ; cnt++)
		{
			*pto++ = 128 + ' ';
		}
	}
	else
	{
		*pto++  = 128 + ' ';
		*ptoc++ = cuc;
		*pto++  = 'H';
		*ptoc++ = cuc;
		*pto++  = ':';
		*ptoc++ = cuc;

		val = UMAX(0, ch->hit);

		switch (val * 10 / UMAX(1, ch->max_hit) / 4)
		{
			case 0:	cuc = 1 + pcd->color[COLOR_BOT_STAT] / 10 * 8; break;
			case 1:	cuc = 3 + pcd->color[COLOR_BOT_STAT] / 10 * 8; break;
			case 2:	cuc = 2 + pcd->color[COLOR_BOT_STAT] / 10 * 8; break;
		}

		sprintf(buf, "%5d", ch->hit);

		*pto++  = 128 + buf[0];
		*ptoc++ = cuc;
		*pto++  = 128 + buf[1];
		*ptoc++ = cuc;
		*pto++  = 128 + buf[2];
		*ptoc++ = cuc;
		*pto++  = 128 + buf[3];
		*ptoc++ = cuc;
		*pto++  = 128 + buf[4];
		*ptoc++ = cuc;

		cuc = color3;

		sprintf(buf, "/%-5d", ch->max_hit);

		*pto		=   0 + buf[0];	pto++;
		*ptoc	= cuc;			ptoc++;
		*pto		= 128 + buf[1];	pto++;
		*ptoc	= cuc;			ptoc++;
		*pto		= 128 + buf[2];	pto++;
		*ptoc	= cuc;			ptoc++;
		*pto		= 128 + buf[3];	pto++;
		*ptoc	= cuc;			ptoc++;
		*pto		= 128 + buf[4];	pto++;
		*ptoc	= cuc;			ptoc++;
		*pto		= 128 + buf[5];	pto++;
		*ptoc	= cuc;			ptoc++;

		*pto		= 128 + ' ';		pto++;
		*ptoc	= cuc;			ptoc++;

		*pto		= 'M';			pto++;
		*ptoc	= cuc;			ptoc++;
		*pto		= 'n';			pto++;
		*ptoc	= cuc;			ptoc++;
		*pto		= ':';			pto++;
		*ptoc	= cuc;			ptoc++;

		val = UMAX(0, ch->mana);

		switch (val * 10 / UMAX(1, ch->max_mana) / 4)
		{
			case 0:	cuc = 1 + pcd->color[COLOR_BOT_STAT] / 10 * 8;	break;
			case 1:	cuc = 3 + pcd->color[COLOR_BOT_STAT] / 10 * 8;	break;
			case 2:	cuc = 2 + pcd->color[COLOR_BOT_STAT] / 10 * 8;	break;
		}

		sprintf(buf, "%-5d", ch->mana);

		*pto		= 128 + buf[0];	pto++;
		*ptoc	= cuc;			ptoc++;
		*pto		= 128 + buf[1];	pto++;
		*ptoc	= cuc;			ptoc++;
		*pto		= 128 + buf[2];	pto++;
		*ptoc	= cuc;			ptoc++;
		*pto		= 128 + buf[3];	pto++;
		*ptoc	= cuc;			ptoc++;
		*pto		= 128 + buf[4];	pto++;
		*ptoc	= cuc;			ptoc++;

		cuc		= color3;

		*pto		= 128 + ' ';	pto++;
		*ptoc	= cuc;		ptoc++;
		*pto		= 'M';		pto++;
		*ptoc	= cuc;		ptoc++;
		*pto		= 'v';		pto++;
		*ptoc	= cuc;		ptoc++;
		*pto		= ':';		pto++;
		*ptoc	= cuc;		ptoc++;

		val = UMAX(0, ch->move);

		switch (val * 10 / UMAX(1, ch->max_move) / 2)
		{
			case 0:	case 1:	cuc = 1 + pcd->color[COLOR_BOT_STAT] / 10 * 8;	break;
			case 2:	case 3:	cuc = 3 + pcd->color[COLOR_BOT_STAT] / 10 * 8;	break;
			case 4:	case 5:	cuc = 2 + pcd->color[COLOR_BOT_STAT] / 10 * 8;	break;
		}

		sprintf(buf, "%-5d", ch->move);

		*pto		= 128 + buf[0];	pto++;
		*ptoc	= cuc;			ptoc++;
		*pto		= 128 + buf[1];	pto++;
		*ptoc	= cuc;			ptoc++;
		*pto		= 128 + buf[2];	pto++;
		*ptoc	= cuc;			ptoc++;
		*pto		= 128 + buf[3];	pto++;
		*ptoc	= cuc;			ptoc++;
		*pto		= 128 + buf[4];	pto++;
		*ptoc	= cuc;			ptoc++;

		for (cnt = 32 ; cnt < size_h - 48 ; cnt++)
		{
			*pto	= 128 + ' ';	pto++;	ptoc++;
		}

		*pto		= 128 + ' ';	pto++;

		cuc = color3;

		if (HAS_BIT(pcd->tactical_flags, TACTICAL_EXPTNL))
		{
			*pto		= 'e';	pto++;
			*pto		= 'X';	pto++;
		}
		else
		{
			*pto		= 'E';	pto++;
			*pto		= 'x';	pto++;
		}
		*pto		= ':';	pto++;

		if (HAS_BIT(pcd->tactical_flags, TACTICAL_EXPTNL))
		{
			if (ch->level >= 95)
			{
				strcpy (buf, " max      ");
			}
			else
			{
				sprintf (buf, "%-10d", exp_level(ch->class, ch->level) - (IS_NPC(ch) ? 0 : ch->pcdata->exp));
			}
		}
		else
		{
			sprintf (buf, "%-10d", IS_NPC(ch) ? 0 : ch->pcdata->exp);
		}

		for (ptb = (bool *) buf ; *ptb != '\0' ; ptb++)
		{
			*pto		= *ptb + 128;	pto++;
		}
		*pto = 'G';	pto++;
		*pto = 'd';	pto++;
		*pto = ':';	pto++;

		sprintf (buf, "%-10d", ch->gold);
		for (ptb = (bool *) buf ; *ptb != '\0' ; ptb++)
		{
			*pto = *ptb + 128;	pto++;
		}
	}
	*pto = 'E';	pto++;
	*pto = 'x';	pto++;
	*pto = 'i';	pto++;
	*pto = 't';	pto++;
	*pto = ':';	pto++;
	*pto = ' ';	pto++;

	cnt = 0;

	if (can_see_in_room(ch, ch->in_room))
	{
		for (door = 0; door < 6; door++)
		{
			if ((pexit = get_exit(ch->in_room->vnum, door)) != NULL
			&&   !IS_SET(pexit->flags, EX_CLOSED)
			&&  (!IS_SET(ch->in_room->room_flags, ROOM_SMOKE)	|| can_see_smoke(ch))
			&&  (!IS_SET(pexit->flags, EX_HIDDEN) || can_see_hidden(ch))
			&&   can_use_exit(ch, pexit))
			{
				cnt++;
				switch (door)
				{
					case 0:	*pto = 'N' + 128;	pto++;	break;
					case 1:	*pto = 'E' + 128;	pto++;	break;
					case 2:	*pto = 'S' + 128;	pto++;	break;
					case 3:	*pto = 'W' + 128;	pto++;	break;
					case 4:	*pto = 'U' + 128;	pto++;	break;
					case 5:	*pto = 'D' + 128;	pto++;	break;
				}
			}
		}
	}
	for ( ; cnt < 7 ; cnt++)
	{
		*pto = 128 + ' ';	pto++;
	}

	ptoc += 40;	*ptoc = cuc - 2;

	if (ch->wait != 0)
	{
		*pto		= '*' + 128;	pto++;
	}
	else
	{
		*pto 	= ' ' + 128;	pto++;
	}

	mn = clk.tm_min  + pcd->clock / 10000;
	hr = clk.tm_hour + pcd->clock % 100 + mn / 60;
	mn = mn % 60;
	hr = hr % 24;

	if (pcd->clock % 10000 / 1000 == 1)
	{
		hr = URANGE(0, (mud->current_time - pcd->last_connect) / 3600, 999);

		sprintf(buf, "%3d", hr);

		buf[3] = ':';
		buf[4] = '0' + ((mud->current_time - pcd->last_connect) % 3600) / 600;
		buf[5] = '0' + ((mud->current_time - pcd->last_connect) % 600) / 60;
		buf[6] = ' ';
		buf[7] = '\0';
	}
	else if (pcd->clock % 1000 / 100 == 1)
	{
		buf[0] = (hr / 10) + '0';
		buf[1] = (hr % 10) + '0';
		buf[2] = ':';
		buf[3] = (mn / 10) + '0';
		buf[4] = (mn % 10) + '0';
		buf[5] = ' ';
		buf[6] = ' ';
		buf[7] = '\0';
	}
	else
	{
		buf[0] = hr == 12 ? '1' : hr % 12 >= 10 ? '1' : ' ';
		buf[1] = hr == 12 ? '2' : '0' + hr % 12 % 10;
		buf[2] = ':';
		buf[3] = '0' + (mn / 10);
		buf[4] = '0' + (mn % 10);
		buf[5] = hr >= 12 ? 'p' : 'a';
		buf[6] = 'm';
		buf[7] = '\0';
	}

	for (ptb = (bool *) buf ; *ptb != '\0' ; ptb++)
	{
		*pto = *ptb + 128;	pto++;
	}

	/* Do the room mobiles  */

	if (size_v - 1 > 1)
	{
		colors = 7;
		cw = pcd->tactical_size_c;

		max_width = (cw == 0) ? size_h - 17 : size_h - 17 - 3 - 3 * cw;

		col = 2;
		row = 1;

		if (can_see_in_room(ch, ch->in_room) && ch->position > POS_SLEEPING)
		{
			for (fch = ch->in_room->first_person ; fch ; fch = fch->next_in_room)
			{
				if (col < max_width && can_see(ch, fch) && !(IS_AFFECTED(fch, AFF_HIDE) || IS_AFFECTED(fch, AFF_STEALTH) || IS_AFFECTED(fch, AFF_ETHEREAL)))
				{
					pti = IS_NPC(fch) ? (bool *) capitalize (fch->short_descr) : (bool *) fch->name;

					if (fch->fighting && (who_fighting(fch) == NULL || fch->fighting->who->in_room != fch->in_room))
					{
						stop_fighting(fch, FALSE);
					}

					bld = (fch->fighting != NULL) ? 128 : 0;

					if (HAS_BIT(pcd->tactical_flags, TACTICAL_TOP))
					{
						pto  = tm + size_h * row + col;
						ptoc = tc + size_h * row + col;
					}
					else
					{
						pto  = tm + size_h * (row - 1) + col;
						ptoc = tc + size_h * (row - 1) + col;
					}
					cnt = 0;

					*pto++ = (fch->hit >= fch->max_hit) ? 'F' : '0' + 10 * fch->hit / UMAX(1, fch->max_hit);

					if ((10 * fch->hit / UMAX(1, fch->max_hit)) > 7)
					{
						*ptoc = 2 + pcd->color[COLOR_TACTICAL] / 10 * 8;
					}
					else if ((10 * fch->hit / UMAX(1, fch->max_hit)) > 3)
					{
						*ptoc = 3 + pcd->color[COLOR_TACTICAL] / 10 * 8;
					}
					else
					{
						*ptoc = 1 + pcd->color[COLOR_TACTICAL] / 10 * 8;
					}
					ptoc++;

					if (HAS_BIT(pcd->tactical_flags, TACTICAL_INDEX))
					{
						if (IS_NPC(fch))
						{
							*pto++ = isupper(*fch->name) ? *fch->name : toupper(*fch->name);
							ptoc++;
							*pto++ = 'a' + ((int) fch % 25);
							ptoc++;
						}
						else if (fch->pcdata->tactical_index[0] == '\0')
						{
							*pto++ = '0' + fch->pcdata->pvnum % 100 / 10;
							ptoc++;
							*pto++ = '0' + fch->pcdata->pvnum % 10;
							ptoc++;
						}
						else
						{
							*pto++ = fch->pcdata->tactical_index[0];
							ptoc++;
							*pto++ = fch->pcdata->tactical_index[1];
							ptoc++;
						}
					}
					else
					{
						*pto++ = '-';
						ptoc++;
					}

					if (who_fighting(fch) && is_same_group(fch->fighting->who, ch))
					{
						colors = pcd->color[COLOR_TACT_ENEMY] % 10   + pcd->color[COLOR_TACT_ENEMY] / 10 * 8;
					}
					else if (is_same_group (fch, ch))
					{
						colors = pcd->color[COLOR_TACT_PARTY] % 10   + pcd->color[COLOR_TACT_PARTY] / 10 * 8;
					}
					else
					{
						colors = pcd->color[COLOR_TACT_NEUTRAL] % 10 + pcd->color[COLOR_TACT_NEUTRAL] / 10 * 8;
					}

					if (who_fighting(fch) == NULL || !HAS_BIT(pcd->tactical_flags, TACTICAL_INDEX))
					{
						for ( ; cnt < 12 && *pti ; cnt++)
						{
							*pto++  = *pti++ + bld;
							*ptoc++ = colors;
						}
					}
					else
					{
						for ( ; cnt < 9 && *pti ; cnt++)
						{
							*pto++  = *pti++ + bld;
							*ptoc++ = colors;
						}
						for ( ; cnt < 9 ; cnt++)
						{
							*pto++  = '-' + bld;
							*ptoc++ = colors;
						}
						*pto++ = '>';
						ptoc++;

						if (IS_NPC(fch->fighting->who))
						{
							*pto++ = isupper(*fch->fighting->who->name) ? *fch->fighting->who->name : toupper(*fch->fighting->who->name);
							ptoc++;
							*pto++ = 'a' + ((int) fch->fighting->who % 25);
							ptoc++;
						}
						else if (fch->fighting->who->pcdata->tactical_index[0] == '\0')
						{
							*pto++ = '0' + ((fch->fighting->who->pcdata->pvnum % 100) / 10);
							ptoc++;
							*pto++ = '0' + (fch->fighting->who->pcdata->pvnum % 10);
							ptoc++;
						}
						else
						{
							*pto++ = fch->fighting->who->pcdata->tactical_index[0];
							ptoc++;
							*pto++ = fch->fighting->who->pcdata->tactical_index[1];
							ptoc++;
						}
					}
					row++;
					if (row > size_v - 2)
					{
						row = 1;
						col += 16;
					}
				}
			}
		}

		/* Compass code  */

		if (can_see_in_room(ch, ch->in_room))
		{
			if (size_v - 1 > 3 && cw > 0)
			{
				ROOM_INDEX_DATA *room = ch->in_room;
				for (door = -1 ; door <= 5 ; door++)
				{
					if (door == -1)
					{
						if (!can_see_in_room(ch, room))
						{
							strcpy(buf3, "Too dark to tell");
						}
						else
						{
							strcpy(buf3, room->name);
						}
						colors = sector_table[room->sector_type].color;
					}
					else
					{
						if ((pexit = get_exit(room->vnum, door)) == NULL)
						{
							continue;
						}
						if (IS_SET(ch->in_room->room_flags, ROOM_SMOKE) && !can_see_smoke(ch))
						{
							continue;
						}
						if (IS_SET(pexit->flags, EX_HIDDEN) && !can_see_hidden(ch))
						{
							continue;
						}
						if (!can_use_exit(ch, pexit))
						{
							continue;
						}
						colors = sector_table[room_index[pexit->to_room]->sector_type].color;
						if (!IS_SET(pexit->flags, EX_CLOSED))
						{
							if (!can_see_in_room(ch, room_index[pexit->to_room]))
							{
								strcpy (buf3, "Too dark to tell");
							}
							else
							{
								strcpy (buf3, room_index[pexit->to_room]->name);
							}
						}
						else
						{
							if (pexit->keyword[0] != '\0')
							{
								strcpy(buf3, capitalize(pexit->keyword));
							}
							else
							{
								strcpy(buf3, "Door");
							}
						}
					}

					if (HAS_BIT(pcd->tactical_flags, TACTICAL_TOP))
					{
						vo = 0;
					}
					else
					{
						vo = -1;
					}
					offset = 0;

					if (buf3[0] == 't' || buf3[0] == 'T')
					{
						if (buf3[1] == 'h' || buf3[1] == 'H')
						{
							if (buf3[2] == 'e' || buf3[2] == 'E')
							{
								if (buf3[3] == ' ')
								{
									offset = 4;
								}
							}
						}
					}
					if (buf3[0] == 'a' || buf3[0] == 'A')
					{
						if (buf3[1] == ' ')
						{
							offset = 2;
						}
					}
					str_cpy_max (buf2, capitalize (buf3 + offset), cw + 1);
					if (size_v - 1 > 5)
					{
						switch (door)
						{
							case -1:
								pto  = tm + size_h * (3 + vo) + size_h - 1 - 2 * cw;
								ptoc = tc + size_h * (3 + vo) + size_h - 1 - 2 * cw;
								break;
							case 0:
								pto  = tm + size_h * (2 + vo) + size_h - 2 - 3 * cw / 2;
								*pto = '|';
								pto  = tm + size_h * (1 + vo) + size_h - 1 - 2 * cw;
								ptoc = tc + size_h * (1 + vo) + size_h - 1 - 2 * cw;
								break;
							case 1:
								pto  = tm + size_h * (3 + vo) + size_h - 1 - cw;
								*pto = '-';
								pto  = tm + size_h * (3 + vo) + size_h - cw;
								ptoc = tc + size_h * (3 + vo) + size_h - cw;
								break;
							case 2:
								pto  = tm + size_h * (4 + vo) + size_h - 2 - 3 * cw / 2;
								*pto = '|';
								pto  = tm + size_h * (5 + vo) + size_h - 1 - 2 * cw;
								ptoc = tc + size_h * (5 + vo) + size_h - 1 - 2 * cw;
								break;
							case 3:
								pto  = tm + size_h * (3 + vo) + size_h - 2 - 2 * cw;
								*pto = '-';
								pto  = tm + size_h * (3 + vo) + size_h - 2 - 3 * cw;
								ptoc = tc + size_h * (3 + vo) + size_h - 2 - 3 * cw;
								break;
							case 4:
								pto  = tm + size_h * (2 + vo) + size_h - 1 - cw;
								*pto = '/';
								pto  = tm + size_h * (1 + vo) + size_h - cw;
								ptoc = tc + size_h * (1 + vo) + size_h - cw;
								break;
							case 5:
								pto  = tm + size_h * (4 + vo) + size_h - 2 - 2 * cw;
								*pto = '/';
								pto  = tm + size_h * (5 + vo) + size_h - 2 - 3 * cw;
								ptoc = tc + size_h * (5 + vo) + size_h - 2 - 3 * cw;
								break;
						}
						for (cnt = 0, found = FALSE, pti = (bool *)buf2 ; cnt < cw ; ptoc++, pto++, cnt++)
						{
							if (found)
							{
								*pto  = ' ';
								*ptoc = (bool) colors;
							}
							else if (*pti == '\n' || *pti == '\r' || *pti == '\0')
							{
								found = TRUE;
								*pto  = ' ';
								*ptoc = (bool) colors;
							}
							else
							{
								*pto  = *pti;
								*ptoc = (bool) colors;
								pti++;
							}
						}
					}
					else
					{
						switch (door)
						{
							case -1:
								pto  = tm + size_h * (2 + vo) + size_h - 1 - 2 * cw;
								ptoc = tc + size_h * (2 + vo) + size_h - 1 - 2 * cw;
								break;
							case 0:
								pto  = tm + size_h * (1 + vo) + size_h - 1 - 2 * cw;
								ptoc = tc + size_h * (1 + vo) + size_h - 1 - 2 * cw;
								break;
							case 1:
								pto  = tm + size_h * (2 + vo) + size_h - 1 - cw;
								*pto = '-';
								pto  = tm + size_h * (2 + vo) + size_h - cw;
								ptoc = tc + size_h * (2 + vo) + size_h - cw;
								break;
							case 2:
								pto  = tm + size_h * (3 + vo) + size_h - 1 - 2 * cw;
								ptoc = tc + size_h * (3 + vo) + size_h - 1 - 2 * cw;
								break;
							case 3:
								pto  = tm + size_h * (2 + vo) + size_h - 2 - 2 * cw;
								*pto = '-';
								pto  = tm + size_h * (2 + vo) + size_h - 2 - 3 * cw;
								ptoc = tc + size_h * (2 + vo) + size_h - 2 - 3 * cw;
								break;
							case 4:
								pto  = tm + size_h * (1 + vo) + size_h - 1 - cw;
								*pto = '/';
								pto  = tm + size_h * (1 + vo) + size_h - cw;
								ptoc = tc + size_h * (1 + vo) + size_h - cw;
								break;
							case 5:
								pto  = tm + size_h * (3 + vo) + size_h - 2 - 2 * cw;
								*pto = '/';
								pto  = tm + size_h * (3 + vo) + size_h - 2 - 3 * cw;
								ptoc = tc + size_h * (3 + vo) + size_h - 2 - 3 * cw;
								break;
						}
						for (cnt = 0, found = FALSE, pti = (bool *) buf2 ; cnt < cw; ptoc++, pto++, cnt++)
						{
							if (found)
							{
								*pto  = ' ';
								*ptoc = (bool) colors;
							}
							else if (*pti == '\n' || *pti == '\r' || *pti == '\0')
							{
								found = TRUE;
								*pto  = ' ';
								*ptoc = (bool) colors;
							}
							else
							{
								*pto  = *pti;
								*ptoc = (bool) colors;
								pti++;
							}
						}
					}
				}
			}
		}
	}
	pop_call();
	return (mud->tactical);
}

TACTICAL_MAP *get_diff_tactical (CHAR_DATA * ch)
{
	int size_v, size_h;

	push_call("get_diff_tactical(%p)",ch);

	size_v = CH(ch->desc)->pcdata->tactical_size_v + 1;
	size_h = CH(ch->desc)->pcdata->screensize_h;

	get_tactical_map(ch);

	if (CH(ch->desc)->pcdata->tactical == NULL)
	{
		pop_call();
		return (mud->tactical);
	}

	if (memcmp(mud->tactical->map, CH(ch->desc)->pcdata->tactical->map, size_v * size_h))
	{
		pop_call();
		return mud->tactical;
	}

	if (memcmp(mud->tactical->color, CH(ch->desc)->pcdata->tactical->color, size_v * size_h))
	{
		pop_call();
		return mud->tactical;
	}
	pop_call();
	return NULL;
}



void vt100on (CHAR_DATA * ch)
{
	DESCRIPTOR_DATA *d;
	char buf[MAX_STRING_LENGTH];
	int n1, n2, n3;

	push_call("vt100on(%p)",ch);

	if (!is_desc_valid(ch))
	{
		pop_call();
		return;
	}

	d = ch->desc;

	write_to_port(d);

	if (CH(d)->pcdata->tactical == NULL)
	{
		ALLOCMEM(CH(d)->pcdata->tactical, TACTICAL_MAP, 1);
	}

	write_to_port(d);

	n1 = 1 + CH(d)->pcdata->tactical_size_v;

	n2 = CH(d)->pcdata->screensize_v - 2;

	n3 = CH(d)->pcdata->screensize_v - 1;

	sprintf(buf, "\033[2J\033[%d;%dr\033[%d;1H", n1, n2, n3);

	write_to_descriptor(d, buf, 0);

	sprintf(buf, "\033[%d;1H\033[0m", CH(d)->pcdata->screensize_v);

	write_to_descriptor(d, buf, 0);

	SET_BIT(CH(d)->pcdata->vt100_flags, VT100_INTERFACE);

	clear_tactical_map(CH(d)->pcdata->tactical);

	DEL_BIT(CH(d)->pcdata->vt100_flags, VT100_REFRESH);

	vt100prompter(ch);

	write_to_port (d);

	pop_call();
	return;
}

void vt100off (CHAR_DATA * ch)
{
	DESCRIPTOR_DATA *d;
	char buf[MAX_STRING_LENGTH];

	push_call("vt100off(%p)",ch);

	if (!is_desc_valid (ch))
	{
		pop_call();
		return;
	}

	if (!HAS_BIT(CH(ch->desc)->pcdata->vt100_flags, VT100_INTERFACE))
	{
		pop_call();
		return;
	}

	d = ch->desc;

	write_to_port (d);

	sprintf(buf, "\033[r\033[0m\033[2J");
	write_to_descriptor (d, buf, 0);

	d->outtop = 0;

	DEL_BIT(CH(d)->pcdata->vt100_flags, VT100_INTERFACE);

	REMOVE_BIT(CH(d)->pcdata->interp, INTERP_TACT_UPDATE);

	if (CH(d)->pcdata->tactical != NULL)
	{
		FREEMEM(CH(d)->pcdata->tactical);
		CH(d)->pcdata->tactical = NULL;
	}
	pop_call();
	return;
}

void vt100prompt(CHAR_DATA * ch)
{
	push_call("vt100prompt(%p)",ch);

	if (is_desc_valid(ch))
	{
		if (HAS_BIT(CH(ch->desc)->pcdata->vt100_flags, VT100_INTERFACE))
		{
			if (CH(ch->desc)->pcdata->tactical == NULL)
			{
				vt100prompter(ch);
			}
			else
			{
				SET_BIT(CH(ch->desc)->pcdata->interp, INTERP_TACT_UPDATE);
			}
		}
	}
	pop_call();
	return;
}

void vt100prompter (CHAR_DATA * ch)
{
	DESCRIPTOR_DATA *d;
	char buf[MAX_STRING_LENGTH];
	TACTICAL_MAP *tact;

	push_call("vt100prompter(%p)",ch);

	tact = NULL;

	if (!is_desc_valid(ch))
	{
		pop_call();
		return;
	}

	d = ch->desc;

	if (CH(d)->pcdata->tactical == NULL)
	{
		vt100on(ch);

		pop_call();
		return;
	}

	if (HAS_BIT(CH(d)->pcdata->vt100_flags, VT100_REFRESH))
	{
		do_refresh(ch, "");
		pop_call();
		return;
	}

	tact = get_diff_tactical(ch);

	REMOVE_BIT(CH(d)->pcdata->interp, INTERP_TACT_UPDATE);

	if (tact == NULL)
	{
		pop_call();
		return;
	}

	sprintf(buf, "\0337%s\0338%s", get_tactical_string(ch, tact), get_color_string(ch, COLOR_PROMPT, VT102_DIM));

	write_to_buffer(d, buf, 1000000);

	pop_call();
	return;
}


/*
	Adjust vt100 settings to given NAWS
*/

void process_naws( DESCRIPTOR_DATA *d, int width, int height )
{
	push_call("process_naws(%p,%d,%d)",d, width, height);

	if (CH(d) == NULL)
	{
		pop_call();
		return;
	}

	if (height)
	{
		height = URANGE(15, height, 99);

		if (height != CH(d)->pcdata->screensize_v)
		{
			CH(d)->pcdata->screensize_v = height;

			height = URANGE(1, CH(d)->pcdata->screensize_v / 2, MAX_TACTICAL_ROW - 1);

			if (CH(d)->pcdata->tactical_size_v > height)
			{
				CH(d)->pcdata->tactical_size_v = height;
			}

			if (d->connected >= CON_PLAYING && HAS_BIT(CH(d)->pcdata->vt100_flags, VT100_INTERFACE))
			{
				SET_BIT(CH(d)->pcdata->vt100_flags, VT100_REFRESH);
				SET_BIT(CH(d)->pcdata->interp, INTERP_TACT_UPDATE);
			}
		}
	}

	if (width)
	{
		width = URANGE(80, width, MAX_TACTICAL_COL);

		if (width != CH(d)->pcdata->screensize_h)
		{
			CH(d)->pcdata->screensize_h = width;

			if (d->connected >= CON_PLAYING && HAS_BIT(CH(d)->pcdata->vt100_flags, VT100_INTERFACE))
			{
				SET_BIT(CH(d)->pcdata->vt100_flags, VT100_REFRESH);
				SET_BIT(CH(d)->pcdata->interp, INTERP_TACT_UPDATE);
			}
		}
	}
	pop_call();
	return;
}
