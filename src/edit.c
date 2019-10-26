/***************************************************************************
 * Lola  1.0 by Igor van den Hoven.                                        *
 *                                                                         *
 * Emud  3.0 by Igor van den Hoven.                                        *
 ***************************************************************************/

#include "mud.h"

void start_editing( CHAR_DATA *ch, char *data )
{
	EDITOR_DATA *edit;
	char c, buffer[MAX_INPUT_LENGTH];
	sh_int line     = 0;
	sh_int leng     = 0;
	sh_int size     = 0;
	sh_int max_line = 150;
	sh_int max_size = 8000;

	push_call("start_editing(%p,%p)",ch,data);

	if (ch->level >= LEVEL_IMMORTAL)
	{
		max_line = 600;
		max_size = 32000;
	}

	if (ch->pcdata->editor) /* likely called by /f or /w in the editor */
	{
		edit = ch->pcdata->editor;

		while (edit->numlines)
		{
			edit->numlines--;
			STRFREE(edit->line[edit->numlines]);
		}
	}
	else
	{
		ch->desc->connected = CON_EDITING;

		send_to_char( "Begin entering your text now (/? = help /s = save /c = clear /l = list)\n\r", ch );
		send_to_char( "-----------------------------------------------------------------------\n\r", ch );

		ALLOCMEM(edit, EDITOR_DATA, 1);
	}

	do
	{
		c = data[size++];

		switch (c)
		{
			case '\0':
				if (leng)
				{
					buffer[leng] = 0;
					edit->line[line++] = STRALLOC(buffer);
				}
				continue;

			case '\r':
				continue;

			case '\n':
				buffer[leng] = 0;
				edit->line[line++] = STRALLOC(buffer);
				leng = 0;
				break;

			default:
				buffer[leng++] = c;
				break;
		}

		if (line == max_line)
		{
			break;
		}

		if (size == max_size)
		{
			buffer[leng] = 0;
			edit->line[line++] = STRALLOC(buffer);
			break;
		}
	}
	while (c);

	edit->numlines     = line;
	edit->size         = size;
	edit->on_line      = line;

	ch->pcdata->editor = edit;

	edit_buffer(ch, "/t");

	edit_buffer(ch, "/p");

	pop_call();
	return;
}

/*
	Copy the buffer, strips tailing spaces - Scandum
*/

char *copy_buffer( CHAR_DATA *ch )
{
	char buf1[MAX_INPUT_LENGTH], buf2[MAX_STRING_LENGTH];
	int x, y;

	push_call("copy_buffer(%p)",ch);

	for (x = buf2[0] = '\0' ; x < ch->pcdata->editor->numlines ; x++)
	{
		strcpy(buf1, ch->pcdata->editor->line[x]);

		for (y = strlen(buf1) - 1 ; y >= 0 ; y--)
		{
			if (buf1[y] == ' ')
			{
				buf1[y] = '\0';
			}
			else
			{
				break;
			}
		}
		cat_sprintf(buf2, "%s\n\r", buf1);
	}
	smash_tilde(buf2);

	pop_call();
	return STRALLOC(buf2);
}

void stop_editing( CHAR_DATA *ch )
{
	int x;

	push_call("stop_editing(%p)",ch);

	if (ch->pcdata->editor)
	{
		for (x = 0 ; x < ch->pcdata->editor->numlines ; x++)
		{
			STRFREE(ch->pcdata->editor->line[x]);
		}
		FREEMEM(ch->pcdata->editor);
		ch->pcdata->editor = NULL;
	}
	send_to_char_color("{300}You stop editing.\n\r", ch);

	ch->pcdata->editmode = ch->pcdata->tempmode;
	ch->pcdata->tempmode = MODE_NONE;

	if (ch->desc)
	{
		ch->desc->connected = CON_PLAYING;
	}
	pop_call();
	return;
}

void edit_buffer( CHAR_DATA *ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	EDITOR_DATA *edit;
	char cmd[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char prompt[15];
	char word1[MAX_INPUT_LENGTH];
	char word2[MAX_INPUT_LENGTH];
	char word3[MAX_INPUT_LENGTH];
	char *data;
	sh_int x        = 0;
	sh_int y        = 0;
	sh_int line     = 0;
	sh_int max_line = 150;
	sh_int max_leng = 1000;
	sh_int max_size = 8000;
	bool save       = FALSE;

	push_call("edit_buffer(%p,%p)",ch,argument);

	if (ch->level >= LEVEL_IMMORTAL)
	{
		max_line = 600;
		max_size = 32000;
	}

	if ((d = ch->desc) == NULL)
	{
		send_to_char( "You have no descriptor.\n\r", ch );
		pop_call();
		return;
	}

	sprintf(prompt, "%s> ", get_color_string(ch, COLOR_PROMPT, VT102_DIM));

	edit = ch->pcdata->editor;

	if (argument[0] == '/' || argument[0] == '\\')
	{
		argument = one_argument(argument, cmd);

		switch (cmd[1])
		{
			case '?':
				ch_printf_color(ch,
					"Editing commands\n\r"
					"-----------------------------------\n\r"
					"/a                 abort editing   \n\r"
					"/c                 clear buffer    \n\r"
					"/d [line] [amount] delete lines    \n\r"
					"/f                 format buffer   \n\r"
					"/g <line>          goto line       \n\r"
					"/i [line] [amount] insert lines    \n\r"
					"/l [range]         list buffer     \n\r"
					"/r <old> <new>     global replace  \n\r"
					"/s                 save buffer     \n\r"
					"/v [range]         view raw buffer \n\r"
					"/w                 wordwrap buffer \n\r");

				write_to_buffer( ch->desc, prompt, 1000000 );
				break;

			case 'p':
				write_to_buffer( ch->desc, prompt, 1000000 );
				break;

			case 'c':
				for (x = 0 ; x < edit->numlines ; x++)
				{
					STRFREE(edit->line[x]);
				}
				edit->numlines = 0;
				edit->on_line  = 0;
				edit->size     = 0;
				send_to_char( "Buffer cleared.\n\r", ch );
				write_to_buffer( ch->desc, prompt, 1000000 );
				break;

			case 'f':
				if (ch->pcdata->editmode == MODE_MOB_PROG || ch->pcdata->editmode == MODE_OBJ_PROG || ch->pcdata->editmode == MODE_ROOM_PROG)
				{
					send_to_char("You wouldn't want to format a program buffer.\n\r", ch);
					write_to_buffer( ch->desc, prompt, 1000000 );
				}
				else
				{
					data = buf;

					for (x = 0 ; x < edit->numlines ; x++)
					{
						for (y = 0 ; edit->line[x][y] ; y++)
						{
							*data++ = edit->line[x][y];
						}

						if (*edit->line[x] == 0)
						{
							*data++ = '\n';
						}
						else if (x + 1 < edit->numlines)
						{
							if (*edit->line[x+1] == 0)
							{
								*data++ = '\n';
							}
							else
							{
								*data++ = ' ';
							}
						}
						STRFREE(edit->line[x]);
					}
					*data = 0;

					start_editing(ch, ansi_justify(buf, 80));

					ch_printf(ch, "Buffer formatted between line 1 and %d.\n\r", x);
					write_to_buffer( ch->desc, prompt, 1000000 );
				}
				break;

			case 't':
				data = copy_buffer(ch);

				if (ch->desc->mth->msdp_data)
				{
					if (msdp_update_var(ch->desc, "EDIT_BUFFER", "%s", data))
					{
						send_to_char("Sending EDIT_BUFFER over MSDP.\n\r", ch);
					}
				}
				STRFREE(data);

				break;

			case 'w':
				if (ch->pcdata->editmode == MODE_MOB_PROG || ch->pcdata->editmode == MODE_OBJ_PROG)
				{
					send_to_char("You wouldn't want to word wrap a program buffer.\n\r", ch);
					write_to_buffer( ch->desc, prompt, 1000000 );
				}
				else
				{
					for (x = 0 ; x < edit->numlines ; x++)
					{
						strcpy(buf, ansi_justify(edit->line[x], 80));
						RESTRING(edit->line[x], buf);
					}

					data = copy_buffer(ch);

					start_editing(ch, data);

					STRFREE(data);

					ch_printf(ch, "Word wrapped text between line 1 and %d.\n\r", x);
					write_to_buffer( ch->desc, prompt, 1000000 );
				}
				break;

			case 'r':
				{
					char *wptr, *lwptr;
					int count, wordln, word2ln;

					argument = one_argument_nolower(argument, word1);
					argument = one_argument_nolower(argument, word2 );

					if (word1[0] == '\0' || word2[0] == '\0')
					{
						send_to_char( "You need a word to replace, and it's replacement.\n\r", ch);
						write_to_buffer( ch->desc, prompt, 1000000 );
						pop_call();
						return;
					}

					count   = 0;
					wordln  = strlen(word1);
					word2ln = strlen(word2);

					ch_printf( ch, "Replacing all occurrences of %s with %s.\n\r", word1, word2 );

					for (x = 0 ; x < edit->numlines ; x++)
					{
						lwptr = strcpy(buf, edit->line[x]);
	
						while ((wptr = strstr(lwptr, word1)) != NULL)
						{
							if (edit->size + (word2ln - wordln) >= max_size)
							{
								break;
							}
							if (strlen(buf) + (word2ln - wordln) >= max_leng)
							{
								break;
							}
							lwptr = wptr + word2ln;

							sprintf(word3, "%s%s", word2, wptr + wordln);
							strcpy(wptr, word3);

							edit->size += (word2ln - wordln);
							count      += 1;
						}
						RESTRING(edit->line[x], buf);
					}
					ch_printf( ch, "Found and replaced %d occurrence%s between line 1 and %d.\n\r", count, count == 1 ? "" : "s",  edit->numlines);
					write_to_buffer( ch->desc, prompt, 1000000 );
				}
				break;

			case 'i':
				if (edit->numlines >= max_line)
				{
					send_to_char( "Editing buffer is full.\n\r", ch );
					write_to_buffer( ch->desc, prompt, 1000000 );
				}
				else
				{
					int count;

					argument = one_argument_nolower(argument, word1);
					argument = one_argument_nolower(argument, word2 );

					if (word1[0] != '\0')
					{
						line = atoi(word1) - 1;
					}
					else
					{
						line = edit->on_line;
					}
					count = URANGE(1, atoi(word2), max_line - edit->numlines);

					if (line < 0 || line > edit->numlines)
					{
						ch_printf(ch, "The insert rage is between 0 and %d.", edit->numlines);
						write_to_buffer( ch->desc, prompt, 1000000 );
					}
					else
					{
						for (y = 0 ; y < count ; y++)
						{
							for (x = ++edit->numlines ; x > line ; x--)
							{
								edit->line[x] = edit->line[x-1];
							}
							edit->line[line] = STRDUPE(str_empty);
						}
						ch_printf(ch, "Inserted %d line%s on line %d.\n\r", count, count > 1 ? "s" : "", line + 1);
						write_to_buffer( ch->desc, prompt, 1000000 );
					}
				}
				break;

			case 'd':
				if (edit->numlines == 0)
				{
					send_to_char( "Editing buffer is empty.\n\r", ch );
					write_to_buffer( ch->desc, prompt, 1000000 );
				}
				else
				{
					int count;

					argument = one_argument_nolower(argument, word1);
					argument = one_argument_nolower(argument, word2 );

					if (word1[0] != '\0')
					{
						line = atoi(word1) - 1;
					}
					else
					{
						line = URANGE(0, edit->on_line, edit->numlines - 1);
					}
					count = URANGE(1, atoi(word2), edit->numlines - line);

					if (line < 0 || line > edit->numlines)
					{
						ch_printf(ch, "The delete range is between 0 and %d\n\r", edit->numlines);
						write_to_buffer( ch->desc, prompt, 1000000 );
					}
					else
					{
						for (y = 0 ; y < count ; y++)
						{
							edit->size -= strlen(edit->line[line]);

							STRFREE(edit->line[line]);

							for (x = line ; x < (edit->numlines - 1) ; x++)
							{
								edit->line[x] = edit->line[x+1];
							}

							edit->numlines--;

							if (edit->on_line > edit->numlines)
							{
								edit->on_line = edit->numlines;
							}

						}
						ch_printf(ch, "Line%s deleted.\n\r", count > 1 ? "s" : "");
						write_to_buffer( ch->desc, prompt, 1000000 );
					}
				}
				break;

			case 'g':
				if (edit->numlines == 0)
				{
					send_to_char( "Editing buffer is empty.\n\r", ch );
					write_to_buffer( ch->desc, prompt, 1000000 );
				}
				else
				{
					if (argument[0] != '\0')
					{
						line = atoi(argument) - 1;
					}
					else
					{
						send_to_char( "Go to which line?\n\r", ch );
						write_to_buffer( ch->desc, prompt, 1000000 );
						pop_call();
						return;
					}
					if (line < 0 || line > edit->numlines)
					{
						ch_printf(ch, "The goto range is between 0 and %d.", edit->numlines);
						write_to_buffer( ch->desc, prompt, 1000000 );
					}
					else
					{
						edit->on_line = line;
						ch_printf( ch, "(Editing line %d)\n\r", line+1 );
						write_to_buffer( ch->desc, prompt, 1000000 );
					}
				}
				break;

			case 'l':
				if (edit->numlines == 0)
				{
					send_to_char( "Editing buffer is empty.\n\r", ch );
					write_to_buffer( ch->desc, prompt, 1000000 );
				}
				else
				{
					argument = one_argument(argument, word1);
					argument = one_argument(argument, word2);

					x = atoi(word1)-1;
					y = atoi(word2);

					x = x ? URANGE(0, x, edit->numlines) : 0;
					y = y ? URANGE(0, y, edit->numlines) : edit->numlines;

					send_to_char("\n\r", ch);

					for ( ; x < y ; x++)
					{
						ch_printf_color(ch, "%c%2d> %s\n\r", x == edit->on_line ? '*' : ' ', x + 1, edit->line[x] );
					}
					write_to_buffer( ch->desc, prompt, 1000000 );
				}
				break;

			case 'v':
				if (edit->numlines == 0)
				{
					send_to_char( "Editing buffer is empty.\n\r", ch );
					write_to_buffer( ch->desc, prompt, 1000000 );
				}
				else
				{
					argument = one_argument(argument, word1);
					argument = one_argument(argument, word2);

					x = atoi(word1)-1;
					y = atoi(word2);

					x = x ? URANGE(0, x, edit->numlines) : 0;
					y = y ? URANGE(0, y, edit->numlines) : edit->numlines;

					ch_printf(ch, "------------------\n\r");

					for ( ; x < y ; x++)
					{
						ch_printf(ch, "%s\n\r", edit->line[x]);
					}

					ch_printf(ch, "------------------\n\r");

					write_to_buffer( ch->desc, prompt, 1000000 );
				}
				break;

			case 'a':
				send_to_char( "\n\rAborting edit.\n\r", ch );
				stop_editing( ch );
				break;

			case 's':
				d->connected = CON_PLAYING;

				if (ch->pcdata->last_cmd)
				{
					(*ch->pcdata->last_cmd) ( ch, "" );
				}
				break;

			default:
				ch_printf(ch, "Unknown command: /%c, /? = help\n\r", cmd[1] ? cmd[1] : ' ');
				write_to_buffer(ch->desc, prompt, 1000000);
				break;
		}
		pop_call();
		return;
	}

	if (strlen(argument) >= max_leng)
	{
		argument[max_leng] = '\0';
	}

	if (edit->size + strlen(argument) >= max_size)
	{
		ch_printf(ch, "Max editing buffer size of %d reached.\n\r", max_size);
		save = TRUE;
	}
	else if (edit->numlines >= max_line)
	{
		ch_printf(ch, "Max editing buffer length of %d lines reached.\n\r", max_line );
		save = TRUE;
	}
	else
	{
		edit->size += strlen(argument);

		edit->line[edit->on_line] = STRALLOC(argument);

		if (++edit->on_line > edit->numlines)
		{
			edit->numlines++;
		}

		if (edit->numlines >= max_line)
		{
			ch_printf(ch, "Max editing buffer length of %d lines reached.\n\r", max_line);
			save = TRUE;
		}
	}

	if (save)
	{
		d->connected = CON_PLAYING;
		if (ch->pcdata->last_cmd)
		{
			(*ch->pcdata->last_cmd) ( ch, "" );
		}
		pop_call();
		return;
	}

	ch_printf_color(ch, "%s\n\r", edit->line[edit->on_line-1]);

	write_to_buffer( ch->desc, prompt, 1000000 );
	pop_call();
	return;
}
