/***************************************************************************
 * Lola  1.0 by Igor van den Hoven.                                        *
 *                                                                         *
 * Emud  2.2 by Igor van den Hoven, Michiel Lange.                         *
 ***************************************************************************/

#include <stdarg.h>
#include "mud.h"

/*
	debugging functions here.
	implemented in emud by Manwe and Scandum,
	improved and optimized by Scandum,
	credits to Hypnos who invented this idea.
*/

#define MAX_STACK_CALL		200
#define MAX_STACK_CALL_LEN	200

char	call_stack[MAX_STACK_CALL][MAX_STACK_CALL_LEN];
int	call_index;


void push_call(char *f, ...)
{
	va_list ap;

	va_start(ap, f);

	if (call_index >= MAX_STACK_CALL)
	{
		call_index = MAX_STACK_CALL / 2;
		log_printf("push_call: max stack size reached");
		dump_stack();
		call_index = 0;
	}

/*	strcpy(call_stack[call_index], f); */

	vsprintf(call_stack[call_index], f, ap);

	call_index++;
}


void push_call_format(char *f, ...)
{
	va_list ap;

	va_start(ap, f);

	if (call_index >= MAX_STACK_CALL)
	{
		call_index = MAX_STACK_CALL / 2;
		log_printf("push_call: max stack size reached");
		dump_stack();
		call_index = 0;
	}

/*	strcpy(call_stack[call_index], f); */

	vsprintf(call_stack[call_index], f, ap);

	call_index++;

	va_end(ap);
}


void pop_call(void)
{
	if (call_index != 0)
	{
		call_index--;
	}
	else
	{
		log_printf("pop_call: index is zero: %s", call_stack[0]);
	}
}


void do_stack( CHAR_DATA *ch, char *argument )
{
	int i;

	ch_printf(ch, "Stack of Lola (index = %d)\n\r\n\r", call_index);

	for (i = 0 ; i < call_index ; i++)
	{
		ch_printf(ch, "%03d: %s\n\r", i, call_stack[i]);
	}
}


void dump_stack(void)
{
	int i;

	log_printf("Stack trace (index = %d)", call_index);

	for (i = 0 ; i < call_index ; i++)
	{
		log_printf("call_stack[%03d] = %s", i, call_stack[i]);
	}
	log_printf("End of stack");
}

void build_dump_stack( int vnum )
{
	int i;

	log_build_printf(vnum, "Stack trace (index = %d)", call_index);

	for (i = 0 ; i < call_index ; i++)
	{
		log_build_printf(vnum, "call_stack[%03d] = %s", i, call_stack[i]);
	}
	log_build_printf(vnum, "End of stack");
}

void dump_desc_characters()
{
	DESCRIPTOR_DATA *d;

	push_call("dump_desc_characters()");

	log_string("List of active descriptors: ");

	for (d = mud->f_desc ; d ; d = d->next)
	{
		log_printf("%d: %p->%p->name = %s", d->descriptor, d, d->character, d->character ? d->character->name : "unknown");
	}
	log_string("End of list\n");

	pop_call();
	return;
}

void print_errno(int number)
{
	push_call("print_errno(%p)",number);

	switch(number)
	{
		case EFAULT		: log_string("EFAULT");		break;
		case EACCES		: log_string("EACCES");		break;
		case EPERM		: log_string("EPERM");		break;
		case ENAMETOOLONG	: log_string("ENAMETOOLONG");	break;
		case ENOTDIR		: log_string("ENOTDIR");		break;
		case ENOMEM		: log_string("ENOMEM");		break;
		case EROFS		: log_string("EROFS");		break;
		case ENOENT		: log_string("ENOENT");		break;
		case EISDIR		: log_string("EISDIR");		break;
		case EXDEV		: log_string("EXDEV");		break;
		case ENOTEMPTY		: log_string("ENOTEMPTY");	break;
		case EEXIST		: log_string("EEXIST");		break;
		case EBUSY		: log_string("EBUSY");		break;
		case EINVAL		: log_string("EINVAL");		break;
		case EMLINK		: log_string("EMLINK");		break;
		case ELOOP		: log_string("ELOOP");		break;
		case ENOSPC		: log_string("ENOSPC");		break;
		default			: log_printf("ErrNo: %d",number);break;
	}
	pop_call();
	return;
}


void open_reserve( void )
{
	FILE_DATA *fdp;

	push_call("open_reserve(void)");

	fpReserve = fopen(NULL_FILE, "r");

	if (fpReserve == NULL)
	{
		log_printf("open_reserve: Failed to open %s (%s)", NULL_FILE, "r");
		print_errno(errno);
	}
	else
	{
		ALLOCMEM(fdp, FILE_DATA, 1);

		fdp->filename = STRALLOC(NULL_FILE);
		fdp->opentype = STRALLOC("r");
		fdp->fp = fpReserve;

		LINK(fdp, mud->f_open_file, mud->l_open_file, next, prev);
	}
	pop_call();
	return;
}


void close_reserve( void )
{
	FILE_DATA *fdp;

	push_call("close_reserve(void)");

	for (fdp = mud->f_open_file ; fdp ; fdp = fdp->next)
	{
		if (fdp->fp == fpReserve)
		{
			fclose(fdp->fp);

			STRFREE(fdp->filename);
			STRFREE(fdp->opentype);

			UNLINK(fdp, mud->f_open_file, mud->l_open_file, next, prev);
			FREEMEM(fdp);

			pop_call();
			return;
		}
	}
	if (fdp == NULL)
	{
		log_printf("close_reserve: file data not found");
		dump_stack();
	}
	pop_call();
	return;
}


FILE *my_fopen(char *filename, char *opentype, bool silent)
{
	char file_buf[100];
	char open_buf[10];

	FILE_DATA *fdp;
	FILE *fp;

	push_call("my_fopen(%p,%p,%p)",filename,opentype,silent);

	strcpy(file_buf, filename);
	strcpy(open_buf, opentype);

	fp = fopen(file_buf, open_buf);

	if (fp == NULL)
	{
		if (silent == FALSE)
		{
			log_printf("Failed to open: %s (%s)", file_buf, open_buf);
/*			print_errno(errno); */
		}
		pop_call();
		return NULL;
	}

	ALLOCMEM(fdp, FILE_DATA, 1);

	fdp->filename = STRALLOC(file_buf);
	fdp->opentype = STRALLOC(open_buf);
	fdp->fp = fp;

	LINK(fdp, mud->f_open_file, mud->l_open_file, next, prev);

	pop_call();
	return fp;
}

void my_fclose(FILE *which)
{
	FILE_DATA *fdp;

	push_call("my_fclose(%p)",which);

	for (fdp = mud->f_open_file ; fdp ; fdp = fdp->next)
	{
		if (fdp->fp == which)
		{
			fclose(fdp->fp);

			STRFREE(fdp->filename);
			STRFREE(fdp->opentype);

			UNLINK(fdp, mud->f_open_file, mud->l_open_file, next, prev);
			FREEMEM(fdp);

			pop_call();
			return;
		}
	}
	if (fdp == NULL)
	{
		log_printf("my_fclose: No matching file found to remove from the lists");
		dump_stack();
	}

	pop_call();
	return;
}

void my_fclose_all()
{
	FILE_DATA *fdp, *fdp_next;

	push_call("my_fclose_all()");

	for (fdp = mud->f_open_file ; fdp ; fdp = fdp_next)
	{
		fdp_next = fdp->next;
		log_printf("my_fclose_all(): closing file %s", fdp->filename);
		my_fclose(fdp->fp);
	}
	pop_call();
	return;
}

void do_openfiles(CHAR_DATA *ch, char *argument)
{
	FILE_DATA *fdp;

	push_call("do_openfiles(%p,%p)",ch,argument);

	ch_printf(ch, "Openfiles:\n\r");

	for (fdp = mud->f_open_file ; fdp ; fdp = fdp->next)
	{
		ch_printf(ch, "Filename: %-20s Type: %s\n\r", fdp->filename, fdp->opentype);
	}
	pop_call();
	return;
}

void do_version(CHAR_DATA *ch, char *argument)
{
	FILE *vfp;
	char str_vers[40];
	char str_date[40];

	push_call("do_version(%p,%p)",ch,argument);

	if (strlen(argument) > 40)
	{
		argument[40] = '\0';
	}

	if (ch->level < MAX_LEVEL || argument[0] == '\0')
	{
		vfp = my_fopen(VERSION_FILE, "r", FALSE);

		if (vfp)
		{
			fread(str_vers, sizeof(char), 40, vfp);
			fread(str_date, sizeof(char), 40, vfp);

			ch_printf_color(ch, " Current version: {168}%s {078}({178}%s{078})\n\r", str_vers, str_date);

			fread(str_vers, sizeof(char), 40, vfp);
			fread(str_date, sizeof(char), 40, vfp);

			ch_printf_color(ch, "Previous version: {168}%s {078}({178}%s{078})\n\r", str_vers, str_date);

			my_fclose(vfp);

			pop_call();
			return;
		}
		else
		{
			send_to_char("It is unknown which version we are now running.\n\r",ch);
			pop_call();
			return;
		}
	}
	else
	{
		if (!strncasecmp(argument, "set ", 3))
		{
			char new_vers[40];
			char new_date[40];

			strcpy(new_vers, &argument[4]);

			if (new_vers[0] == '\0')
			{
				send_to_char("Usage: version set {<version number>}\n\r", ch);
				pop_call();
				return;
			}

			vfp = my_fopen(VERSION_FILE, "r", FALSE);

			if (vfp)
			{
				fread(str_vers, sizeof(char), 40, vfp);
				fread(str_date, sizeof(char), 40, vfp);

				my_fclose(vfp);
			}
			else
			{
				ch_printf(ch, "Could not open %s to read old information!\n\r", VERSION_FILE);
				str_vers[0] = '\0';
				str_date[0] = '\0';
			}

			vfp = my_fopen(VERSION_FILE, "w", FALSE);

			if (vfp)
			{
				strftime(new_date, 40, "%d{078}-{178}%m{078}-{178}%Y", &mud->time);

				fwrite(new_vers, sizeof(char), 40, vfp);
				fwrite(new_date, sizeof(char), 40, vfp);

				fwrite(str_vers, sizeof(char), 40, vfp);
				fwrite(str_date, sizeof(char), 40, vfp);

				fflush(vfp);
				my_fclose(vfp);

				ch_printf(ch, "You set the version from %s to %s\n\r", str_vers, new_vers);

				log_printf("%s set the version from %s to %s", ch->name, str_vers, new_vers);

				pop_call();
				return;
			}
			else
			{
				send_to_char("Failed to open the version file to write to.\n\r",ch);
				perror("Could not write version file to write to.");
				pop_call();
				return;
			}
		}
		else
		{
			send_to_char("Syntax: version set <version number>\n\r",ch);
			pop_call();
			return;
		}
	}
	pop_call();
	return;
}

char *cut_at_eol(char *string)
{
	int i=0;
	push_call("cut_at_eol(%p)",string);
	for(i=0;i<strlen(string);i++)
	{
		if(string[i] == '\n' || string[i] == '\r')
		{
			string[i] = '\0';
		}
	}
	pop_call();
	return string;
}
