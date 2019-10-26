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

#include <stdarg.h>
#include "mud.h"

/*
	Local functions.
*/

bool is_note_to args ((CHAR_DATA * ch, NOTE_DATA * pnote));
void note_attach args ((CHAR_DATA * ch));
void note_remove args ((CHAR_DATA * ch, NOTE_DATA * pnote, bool remove_all));


bool is_note_to (CHAR_DATA * ch, NOTE_DATA * pnote)
{
	push_call("is_note_to (%p, %p)",ch,pnote);

	if (IS_NPC(ch) || IS_SET(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_MUTED))
	{
		pop_call();
		return FALSE;
	}

	if (ch->pcdata->note_topic != pnote->topic && !IS_SET(ch->in_room->room_flags, ROOM_NOTE_BOARD))
	{
		pop_call();
		return FALSE;
	}

	if (ch->name[0] == pnote->sender[0] && !strcasecmp(ch->name, pnote->sender))
	{
		pop_call();
		return TRUE;
	}

	if (is_name(ch->name, pnote->to_list))
	{
		pop_call();
		return TRUE;
	}

	if (is_name ("all", pnote->to_list))
	{
		pop_call();
		return TRUE;
	}

	if (IS_IMMORTAL(ch) && is_name("immortal", pnote->to_list))
	{
		pop_call();
		return TRUE;
	}
	pop_call();
	return FALSE;
}

void note_attach (CHAR_DATA * ch)
{
	NOTE_DATA *pnote;

	push_call("note_attach(%p)",ch);

	if (ch->pcdata->pnote != NULL)
	{
		pop_call();
		return;
	}
	ALLOCMEM (pnote, NOTE_DATA, 1);

	pnote->next		= NULL;
	pnote->prev		= NULL;
	pnote->sender		= STRALLOC(ch->name);
	pnote->date		= STRALLOC ("");
	pnote->to_list 	= STRALLOC ("");
	pnote->subject 	= STRALLOC ("");
	pnote->text		= STRALLOC ("");
	pnote->topic		= -1;
	pnote->room_vnum	= 0;
	ch->pcdata->pnote	= pnote;
	pop_call();
	return;
}

void note_remove (CHAR_DATA * ch, NOTE_DATA * pnote, bool remove_all)
{
	char to_new[MAX_INPUT_LENGTH];
	char to_one[MAX_INPUT_LENGTH];
	char *to_list;

	push_call("note_remove(%p,%p,%p)",ch,pnote,remove_all);

	/*
		Build a new to_list.
		Strip out this recipient.
	*/

	to_new[0] = '\0';
	to_list = pnote->to_list;
	while (*to_list != '\0')
	{
		to_list = one_argument (to_list, to_one);
		if (to_one[0] != '\0' && strcasecmp (ch->name, to_one))
		{
			strcat (to_new, " ");
			strcat (to_new, to_one);
		}
	}

	/*
		Just a simple recipient removal?
	*/

	if (!remove_all && strcasecmp (ch->name, pnote->sender) && to_new[0] != '\0')
	{
		STRFREE (pnote->to_list);
		pnote->to_list = STRALLOC (to_new + 1);
		pop_call();
		return;
	}

	/*
		Remove note from linked list.
	*/

	UNLINK(pnote, mud->f_note, mud->l_note, next, prev);

	STRFREE (pnote->text);
	STRFREE (pnote->subject);
	STRFREE (pnote->to_list);
	STRFREE (pnote->date);
	STRFREE (pnote->sender);
	FREEMEM (pnote);

	/*
		Rewrite entire list.
	*/

	save_notes();

	pop_call();
	return;
}

bool new_notes (CHAR_DATA *ch)
{
	NOTE_DATA *pnote;
	int oldTopic;

	push_call("new_notes(%p)",ch);

	if (ch->level < 5)
	{
		pop_call();
		return FALSE;
	}

	oldTopic = ch->pcdata->note_topic;

	for (pnote = mud->f_note ; pnote ; pnote = pnote->next)
	{
		ch->pcdata->note_topic = pnote->topic;

		if (is_note_to(ch, pnote) && pnote->time >= ch->pcdata->last_time)
		{
			if (pnote->room_vnum == 0 || pnote->room_vnum == ch->in_room->vnum)
			{
				ch->pcdata->note_topic = oldTopic;
				pop_call();
				return TRUE;
			}
		}
	}
	ch->pcdata->note_topic = oldTopic;
	pop_call();
	return FALSE;
}


void do_note (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	PLAYER_GAME *fch;
	NOTE_DATA *pnote;
	int vnum, anum, cnt, room_vnum;
	bool room_board;

	push_call("do_note(%p,%p)",ch,argument);

	if (IS_NPC (ch))
	{
		pop_call();
		return;
	}

	if (!ch->desc)
	{
		bug ("do_note: no descriptor", 0);
		pop_call();
		return;
	}

	if (ch->level < 5 && ch->pcdata->reincarnation == 0)
	{
		send_to_char ("You must be at least level 5 to use the Note Boards.\n\r", ch);
		pop_call();
		return;
	}

	switch (ch->pcdata->editmode)
	{
		default:
			break;
		case MODE_RESTRICTED:
			send_to_char ("You cannot use this command from while editing something else.\n\r", ch);
			pop_call();
			return;
			break;

		case MODE_WRITING_NOTE:
			STRFREE(ch->pcdata->pnote->text);
			ch->pcdata->pnote->text = copy_buffer (ch);
			stop_editing (ch);
			pop_call();
			return;
			break;
	}

	argument = one_argument (argument, arg);
	smash_tilde (argument);

	if (ch->in_room != NULL && IS_SET(ch->in_room->room_flags, ROOM_NOTE_BOARD) && ch->in_room->vnum > 0)
	{
		room_board = TRUE;
		room_vnum  = ch->in_room->vnum;
	}
	else
	{
		room_board = FALSE;
		room_vnum  = 0;
	}

	*buf = '\0';

	if (!strcasecmp(arg, "list"))
	{
		if (room_vnum)
		{
			strcpy (buf, "{078}Note Board:\n\r");
		}
		else
		{
			sprintf(buf, "{078}TOPIC: %s\n\r", topic_table[ch->pcdata->note_topic].name);
		}

		if (is_number (argument))
		{
			int curtime;

			send_to_char_color(buf, ch);

			curtime = mud->current_time - (24 * 60 * 60 * atol (argument));
			vnum = 0;

			for (buf[0] = '\0', pnote = mud->f_note ; pnote ; pnote = pnote->next)
			{
				if (is_note_to(ch, pnote) && pnote->time >= curtime && pnote->room_vnum == room_vnum)
				{
					if (room_vnum || pnote->time <= ch->pcdata->topic_stamp[pnote->topic])
					{
						cat_sprintf(buf, "{078}[%3u]{038} %s to %s:{078} %s\n\r", vnum, pnote->sender, pnote->to_list, pnote->subject);
					}
					else
					{
						cat_sprintf(buf, "{178}[%3u]{128}*{138}%s to %s:{178} %s\n\r", vnum, pnote->sender, pnote->to_list, pnote->subject);
					}
				}
				vnum++;
			}
			cat_sprintf(buf, "{078}The {128}*{078} denotes an unread note.\n\r");
			send_to_char_color(buf, ch );
		}
		else if (!strcasecmp (argument, "new"))
		{
			int oldTopic	= ch->pcdata->note_topic;

			vnum = 0;

			for (buf[0] = '\0', pnote = mud->f_note ; pnote ; pnote = pnote->next)
			{
				ch->pcdata->note_topic = pnote->topic;

				if (is_note_to(ch, pnote) && pnote->time >= ch->pcdata->last_time && pnote->room_vnum == room_vnum)
				{
					if (pnote->time <= ch->pcdata->topic_stamp[pnote->topic])
					{
						cat_sprintf(buf, "{078}[%3u]{038} %s to %s:{078} %s\n\r", vnum, pnote->sender, pnote->to_list, pnote->subject);
					}
					else
					{
						cat_sprintf(buf, "{178}[%3u]{128}*{138}%s to %s:{178} %s\n\r", vnum, pnote->sender, pnote->to_list, pnote->subject);
					}
				}
				vnum++;
			}
			if (buf[0] == '\0')
			{
				send_to_char ("There have been no new notes since you last logged out.\n\r", ch);
			}
			else
			{
				cat_sprintf(buf, "{078}The {128}*{078} denotes an unread note.\n\r");
				send_to_char_color(buf, ch);
			}
			ch->pcdata->note_topic = oldTopic;
		}
		else if (!strcasecmp (argument, "unread"))
		{
			int unreadNum[MAX_TOPIC]	= {0};
			int oldTopic			= ch->pcdata->note_topic;

			buf[0] = '\0';

			for (pnote = mud->f_note ; pnote ; pnote = pnote->next)
			{
				ch->pcdata->note_topic = pnote->topic;

				if (is_note_to(ch, pnote) && pnote->room_vnum == room_vnum && pnote->time > ch->pcdata->topic_stamp[pnote->topic])
				{
					unreadNum[pnote->topic]++;
				}
			}

			for (cnt = 0 ; cnt < MAX_TOPIC - 1 ; cnt++)
			{
				cat_sprintf(buf, "{038}%2d  {078}%-22s {138}%d\n\r", cnt, topic_table[cnt].name, unreadNum[cnt]);
			}
			send_to_char_color(buf, ch);

			ch->pcdata->note_topic = oldTopic;
		}
		else if (argument[0] != '\0')
		{
			send_to_char_color (buf, ch);
			vnum = 0;

			for (buf[0] = '\0', pnote = mud->f_note ; pnote ; pnote = pnote->next)
			{
				if (is_note_to(ch, pnote) && pnote->room_vnum == room_vnum && ((!str_infix(argument, pnote->sender)) || (!str_infix(argument, pnote->subject)) || (!str_infix(argument, pnote->to_list))))
				{
					if (room_vnum || pnote->time <= ch->pcdata->topic_stamp[pnote->topic])
					{
						cat_sprintf(buf, "{078}[%3u]{038} %s to %s:{078} %s\n\r", vnum, pnote->sender, pnote->to_list, pnote->subject);
					}
					else
					{
						cat_sprintf (buf, "{178}[%3u]{128}*{138}%s to %s:{178} %s\n\r", vnum, pnote->sender, pnote->to_list, pnote->subject);
					}
				}
				vnum++;
			}
			cat_sprintf(buf, "{078}The {128}*{078} denotes an unread note.\n\r");
			send_to_char_color (buf, ch );
		}
		else
		{
			send_to_char_color (buf, ch);
			vnum = 0;

			for (buf[0] = '\0', pnote = mud->f_note ; pnote ; pnote = pnote->next)
			{
				if (is_note_to(ch, pnote) && pnote->room_vnum == room_vnum)
				{
					if (room_vnum || pnote->time <= ch->pcdata->topic_stamp[pnote->topic])
					{
						cat_sprintf(buf, "{078}[%3u]{038} %s to %s:{078} %s\n\r", vnum, pnote->sender, pnote->to_list, pnote->subject);
					}
					else
					{
						cat_sprintf(buf, "{178}[%3u]{128}*{138}%s to %s:{178} %s\n\r", vnum, pnote->sender, pnote->to_list, pnote->subject);
					}
				}
				vnum++;
			}
			cat_sprintf(buf, "{078}The {128}*{078} denotes an unread note.\n\r");
			send_to_char_color (buf, ch );
		}
		pop_call();
		return;
	}

	if (!strcasecmp (arg, "next"))
	{
		vnum = 0;

		for (pnote = mud->f_note ; pnote ; pnote = pnote->next, vnum++)
		{
			if (pnote->time <= ch->pcdata->last_note)
			{
				continue;
			}
			if (is_note_to(ch, pnote) && pnote->room_vnum == room_vnum)
			{
				sprintf(buf, "{178}[%3u]{138} %s:{178} %s\n\r%s\t\tTopic: %s\n\rTo: %s{078}\n\r", vnum, pnote->sender, pnote->subject, pnote->date, room_vnum ? "Private Note Board" : topic_table[pnote->topic].name, pnote->to_list);

				ch_printf_color(ch, "%s%s", buf, ansi_justify(pnote->text, 80));

				if (ch->pcdata->topic_stamp[pnote->topic] < pnote->time)
				{
					ch->pcdata->topic_stamp[ch->pcdata->note_topic] = pnote->time;
				}
				ch->pcdata->last_note = pnote->time;
				pop_call();
				return;
			}
		}
		send_to_char ("End of notes.\n\r", ch);
		pop_call();
		return;
	}

	if (!strcasecmp(arg, "unread") || arg[0] == '\0')
	{
		vnum = 0;

		if (room_vnum)
		{
			ch->pcdata->note_topic = 6;
		}

		for (pnote = mud->f_note ; pnote ; pnote = pnote->next, vnum++)
		{
			if (pnote->time <= ch->pcdata->topic_stamp[ch->pcdata->note_topic])
			{
				continue;
			}

			if (pnote->topic != ch->pcdata->note_topic)
			{
				continue;
			}

			if (is_note_to(ch, pnote) && pnote->room_vnum == room_vnum)
			{
				sprintf(buf, "{178}[%3u]{138} %s:{178} %s\n\r%s\t\tTopic: %s\n\rTo: %s{078}\n\r", vnum, pnote->sender, pnote->subject, pnote->date, room_vnum ? "Private Note Board" : topic_table[pnote->topic].name, pnote->to_list);

				ch_printf_color(ch, "%s%s", buf, ansi_justify(pnote->text, 80));

				if (ch->pcdata->topic_stamp[pnote->topic] < pnote->time)
				{
					ch->pcdata->topic_stamp[ch->pcdata->note_topic] = pnote->time;
				}
				ch->pcdata->last_note = pnote->time;
				pop_call();
				return;
			}
		}
		if (!room_vnum && ch->pcdata->note_topic < 5)
		{
			ch->pcdata->note_topic++;
			ch_printf(ch, "You've read all the notes in this topic. Proceeding to %s.\n\r", topic_table[ch->pcdata->note_topic].name);
			do_note(ch, "unread");
		}
		else
		{
			send_to_char ("End of notes.\n\r", ch);
			ch->pcdata->note_topic = 0;
		}
		pop_call();
		return;
	}

	if (!strcasecmp(arg, "catchup"))
	{
		for (vnum = 0 ; vnum < MAX_TOPIC ; vnum++)
		{
			ch->pcdata->topic_stamp[vnum] = mud->current_time;
		}
		ch->pcdata->last_note = mud->current_time;

		ch_printf(ch, "All notes have been marked as read.\n\r");

		pop_call();
		return;
	}

	if (!strcasecmp(arg, "read") || is_number(arg))
	{
		int old_topic = ch->pcdata->note_topic;

		if (is_number(arg))
		{
			anum = atol(arg);
		}
		else
		{
			if (is_number(argument))
			{
				anum = atol(argument);
			}
			else
			{
				send_to_char("Read what note number?\n\r", ch );
				pop_call();
				return;
			}
		}

		vnum = 0;

		for (pnote = mud->f_note ; pnote ; pnote = pnote->next)
		{
			ch->pcdata->note_topic = pnote->topic;

			if (is_note_to(ch, pnote) && vnum == anum && pnote->room_vnum == room_vnum)
			{
				sprintf(buf, "{178}[%3u]{138} %s:{178} %s\n\r%s\t\tTopic: %s\n\rTo: %s{078}\n\r", vnum, pnote->sender, pnote->subject, pnote->date, room_vnum ? "Private Note Board" : topic_table[pnote->topic].name, pnote->to_list);

				ch_printf_color(ch, "%s%s", buf, ansi_justify(pnote->text, 80));

				ch->pcdata->last_note = pnote->time;
				if (ch->pcdata->topic_stamp[pnote->topic] < pnote->time)
				{
					ch->pcdata->topic_stamp[pnote->topic] = pnote->time;
				}
				ch->pcdata->note_topic = old_topic;
				pop_call();
				return;
			}
			vnum++;
		}
		ch->pcdata->note_topic = old_topic;

		send_to_char ("No such note.\n\r", ch);
		pop_call();
		return;
	}

	if (!strcasecmp (arg, "subject"))
	{
		note_attach (ch);
		STRFREE(ch->pcdata->pnote->subject);
		ch->pcdata->pnote->subject = STRALLOC (argument);
		send_to_char ("Ok.\n\r", ch);
		pop_call();
		return;
	}

	if (!strcasecmp (arg, "to"))
	{
		note_attach (ch);
		STRFREE (ch->pcdata->pnote->to_list);
		ch->pcdata->pnote->to_list = STRALLOC (argument);
		send_to_char ("Ok.\n\r", ch);
		pop_call();
		return;
	}

	if (!strcasecmp (arg, "write"))
	{
		note_attach (ch);
		ch->pcdata->editmode = MODE_WRITING_NOTE;
		ch->pcdata->tempmode = MODE_NONE;
		start_editing (ch, ch->pcdata->pnote->text);
		pop_call();
		return;
	}

	if (!strcasecmp (arg, "reply"))
	{
		NOTE_DATA *oldNote;

		if (!is_number(argument))
		{
			send_to_char ("Use: note reply <note number>.\n\r", ch);
			pop_call();
			return;
		}
		vnum = atol (argument);
		anum = 0;
		for (oldNote = mud->f_note ; oldNote ; oldNote = oldNote->next)
		{
			if (anum++ == vnum)
			{
				break;
			}
		}

		if (oldNote == NULL)
		{
			sprintf (buf, "There is no note #%u.\n\r", vnum);
			send_to_char (buf, ch);
			pop_call();
			return;
		}

		if (oldNote->topic == 0 && oldNote->room_vnum == 0 && !IS_GOD(ch))
		{
			ch_printf(ch, "You are not allowed to reply on the Announcements topic.\n\r");
			pop_call();
			return;
		}

		if (oldNote->room_vnum != room_vnum)
		{
			send_to_char ("There is no such note.\n\r", ch);
			pop_call();
			return;
		}

		note_attach (ch);
		STRFREE (ch->pcdata->pnote->to_list);
		STRFREE (ch->pcdata->pnote->subject);

		if (is_name ("all", oldNote->to_list) || is_name (oldNote->sender, oldNote->to_list))
		{
			strcpy (buf, oldNote->to_list);
		}
		else
		{
			sprintf (buf, "%s %s", oldNote->sender, oldNote->to_list);
		}
		ch->pcdata->pnote->to_list = STRALLOC (buf);
		ch->pcdata->pnote->topic   = oldNote->topic;

		if (oldNote->subject[0] == 'R' && oldNote->subject[1] == 'e' && oldNote->subject[2] == ':' && oldNote->subject[3] == ' ')
		{
			ch->pcdata->pnote->subject = STRALLOC (oldNote->subject);
		}
		else
		{
			sprintf (buf, "Re: %s", oldNote->subject);
			ch->pcdata->pnote->subject = STRALLOC (buf);
		}

		sprintf (buf, "%s: %s\n\rTo: %s\n\r", ch->pcdata->pnote->sender, ch->pcdata->pnote->subject, ch->pcdata->pnote->to_list);
		send_to_char (buf, ch);
		pop_call();
		return;
	}

	if (!strcasecmp (arg, "clear"))
	{
		if (ch->pcdata->pnote != NULL)
		{
			STRFREE (ch->pcdata->pnote->text);
			STRFREE (ch->pcdata->pnote->subject);
			STRFREE (ch->pcdata->pnote->to_list);
			STRFREE (ch->pcdata->pnote->date);
			STRFREE (ch->pcdata->pnote->sender);
			FREEMEM (ch->pcdata->pnote);
			ch->pcdata->pnote = NULL;
		}
		else
		{
			send_to_char("There was nothing to clear.\n\r",ch);
		}
		send_to_char ("Ok.\n\r", ch);
		pop_call();
		return;
	}

	if (!strcasecmp (arg, "show"))
	{
		if (ch->pcdata->pnote == NULL)
		{
			send_to_char ("You have no note in progress.\n\r", ch);
			pop_call();
			return;
		}
		ch_printf_color(ch, "{138}%s:{178} %s\n\rTo: %s\n\r", ch->pcdata->pnote->sender, ch->pcdata->pnote->subject, ch->pcdata->pnote->to_list);
		ch_printf_color(ch, "{078}%s", ansi_justify(ch->pcdata->pnote->text, 80));
		pop_call();
		return;
	}

	if (!strcasecmp (arg, "post"))
	{
		bool save_all = FALSE, can_announce = FALSE;
		NOTE_DATA *pnote_next;

		if (!IS_GOD(ch))
		{
			can_announce = FALSE;
		}
		else
		{
			can_announce = TRUE;
		}

		if (ch->pcdata->pnote == NULL)
		{
			send_to_char ("You have no note in progress.\n\r", ch);
			pop_call();
			return;
		}

		if (!strcasecmp(ch->pcdata->pnote->to_list, ""))
		{
			send_to_char ("This note is addressed to no one!\n\r", ch);
			pop_call();
			return;
		}

		if (!strcasecmp (ch->pcdata->pnote->subject, ""))
		{
			send_to_char ("This note has no subject.\n\r", ch);
			pop_call();
			return;
		}

		if (!strcasecmp (ch->pcdata->pnote->text, ""))
		{
			send_to_char ("There is nothing written on this note.\n\r", ch);
			pop_call();
			return;
		}

		if (ch->pcdata->pnote->topic == -1)
		{
			if (room_vnum)
			{
				ch->pcdata->pnote->topic = MAX_TOPIC -1;
			}
			else if (is_number(argument))
			{
				if (atol(argument) >= 0 && atol(argument) < MAX_TOPIC - 1)
				{
					ch->pcdata->pnote->topic = atol(argument);
				}
			}
		}

		if (ch->pcdata->pnote->topic == 0 && can_announce == FALSE)
		{
			send_to_char("You are not allowed to post on the Announcements topic.\n\r", ch);
			pop_call();
			return;
		}

		if (ch->pcdata->pnote->topic == -1)
		{
			ch_printf(ch, "You must specify a valid topic number for this note to be posted under.\n\rAvailable topics are:\n\r\n\r");

			for (cnt = !can_announce ; cnt < MAX_TOPIC - 1 ; cnt++)
			{
				if (ch->level >= topic_table[cnt].min_level)
				{
					ch_printf_color(ch, "{038}%2d  {078}%s\n\r", cnt, topic_table[cnt].name);
				}
			}
			pop_call();
			return;
		}

		ch->pcdata->pnote->date = STRALLOC(get_time_string(mud->current_time));

		if (IS_SET(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_MUTED))
		{
			ch->pcdata->pnote->time = 0;
		}
		else
		{
			ch->pcdata->pnote->time = mud->current_time;
		}
		if (IS_SET(ch->in_room->room_flags, ROOM_NOTE_BOARD))
		{
			ch->pcdata->pnote->room_vnum = ch->in_room->vnum;
		}
		else
		{
			ch->pcdata->pnote->room_vnum = 0;
		}
		LINK(ch->pcdata->pnote, mud->f_note, mud->l_note, next, prev);

		for (pnote = mud->f_note ; pnote->next ; pnote = pnote_next)
		{
			pnote_next = pnote->next;

			if (pnote->time <= mud->current_time - 60 * 60 * 24 * 7 * 4 * 3)
			{
				UNLINK(pnote, mud->f_note, mud->l_note, next, prev);

				STRFREE (pnote->text);
				STRFREE (pnote->subject);
				STRFREE (pnote->to_list);
				STRFREE (pnote->date);
				STRFREE (pnote->sender);
				FREEMEM (pnote);
				save_all = TRUE;
			}
		}
		ch->pcdata->pnote = NULL;

		/*  Added for recieving mail while on the game.  Chaos 12/15/93  */

		if (!room_board)
		{
			for (fch = mud->f_player ; fch ; fch = fch->next)
			{
				if (is_name(fch->ch->name, pnote->to_list)
				|| (is_name("immortal", pnote->to_list) && IS_IMMORTAL(fch->ch)))
				{
					send_to_char_color("{088}{506}You just received a note.{088}\n\r", fch->ch);
				}
			}
		}

		if (save_all)
		{
			save_notes();
		}

		if (!room_board)
		{
			send_to_char ("Ok.\n\r", ch);
		}
		else
		{
			send_to_char ("You post your note onto the noteboard.\n\r", ch);
		}
		pop_call();
		return;
	}

	if (!strcasecmp (arg, "remove"))
	{
		if (!is_number (argument))
		{
			send_to_char ("Note remove which number?\n\r", ch);
			pop_call();
			return;
		}
		anum = atol (argument);
		vnum = 0;
		for (pnote = mud->f_note ; pnote ; pnote = pnote->next)
		{
			if (is_note_to(ch, pnote) && vnum == anum)
			{
				note_remove (ch, pnote, FALSE);
				send_to_char ("Ok.\n\r", ch);
				pop_call();
				return;
			}
			vnum++;
		}
		send_to_char ("No such note.\n\r", ch);
		pop_call();
		return;
	}

	if (is_name(arg, "delete"))
	{
		if (!is_number(argument))
		{
			send_to_char ("Note delete which number?\n\r", ch);
			pop_call();
			return;
		}

		anum = atol (argument);
		vnum = 0;
		for (pnote = mud->f_note ; pnote ; pnote = pnote->next)
		{
			if (vnum == anum)
			{
				if (IS_IMMORTAL(ch) || !strcmp(ch->name, pnote->sender))
				{
					note_remove(ch, pnote, TRUE);
					ch_printf(ch, "Note %d has been deleted.\n\r", anum);
				}
				else
				{
					ch_printf(ch, "You can only delete your own notes.\n\r");
				}
				pop_call();
				return;
			}
			vnum++;
		}
		send_to_char ("No such note.\n\r", ch);
		pop_call();
		return;
	}

	if (!strcasecmp (arg, "topic"))
	{
		if (room_board)
		{
			send_to_char ("You are in a room containing a note board.\n\r", ch);
			pop_call();
			return;
		}
		if (is_number (argument))
		{
			bool num = atol(argument);

			if (num >= MAX_TOPIC - 1)
			{
				send_to_char("There is no such topic.\n\r", ch);
				pop_call();
				return;
			}
			ch->pcdata->note_topic = num;
			ch_printf( ch, "Note topic changed to: %s\n\r", topic_table[ch->pcdata->note_topic].name);
			pop_call();
			return;
		}
		else
		{
			int unreadNum[MAX_TOPIC]	= {0};
			int oldTopic			= ch->pcdata->note_topic;

			for (pnote = mud->f_note ; pnote ; pnote = pnote->next)
			{
				ch->pcdata->note_topic = pnote->topic;

				if (is_note_to(ch, pnote) && pnote->room_vnum == room_vnum && pnote->time > ch->pcdata->topic_stamp[pnote->topic])
				{
					unreadNum[pnote->topic]++;
				}
			}

			ch->pcdata->note_topic = oldTopic;

			send_to_char_color ("{178}List of note topics:\n\r\n\r", ch);
			send_to_char_color ("{138}Reference Number    {178}Topic Name:                 {128}Unread Notes\n\r", ch);
			for (cnt = 0 ; cnt < MAX_TOPIC - 1 ; cnt++)
			{
				if (ch->level >= topic_table[cnt].min_level)
				{
					ch_printf_color(ch, "{038}%2d                  {078}%-25s  {028}%2d\n\r", cnt, topic_table[cnt].name, unreadNum[cnt]);
				}
			}
		}
		pop_call();
		return;
	}
	send_to_char ("Huh?  Type 'help note' for usage.\n\r", ch);
	pop_call();
	return;
}

void do_immtalk (CHAR_DATA * ch, char *argument)
{
	PLAYER_GAME *fpl;
	char buf[MAX_STRING_LENGTH];
	char text2[MAX_STRING_LENGTH];

	push_call("do_immtalk(%p,%p)",ch,argument);

	if (argument[0] == '\0')
	{
		send_to_char ("Immtalk what?\n\r", ch);
		pop_call();
		return;
	}

	if (is_drunk(ch))
	{
		strcpy(text2, drunkify(argument));
	}
	else
	{
		strcpy(text2, argument);
	}

	sprintf(buf, "You immtalk '%s'", text2);
	ch_printf_color(ch, "{128}%s\n\r", ansi_justify(buf, get_page_width(ch)));

	sprintf(buf, "%s immtalks '%s'", ch->name, text2);

	for (fpl = mud->f_player ; fpl ; fpl = fpl->next)
	{
		if (ch == fpl->ch || fpl->ch->level < LEVEL_HERO)
		{
			continue;
		}
		ch_printf_color(fpl->ch, "{128}%s\n\r", ansi_justify(buf, get_page_width(fpl->ch)));
	}
	pop_call();
	return;
}

void do_devtalk (CHAR_DATA * ch, char *argument)
{
	DESCRIPTOR_DATA *d;

	push_call("do_devtalk(%p,%p)",ch,argument);

	if (argument[0] == '\0')
	{
		send_to_char ("Devtalk what?\n\r", ch);
		pop_call();
		return;
	}

	for (d = mud->f_desc ; d ; d = d->next)
	{
		msdp_update_var_instant(d, "ARACHNOS_DEVEL", "%c\001%s\002%s\001%s\002%s%c",
			MSDP_TABLE_OPEN,
				"MSG_USER", ch->name,
				"MSG_BODY", argument,
			MSDP_TABLE_CLOSE);
	}
	pop_call();
	return;
}

void arachnos_devel(char *fmt, ...)
{
	PLAYER_GAME *fpl;
	char buf[MAX_STRING_LENGTH];
	va_list args;

	push_call("arachnos_devel(%p)",fmt);

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	for (fpl = mud->f_player ; fpl ; fpl = fpl->next)
	{
		if (fpl->ch->level < MAX_LEVEL)
		{
			continue;
		}
		ch_printf_color(fpl->ch, "{128}%s\n\r", ansi_justify(buf, get_page_width(fpl->ch)));
	}
	pop_call();
	return;
}

char *mudlist[50];

void do_mudlist (CHAR_DATA * ch, char *argument)
{
	int i;

	push_call("do_mudlist(%p,%p)",ch,argument);

	ch_printf_color(ch, "{128}%18.18s %14.14s %5.5s %17.17s %17.17s %4.4s\n\r", "name", "host", "port", "last update", "boot time", "#");

	for (i = 0 ; i < 50 ; i++)
	{
		if (mudlist[i])
		{
			ch_printf_color(ch, "{128}%s\n\r", mudlist[i]);
		}
	}
}

void arachnos_mudlist(char *fmt, ...)
{
	char buf[MAX_STRING_LENGTH];

	int i;
	va_list args;

	push_call("arachnos_mudlist(%p)",fmt);

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	if (mudlist[0] && !strncmp(mudlist[0], buf, 18))
	{
		RESTRING(mudlist[0], buf);
		pop_call();
		return;
	}

	if (mudlist[49])
	{
		STRFREE(mudlist[49]);
	}

	for (i = 1 ; i < 50 ; i++)
	{
		if (mudlist[i] == NULL || !strncmp(mudlist[i], buf, 18))
		{
			memmove(&mudlist[1], &mudlist[0], i * sizeof(char *));
			break;
		}
	}

	mudlist[0] = STRALLOC(buf);

	pop_call();
	return;
}

void do_plan (CHAR_DATA * ch, char *argument)
{
	PLAYER_GAME *fpl;
	char buf[MAX_STRING_LENGTH];
	char text2[MAX_STRING_LENGTH];

	push_call("do_plan(%p,%p)",ch,argument);

	if (which_god(ch) == GOD_NEUTRAL)
	{
		send_to_char ("You are not a follower of a god.\n\r", ch);
		pop_call();
		return;
	}

	if (argument[0] == '\0')
	{
		send_to_char ("Plan what?\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && IS_SET(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_SILENCED))
	{
		send_to_char("You cannot talk.\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && !IS_SET(ch->pcdata->channel, CHANNEL_PLAN))
	{
		send_to_char ("You decided not to plan.\n\r", ch);
		pop_call();
		return;
	}

	if (is_drunk(ch))
	{
		strcpy(text2, drunkify(argument));
	}
	else
	{
		strcpy(text2, argument);
	}

	sprintf(buf, "You plan '%s'", text2);
	ch_printf(ch, "%s%s\n\r", get_color_string(ch, COLOR_PLAN, VT102_BOLD), justify(buf, get_page_width(ch)));

	sprintf(buf, "%s plans '%s'", ch->name, text2);

	for (fpl = mud->f_player ; fpl ; fpl = fpl->next)
	{
		if (!IS_SET(fpl->ch->pcdata->channel, CHANNEL_PLAN))
		{
			continue;
		}

		if (ch == fpl->ch || which_god(ch) != which_god(fpl->ch))
		{
			continue;
		}
		ch_printf(fpl->ch, "%s%s\n\r", get_color_string(fpl->ch, COLOR_PLAN, VT102_BOLD), justify(buf, get_page_width(fpl->ch)));
	}
	pop_call();
	return;
}

void do_beep (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *fch;

	push_call("do_beep(%p,%p)",ch,argument);

	if (argument[0] == '\0')	/*  Beeping Room */
	{
		for (fch = ch->in_room->first_person ; fch ; fch = fch->next_in_room)
		{
			if (!IS_NPC(fch) && fch != ch)
			{
				if (blocking(fch, ch))
				{
					continue;
				}
				if (IS_SET(fch->pcdata->vt100_flags, VT100_BEEP))
				{
					ch_printf(fch, "%s beeped.\007\n\r", get_name(ch));
				}
				else
				{
					ch_printf(fch, "%s beeped.\n\r", get_name(ch));
				}
			}
		}
		wait_state(ch, PULSE_PER_SECOND * 2);
		send_to_char ("You beep the room.\n\r", ch);
		pop_call();
		return;
	}

	if ((fch = get_player_world(ch, argument)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		pop_call();
		return;
	}

	if (blocking(fch, ch))
	{
		ch_printf(ch, "%s refuses to hear you.\n\r", capitalize(get_name(fch)));
		pop_call();
		return;
	}
	if (IS_SET(fch->pcdata->vt100_flags, VT100_BEEP))
	{
		ch_printf(fch, "%s beeped.\007\n\r", get_name(ch));
	}
	else
	{
		ch_printf(fch, "%s beeped.\n\r", get_name(ch));
	}
	wait_state(ch, PULSE_PER_SECOND * 2);
	send_to_char ("You beep.\n\r", ch);
	pop_call();
	return;
}

void do_chat (CHAR_DATA * ch, char *argument)
{
	PLAYER_GAME *fpl;
	char jbuf[MAX_STRING_LENGTH];
	char text2[MAX_STRING_LENGTH];

	push_call("do_chat(%p,%p)",ch,argument);

	if (argument[0] == '\0')
	{
		send_to_char ("Chat what?\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && !IS_SET(ch->pcdata->channel, CHANNEL_CHAT))
	{
		send_to_char ("You decided not to chat.\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && IS_SET(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_SILENCED))
	{
		send_to_char ("You cannot talk.\n\r", ch);
		pop_call();
		return;
	}

	if (is_drunk(ch))
	{
		strcpy(text2, drunkify(argument));
	}
	else
	{
		strcpy(text2, argument);
	}
	sprintf (jbuf, "You chat '%s'", text2);
	ch_printf (ch, "%s%s\n\r", get_color_string(ch, COLOR_CHAT, VT102_BOLD), justify(jbuf, get_page_width(ch)));

	sprintf(jbuf, "%s chats '%s'", capitalize(get_name(ch)), text2);

	for (fpl = mud->f_player ; fpl ; fpl = fpl->next)
	{
		if (ch == fpl->ch || !IS_SET(fpl->ch->pcdata->channel, CHANNEL_CHAT) || blocking(fpl->ch, ch))
		{
			continue;
		}
		ch_printf(fpl->ch, "%s%s\n\r", get_color_string(fpl->ch, COLOR_CHAT, VT102_BOLD), justify(jbuf, get_page_width(fpl->ch)));
	}
	pop_call();
	return;
}

void do_fos (CHAR_DATA * ch, char *argument)
{
	PLAYER_GAME *fpl;
	char jbuf[MAX_STRING_LENGTH];
	char text2[MAX_STRING_LENGTH];

	push_call("do_fos(%p,%p)",ch,argument);

	if (!IS_NPC(ch) && !IS_SET(ch->pcdata->channel, CHANNEL_FOS))
	{
		send_to_char("You decided not to fos.\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && IS_SET(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_SILENCED))
	{
		send_to_char ("You cannot talk.\n\r", ch);
		pop_call();
		return;
	}

	if (argument[0] == '\0')
	{
		send_to_char ("Fos what?\n\r", ch);
		pop_call();
		return;
	}

	if (is_drunk(ch))
	{
		strcpy(text2, drunkify(argument));
	}
	else
	{
		strcpy(text2, argument);
	}

	sprintf (jbuf, "You babble '%s'", text2);
	ch_printf (ch, "%s%s\n\r", get_color_string(ch, COLOR_PLAN, VT102_BOLD), justify(jbuf, get_page_width(ch)));

	sprintf(jbuf, "%s babbles '%s'", capitalize(get_name(ch)), text2);

	for (fpl = mud->f_player ; fpl ; fpl = fpl->next)
	{
		if (ch == fpl->ch || !IS_SET(fpl->ch->pcdata->channel, CHANNEL_FOS) || blocking(fpl->ch, ch))
		{
			continue;
		}
		ch_printf(fpl->ch, "%s%s\n\r", get_color_string(fpl->ch, COLOR_PLAN, VT102_BOLD), justify(jbuf, get_page_width(fpl->ch)));
	}
	pop_call();
	return;
}

void do_battle (const char *fmt, ...)
{
	PLAYER_GAME *fpl;
	char buf1[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH], buf3[MAX_STRING_LENGTH];
	va_list args;

	push_call("do_battle(%p)",fmt);

	va_start(args, fmt);
	vsprintf(buf1, fmt, args);
	va_end(args);

	for (fpl = mud->f_player ; fpl ; fpl = fpl->next)
	{
		if (IS_SET(fpl->ch->pcdata->channel, CHANNEL_BATTLE))
		{
			if (IS_GOD(fpl->ch))
			{
				snprintf(buf2, 20, "%s", get_time_string(mud->current_time));
				sprintf(buf3, "(%s) %s", &buf2[11], buf1);

				ch_printf(fpl->ch, "%s%s\n\r", get_color_string(fpl->ch, COLOR_YOU_ARE_HIT, VT102_BOLD), justify(buf3, get_page_width(fpl->ch)));
			}
			else
			{
				ch_printf(fpl->ch, "%s%s\n\r", get_color_string(fpl->ch, COLOR_YOU_ARE_HIT, VT102_BOLD), justify(buf1, get_page_width(fpl->ch)));
			}
		}
	}
	pop_call();
	return;
}

void do_channel_talk (CHAR_DATA * ch, char *argument)
{
	PLAYER_GAME *fpl;
	char jbuf[MAX_STRING_LENGTH];
	char text2[MAX_STRING_LENGTH];

	push_call("do_channel_talk(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (argument[0] == '\0')
	{
		send_to_char ("Talk what?\n\r", ch);
		pop_call();
		return;
	}

	if (ch->pcdata->clan == NULL)
	{
		send_to_char ("You are not a member of a clan.\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && IS_SET(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_SILENCED))
	{
		send_to_char ("You cannot talk.\n\r", ch);
		pop_call();
		return;
	}

	if (is_drunk(ch))
	{
		strcpy(text2, drunkify(argument));
	}
	else
	{
		strcpy(text2, argument);
	}

	sprintf (jbuf, "You talk '%s'", text2);
	ch_printf(ch, "%s%s\n\r", get_color_string(ch, COLOR_TALK, VT102_BOLD), justify(jbuf, get_page_width(ch)));

	sprintf(jbuf, "%s talks '%s'", get_name (ch), text2);

	for (fpl = mud->f_player ; fpl ; fpl = fpl->next)
	{
		if (ch == fpl->ch || ch->pcdata->clan != fpl->ch->pcdata->clan)
		{
			continue;
		}
		ch_printf(fpl->ch, "%s%s\n\r", get_color_string(fpl->ch, COLOR_TALK, VT102_BOLD), justify(jbuf, get_page_width(fpl->ch)));
	}
	pop_call();
	return;
}

void do_shout (CHAR_DATA * ch, char *argument)
{
	DESCRIPTOR_DATA *d;

	char buf[MAX_STRING_LENGTH];
	char text2[MAX_STRING_LENGTH];

	push_call("do_shout(%p,%p)",ch,argument);

	if (ch->in_room->area->nplayer == 0)
	{
		pop_call();
		return;
	}

	if (argument[0] == '\0')
	{
		send_to_char ("Shout what?\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && IS_SET(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_SILENCED))
	{
		send_to_char ("You cannot shout.\n\r", ch);
		pop_call();
		return;
	}

	if (IS_NPC(ch) && !IS_SET(ch->act, ACT_SMART))
	{
		send_to_char ("You're too dumb to talk.\n\r", ch);
		pop_call();
		return;
	}

	if (is_drunk(ch))
	{
		strcpy(text2, drunkify(argument));
	}
	else
	{
		strcpy(text2, argument);
	}

	sprintf(buf, "You shout '%s'", text2);
	ch_printf(ch, "%s%s\n\r", get_color_string(ch, COLOR_SPEACH, VT102_BOLD), justify(buf, get_page_width(ch)));

	sprintf(buf, "%s shouts '%s'", capitalize(get_name(ch)), text2);

	for (d = mud->f_desc ; d ; d = d->next)
	{
		if (!d->character || d->connected < 0)
		{
			continue;
		}
		if (ch == d->character || d->character->in_room->area != ch->in_room->area || d->character->position <= POS_SLEEPING)
		{
			continue;
		}
		if (blocking(d->character, ch))
		{
			continue;
		}
		if (!can_understand(d->character, ch, FALSE))
		{
			continue;
		}
		ch_printf(d->character, "%s%s\n\r", get_color_string(d->character, COLOR_SPEACH, VT102_BOLD), justify(buf, get_page_width(d->character)));
	}
	wait_state(ch, PULSE_VIOLENCE);

	pop_call();
	return;
}

void do_buffer (CHAR_DATA * ch, char *argument)
{
	push_call("do_buffer(%p,%p)",ch,argument);

	pop_call();
	return;
}

/*
	1) Find a block of text, this will be done by looking for \r+color or \n
	2) Search the block of text for the search key.
	3) Store the block of text
	4) Return at end of search or 10 blocks of text are found
	5) Buffer is searched downwards from scroll_end till scroll_end + 1
	   unless scroll_start == 0 (means buffer hasn't been filled yet)
	-) Rewritten by Scandum 24-05-2002
*/

void do_grep (CHAR_DATA * ch, char *argument)
{
	int cnt, buf_beg, buf_end, line, fnd, fnd_max, lines[20];
	char buf[MAX_STRING_LENGTH];
	char tmp[MAX_STRING_LENGTH];

	push_call("do_grep(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (argument[0] == '\0')
	{
		send_to_char("Grep what?\n\r", ch);
		pop_call();
		return;
	}

	memset(lines, 0, 20 * sizeof(int));

	fnd_max = URANGE(10, get_pager_breakpt(ch) / 2, 20);

	buf_beg = ch->pcdata->scroll_beg ? ch->pcdata->scroll_end + 1 : 0;
	buf_end = ch->pcdata->scroll_end;

	fnd = 0;

	for (line = buf_beg ; line != buf_end ; line++)
	{
		if (line >= MAX_SCROLL_BUF)
		{
			line = 0;
		}

		if (ch->pcdata->scroll_buf[line] == NULL)
		{
			log_printf("do_grep: line %d is NULL", line);
			break;
		}

		if (strstr(ch->pcdata->scroll_buf[line], argument))
		{
			lines[fnd++] = line;

			if (fnd >= fnd_max)
			{
				break;
			}
		}
	}

	sprintf (tmp, " Search for the word: %s ", argument);

	if (strlen(tmp) % 2 == 1)
	{
		strcat(tmp, " ");
	}

	strcpy(buf, get_color_string(ch, COLOR_PROMPT, VT102_BOLD));

	for (cnt = 0 ; cnt < 40 - strlen(tmp) / 2 ; cnt++)
	{
		strcat (buf, "*");
	}
	strcat (buf, tmp);
	for (cnt = 0 ; cnt < 40 - strlen(tmp) / 2 ; cnt++)
	{
		strcat (buf, "*");
	}
	cat_sprintf(buf, "%s\n\r", get_color_string(ch, COLOR_TEXT, VT102_DIM));

	if (fnd == 0)
	{
		strcat(buf, ansi_translate_text(ch, "{128}Search string not found.\n\r"));
	}
	else
	{
		for (cnt = 0 ; cnt < fnd ; cnt++)
		{
			cat_sprintf(buf, "%s\n\r", ch->pcdata->scroll_buf[lines[cnt]]);
		}
	}

	strcat(buf, get_color_string(ch, COLOR_PROMPT, VT102_BOLD));
	strcat(buf, "********************************************************************************\n\r");

	SET_BIT(ch->pcdata->interp, INTERP_SCROLL);
	send_to_char(buf, ch);
	REMOVE_BIT(ch->pcdata->interp, INTERP_SCROLL);

	pop_call();
	return;
}

struct say_type
{
	char         *name;
	char         *you;
	char         *all;
	char         *app;
	char         *how;
};

struct say_type saysocial_table[] =
{
	{
		":)",
		"smile and ",
		"smiles and ",
		"",
		""
	},

	{
		":(",
		"frown and ",
		"frowns and ",
		"",
		""
	},

	{
		";)",
		"wink and ",
		"winks and ",
		"",
		""
	},

	{
		":P",
		"grin and ",
		"grins and ",
		"",
		""
	},

	{
		":D",
		"laugh and ",
		"laughs and ",
		"",
		""
	},

	{
		":>",
		"giggle and ",
		"giggles and ",
		"",
		""
	},

	{
		":<",
		"sigh and ",
		"sighs and ",
		"",
		""
	},


	{
		":o",
		"gasp and ",
		"gasps and ",
		"",
		""
	},

	{
		":]",
		"smirk and ",
		"smirks and ",
		"",
		""
	},

	{
		":[",
		"scowl and ",
		"scowls and ",
		"",
		""
	},

	{
		":}",
		"chuckle and ",
		"chuckles and ",
		"",
		""
	},

	{
		":{",
		"pout and ",
		"pouts and ",
		"",
		""
	},

	{
		"+",
		"nod your head and ",
		"nods [his] head and ",
		"",
		""
	},

	{
		"-",
		"shake your head and ",
		"shakes [his] head and ",
		"",
		""
	},

	{
		"%",
		"roll your eyes and ",
		"rolls [his] eyes and ",
		"",
		""
	},

	{
		"^^",
		"shrug your shoulders and ",
		"shrugs [his] shoulders and ",
		"",
		""
	},

	{
		"^",
		"raise your eyebrow and ",
		"raises [his] eyebrow and ",
		"",
		""
	},

	{
		"",
		"",
		"",
		"",
		""
	}
};

struct say_type saytone_table[] =
{
	{
		"!?",
		"ask",
		"asks",
		" in astonishment",
		" "
	},

	{
		"?!",
		"ask",
		"asks",
		" in bewilderment",
		" "
	},

	{
		"?",
		"ask",
		"asks",
		"",
		" "
	},

	{
		"!!!",
		"scream",
		"screams",
		"",
		" at "
	},

	{
		"!!",
		"yell",
		"yells",
		"",
		" at "
	},

	{
		"!",
		"exclaim",
		"exclaims",
		"",
		" to "
	},

	{
		"....",
		"whisper",
		"whispers",
		"",
		" to "
	},

	{
		"...",
		"mumble",
		"mumbles",
		"",
		" to "
	},

	{
		"..",
		"mutter",
		"mutters",
		"",
		" to "
	},

	{
		"",
		"say",
		"says",
		"",
		" to "
	}
};

void do_say (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *fch, *tch;
	char buf[MAX_STRING_LENGTH], text[MAX_STRING_LENGTH], arg[MAX_INPUT_LENGTH];
	int social, tone;

	push_call("do_say(%p,%p)",ch,argument);

	if (argument[0] == '\0')
	{
		send_to_char ("Say what?\n\r", ch);

		pop_call();
		return;
	}

	if (IS_NPC(ch) && !IS_SET(ch->act, ACT_SMART))
	{
		if (MP_VALID_MOB(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "mob is trying to talk, but doesn't have ACT_SMART set");
		}
		else
		{
			send_to_char ("You're too dumb to talk.\n\r", ch);
		}
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && IS_SET(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_SILENCED))
	{
		send_to_char ("You cannot talk.\n\r", ch);

		pop_call();
		return;
	}

	if (!str_prefix("to ", argument))
	{
		char *backup = argument;

		argument = one_argument(argument, arg);
		argument = one_argument(argument, arg);

		tch = get_char_room(ch, arg);

		if (tch == NULL)
		{
			argument = backup;
		}
	}
	else
	{
		tch = NULL;
	}

	strcpy(text, argument);

	if (is_drunk(ch))
	{
		strcpy(text, drunkify(text));
	}

	for (social = 0 ; *saysocial_table[social].name != 0 ; social++)
	{
		if (!str_suffix(saysocial_table[social].name, text))
		{
			text[strlen(text) - strlen(saysocial_table[social].name)] = 0;

			if (!str_suffix(" ", text))
			{
				text[strlen(text) - 1] = 0;
			}
			break;
		}
	}

	for (tone = 0 ; *saytone_table[tone].name != 0 ; tone++)
	{
		if (!str_suffix(saytone_table[tone].name, text))
		{
			break;
		}
	}

	if (!strcmp(text, ""))
	{
		strcpy(text, "...");
	}

	sprintf(buf, "You %s%s%s%s%s '%s'\n\r", saysocial_table[social].you, saytone_table[tone].you, tch ? saytone_table[tone].how : "", tch ? get_name(tch) : "", saytone_table[tone].app, text);

	ch_printf(ch, "%s%s", get_color_string(ch, COLOR_SPEACH, VT102_DIM), justify(buf, get_page_width(ch)));

	for (fch = ch->in_room->first_person ; fch ; fch = fch->next_in_room)
	{
		if (ch == fch || fch->position <= POS_SLEEPING)
		{
			continue;
		}

		if (blocking(fch, ch))
		{
			continue;
		}

		if (!can_understand(fch, ch, FALSE))
		{
			if (!IS_NPC(fch) && can_see(ch, fch))
			{
				can_understand(fch, ch, TRUE);
			}
			sprintf(buf, "%s %s%s%s%s%s '%s'\n\r", capitalize(get_name(ch)), get_dynamic_player_description(ch, fch, saysocial_table[social].all), saytone_table[tone].all, tch ? saytone_table[tone].how : "", tch ? (tch == fch ? "you" : get_name(tch)) : "", saytone_table[tone].app, translate(ch, text));
		}
		else
		{
			sprintf(buf, "%s %s%s%s%s%s '%s'\n\r", capitalize(get_name(ch)), get_dynamic_player_description(ch, fch, saysocial_table[social].all), saytone_table[tone].all, tch ? saytone_table[tone].how : "", tch ? (tch == fch ? "you" : get_name(tch)) : "", saytone_table[tone].app, text);
		}
		ch_printf(fch, "%s%s", get_color_string(fch, COLOR_SPEACH, VT102_DIM), justify(buf, get_page_width(fch)));
	}

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	mprog_speech_trigger(argument, ch);
	pop_call();
	return;
}

void do_sing (CHAR_DATA * ch, char *argument)
{
	CHAR_DATA *fch;
	char buf[MAX_STRING_LENGTH];
	char text2[MAX_STRING_LENGTH];

	push_call("do_sing(%p,%p)",ch,argument);

	if (ch == NULL)
	{
		pop_call();
		return;
	}

	if (argument[0] == '\0')
	{
		send_to_char ("Sing what?\n\r", ch);
		pop_call();
		return;
	}

	if (IS_NPC(ch) && !IS_SET(ch->act, ACT_SMART))
	{
		send_to_char ("You're too dumb to sing.\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && IS_SET(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_SILENCED))
	{
		send_to_char ("You cannot sing.\n\r", ch);
		pop_call();
		return;
	}

	if (is_drunk(ch))
	{
		strcpy(text2, drunkify(argument));
	}
	else
	{
		strcpy(text2, argument);
	}

	sprintf(buf, "You sing '%s'", text2);

	ch_printf(ch, "%s%s\n\r", get_color_string(ch, COLOR_SPEACH, VT102_DIM), justify(buf, get_page_width(ch)));

	for (fch = ch->in_room->first_person ; fch ; fch = fch->next_in_room)
	{
		if (ch == fch || fch->position <= POS_SLEEPING)
		{
			continue;
		}

		if (blocking (fch, ch))
		{
			continue;
		}

		if (!can_understand(fch, ch, FALSE))
		{
			if (!IS_NPC(fch) && can_see(ch, fch))
			{
				can_understand(fch, ch, TRUE);
			}
			sprintf(buf, "%s sings '%s'", capitalize(get_name(ch)), translate(ch, text2));
			ch_printf(fch, "%s%s\n\r",  get_color_string(fch, COLOR_SPEACH, VT102_DIM), justify(buf, get_page_width(fch)));
			continue;
		}
		if (strstr(text2, "!"))
		{
			sprintf(buf, "%s screams '%s'", capitalize(get_name(ch)), text2);
		}
		else
		{
			sprintf(buf, "%s sings '%s'", capitalize(get_name(ch)), text2);
		}
		ch_printf(fch, "%s%s\n\r", get_color_string(fch, COLOR_SPEACH, VT102_DIM), justify(buf, get_page_width(fch)));
	}

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	mprog_speech_trigger (argument, ch);
	pop_call();
	return;
}

void do_dump (CHAR_DATA * ch, char *argument)
{
	push_call("do_dump(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && IS_SET(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_SILENCED))
	{
		send_to_char ("You cannot talk.\n\r", ch);
		pop_call();
		return;
	}

	if (!strcasecmp(argument, "buffer"))
	{
		FILE *dumpFile;
		int buf_beg, buf_end, line;
		char dump_dir[200];

		if (ch->level < 10)
		{
			ch_printf(ch, "You must be at least level 10 to dump your buffer.\n\r");
			pop_call();
			return;
		}
		ch_printf(ch, "Saving buffer..\n\r");

		buf_beg = ch->pcdata->scroll_beg ? ch->pcdata->scroll_end + 1 : 0;
		buf_end = ch->pcdata->scroll_end;

		sprintf(dump_dir, "%s/%c/%c/dmp/%s.dmp", PLAYER_DIR, tolower(ch->name[0]), tolower(ch->name[1]), ch->name);

		close_reserve();

		if ((dumpFile = my_fopen(dump_dir, "wt", FALSE)) != NULL)
		{
			for (line = buf_beg ; line != buf_end ; line++)
			{
				if (line > MAX_SCROLL_BUF)
				{
					line = 0;
				}
				fprintf(dumpFile, "%s", fixer(ch->pcdata->scroll_buf[line]));
			}
			my_fclose(dumpFile);

			log_printf("%s has dumped their buffer", get_name(ch));

			wait_state(ch, PULSE_VIOLENCE);
		}

		open_reserve();

		pop_call();
		return;
	}

	send_to_char ("Waiting for Dump.  Issue 'stop' to cancel.\n\r", ch);

	SET_BIT(ch->pcdata->interp, INTERP_DUMP);

	pop_call();
	return;
}


void do_tell (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char jbuf[MAX_STRING_LENGTH];
	char text2[MAX_STRING_LENGTH];
	CHAR_DATA *victim;

	push_call("do_tell(%p,%p)",ch,argument);

	if (!IS_NPC(ch) && IS_SET(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_SILENCED))
	{
		send_to_char ("Your message didn't get through.\n\r", ch);
		pop_call();
		return;
	}

	if (IS_NPC(ch) && !IS_SET(ch->act, ACT_SMART))
	{
		if (MP_VALID_MOB(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "mob is trying to talk, but doesn't have ACT_SMART set");
		}
		else
		{
			send_to_char ("You're too dumb to talk.\n\r", ch);
		}
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && IS_SET(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_SILENCED))
	{
		send_to_char ("You cannot talk.\n\r", ch);
		pop_call();
		return;
	}

	argument = one_argument(argument, arg);

	if (arg[0] == '\0' || argument[0] == '\0')
	{
		send_to_char ("Tell whom what?\n\r", ch);
		pop_call();
		return;
	}

	if ((victim = get_player_world(ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		pop_call();
		return;
	}

	if (victim->desc == NULL)
	{
		ch_printf(ch, "%s is link-dead.\n\r", get_name(victim));
		pop_call();
		return;
	}

	if (blocking(victim, ch) || IS_SET(pvnum_index[victim->pcdata->pvnum]->flags, PVNUM_MUTED))
	{
		ch_printf(ch, "%s refuses to hear you.\n\r", get_name(victim));
		pop_call();
		return;
	}

	if (IS_SET(victim->act, PLR_AFK))
	{
		ch_printf (ch, "%s is afk and may not see your message.\n\r", get_name(victim));
	}

	if (victim->desc && victim->desc->connected == CON_EDITING && get_trust (ch) < LEVEL_IMMORTAL)
	{
		ch_printf(ch, "%s is currently in a writing buffer.  Please try again in a few minutes.\n\r", get_name(victim));
		pop_call();
		return;
	}

	if (is_drunk(ch))
	{
		strcpy(text2, drunkify(argument));
	}
	else
	{
		strcpy(text2, argument);
	}

	if (!can_understand(victim, ch, TRUE))
	{
		sprintf(jbuf, "%s tells you '%s'", capitalize(get_name(ch)), translate(ch, text2));
		ch_printf(victim, "%s%s\n\r", get_color_string(victim, COLOR_SPEACH, VT102_BOLD), justify(jbuf, get_page_width(victim)));

		pop_call();
		return;
	}
	sprintf(jbuf, "You tell %s '%s'", get_name(victim), text2);
	ch_printf(ch, "%s%s\n\r", get_color_string(ch, COLOR_SPEACH, VT102_BOLD), justify(jbuf, get_page_width(ch)));

	sprintf(jbuf, "%s tells you '%s'", capitalize(get_name(ch)), text2);
	ch_printf(victim, "%s%s\n\r", get_color_string(victim, COLOR_SPEACH, VT102_BOLD), justify(jbuf, get_page_width(victim)));

	if (!IS_NPC(ch) && !IS_NPC(victim))
	{
		victim->pcdata->reply = ch;
	}
	pop_call();
	return;
}

void do_reply (CHAR_DATA * ch, char *argument)
{
	char jbuf[MAX_STRING_LENGTH];
	char text2[MAX_STRING_LENGTH];
	CHAR_DATA *victim;

	push_call("do_reply(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		send_to_char("You cannot reply.\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && IS_SET(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_SILENCED))
	{
		send_to_char ("Your message didn't get through.\n\r", ch);
		pop_call();
		return;
	}

	if ((victim = ch->pcdata->reply) == NULL || IS_NPC(ch->pcdata->reply))
	{
		send_to_char ("They aren't here.\n\r", ch);
		pop_call();
		return;
	}

	if (argument[0] == '\0')
	{
		send_to_char ("Reply what?\n\r", ch);
		pop_call();
		return;
	}

	if (IS_SET(victim->act, PLR_AFK))
	{
		ch_printf(ch, "%s is afk and may not see your message.\n\r", get_name (victim));
	}

	if (victim->desc == NULL)
	{
		ch_printf (ch, "%s is link-dead.\n\r", get_name(victim));
		pop_call();
		return;
	}

	if (blocking(victim, ch))
	{
		ch_printf(ch, "%s refuses to hear you.\n\r", capitalize(get_name(victim)));
		pop_call();
		return;
	}

	if (is_drunk(ch))
	{
		strcpy(text2, drunkify(argument));
	}
	else
	{
		strcpy(text2, argument);
	}

	if (!can_understand(victim, ch, TRUE))
	{
		sprintf(jbuf, "%s replies to you '%s'", get_name(ch), translate(ch, text2));
		ch_printf(victim, "%s%s\n\r", get_color_string(victim, COLOR_SPEACH, VT102_BOLD), justify(jbuf, get_page_width(victim)));
		pop_call();
		return;
	}

	if (is_drunk(ch))
	{
		strcpy(text2, drunkify(argument));
	}
	else
	{
		strcpy(text2, argument);
	}

	sprintf(jbuf, "You reply to %s '%s'", get_name(victim), text2);
	ch_printf(ch, "%s%s\n\r", get_color_string(ch, COLOR_SPEACH, VT102_BOLD), justify(jbuf, get_page_width(ch)));

	sprintf(jbuf, "%s replies to you '%s'", capitalize(get_name(ch)), text2);
	ch_printf (victim, "%s%s\n\r", get_color_string(victim, COLOR_SPEACH, VT102_BOLD), justify(jbuf, get_page_width(victim)));

	victim->pcdata->reply = ch;

	pop_call();
	return;
}

void do_emote (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *fch;

	push_call("do_emote(%p,%p)",ch,argument);

	if (!IS_NPC(ch) && IS_SET(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_SILENCED))
	{
		send_to_char ("You cannot talk.\n\r", ch);
		pop_call();
		return;
	}

	if (argument[0] == '\0')
	{
		send_to_char ("Emote what?\n\r", ch);
		pop_call();
		return;
	}

	sprintf(buf, "%s %s\n\r", get_name(ch), argument);

	for (fch = ch->in_room->first_person ; fch ; fch = fch->next_in_room)
	{
		if (IS_AWAKE(fch) && !blocking(fch, ch))
		{
			ch_printf(fch, "%s", capitalize(justify(buf, get_page_width(fch))));
		}
	}
	pop_call();
	return;
}

/*
	All the posing stuff.
*/

struct pose_table_type
{
	char *message[2 * MAX_CLASS];
};

/*
	Pose Table, updated by Scandum
*/

const struct pose_table_type pose_table[] =
{
	{
		{
			"You show your bulging muscles.",
			"$n shows $s bulging muscles.",
			"The townfolk cheer as you emerge victorious",
			"The townfolk cheer as $n emerges victorious",
			"You perform a small card trick.",
			"$n performs a small card trick.",
			"The shadows slide apart and you emerge.",
			"The shadows emerge and $n emerges.",
			"An angel consults you.",
			"An angel consults $n.",
			"You sizzle with energy.",
			"$n sizzles with energy.",
			"You feel very holy.",
			"$n looks very holy.",
			"Two little demons appear to polish your boots.",
			"Two little demons appear to polish $n's boots."
		}
	},

	{
		{
			"You crack nuts between your fingers.",
			"$n cracks nuts between $s fingers.",
			"The emperor's thumb goes up for you.",
			"The emperor's thumb goes up for $n.",
			"You wiggle your ears alternately.",
			"$n wiggles $s ears alternately.",
			"You carefully place a stopper on a bottle of lethal poison.",
			"$n looks at you as $e carefully stoppers a dirty looking bottle.",
			"You nonchalantly turn wine into water.",
			"$n nonchalantly turns wine into water.",
			"You turn into a butterfly, then return to your normal shape.",
			"$n turns into a butterfly, then returns to $s normal shape.",
			"Your bones and muscles make cracking noises as you clench your fists.",
			"You hear cracking noises as $n clenches $s fists.",
			"Blood spurts from the ground forming a pentagram around you.",
			"Blood spurts from the ground forming a pentagram around $n."
		}
	},

	{
		{
			"You grizzle your teeth and look mean.",
			"$n grizzles $s teeth and looks mean.",
			"You size up those around you and edge quietly through the room.",
			"Flexing $s weapon arm $n sizes you up with an icing glare.",
			"You nimbly tie yourself into a knot.",
			"$n nimbly ties $mself into a knot.",
			"You vanish and reappear without anyone's notice.",
			"*poof* $n just vanished and reappeared without your notice!",
			"Tongues of flame dance and twirl as you lose yourself in a trance.",
			"Tongues of flame dance and twirl as $n loses $mself in a trance.",
			"Blue sparks fly from your fingers.",
			"Blue sparks fly from $n's fingers.",
			"A halo appears over your head.",
			"A halo appears over $n's head.",
			"With a snap of your fingers the undead dance to your arcane harmony.",
			"The undead dance to $n's arcane harmony as $e snaps $s finger."
		}
	},

	{
		{
			"You hit your head, and your eyes roll.",
			"$n hits $s head, and $s eyes roll.",
			"You twirl your swords around in a remarkable display of swordsmanship.",
			"You watch $n twirl $s swords around in a remarkable display of swordsmanship.",
			"You juggle with daggers, apples, and eyeballs.",
			"$n juggles with daggers, apples, and eyeballs.",
			"You dazzle your audience executing triple back flip kicks.",
			"You get all dizzy watching $n executing triple back flip kicks.",
			"The heavens boom in thunderous clasps of lightning, just as rose petals fall gently around you.",
			"The heavens boom in thunderous clasps of lightning, just as rose petals fall gently around $n.",
			"Little green lights dance in your eyes.",
			"Little green lights dance in $n's eyes.",
			"You recite words of wisdom.",
			"$n recites words of wisdom.",
			"The mist gathers about you with screams of the undead.",
			"The mist gathers about $n with screams of the undead."
		}
	},

	{
		{
			"Crunch, crunch -- you munch a bottle.",
			"Crunch, crunch -- $n munches a bottle.",
			"You lift the dead lion over your head, proclaiming victory.",
			"$n lifts the dead lion over $s head, proclaiming victory.",
			"You steal the underwear off every person in the room.",
			"Your underwear is gone!  $n stole it!",
			"Wow!  You just caught a fly with chopsticks!",
			"Wow!  $n just caught a fly with chopsticks!",
			"Deep in prayer, you levitate.",
			"Deep in prayer, $n levitates.",
			"A slimy green monster appears before you and bows.",
			"A slimy green monster appears before $n and bows.",
			"*CRUSH* You just broke a big rock on your forehead!",
			"*CRUSH* $n just broke a big rock on $s forehead!",
			"You summon the grim reaper to stand by your side.",
			"$n summons the grim reaper to stand by $s side."
		}
	},

	{
		{
			"Four matched Percherons bring in your chariot.",
			"Four matched Percherons bring in $n's chariot.",
			"The crowd cheers in a standing ovation as you draw your blades.",
			"The crowd cheers in a standing ovation as $n draws $s blades.",
			"The dice roll ... and you win again.",
			"The dice roll ... and $n wins again.",
			"You shoot an arrow with great skill.",
			"An arrow from $n's bow just impaled the apple you were about to eat.",
			"An angel consults you.",
			"An angel consults $n.",
			"You turn everybody into a little pink elephant.",
			"You are turned into a little pink elephant by $n.",
			"A great light from the heavens enshroud you as you pray.",
			"A great light from the heavens enshrouds $n as $e prays.",
			"Dark clouds loom overhead as you chant a magic incarnation.",
			"Dark clouds loom overhead as $n chants a magic incarnation."
		}
	},

	{
		{
			"Arnold Schwarzenegger admires your physique.",
			"Arnold Schwarzenegger admires $n's physique.",
			"Blood splatters around as you wipe your weapon clean.",
			"Blood splatters around as $n wipes $s weapon clean.",
			"You count the money in everyone's pockets.",
			"Check your money, $n is counting it.",
			"You offer poisoned beer to everyone in the room.",
			"$n offers beer to everyone present.  It smells a little funny..",
			"Your body glows with an unearthly light.",
			"$n's body glows with an unearthly light.",
			"A small ball of light dances on your fingertips.",
			"A small ball of light dances on $n's fingertips.",
			"Your prayers have converted Lord Demise from his evil ways.",
			"The prayers of $n have converted Lord Demise from his evil ways.",
			"A skeletal hand bursts out of the ground to massage your back.",
			"A skeletal hand bursts out of the ground to massage $n's back."
		}
	},

	{
		{
			"Mercenaries arrive to do your bidding.",
			"Mercenaries arrive to do $n's bidding.",
			"With a sweep of your weapon, you knock down everyone in the room.",
			"$n sweeps with $s weapon, and you find yourself on the ground.",
			"You balance a pocket knife on your tongue.",
			"$n balances a pocket knife on $s tongue.",
			"You string up the hangman with $s own noose.",
			"$n strings up the hangman with $s own noose.",
			"Smoke and fumes leak from your nostrils.",
			"Smoke and fumes leak from $n's nostrils.",
			"A spot light hits you.",
			"A spot light hits $n.",
			"Your mighty upper cut sends your opponent flying.",
			"$n's mighty upper cut sends $s opponent flying.",
			"A strange glow emits from you as you start to chant.",
			"A strange glow emits from $n as $e starts to chant."
		}
	},

	{
		{
			"A fiery stallion kneels for you to mount.",
			"A fiery stallion kneels so that $n may mount.",
			"Swing!  You just decapitated your own shadow.",
			"Swing!  $n just decapitated $s own shadow.",
			"You produce a coin from everyone's ear.",
			"$n produces a coin from your ear.",
			"You step behind your own shadow.",
			"$n steps behind $s own shadow.",
			"Everyone levitates as you pray.",
			"You levitate as $n prays.",
			"The light flickers as you rap in magical languages.",
			"The light flickers as $n raps in magical languages.",
			"Oomph!  You squeeze water out of a granite boulder.",
			"Oomph!  $n squeezes water out of a granite boulder.",
			"Time collapses as you blink.",
			"Time collapses as $n blinks."
		}
	},

	{
		{
			"Elven Warhorses answer your call to battle.",
			"Elven Warhorses arrive as $n screams an ancient battlecry.",
			"You pick your teeth with a spear.",
			"$n picks $s teeth with a spear.",
			"You step behind your shadow.",
			"$n steps behind $s shadow.",
			"You appear out of nowhere.",
			"$n appears mysteriously from the gleam of your eye.",
			"A cool breeze refreshes you.",
			"A cool breeze refreshes $n.",
			"Your head disappears.",
			"$n's head disappears.",
			"The oracle asks you for your advice.",
			"The oracle asks for $n's advice.",
			"A demon host bows in submission to your might.",
			"A demon host bows in submission to $n's might."
		}
	},

	{
		{
			"You crack open a goblin's skull and drink the brains.",
			"$n cracks open a goblin's skull and drinks the brains.",
			"Your deathly stare sends shivers into your opponents hearts.",
			"$n's deathly stare sends shivers into your heart.",
			"Your eyes dance with greed.",
			"$n's eyes dance with greed.",
			"You flick your wrist and a throwing star appears from nowhere.",
			"With a flick of $s wrist a throwing star appears in $n's hand.",
			"A fire elemental singes your hair.",
			"A fire elemental singes $n's hair.",
			"The sun pierces through the clouds to illuminate you.",
			"The sun pierces through the clouds to illuminate $n.",
			"Everyone is swept off their feet by your hug.",
			"You are swept off your feet by $n's hug.",
			"The whispers of death descends on all as you emerge from the mists.",
			"The whispers of death descends on your soul as $n emerges from the mists."
		}
	},

	{
		{
			"Everyone moves out of your way as you grumble.",
			"You quickly move out of $n's way as $e grumbles.",
			"As you draw your blades, your enemies scatter before you.",
			"$n's enemies scatter as $e draws $s blades.",
			"You deftly steal everyone's weapon.",
			"$n deftly steals your weapon.",
			"Your mercurial dance leaves only your opponent's blood stains on the ground.",
			"$n's mercurial dance leaves only $s opponent's blood stains on the ground.",
			"The ocean parts before you.",
			"The ocean parts before $n.",
			"The sky changes color to match your eyes.",
			"The sky changes color to match $n's eyes.",
			"Your haymaker splits a tree.",
			"$n's haymaker splits a tree.",
			"Poisonous vines crack in whiplike fashion as you frown.",
			"Poisonous vines crack in whiplike fashion as $n frowns."
		}
	},

	{
		{
			"The sound of your battle cry rips havoc and fear to all around you.",
			"The sound of $n's battle cry rips havoc and fear all around $m.",
			"Your whirling blades massacre the titan.",
			"$n's whirling blades massacre the titan.",
			"The Grey Mouser buys you a beer.",
			"The Grey Mouser buys $n a beer.",
			"The shadows flicker as you descend and embed your knife into your victim.",
			"The shadows flicker as $n descends and embeds $s knife into $s victim.",
			"A thunder cloud kneels to you.",
			"A thunder cloud kneels to $n.",
			"The stones dance to your command.",
			"The stones dance to $n's command.",
			"A strap of your armor breaks over your mighty thews.",
			"A strap of $n's armor breaks over $s mighty thews.",
			"You order some eye of newt from Doogan's shop.",
			"$n orders some eye of newt from Doogan's shop."
		}
	},

	{
		{
			"You smile as you lick your opponent's blood from your blade.",
			"$n smiles as $e licks $s opponent's blood from $s blade.",
			"Valkyries sing praises to the legends of your victory",
			"Valkyries sing praises to the legends of $n's victories",
			"Everyone's pocket explodes with your fireworks.",
			"Your pocket explodes with $n's fireworks.",
			"Everyone discovers your dagger an inch from their eyes.",
			"You discover $n's dagger an inch from your eye.",
			"Meteors swoop down and circle you.",
			"Meteors sweep down and circle $n.",
			"The heavens and grass change color as you smile.",
			"The heavens and grass change color as $n smiles.",
			"A boulder cracks at your frown.",
			"A boulder cracks at $n's frown.",
			"You peer into everyone's soul, draining their life away.",
			"You feel your life drain away as $n's peers into your soul."
		}
	},

	{
		{
			"Your furious rage devours all that stand before you.",
			"$n's furious rage devours all that stand before $m.",
			"You blow the horn of Valhalla before entering the battle fields.",
			"You hear the horn of Valhalla as $n enters the battle fields.",
			"Everyone discovers your dagger a centimeter from their eye.",
			"You discover $n's dagger a centimeter from your eye.",
			"You emerge from a dark alley.",
			"$n emerges from the dark alley of your mind.",
			"You utter a few words and the earth glistens with new green life.",
			"$n utters a few words and the earth glistens with new green life.",
			"Everyone's clothes are transparent, and you are laughing.",
			"Your clothes are transparent, and $n is laughing.",
			"Watch your feet, you are juggling granite boulders.",
			"Watch your feet, $n is juggling granite boulders.",
			"The angel of death asks you for some pointers.",
			"The angel of death asks $n for some pointers."
		}
	},

	{
		{
			"The smell of rage leaks from your body as you go berserk.",
			"The smell of rage leaks from $n's body as $e goes berserk.",
			"You finish your job as the emperor's thumb goes down.",
			"The emperor gives the thumbs down, and $n finishes the job.",
			"Where did you go?",
			"Where did $n go?",
			"Millions cringe at the thought of your wrath.",
			"Millions cringe at the thought of $n's wrath.",
			"The great god Mota gives you a staff.",
			"The great god Mota gives $n a staff.",
			"A black hole swallows you.",
			"A black hole swallows $n.",
			"... 98, 99, 100 ... you do pushups.",
			"... 98, 99, 100 ... $n does pushups.",
			"An ill wind blows as you summon a demon steed to serve you.",
			"An ill wind blows as a demon steed approaches $n and offers its service."
		}
	},

	{
		{
			"Silence suddenly fills the area as you draw your sword and shield.",
			"Silence suddenly fills the area as $n draws $s sword and shield.",
			"Your slashing stance rips shreds in the wind.",
			"$n's slashing stance rips shreds in the wind.",
			"Click.",
			"Click.",
			"You post a 'Murder for Hire' sign.",
			"$n puts out $s 'Murder for Hire' sign.",
			"You spit on the ground and flowers sprout forth.",
			"$n spits on the ground and flowers sprout forth.",
			"The world shimmers in time with your whistling.",
			"The world shimmers in time with $n's whistling.",
			"A multitude of angels sing your praises for all the world to hear.",
			"A multitude of angels sing $n's praises for all the world to hear.",
			"Your evil aura drives stakes into the hearts of the holy.",
			"$n's evil aura drives stakes into the hearts of the holy."
		}
	}
};

void do_pose (CHAR_DATA * ch, char *argument)
{
	int pose;

	push_call("do_pose(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	pose = number_range(0, sizeof(pose_table) / sizeof(pose_table[0]) - 1);

	act (pose_table[pose].message[2 * ch->class + 0], ch, NULL, NULL, TO_CHAR);
	act (pose_table[pose].message[2 * ch->class + 1], ch, NULL, NULL, TO_ROOM);

	pop_call();
	return;
}


void do_qui (CHAR_DATA * ch, char *argument)
{
	push_call("do_qui(%p,%p)",ch,argument);

	send_to_char ("If you want to QUIT, you have to spell it out.\n\r", ch);
	pop_call();
	return;
}


void do_quit (CHAR_DATA * ch, char *argument)
{
	DESCRIPTOR_DATA *d;
	PLAYER_GAME *gpl;

	push_call("do_quit(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		raw_kill(ch);
		pop_call();
		return;
	}

	if (argument && ch->position == POS_FIGHTING)
	{
		send_to_char ("No way! You are fighting.\n\r", ch);
		pop_call();
		return;
	}

	if (argument && ch->position < POS_STUNNED)
	{
		send_to_char ("You're not DEAD yet.\n\r", ch);
		pop_call();
		return;
	}

	if (argument && IS_SET(ch->in_room->room_flags, ROOM_NO_SAVE))
	{
		send_to_char("You can't save here, so you had better not quit.\n\r", ch);
		pop_call();
		return;
	}

	if (argument && ch->pcdata->last_combat + 20 > mud->current_time)
	{
		ch_printf(ch, "You must wait %d seconds before you can quit.\n\r", ch->pcdata->last_combat + 20 - mud->current_time);
		pop_call();
		return;
	}

	if (argument && ch->pcdata->pnote != NULL && strcasecmp(argument, "now"))
	{
		send_to_char ("You must specify 'quit now'.\n\rYou have a note in progress.\n\r", ch);
		pop_call();
		return;
	}

	for (d = mud->f_desc ; d ; d = d->next)
	{
		if (d->original == ch)
		{
			do_return(d->character, NULL);
		}
	}

	if (ch->in_room->vnum == ROOM_VNUM_LIMBO)
	{
		char_from_room(ch);
		char_to_room(ch, ch->pcdata->was_in_room);
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_NO_SAVE))
	{
		char_from_room (ch);
		char_to_room(ch, ROOM_VNUM_TEMPLE);
	}

	if (ch->master != NULL)
	{
		stop_follower(ch);
	}

	if (ch->pcdata->editor)
	{
		stop_editing(ch);
	}

	save_char_obj(ch, NORMAL_SAVE);

	for (gpl = mud->f_player ; gpl ; gpl = gpl->next)
	{
		if (gpl->ch->pcdata->reply == ch)
		{
			gpl->ch->pcdata->reply = NULL;
		}
	}

	stop_fighting(ch, TRUE);

	show_who_can_see(ch, " has left the game.\n\r");

	log_god_printf("%s has quit!", get_name (ch));

	/*
		After extract_char the ch is no longer valid!
	*/

	if (ch->desc && ch->desc->snoop_by != NULL)
	{
		write_to_buffer(ch->desc->snoop_by, "Your victim has quit.\n\r", 0);
		ch->desc->snoop_by = NULL;
	}

	vt100off(ch);

	d = NULL;

	if (ch->desc != NULL)
	{
		d = ch->desc;
		send_goodbye(d);
		write_to_port(d);
		*d->inbuf = '\0';
	}

	extract_char(ch);

	if (d != NULL)
	{
		d->character = NULL;

		close_socket(d);
	}
	pop_call();
	return;
}

void send_goodbye(DESCRIPTOR_DATA *d)
{
	CHAR_DATA *ch = CH(d);
	char buf1[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];

	push_call("send_goodbye(%p)",d);

	strcpy(buf1, god_table[which_god(ch)].logout_msg);
	strcpy(buf2, god_table[which_god(ch)].color);

	cat_sprintf(buf2, "\n\r%s\n\r", center(buf1, get_page_width(ch)));
	cat_sprintf(buf2, "\n\r%s\n\r", center("Thank you for playing", get_page_width(ch)));
	cat_sprintf(buf2, "\n\r%s\n\r", center("Lowlands", get_page_width(ch)));
	cat_sprintf(buf2, "\n\r%s\n\r", "{088}");

	write_to_descriptor(d, ansi_translate(buf2), 0);

	pop_call();
	return;
}

void do_relog (CHAR_DATA * ch, char *argument)
{
	DESCRIPTOR_DATA *d;
	PLAYER_GAME *gpl;
	CHAR_DATA *fch;

	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char name[MAX_INPUT_LENGTH];

	bool loaded = FALSE;

	push_call("do_relog(%p,%p)",ch,argument);

	if (IS_NPC(ch) || ch->desc == NULL)
	{
		pop_call();
		return;
	}

	if (ch->position == POS_FIGHTING)
	{
		send_to_char ("No way! You are fighting.\n\r", ch);
		pop_call();
		return;
	}

	if (ch->position < POS_STUNNED)
	{
		send_to_char ("You're not DEAD yet.\n\r", ch);
		pop_call();
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_NO_SAVE))
	{
		send_to_char("You can't save here, so you had better not quit.\n\r", ch);
		pop_call();
		return;
	}

	if (ch->pcdata->last_combat + 20 > mud->current_time)
	{
		ch_printf(ch, "You must wait %d seconds before you can quit.\n\r", ch->pcdata->last_combat + 20 - mud->current_time);
		pop_call();
		return;
	}

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	strcpy(name, capitalize_name(arg1));

	remove_bad_desc_name(name);

	fch = lookup_char(name);

	if (fch == NULL)
	{
		fch = start_partial_load(ch, name);

		if (fch == NULL)
		{
			pop_call();
			return;
		}
		loaded = TRUE;
	}

	if (fch == ch)
	{
		send_to_char("You are already yourself.\n\r", ch);
		pop_call();
		return;
	}

	if (ch->pcdata->password != fch->pcdata->password && encrypt64(arg2) != fch->pcdata->password)
	{
		send_to_char("Wrong password, please wait 5 seconds.\n\r", ch);
		wait_state(ch, PULSE_PER_SECOND * 5);

		if (loaded)
		{
			clear_partial_load(fch);
		}
		pop_call();
		return;
	}

	if (ch->in_room->vnum == ROOM_VNUM_LIMBO)
	{
		char_from_room(ch);
		char_to_room(ch, ch->pcdata->was_in_room);
	}

	if (ch->master != NULL)
	{
		stop_follower(ch);
	}

	if (ch->pcdata->editor)
	{
		stop_editing(ch);
	}

	save_char_obj(ch, NORMAL_SAVE);

	for (gpl = mud->f_player ; gpl ; gpl = gpl->next)
	{
		if (gpl->ch->pcdata->reply == ch)
		{
			gpl->ch->pcdata->reply = NULL;
		}
	}

	stop_fighting(ch, TRUE);

	show_who_can_see(ch, " has left the game.\n\r");

	d = ch->desc;

	if (loaded)
	{
		clear_partial_load(fch);

		load_char_obj(d, name);

		fch = d->character;

		add_char(fch);
		add_player(fch);

		char_to_room(fch, fch->pcdata->was_in_room);
	}
	else
	{
		if (fch->desc)
		{
			close_socket(fch->desc);
		}
		d->character = fch;
		fch->desc    = d;

		act("$n has relogged as $N.", ch, NULL, fch, TO_ROOM);
	}

	char_to_room(ch, ROOM_VNUM_LIMBO);

	fch->pcdata->screensize_v = ch->pcdata->screensize_v;
	fch->pcdata->screensize_h = ch->pcdata->screensize_h;

	do_refresh(fch, "");

	if (loaded)
	{
		enter_game(fch);
	}
	else
	{
		do_look(fch, "auto");
	}
	act("You relogged as $n.", fch, NULL, NULL, TO_CHAR);

	log_god_printf("%s has relogged as %s!", ch->name, fch->name);

	ch->desc = NULL;

	extract_char(ch);
	pop_call();
	return;
}


void do_save (CHAR_DATA * ch, char *argument)
{
	PLAYER_GAME *gch;
	CHAR_DATA   *tch;

	push_call("do_save(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (ch->level < 99)
	{
		send_to_char("Your character is saved every 10 minutes, and at important events.\n\r", ch);
		pop_call();
		return;
	}

	if (!strcasecmp(argument, "all"))
	{
		send_to_char("Saving all players.\n\r", ch);
		for (gch = mud->f_player ; gch ; gch = gch->next)
		{
			save_char_obj(gch->ch, NORMAL_SAVE);
		}
		pop_call();
		return;
	}

	if (argument[0] == '\0')
	{
		send_to_char("Syntax: save <all|player>\n\r", ch);
		pop_call();
		return;
	}
	else if ((tch = get_player_world(ch, argument)) == NULL)
	{
		do_save(ch, "");
		pop_call();
		return;
	}

	ch_printf(ch, "Saving %s.\n\r", get_name(tch));

	save_char_obj(tch, NORMAL_SAVE);

	pop_call();
	return;
}

void do_follow (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	push_call("do_follow(%p,%p)",ch,argument);

	one_argument (argument, arg);

	if (arg[0] == '\0')
	{
		send_to_char ("Follow whom?\n\r", ch);
		pop_call();
		return;
	}

	if (ch->fighting)
	{
		send_to_char("No way!  You are still fighting!\n",ch);
		pop_call();
		return;
	}

	if ((victim = get_char_room (ch, arg)) == NULL)
	{
		send_to_char ("They aren't here.\n\r", ch);
		pop_call();
		return;
	}

	if (victim == ch)
	{
		if (ch->master == NULL)
		{
			send_to_char ("You already follow yourself.\n\r", ch);
			pop_call();
			return;
		}
		stop_follower(ch);
		pop_call();
		return;
	}

	if (!IS_NPC(ch))
	{
		if ((ch->level < (victim->level * 3 / 4) - 2 || ch->level > (victim->level * 5 / 4) + 2) && ch->level < MAX_LEVEL)
		{
			send_to_char ("You are not of the right caliber to follow.\n\r", ch);
			pop_call();
			return;
		}
		if (IS_AFFECTED (victim, AFF_CHARM) && IS_NPC (victim))
		{
			send_to_char ("You may not follow a pet.\n\r", ch);
			pop_call();
			return;
		}
	}

	/* poll the block list of the victim */

	if (blocking(victim, ch))
	{
		act ("$E eludes you at every step.", ch, 0, victim, TO_CHAR);
		pop_call();
		return;
	}

	if (ch->master != NULL)
	{
		stop_follower (ch);
	}
	add_follower (ch, victim);
	pop_call();
	return;
}

void add_follower (CHAR_DATA * ch, CHAR_DATA * master)
{
	PET_DATA *pet;

	push_call("add_follower(%p,%p)",ch,master);

	if (ch->master != NULL)
	{
		bug ("Add_follower: non-null master.", 0);
		pop_call();
		return;
	}

	ch->master = master;
	ch->leader = NULL;
	if (can_see(master, ch))
	{
		act ("$n now follows you.", ch, NULL, master, TO_VICT);
	}
	act ("You now follow $N.", ch, NULL, master, TO_CHAR);

	ALLOCMEM(pet, PET_DATA, 1);
	pet->ch = ch;
	LINK(pet, mud->f_pet, mud->l_pet, next, prev);

	pop_call();
	return;
}


void stop_follower (CHAR_DATA * ch)
{
	PET_DATA *pet;

	push_call("stop_follower(%p)",ch);

	if (ch == NULL)
	{
		bug( "stop_follower : ch == NULL.",0);
		pop_call();
		return;
	}

	if (ch->master == NULL)
	{
		bug( "Stop_follower: null master.", 0 );
		pop_call();
		return;
	}

	if (can_see(ch->master, ch))
	{
		act ("$n stops following you.", ch, NULL, ch->master, TO_VICT);
	}
	act ("You stop following $N.", ch, NULL, ch->master, TO_CHAR);

	if (ch->master->mounting == ch)
	{
		ch->master->mounting = NULL;
	}
	ch->master = NULL;
	ch->leader = NULL;

	for (pet = mud->f_pet ; pet ; pet = pet->next)
	{
		if (pet->ch == ch)
		{
			UNLINK(pet, mud->f_pet, mud->l_pet, next, prev);
			FREEMEM(pet);
			break;
		}
	}

	if (IS_AFFECTED(ch, AFF_CHARM))
	{
		raw_kill(ch);
	}

	if (IS_AFFECTED(ch, AFF2_POSSESS))
	{
		raw_kill(ch);
	}
	pop_call();
	return;
}


void die_follower (CHAR_DATA * ch)
{
	PET_DATA *pch, *pch_next;

	push_call("die_follower(%p)",ch);

	ch->position = POS_DEAD;

	if (ch->master != NULL)
	{
		stop_follower(ch);
	}
	ch->leader = NULL;

	for (pch = mud->f_pet ; pch ; pch = pch_next)
	{
		pch_next = pch->next;

		if (pch->ch->master == ch)
		{
			stop_follower(pch->ch);
		}
	}
	pop_call();
	return;
}


void do_shadow (CHAR_DATA * ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int door;
	ROOM_INDEX_DATA *old_room;

	push_call("do_shadow(%p,%p)",ch,argument);

	if (IS_NPC(ch) || learned(ch, gsn_shadow) == 0)
	{
		send_to_char("You failed.\n\r", ch);
		pop_call();
		return;
	}

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if ((door = direction_door(arg1)) == -1)
	{
		if ((door = direction_door(arg2)) == -1)
		{
			send_to_char ("What direction?\n\r", ch);
			pop_call();
			return;
		}
		else
		{
			strcpy(arg2, arg1);
		}
	}

	if (arg2[0] == '\0')
	{
		send_to_char ("Shadow whom?\n\r", ch);
		pop_call();
		return;
	}

	if (!is_valid_exit(ch, ch->in_room, door))
	{
		send_to_char ("There is no exit there.\n\r", ch);
		pop_call();
		return;
	}

	old_room = ch->in_room;

	ch->in_room = room_index[get_exit(ch->in_room->vnum, door)->to_room];

	victim = get_char_room(ch, arg2);

	ch->in_room = old_room;

	if (victim == NULL || victim->in_room == ch->in_room)
	{
		send_to_char ("They aren't there.\n\r", ch);
		pop_call();
		return;
	}

	if (number_percent() >= learned(ch, gsn_shadow))
	{
		act ("You fail to shadow $m.", ch, NULL, victim, TO_CHAR);
		pop_call();
		return;
	}

	if (IS_NPC(victim) || victim->pcdata->shadowed_by || victim->pcdata->shadowing)
	{
		act ("You can't seem to shadow $m.", ch, NULL, victim, TO_CHAR);
		pop_call();
		return;
	}

	if (ch->master)
	{
		stop_follower (ch);
	}
	if (ch->pcdata->shadowing)
	{
		stop_shadow(ch);
	}
	add_shadow(ch, victim);
	pop_call();
	return;
}

void add_shadow (CHAR_DATA * ch, CHAR_DATA * master)
{
	push_call("add_shadow(%p,%p)",ch,master);

	if (ch->pcdata->shadowing)
	{
		bug("add_shadow : already shadowing.", 0);
		pop_call();
		return;
	}
	ch->pcdata->shadowing       = master;
	master->pcdata->shadowed_by = ch;
	act ("You are now shadowing $N.", ch, NULL, master, TO_CHAR);
	pop_call();
	return;
}


void stop_shadow (CHAR_DATA * ch)
{
	push_call("stop_shadow(%p)",ch);

	if (IS_NPC(ch) || ch->pcdata->shadowing == NULL)
	{
		pop_call();
		return;
	}
	act ("You stop shadowing $N.", ch, NULL, ch->pcdata->shadowing, TO_CHAR);

	ch->pcdata->shadowing->pcdata->shadowed_by = NULL;
	ch->pcdata->shadowing = NULL;

	pop_call();
	return;
}


void do_order (CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *och;
	CHAR_DATA *och_next;
	bool found;
	bool fAll;

	push_call("do_order(%p)",ch,argument);

	argument = one_argument (argument, arg);
	if (arg[0] == '\0' || argument[0] == '\0')
	{
		send_to_char ("Order whom to do what?\n\r", ch);
		pop_call();
		return;
	}

	if (IS_AFFECTED (ch, AFF_CHARM))
	{
		send_to_char ("You feel like taking, not giving, orders.\n\r", ch);
		pop_call();
		return;
	}

	if (!strcasecmp (arg, "all"))
	{
		fAll = TRUE;
		victim = NULL;
	}
	else
	{
		fAll = FALSE;
		if ((victim = get_char_room (ch, arg)) == NULL)
		{
			send_to_char ("They aren't here.\n\r", ch);
			pop_call();
			return;
		}

		if (victim == ch)
		{
			send_to_char ("Aye aye, right away!\n\r", ch);
			pop_call();
			return;
		}

		if (!IS_AFFECTED (victim, AFF_CHARM) || victim->master != ch)
		{
			send_to_char ("Do it yourself!\n\r", ch);
			pop_call();
			return;
		}
	}

	found = FALSE;
	for (och = ch->in_room->first_person; och != NULL; och = och_next)
	{
		och_next = och->next_in_room;
		if (IS_AFFECTED (och, AFF_CHARM) && och->master == ch && (fAll || och == victim))
		{
			found = TRUE;
			act ("$n orders you to '$t'.", ch, argument, och, TO_VICT);
			interpret (och, argument);
		}
	}

	if (found)
	{
		send_to_char ("Ok.\n\r", ch);
	}
	else
	{
		send_to_char ("You have no followers here.\n\r", ch);
	}
	pop_call();
	return;
}


void do_group (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	char dim[10], bld[10];
	int t_hp, t_current_hp, t_mana, t_current_mana, t_level, t_move, t_current_move;
	CHAR_DATA *victim, *leader;
	FIGHT_DATA *fight;
	PET_DATA	 *pch;

	push_call("do_group(%p,%p)",ch,argument);

	one_argument (argument, arg);

	t_hp = t_current_hp = t_current_mana = t_current_move = t_mana = t_level = t_move = 0;

	strcpy(dim, get_color_string(ch, COLOR_PROMPT, VT102_DIM));
	strcpy(bld, get_color_string(ch, COLOR_PROMPT, VT102_BOLD));

	leader = (ch->leader != NULL) ? ch->leader : ch;

	if (arg[0] == '\0')
	{
		ch_printf_color(ch, "{078}%s's group:\n\r", get_name(leader));

		ch_printf_color(ch, "{038}[%3d %s]{178} %-12s {128}%5d{028}/{128}%5d{028} Hp {128}%5d{028}/{128}%5d{028} Ma {128}%5d{028}/{128}%5d{028} Mv {128}%9d{028} Xp\n\r",
			leader->level,
			IS_NPC (leader) ? "Mob" : class_table[leader->class].who_name,
			str_resize(IS_NPC(leader) ? capitalize(leader->short_descr) : leader->name, buf, -12),
			leader->hit,  leader->max_hit,
			leader->mana, leader->max_mana,
			leader->move, leader->max_move,
			exp_level(leader->class, leader->level) - (IS_NPC(leader) ? 0 : leader->pcdata->exp));

			t_hp           += leader->max_hit;
			t_current_hp   += leader->hit;
			t_mana         += leader->max_mana;
			t_current_mana += leader->mana;
			t_move         += leader->max_move;
			t_current_move += leader->move;
			t_level        += leader->level;

		for (pch = mud->f_pet ; pch ; pch = pch->next)
		{
			if (pch->ch != leader && is_same_group(pch->ch, ch))
			{
				ch_printf_color(ch, "{038}[%3d %s]{178} %-12s {128}%5d{028}/{128}%5d{028} Hp {128}%5d{028}/{128}%5d{028} Ma {128}%5d{028}/{128}%5d{028} Mv {128}%9d{028} Xp\n\r",
					pch->ch->level,
					IS_NPC (pch->ch) ? "Mob" : class_table[pch->ch->class].who_name,
					str_resize(IS_NPC(pch->ch) ? capitalize(pch->ch->short_descr) : pch->ch->name, buf, -12),
					pch->ch->hit,  pch->ch->max_hit,
					pch->ch->mana, pch->ch->max_mana,
					pch->ch->move, pch->ch->max_move,
					exp_level(pch->ch->class, pch->ch->level) - (IS_NPC(pch->ch) ? 0 : pch->ch->pcdata->exp));

				t_hp           += pch->ch->max_hit;
				t_current_hp   += pch->ch->hit;
				t_mana         += pch->ch->max_mana;
				t_current_mana += pch->ch->mana;
				t_move         += pch->ch->max_move;
				t_current_move += pch->ch->move;
				t_level        += pch->ch->level;
			}
		}
		ch_printf_color(ch, "{038}[%3d    ]{178} %-12s {128}%5d{028}/{128}%5d{028} Hp {128}%5d{028}/{128}%5d{028} Ma {128}%5d{028}/{128}%5d{028} Mv\n\r",
			t_level,
			"Totals",
			t_current_hp,   t_hp,
			t_current_mana, t_mana,
			t_current_move, t_move);

		pop_call();
		return;
	}

	for (fight = mud->f_fight ; fight ; fight = fight->next)
	{
		if (is_same_group(ch, fight->ch))
		{
			send_to_char ("You cannot modify the group while a member is fighting.\n\r", ch);
			pop_call();
			return;
		}
	}

	if (!strcmp (arg, "disband"))
	{
		int count = 0;

		if (ch->master)
		{
			send_to_char ("You cannot disband a group if you're following someone.\n\r ", ch);
			pop_call();
			return;
		}

		for (pch = mud->f_pet ; pch ; pch = pch->next)
		{
			if (ch != pch->ch && is_same_group(ch, pch->ch))
			{
				pch->ch->leader = NULL;
				count++;
				send_to_char("Your group is disbanded.\n\r", pch->ch);
			}
		}

		if (count == 0)
		{
			send_to_char ("You have no group members to disband.\n\r", ch);
		}
		else
		{
			send_to_char ("You disband your group.\n\r", ch);
		}
		pop_call();
		return;
	}

	if (!strcmp (arg, "all"))
	{
		CHAR_DATA *rch;
		int count = 0;

		for (rch = ch->in_room->first_person ; rch ; rch = rch->next_in_room)
		{
			if (ch != rch && can_see(ch, rch) && rch->master == ch && !rch->fighting && rch->position != POS_FIGHTING && !ch->master && !ch->leader && !is_same_group(rch, ch))
			{
				rch->leader = ch;
				count++;
			}
		}

		if (count == 0)
		{
			send_to_char ("You have no eligible group members.\n\r", ch);
		}
		else
		{
			act ("$n groups $s followers.", ch, NULL, NULL, TO_ROOM);
			send_to_char ("You group your followers.\n\r", ch);
		}
		pop_call();
		return;
	}

	if (ch->master)
	{
		send_to_char ("But you are following someone else!\n\r", ch);
		pop_call();
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		if ((victim = get_player_world(ch, arg)) == NULL)
		{
			send_to_char ("Hmmm...where did they go?\n\r", ch);
			pop_call();
			return;
		}
	}

	if (victim->master != ch)
	{
		act ("$N isn't following you.", ch, NULL, victim, TO_CHAR);
		pop_call();
		return;
 	}

	if (is_same_group (victim, ch))
	{
		victim->leader = NULL;

		act ("$n removes $N from $s group.", ch, NULL, victim, TO_NOTVICT);
		act ("$n removes you from $s group.", ch, NULL, victim, TO_VICT);
		act ("You remove $N from your group.", ch, NULL, victim, TO_CHAR);

		pop_call();
		return;
	}

	if (victim->fighting != NULL || victim->position == POS_FIGHTING )
	{
		send_to_char ("You cannot group someone that is fighting.\n\r", ch);
		pop_call();
		return;
	}

	victim->leader = ch;

	act ("$N joins $n's group.", ch, NULL, victim, TO_NOTVICT);
	act ("You join $n's group.", ch, NULL, victim, TO_VICT);
	act ("$N joins your group.", ch, NULL, victim, TO_CHAR);

	pop_call();
	return;
}

/*
	'Split' originally by Gnort, God of Chaos.
*/

void do_split (CHAR_DATA * ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *gch;
	int members;
	int amount;
	int share;
	int extra;

	push_call("do_split(%p,%p)",ch,argument);

	one_argument (argument, arg);

	if (arg[0] == '\0')
	{
		send_to_char ("How much gold do you want to split ?\n\r", ch);
		pop_call();
		return;
	}

	amount = atol (arg);

	if (amount < 0)
	{
		send_to_char ("Your group wouldn't like that.\n\r", ch);
		pop_call();
		return;
	}

	if (amount == 0)
	{
		send_to_char ("You hand out zero coins, but no one notices.\n\r", ch);
		pop_call();
		return;
	}

	if (ch->gold < amount)
	{
		send_to_char ("You don't have that much gold.\n\r", ch);
		pop_call();
		return;
	}

	members = 0;
	for (gch = ch->in_room->first_person ; gch != NULL ; gch = gch->next_in_room)
	{
		if (is_same_group (gch, ch) && !IS_NPC (gch))
		{
			members++;
		}
	}

	if (members < 2)
	{
		send_to_char ("Just keep it all.\n\r", ch);
		pop_call();
		return;
	}

	share = amount / members;
	extra = amount % members;

	if (share == 0)
	{
		send_to_char ("Generousity is an art.\n\r", ch);
		pop_call();
		return;
	}

	ch->gold -= amount;
	ch->gold += share + extra;

	ch_printf(ch, "You split %d gold coins.  Your share is %d gold coins.\n\r", amount, share + extra);

	sprintf (buf, "$n splits %d gold coins.  Your share is %d gold coins.", amount, share);

	for (gch = ch->in_room->first_person; gch != NULL; gch = gch->next_in_room)
	{
		if (gch != ch && is_same_group (gch, ch) && !IS_NPC (gch))
		{
			act (buf, ch, NULL, gch, TO_VICT);
			gch->gold += share;
		}
	}
	pop_call();
	return;
}

void do_gtell (CHAR_DATA * ch, char *argument)
{
	char jbuf[MAX_STRING_LENGTH];
	char text2[MAX_STRING_LENGTH];
	PLAYER_GAME *gpl;

	push_call("do_gtell(%p,%p)",ch,argument);

	if (argument[0] == '\0')
	{
		send_to_char ("Tell your group what?\n\r", ch);
		pop_call();
		return;
	}

	if (!IS_NPC(ch) && IS_SET(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_SILENCED))
	{
		send_to_char ("You cannot talk.\n\r", ch);
		pop_call();
		return;
	}

	if (is_drunk(ch))
	{
		strcpy(text2, drunkify(argument));
	}
	else
	{
		strcpy(text2, argument);
	}
	sprintf(jbuf, "You tell the group '%s'", text2);
	ch_printf(ch, "%s%s\n\r", get_color_string(ch, COLOR_SPEACH, VT102_BOLD), justify(jbuf, get_page_width(ch)));

	for (gpl = mud->f_player ; gpl ; gpl = gpl->next)
	{
		if (is_same_group(gpl->ch, ch) && ch != gpl->ch && !IS_NPC(gpl->ch))
		{
			if (!can_understand(gpl->ch, ch, TRUE))
			{
				continue;
			}
			sprintf(jbuf, "%s tells the group '%s'", capitalize(get_name(ch)), text2);
			ch_printf(gpl->ch, "%s%s\n\r", get_color_string(gpl->ch, COLOR_SPEACH, VT102_BOLD), justify(jbuf, get_page_width(gpl->ch)));
		}
	}
	pop_call();
	return;
}

/*
	It is very important that this be an equivalence relation:
	(1) A ~ A
	(2) if A ~ B then B ~ A
	(3) if A ~ B  and B ~ C, then A ~ C
*/

bool is_same_group (CHAR_DATA * ach, CHAR_DATA * bch)
{
	push_call("is_same_group(%p,%p)",ach,bch);
	/*
		the idea is this :
		if my leader is your leader, then we are in the same
		group, else we're in different groups, if at all in a group
	*/

	if (ach->leader != NULL)
	{
		ach = ach->leader;
	}
	if (bch->leader != NULL)
	{
		bch = bch->leader;
	}
	pop_call();
	return ach == bch;
}

bool vnum_in_group (CHAR_DATA * ach, int mobvnum)
{
	CHAR_DATA *gch;

	push_call("vnum_in_group(%p,%p)",ach,mobvnum);

	for (gch = ach->in_room->first_person ; gch ; gch = gch->next_in_room)
	{
		if (IS_NPC(gch) && gch->pIndexData->vnum == mobvnum && is_same_group(ach, gch))
		{
			pop_call();
			return TRUE;
		}
	}
	pop_call();
	return FALSE;
}

void do_port (CHAR_DATA * ch, char *argument)
{
	int size;

	push_call("do_port(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (ch->desc->original != NULL)
	{
		pop_call();
		return;
	}

	size = atol (argument);

	if (size == 0)
	{
		ch_printf(ch, "Current port settings:\n\rBlock size: %d\n\r", ch->desc->port_size);
		pop_call();
		return;
	}

	if (size < 100 || size > 10000)
	{
		send_to_char ("The range of sizes are 100 to 10000\n\r", ch);
	}

	size = URANGE(100, size, 10000);

	ch->desc->port_size		= size;
	ch->pcdata->port_size	= size;

	ch_printf(ch, "Port size set to %d.\n\r", size);
	pop_call();
	return;
}


void do_voice (CHAR_DATA * ch, char *argument)
{
	char buf1[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char speaker[MAX_INPUT_LENGTH];
	char text2[MAX_STRING_LENGTH];
	CHAR_DATA *vch;

	push_call("do_voice(%p,%p)",ch,argument);

	if (learned(ch, gsn_voice) == 0)
	{
		send_to_char("They'd just laugh at you.\n\r", ch);
		pop_call();
		return;
	}

	for (vch = ch->in_room->first_person ; vch != NULL ; vch = vch->next_in_room)
	{
		if (!IS_NPC(vch) && vch->level == MAX_LEVEL && ch->level < MAX_LEVEL && can_see(ch, vch))
		{
			ch_printf(ch, "You may not voice with %s in the room.\n\r", capitalize(get_name(vch)));
			pop_call();
			return;
		}
	}

	argument = one_argument_nolower(argument, speaker);

	if (argument[0] == '\0')
	{
		send_to_char ("What do you want them to say?", ch);
		pop_call();
		return;
	}

	if (is_drunk(ch))
	{
		strcpy(text2, drunkify(argument));
	}
	else
	{
		strcpy(text2, argument);
	}
	sprintf (buf1, "%s says '%s'", capitalize(speaker), text2);
	sprintf (buf2, "Someone say's '%s'", text2);

	for (vch = ch->in_room->first_person ; vch ; vch = vch->next_in_room)
	{
		if (vch->position >= POS_RESTING)
		{
			if (number_percent() > learned(ch, gsn_voice))
			{
				ch_printf(vch, "%s%s\n\r", get_color_string(vch, COLOR_SPEACH, VT102_DIM), justify(buf2, get_page_width(vch)));
			}
			else
			{
				ch_printf(vch, "%s%s\n\r", get_color_string(vch, COLOR_SPEACH, VT102_DIM), justify(buf1, get_page_width(vch)));
			}
		}
	}
	check_improve(ch, gsn_voice);

	pop_call();
	return;
}


bool pvnum_in_group (CHAR_DATA * ch, int pvnum)
{
	CHAR_DATA *fch;

	push_call("pvnum_in_group(%p,%p)",ch,pvnum);

	if ((fch = get_char_pvnum(pvnum)) == NULL)
	{
		pop_call();
		return FALSE;
	}
	pop_call();
	return is_same_group(fch, ch);
}

void do_repeat (CHAR_DATA * ch, char *argument)
{
	char buf1[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
	int amount, cnt;
	char bbuf[MAX_INPUT_LENGTH], *bptr;

	push_call("do_repeat(%p,%p)",ch,argument);

	if (ch->desc == NULL)
	{
		pop_call();
		return;
	}

	argument = one_argument(argument, buf1);

	amount = atol(buf1);

	if (amount < 2 || amount > 99 || isdigit(argument[0]))
	{
		send_to_char ("Syntax: <amount> <command string>\n\r", ch);
		send_to_char ("Amount range is 2 to 99, or whatever the input line can hold.\n\r", ch);
		pop_call();
		return;
	}

	if (argument[0] == '.' || argument[0] == '!')
	{
		send_to_char("That would be a bad idea.\n\r", ch);
		pop_call();
		return;
	}

	if ((bptr = strstr(argument, "&")) != NULL)
	{
		sprintf(bbuf, "%s\n", bptr+1);
		*bptr = '\0';
	}
	else
	{
		strcpy(bbuf, ch->desc->inbuf);
	}
	sprintf(buf2, "%s\n", argument);

	*ch->desc->inbuf = 0;
	 ch->desc->intop = 0;

	for (cnt = 0 ; cnt < amount ; cnt++)
	{
		if (ch->desc->intop >= MAX_INPUT_LENGTH - 6)
		{
			break;
		}
		ch->desc->intop = str_apd_max(ch->desc->inbuf, buf2, ch->desc->intop, MAX_INPUT_LENGTH - 5);
	}
	ch->desc->intop = str_apd_max(ch->desc->inbuf, bbuf, ch->desc->intop, MAX_INPUT_LENGTH - 6);

	pop_call();
	return;
}

void do_block (CHAR_DATA * ch, char *argument)
{
	char buf1[MAX_STRING_LENGTH];

	push_call("do_block(%p,%p)",ch,argument);

	if (!is_desc_valid (ch) || IS_NPC (ch))
	{
		pop_call();
		return;
	}


	argument = one_argument(argument, buf1);

	if (buf1[0] == '\0')
	{
		ch_printf(ch, "Your block-list is:\n\r > %s\n\r", ch->pcdata->block_list);
		pop_call();
		return;
	}
	smash_tilde (buf1);

	if (!strcasecmp ("clear", buf1))
	{
		STRFREE(ch->pcdata->block_list);
		ch->pcdata->block_list = STRALLOC("");
		send_to_char("Your block-list is now empty.\n\r", ch);
		pop_call();
		return;
	}

	if (!strcasecmp ("del", buf1))
	{
		char *block_list,
			block_names	[MAX_INPUT_LENGTH],
			name_to_delete	[MAX_INPUT_LENGTH],
			cur_name		[MAX_INPUT_LENGTH],
			new_block		[MAX_INPUT_LENGTH];

		argument = one_argument(argument, name_to_delete);

		strcpy (block_names, ch->pcdata->block_list);

		block_list = &(block_names[0]);
		block_list = one_argument(block_list, cur_name);

		for (new_block[0] = '\0' ; cur_name[0] != '\0' ; block_list = one_argument(block_list, cur_name))
		{
			if (!strcasecmp (cur_name, name_to_delete))
			{
				strcat(new_block, block_list);
				STRFREE(ch->pcdata->block_list);
				ch->pcdata->block_list = STRALLOC (new_block);

				ch_printf(ch, "Deleted '%s' from your block-list.\n\r", name_to_delete);
				ch_printf(ch, "Your block-list is now:\n\r > %s\n\r", ch->pcdata->block_list);
				pop_call();
				return;
			}
			else
			{
				cat_sprintf(new_block, "%s ", cur_name);
			}
		}
		ch_printf(ch, "I was unable to find '%s' in your block-list.\n\r", name_to_delete);
		pop_call();
		return;
	}

	if (!strcasecmp ("add", buf1))
	{
		if (strlen(ch->pcdata->block_list) > 150)
		{
			send_to_char ("Your block-list is too long to add a new name.\n\r", ch);
			pop_call();
			return;
		}
		sprintf(buf1, "%s %s", ch->pcdata->block_list, argument);

		STRFREE (ch->pcdata->block_list);
		ch->pcdata->block_list = STRALLOC(buf1);

		ch_printf(ch, "Your block-list is now:\n\r > %s\n\r", ch->pcdata->block_list);
		pop_call();
		return;
	}
	send_to_char ("Incorrect blocking format.\n\rUse: block [<add/clear/del>] [namelist]\n\r", ch);
	pop_call();
	return;
}

void do_death (CHAR_DATA * ch, char *arg)
{
	push_call("do_death(%p,%p)",ch,arg);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_ALTAR))
	{
		ch->pcdata->death_room = ch->in_room->vnum;
		send_to_char ("Death room set.\n\r", ch);
		pop_call();
		return;
	}
	ch_printf(ch, "You may not set your death room here.\n\rYour death room is currently set at: %s\n\r", room_index[ch->pcdata->death_room]->name);

	pop_call();
	return;
}

/*
	Returns centered buffer - Scandum
*/

char *center (char *inp, int length)
{
	static char center_buf[MAX_STRING_LENGTH];
	int lead_cnt, cnt;

	push_call("center(%p,%p)",inp,length);

	lead_cnt = (length - strlen(inp)) / 2;

	for (center_buf[0] = cnt = 0 ; cnt < lead_cnt ; cnt++)
	{
		strcat(center_buf, " ");
	}

	strcat(center_buf, inp);

	pop_call();
	return center_buf;
}

/*
	Returns justified buffer for ansi translated text - Scandum 13-05-2002
*/

char *ansi_justify (char *inp, int length)
{
	static char justified_buf[MAX_STRING_LENGTH];
	char *pti, *pto, *last_i_space, *last_o_space;
	int cnt, skp;

	push_call("ansi_justify(%p)",inp);

	last_o_space = pto = (char *) justified_buf;
	last_i_space = pti = inp;

	length = 80;

	cnt = skp = 0;

	while (TRUE)
	{
		*pto = *pti;
		switch (*pto)
		{
			case '\0':
				pop_call();
				return ((char *) justified_buf);
				break;
			case ' ':
				last_o_space = pto;
				last_i_space = pti;
				break;
			case '\n':
			case '\r':
				cnt = 0;
				skp = 0;
				pto++;
				pti++;
				continue;
				break;
			case '{':
				if (*(pti+1) >= '0'	&& *(pti+1) <= '8'
				&&  *(pti+2) >= '0' && *(pti+2) <= '8'
				&&  *(pti+3) >= '0' && *(pti+3) <= '8'
				&&  *(pti+4) == '}')
				{
					pto++; pti++; *pto = *pti;
					pto++; pti++; *pto = *pti;
					pto++; pti++; *pto = *pti;
					pto++; pti++; *pto = *pti;
					pto++;
					pti++;
					skp += 5;
					continue;
				}
				break;
		}
		pto++;
		pti++;
		cnt++;
		if (cnt >= length)
		{
			if (*pti == '\n' || *pti == '\0')
			{
				continue;
			}
			if (*pti == ' ')
			{
				last_o_space = pto;
				last_i_space = pti;
			}
			if (pto - last_o_space > 20 + skp)
			{
				*pto = '\n';
				pto++;
				*pto = '\r';
				pto++;
				cnt  = 0;
				skp  = 0;
			}
			else
			{
				pto  = last_o_space;
				*pto = '\n';
				pto++;
				*pto = '\r';
				pto++;
				pti  = last_i_space;
				pti++;
				cnt  = 0;
				skp  = 0;
			}
		}
	}
	pop_call();
	return ((char *)justified_buf);
}

/*
	Switched to flawless protocol - Scandum 13-05-2002
*/

char *justify (char *inp, int length)
{
	static char justified_buf[MAX_STRING_LENGTH];
	char *pti, *pto, *last_i_space, *last_o_space;
	int cnt = 0;

	push_call("justify(%p)",inp);

	last_o_space = pto = justified_buf;
	last_i_space = pti = inp;

	length = 80;

	while (TRUE)
	{
		*pto = *pti;

		switch (*pto)
		{
			case '\0':
				pop_call();
				return justified_buf;
				break;
			case ' ':
				last_o_space = pto;
				last_i_space = pti;
				break;
			case '\n':
			case '\r':
				cnt = 0;
				pto++;
				pti++;
				continue;
		}
		pto++;
		pti++;
		cnt++;

		if (cnt >= length)
		{
			if (*pti == '\n' || *pti == '\0')
			{
				continue;
			}
			if (*pti == ' ')
			{
				last_o_space = pto;
				last_i_space = pti;
			}
			if (pto - last_o_space > 20)
			{
				*pto++ = '\n';
				*pto++ = '\r';
			}
			else
			{
				 pto   = last_o_space;
				*pto++ = '\n';
				*pto++ = '\r';
				 pti   = last_i_space;
				 pti++;
			}
			cnt = 0;
		}
	}
	pop_call();
	return justified_buf;
}


void do_reincarnate (CHAR_DATA * ch, char *arg)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	sh_int future_reincarnate, future_class, future_race, future_sex, reinc;
	int cnt, sn, future_hours;
	bool was_95;

	push_call("do_reincarnate(%p,%p)",ch,arg);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (ch->level == 95 && ch->pcdata->reincarnation == 10)
	{
		send_to_char ("You have reached the pinnacle of mortality. Rejoice and be at peace.\n\r", ch);
		pop_call();
		return;
	}

	if (arg[0] == '\0')
	{
		if ((ch->pcdata->reincarnation != 0 && ch->level == 2) || ch->level == 95)
		{
			send_to_char ("Syntax: reincarnate <class> <race> <sex> <password>\n\r", ch);
		}
		else
		{
			send_to_char ("You may not reincarnate yet.\n\r",ch);
		}
		pop_call();
		return;
	}

	if ((ch->pcdata->reincarnation == 0 || ch->level != 2) && ch->level != 95)
	{
		send_to_char ("You may only reincarnate yourself at 95th level or as a new reincarnate.\n\r", ch);
		pop_call();
		return;
	}

	if (ch->level == 2)
	{
		was_95 = FALSE;
	}
	else
	{
		was_95 = TRUE;
	}

	arg = one_argument (arg, arg1);
	arg = one_argument (arg, arg2);
	arg = one_argument (arg, arg3);

	if (was_95)
	{
		reinc = ch->pcdata->reincarnation+1;
	}
	else
	{
		reinc = ch->pcdata->reincarnation;
	}

	future_class = -1;
	future_race  = -1;
	future_sex   = -1;

	for (cnt = 0 ; cnt < MAX_CLASS ; cnt++)
	{
		if (is_name_short(arg1, (char *) class_table[cnt].name))
		{
			future_class = cnt;
			break;
		}
	}
	for (cnt = 0 ; cnt < MAX_RACE ; cnt++)
	{
		if (is_name_short(arg2, (char *) race_table[cnt].race_name))
		{
			if (reinc >= race_table[cnt].enter && reinc <= race_table[cnt].leave)
			{
				future_race = cnt;
				break;
			}
			else
			{
				ch_printf(ch, "%ss are for other reincarnations.\n\r", race_table[cnt].race_name);
				pop_call();
				return;
			}
		}
	}
	if (is_name_short(arg3, "neutral"))
	{
		future_sex = 0;
	}
	else if (is_name_short(arg3, "male"))
	{
		future_sex = 1;
	}
	else if (is_name_short(arg3, "female"))
	{
		future_sex = 2;
	}
	if (future_class == -1)
	{
		ch_printf(ch, "%s is not a valid class.\n\r", capitalize(arg1));
		pop_call();
		return;
	}
	if (future_race == -1)
	{
		ch_printf(ch, "%s is not a valid class.\n\r", capitalize(arg2));
		pop_call();
		return;
	}
	if (future_sex == -1)
	{
		ch_printf(ch, "%s is not a valid gender.\n\r", capitalize(arg3));
	}
	if (race_table[future_race].race_class[future_class] == 0)
	{
		send_to_char ("That combination of class and race is not allowed.\n\r", ch);
		pop_call();
		return;
	}
	if (encrypt64(arg) != ch->pcdata->password)
	{
		send_to_char ("You must include your password to reincarnate.\n\r", ch);
		pop_call();
		return;
	}

	if (was_95)
	{
		future_reincarnate = ch->pcdata->reincarnation + 1;
		future_hours = ch->pcdata->played / 3600;
		ch->pcdata->played = 0;
	}
	else
	{
		future_reincarnate = ch->pcdata->reincarnation;
		future_hours = 0;
	}

	/*
		Reset all the character stats here
	*/

	die_follower(ch);
	stop_fighting(ch, TRUE);

	char_from_room(ch);
	char_to_room(ch, ROOM_VNUM_SCHOOL);

	while (ch->first_carrying)
	{
		junk_obj(ch->first_carrying);
	}

	while (ch->first_affect)
	{
		affect_from_char(ch, ch->first_affect);
	}
	ch->affected_by  = 0;
	ch->affected2_by = 0;

	REMOVE_BIT(ch->act, PLR_KILLER);
	REMOVE_BIT(ch->act, PLR_THIEF);

	ch->level                          = 2;
	ch->trust                          = ch->level;
	ch->class                          = future_class;
	ch->pcdata->honorific              = future_class * 8;
	ch->race                           = future_race;
	ch->sex                            = future_sex;
	ch->pcdata->speak                  = 0;
	ch->pcdata->language               = 0;
	ch->leader                         = NULL;

	ch->hit                            = future_reincarnate * 15 + 30;
	ch->max_hit                        = future_reincarnate * 15 + 30;
	ch->pcdata->actual_max_hit         = future_reincarnate * 15 + 30;
	ch->mana                           = future_reincarnate * 15 + 100;
	ch->max_mana                       = future_reincarnate * 15 + 100;
	ch->pcdata->actual_max_mana        = future_reincarnate * 15 + 100;
	ch->move                           = future_reincarnate * 15 + 100;
	ch->max_move                       = future_reincarnate * 15 + 100;
	ch->pcdata->actual_max_move        = future_reincarnate * 15 + 100;

	ch->carry_weight                   = 0;
	ch->carry_number                   = 0;
	ch->saving_throw                   = 0;
	ch->gold                           = 0;
	ch->alignment                      = 0;
	ch->speed                          = 1;
	ch->pcdata->exp                    = exp_level (future_class, 1) + 1;
	ch->pcdata->recall                 = ROOM_VNUM_TEMPLE;
	ch->pcdata->death_room             = ROOM_VNUM_TEMPLE;
	ch->pcdata->practice               = 10 + 3 * future_reincarnate;
	ch->pcdata->wimpy                  = 10;
	ch->pcdata->condition[COND_THIRST] = 48;
	ch->pcdata->condition[COND_FULL]   = 48;
	ch->pcdata->condition[COND_DRUNK]  = 0;
	ch->pcdata->god                    = GOD_NEUTRAL;
	ch->pcdata->eqsaves                = 0;
	ch->pcdata->eqhitroll              = 0;
	ch->pcdata->eqdamroll              = 0;
	ch->pcdata->played                 = 0;

	STRFREE(ch->long_descr);
	ch->long_descr                     = STRALLOC("");
	STRFREE(ch->pcdata->auto_command);
	ch->pcdata->auto_command           = STRALLOC("");
	ch->pcdata->auto_flags             = AUTO_OFF;

	for (cnt = 0 ; cnt < MAX_CLASS; cnt++)
	{
		ch->pcdata->mclass[cnt] = (future_class != cnt) ? 0 : 2;
	}

	for (cnt = 0 ; cnt < MAX_AREA ; cnt++)
	{
		if (ch->pcdata->quest[cnt] != NULL)
		{
			FREEMEM(ch->pcdata->quest[cnt]);
			ch->pcdata->quest[cnt]	= NULL;
		}
	}

	for (sn = 0 ; sn < MAX_SKILL ; sn++)
	{
		ch->pcdata->learned[sn] = 0;
	}
	ch->pcdata->corpse_room            = 0;
	ch->pcdata->corpse                 = NULL;
	ch->pcdata->account                = 0;
	ch->pcdata->asn_obj                = NULL;
	ch->furniture                      = NULL;
	ch->mounting                       = NULL;

	if (ch->pcdata->shadowed_by)
	{
		stop_shadow(ch->pcdata->shadowed_by);
	}
	stop_shadow(ch);

	sub_player(ch);
	add_player(ch);

	ch->pcdata->reincarnation          = future_reincarnate;
	ch->pcdata->previous_hours        += future_hours;

	roll_race(ch);

	char_reset (ch);

	do_status (ch, "r");

	pop_call();
	return;
}
