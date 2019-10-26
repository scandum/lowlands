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

#include <sys/time.h>
#include <sys/socket.h>
#include <sys/resource.h>

#include <unistd.h>
#include <stdarg.h>

#include "mud.h"

/*
	Socket and TCP/IP stuff.
*/

#include <signal.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/wait.h>

#define IAC 255
#define EOR 239

void		pipe_handler(int signal);
void		trap_handler(int signal);
void		scan_object_for_dup(CHAR_DATA *ch, OBJ_DATA *obj);
char	*	prompt_return(CHAR_DATA *, char *);
int		pager(DESCRIPTOR_DATA *, const char *, int, char *);
void		scroll(DESCRIPTOR_DATA *, const char *, int);
char		ansi_strip_txt[MAX_STRING_LENGTH];
bool		scroll_you(DESCRIPTOR_DATA *, const char *, bool);


/*
	OS-dependent declarations.
*/

#if defined(linux)
int	close		args((int fd));
#ifndef __GLIBC__2__
int	getpeername	args((int s, struct sockaddr *name, int *namelen));
int	getsockname	args((int s, struct sockaddr *name, int *namelen));
int	listen		args((int s, int backlog));
#endif

int	gettimeofday	args((struct timeval *tp, struct timezone *tzp));
int	select		args((int width, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout));
int	socket		args((int domain, int type, int protocol));
#endif

/*
	Locals.
*/

char				logfilename[25];
OBJ_REF_HASH		obj_ref_list[MAX_OBJ_REF];
long long			obj_ref_key;

/*
	OS-dependent local functions.
*/

void	game_loop_unix			args((void));
int	init_socket			args(());
void	new_descriptor			args((void));
bool	read_from_descriptor	args((DESCRIPTOR_DATA *d));


/*
	Other local functions (OS-independent).
*/

bool		check_reconnecting	args((DESCRIPTOR_DATA *d,CHAR_DATA *ch));
int		main				args((int argc, char **argv));
bool		nanny			args((DESCRIPTOR_DATA *d, char *argument));
bool		process_output		args((DESCRIPTOR_DATA *d, bool fPrompt));
void		read_from_buffer	args((DESCRIPTOR_DATA *d));
void		stop_idling		args((DESCRIPTOR_DATA *d));

int main (int argc, char **argv)
{
	struct timeval now_time;
	int count;
	int cntr, i;
	FILE *pid_file;

	push_call("main(%p,%p)",argc,argv);

	ALLOCMEM(mud,				MUD_DATA,			1);
	ALLOCMEM(mud->usage,			USAGE_DATA,		1);
	ALLOCMEM(mud->time_info,		TIME_INFO_DATA,	1);
	ALLOCMEM(mud->tactical,		TACTICAL_MAP,		1);
	ALLOCMEM(mud->last_player_cmd,	unsigned char, 	MAX_INPUT_LENGTH);

	init_mth();

	for (count = 0 ; count < MOST_MOST ; count++)
	{
		ALLOCMEM(mud->high_scores[count], HISCORE_LIST, 1);
	}

	str_empty				= STRALLOC("");

	obj_ref_key			= get_game_usec();

	/*
		place the pid of this process in the file lola.pid
	*/

	pid_file = my_fopen("../lola.pid","w",FALSE);

	if (pid_file)
	{
		fprintf(pid_file, "%d\n", getpid());
		fflush(pid_file);
		my_fclose(pid_file);
	}
	else
	{
		log_printf("Could not write the pid number (%d) in ../lola.pid", getpid());
	}

	/*
		Let's log the current system constraints as seen by MrMud - Chaos 5/3/98
	*/

	{
		struct rlimit rlpt;

		getrlimit(RLIMIT_DATA, &rlpt);
		log_printf("System memory usage: %ld max %ld current", (long)rlpt.rlim_max, (long)rlpt.rlim_cur);

		rlpt.rlim_max = RLIM_INFINITY;
		rlpt.rlim_cur = RLIM_INFINITY;
		setrlimit(RLIMIT_DATA, &rlpt);
		getrlimit(RLIMIT_DATA, &rlpt);
		log_printf("Unlimited memory usage: %ld max %ld current", (long)rlpt.rlim_max, (long)rlpt.rlim_cur);
	}

	for (cntr = 0 ; cntr < argc ; cntr++)
	{
		log_printf("argv[%d] = %s",cntr,argv[cntr]);
	}


	/*
		Init time.
	*/

	gettimeofday(&now_time, NULL);

	mud->current_time = now_time.tv_sec;
	mud->boot_time	   = now_time.tv_sec;
	mud->port         = 4321;
	logfilename[0]    = '\0';

	if (argc > 1)
	{
		for (count = 1 ; count < argc ; count++)
		{
			if (!strcmp(argv[count], "-b"))
			{
				mud->boot_time = atol(argv[++count]);
			}

			if (!strcmp(argv[count], "-c"))
			{
				mud->control = atol(argv[++count]);
				log_printf("Set control to %d", mud->control);
				SET_BIT(mud->flags, MUD_EMUD_COPYOVER);
			}

			if (!strcmp(argv[count], "-p"))
			{
				mud->port = atol(argv[++count]);
			}
		}
	}

	{
		FILE *logtst;

		for (i = 1000 ; i < 10000 ; i++)
		{
			sprintf(logfilename, "../log/%d.log", i);

			if ((logtst = my_fopen(logfilename, "r", TRUE)) != NULL)
			{
				my_fclose(logtst);
			}
			else
			{
				break;
			}
		}
		sprintf(logfilename, "../log/%d.log", --i);

		log_printf("Found logfile %s", logfilename);
	}

	if (!IS_SET(mud->flags, MUD_EMUD_COPYOVER))
	{
		nice(5);
	}

	if ((i = check_dirs()) == -1)
	{
		log_printf("Checking of directories failed: %s", strerror(errno));
		abort();
	}
	else
	{
		log_printf("Directories are okay (%d dirs were newly created).", i);
	}

	if (mud->port == 4321)
	{
		SET_BIT(mud->flags, MUD_EMUD_REALGAME);
	}

	/*
		Ignored Signals
	*/

	signal(SIGPIPE,	pipe_handler);

	/*
		Known signals
	*/

	signal(SIGTERM,	trap_handler);
	signal(SIGSEGV,	trap_handler);
	signal(SIGALRM,	trap_handler);

	/*
		Added just in case
	*/

	signal(SIGINT,		trap_handler);
	signal(SIGHUP,		trap_handler);
	signal(SIGFPE,		trap_handler);
	signal(SIGABRT,	trap_handler);

	/*
		Run the game.
	*/

	if (fpReserve)
	{
		log_string ("fpReserve : already opened");
	}
	else
	{
		fpReserve = my_fopen(NULL_FILE, "r", FALSE);

		if (fpReserve)
		{
			log_string ("fpReserve : opened readonly");
		}
		else
		{
			log_string ("fpReserve : failed to open readonly");
		}
	}

	if (fpAppend)
	{
		log_string("fpAppend  : already opened");
	}
	else
	{
		fpAppend = my_fopen(NULL_FILE, "r",FALSE);

		if (fpAppend)
		{
			log_string ("fpAppend  : opened readonly");
		}
		else
		{
			log_string ("fpAppend  : failed to open readonly");
		}
	}

	boot_db(IS_SET(mud->flags, MUD_EMUD_COPYOVER));

	if (!IS_SET(mud->flags, MUD_EMUD_COPYOVER))
	{
		log_printf("Initializing socket for port %d", mud->port);
		mud->control = init_socket();
	}

	log_printf("starting game_loop_unix: control = %d, port = %d", mud->control, mud->port);

	game_loop_unix();

	close(mud->control);

	log_string("Normal termination of game.");

	unlink("../lola.pid");

	exit(0);
}


void pipe_handler(int signal)
{
	log_printf("broken_pipe: dumping stack");
	dump_stack();
}


void trap_handler(int signal)
{
	DESCRIPTOR_DATA *d;
	PLAYER_GAME	 *gpl;
	FILE_DATA       *fdp;
	char stoptype[80];

	if (IS_SET(mud->flags, MUD_EMUD_CRASH))
	{
		log_string("Emud crashed twice: shutting down");
		dump_stack();
		exit(-1);
	}
	else
	{
		SET_BIT(mud->flags, MUD_EMUD_CRASH);
	}

	if (fflush(NULL) != 0)
	{
		perror("failed to flush");
		dump_stack();
		exit (-1);
	}

	dump_stack();
	dump_desc_characters();

	log_printf("Last player command: %s", mud->last_player_cmd);

	switch (signal)
	{
		case SIGSEGV:
			log_string("*** SEGV SIGNAL CAUGHT *** trying to shutdown gracefully.\n");
			sprintf(stoptype, "*** SIGNAL_SEGV ***");
			break;
		case SIGHUP:
			log_string("*** HUP SIGNAL CAUGHT *** trying to shutdown gracefully.\n");
			sprintf(stoptype, "*** SIGNAL_HUP ***");
			break;
		case SIGTERM:
			log_string("*** TERM SIGNAL CAUGHT *** shutting down gracefully.\n");
			sprintf(stoptype, "*** SIGNAL_TERM ***");
			break;
		case SIGALRM:
			log_string("*** ALARM SIGNAL CAUGHT *** shutting down gracefully.\n");
			sprintf(stoptype, "*** SIGNAL_ALRM ***");
			break;
		default :
			log_printf("*** UNKNOWN SIGNAL PASSED (%d) *** not shutting down gracefully.\n", signal);
			sprintf(stoptype, "*** UNKNOWN SIGNAL (%d) TRAPPED! ***", signal);
			exit (-1);
			break;
	}

	log_printf("Saving players...");

	for (d = mud->f_desc ; d ; d = d->next)
	{
		if (d->original)
		{
			do_return(d->character, NULL);
		}
	}

	for (gpl = mud->f_player ; gpl ; gpl = gpl->next)
	{
		save_char_obj(gpl->ch, NORMAL_SAVE);

		if (gpl->ch->desc)
		{
			vt100off(gpl->ch);

			ch_printf_color(gpl->ch, "\n\r{078}Lowlands has crashed but will probably soon recover.\n\r\n\r");

			write_to_port(gpl->ch->desc);
		}
	}

	save_world();

	log_string("Listing opened files...");

	for (fdp = mud->f_open_file ; fdp ; fdp = fdp->next)
	{
		log_printf("%s [%s]", fdp->filename, fdp->opentype);
	}

	log_string("\nSHUTTING DOWN NOW");

	/*
		the developer (defined in emud.h) will automatically
		receive a mail with the log to trace down the error
		Manwe, 08 Jan 2000

		Disabled, only Michiel used it and he's no
		longer coding for the Emud project.
		Scandum 14-04-2002

		{
			char sysbuf[250];
			sprintf(sysbuf, "mail -s '%s' %s < %s", stoptype, DEVELOPER_MAIL, logfilename);
			log_printf("Mailed log %s to %s.",logfilename,DEVELOPER_MAIL);
			system(sysbuf);
		}
	*/

	exit(-1);
}

int init_socket(void)
{
	static struct sockaddr_in sa_zero;
	struct sockaddr_in sa;
	int x=1;
	int fd;

	push_call("init_socket(void)");

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("init_socket: socket");
		abort();
	}

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &x, sizeof(x)) < 0)
	{
		perror("init_socket: SO_REUSEADDR");
		close(fd);
		abort();
	}

	{
		struct linger ld;

		ld.l_onoff  = 0;
		ld.l_linger = 100;

		if (setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld)) < 0)
		{
			perror("init_socket: SO_LINGER");
			close(fd);
			abort();
		}
	}

/*
	{
		int sockbuf;
		unsigned int socksize;

		socksize = sizeof(sockbuf);

		if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *) &sockbuf, &socksize) < 0)
		{
			perror("getsockopt: SO_RCVBUF");
			close(fd);
			abort();
		}

		log_printf("Initial SO_RCVBUF size:  %d", sockbuf);


		if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *) &sockbuf, &socksize) < 0)
		{
			perror("getsockopt: SO_SNDBUF");
			close(fd);
			abort();
		}

		log_printf("Initial SO_SNDBUF size: %d", sockbuf);

		sockbuf = MAX_INPUT_LENGTH;

		if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *) &sockbuf, sizeof(socksize)) < 0)
		{
			perror("getsockopt: SO_RCVBUF");
			close(fd);
			abort();
		}

		if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *) &sockbuf, &socksize) < 0)
		{
			perror("getsockopt: SO_RCVBUF");
			close(fd);
			abort();
		}

		log_printf("Modified SO_RCVBUF size: %d", sockbuf);


		sockbuf = MAX_STRING_LENGTH;

		if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *) &sockbuf, sizeof(socksize)) < 0)
		{
			perror("getsockopt: SO_SNDBUF");
			close(fd);
			abort();
		}

		if (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *) &sockbuf, &socksize) < 0)
		{
			perror("getsockopt: SO_SNDBUF");
			close(fd);
			abort();
		}

		log_printf("Modified SO_SNDBUF size: %d", sockbuf);
	}
*/
	sa			= sa_zero;
	sa.sin_family	= AF_INET;
	sa.sin_port	= htons(mud->port);

	if (bind(fd, (struct sockaddr *) &sa, sizeof(sa)) < 0)
	{
		perror("init_socket: bind");
		close(fd);
		abort();
	}

	if (listen(fd, 20) < 0)
	{
		perror("init_socket: listen");
		close(fd);
		abort();
	}
	pop_call();
	return fd;
}


void game_loop_unix(void)
{
	static struct timeval null_time;
	struct timeval last_time;
	DESCRIPTOR_DATA *d;
	char dbuf[180];

	fd_set in_set;
	fd_set out_set;
	fd_set exc_set;
	int maxdesc;

	struct timeval now_time, stall_time;

	int usec_gameloop, usec_gamewait;

	push_call("game_loop_unix(void)");

	gettimeofday(&last_time, NULL);
	mud->current_time = (time_t) last_time.tv_sec;

	stall_time.tv_sec  = 0;

	/*
		Main loop
	*/

	while (!IS_SET(mud->flags, MUD_EMUD_DOWN))
	{
		gettimeofday(&last_time, NULL);

		mud->current_time = last_time.tv_sec;

		if (mud->f_desc == NULL && IS_SET(mud->flags, MUD_EMUD_REBOOT))
		{
			SET_BIT(mud->flags, MUD_EMUD_DOWN);
		}

		alarm(30);

		mud->time = *localtime(&mud->current_time);

		/*
			Take care of various update routines
		*/

		update_handler();

		start_timer(TIMER_SCAN_DESC);

		/*
			Poll all active descriptors.
		*/

		FD_ZERO(&in_set );
		FD_ZERO(&out_set);
		FD_ZERO(&exc_set);
		FD_SET(mud->control, &in_set);

		maxdesc = mud->control;

		for (d = mud->f_desc ; d ; d = d->next)
		{
			maxdesc = UMAX(maxdesc, d->descriptor);
			FD_SET(d->descriptor, &in_set );
			FD_SET(d->descriptor, &out_set);
			FD_SET(d->descriptor, &exc_set);
		}

		if (select(maxdesc+1, &in_set, &out_set, &exc_set, &null_time) < 0)
		{
			perror("Game_loop: select: poll");
			abort();
		}

		/*
			New connection?
		*/

		if (FD_ISSET(mud->control, &in_set))
		{
			new_descriptor();
		}

		/*
			Kick out the freaky folks.
		*/

		for (d = mud->f_desc ; d ; d = mud->update_gld)
		{
			mud->update_gld = d->next;

			if (FD_ISSET(d->descriptor, &exc_set) || IS_SET(d->mth->comm_flags, COMM_FLAG_DISCONNECT))
			{
				FD_CLR(d->descriptor, &in_set );
				FD_CLR(d->descriptor, &out_set);
				d->outtop = 0;

				if (d->character != NULL)
				{
					log_printf("Kicking off %s for being freaky", d->character->name);
				}
				close_socket(d);
			}
		}

		if (mud->f_desc && !IS_SET(mud->flags, MUD_EMUD_BOOTING))
		{
			for (d = mud->f_desc ; d ; d = d->next)
			{
				REMOVE_BIT(d->mth->comm_flags, COMM_FLAG_COMMAND);
			}
		}

		if (mud->mp_group_greeter && mud->mp_group_greeted)
		{
			if (MP_VALID_MOB(mud->mp_group_greeter))
			{
				mudprog_percent_check(mud->mp_group_greeter->pIndexData->first_prog, mud->mp_group_greeter, mud->mp_group_greeted, NULL, NULL, GROUP_GREET_PROG, mud->mp_group_greeter->pIndexData->vnum);
			}
			mud->mp_group_greeter = mud->mp_group_greeted = NULL;
		}

		close_timer(TIMER_SCAN_DESC);

		start_timer(TIMER_PROCESS_INPUT);

		/*
			Process input.
		*/

		if (mud->f_desc != NULL && !IS_SET(mud->flags, MUD_EMUD_BOOTING))
		{
			for (d = mud->f_desc ; d ; d = mud->update_gld)
			{
				mud->update_gld = d->next;

				if (FD_ISSET(d->descriptor, &in_set))
				{
					if (d->character == NULL || d->character->desc != d || !IS_SET(CH(d)->pcdata->interp, INTERP_ALIAS))
					{
						if (!read_from_descriptor(d))
						{
							FD_CLR(d->descriptor, &out_set);
							d->outtop = 0;
							SET_BIT(d->mth->comm_flags, COMM_FLAG_DISCONNECT);
							close_socket(d);
							continue;
						}
					}
				}

				if (d->connected < CON_PLAYING && d->connecting_time + DESCRIPTOR_TIMEOUT < mud->current_time)
				{
					write_to_descriptor(d, "\n\r\n\rLogin time out reached.\n\r\n\r", 0);
					log_god_printf("Login timeout reached for D%d@%s", d->descriptor, d->host);
					close_socket(d);
					continue;
				}

				process_naws(d, d->mth->cols, d->mth->rows);

				if (d->character != NULL && d->character->wait > 0)
				{
					bool HighSpeed = FALSE;

					if (IS_NPC(d->character))
					{
						d->character->wait--;
						if (d->character->wait == 0)
						{
							vt100prompt(d->character);
						}
						continue;
					}

					if (d->incomm[0] == '\0')
					{
						if (d->connected >= CON_PLAYING && d->character && !IS_NPC(d->character) && who_fighting(d->character))
						{
							if (d->character->pcdata->auto_flags == AUTO_QUICK && d->character->hit > d->character->pcdata->wimpy)
							{
								HighSpeed = TRUE;
							}
						}
					}
					if (HighSpeed)
					{
						d->character->wait -= 2;
						if (d->character->wait < 0)
						{
							d->character->wait = 0;
						}
					}
					else
					{
						--d->character->wait;
						if (d->character->wait < 0)
						{
							d->character->wait = 0;
						}
					}
					if (d->character->wait == 0)
					{
						vt100prompt(d->character);
					}
					continue;
				}

				/*
					Reset characters from aliasses
				*/

				if (*d->incomm == '\0' && d->connected >= CON_PLAYING && d->character && d == d->character->desc && *d->inbuf == '\0' && d->intop == 0)
				{
					REMOVE_BIT(CH(d)->pcdata->interp, INTERP_ALIAS);

					if (d->back_buf != NULL)
					{
						d->intop = str_cpy_max(d->inbuf, d->back_buf, MAX_INPUT_LENGTH-10);
						STRFREE(d->back_buf);
						d->back_buf = NULL;
					}
					else
					{
						SET_BIT(d->mth->comm_flags, COMM_FLAG_NOINPUT);
					}
				}
				read_from_buffer(d);

				/*
					Auto reactions
				*/

				if (d->incomm[0] == '\0'
				&&  d->connected == CON_PLAYING
				&&  d->character != NULL
				&&  d->original == NULL
				&&  d == d->character->desc
				&&  d->character->wait == 0
				&&  d->character->in_room != room_index[ROOM_VNUM_LIMBO])
				{
					if (who_fighting(d->character) && d->character->pcdata->auto_flags != AUTO_OFF && d->character->hit > d->character->pcdata->wimpy)
					{
						int sn;
						bool hadtostand	= FALSE;
						bool ms			= FALSE;

						switch(d->character->position)
						{
							case POS_SLEEPING:
							case POS_RESTING:
							case POS_SITTING:
								do_stand(d->character,"");
								hadtostand = TRUE;
								break;
							case POS_STANDING:
								d->character->position = POS_FIGHTING;
								break;
							case POS_FIGHTING:
								break;
							default:
								hadtostand = TRUE;
								break;
						}
						if (hadtostand)
						{
							break;
						}

						if (!str_prefix("mass ", d->character->pcdata->auto_command))
						{
							ms = TRUE;
							sn = skill_lookup(&d->character->pcdata->auto_command[5]);
						}
						else
						{
							sn = skill_lookup(d->character->pcdata->auto_command);
						}

						if (sn != -1)
						{
							if (is_spell(sn))
							{
								if (d->character->mana > 2 * get_mana(d->character, sn))
								{
									SET_BIT(d->character->pcdata->interp, INTERP_AUTO);
									d->character->wait = 2;
									if (ms)
									{
										sprintf(dbuf, "mass '%s'", skill_table[sn].name);
									}
									else
									{
										sprintf(dbuf, "cast '%s'", skill_table[sn].name);
									}
									write_to_buffer(d, "", 0);
									str_cpy_max(d->incomm, dbuf, MAX_INPUT_LENGTH);
								}
							}
							else
							{
								if (d->character->move > 2 * skill_table[sn].cost)
								{
									SET_BIT(d->character->pcdata->interp, INTERP_AUTO);
									d->character->wait = 2;
									sprintf(dbuf, "%s", skill_table[sn].name);
									write_to_buffer(d, "", 0);
									str_cpy_max(d->incomm, dbuf, MAX_INPUT_LENGTH);
								}
							}
						}
						else
						{
							ch_printf(d->character, "%s is not a spell or skill.\n\r", d->character->pcdata->auto_command);
							log_printf("%s has auto command set to: %s", d->character->name, d->character->pcdata->auto_command);
							d->character->pcdata->auto_flags = AUTO_OFF;
							STRFREE(d->character->pcdata->auto_command);
							d->character->pcdata->auto_command = STRALLOC("");
						}
					}
					else if (*d->character->pcdata->tracking != '\0')
					{
						CHAR_DATA * rch;

						/*
							Tracking Skill
						*/

						for (rch = d->character->in_room->first_person ; rch ; rch = rch->next_in_room)
						{
							if (!strcasecmp(rch->name, d->character->pcdata->tracking) || !strcasecmp(rch->short_descr, d->character->pcdata->tracking))
							{
								send_to_char("You've found your prey!\n\r", d->character);

								if (IS_AFFECTED(d->character, AFF_HUNT) && number_percent() < d->character->pcdata->learned[gsn_greater_hunt])
								{
									if (!IS_NPC(rch))
									{
										do_murder(d->character, d->character->pcdata->tracking);
									}
									else
									{
										do_kill(d->character, rch->name);
									}
								}
								STRFREE(d->character->pcdata->tracking);
								d->character->pcdata->tracking = STRALLOC("");
								break;
							}
						}
						if (rch == NULL)
						{
							do_track(d->character, d->character->pcdata->tracking);
						}
					}
					else if (d->character->pcdata->travel >= 0)
					{
						int exit_cnt, new_exit, cnt;
						CHAR_DATA *ch;

						/*
							Travel Command
						*/

					 	if (d->character->position != POS_STANDING || d->character->fighting || d->character->hit < d->character->pcdata->wimpy)
					 	{
							d->character->pcdata->travel		= -1;
							d->character->pcdata->travel_from	= NULL;
							send_to_char("You stop traveling.\n\r", d->character);
						}
						else
						{
							ch		= d->character;
							exit_cnt	= 0;
							new_exit	= 0;
							cnt		= 0;

							for (; cnt < 6 ; cnt++)
							{
								if (cnt != rev_dir[d->character->pcdata->travel] && get_exit(ch->in_room->vnum, cnt) != NULL)
								{
									if (can_move_char(ch, cnt))
									{
										if (ch->pcdata->travel_from == NULL || ch->pcdata->travel_from != room_index[ch->in_room->exit[cnt]->to_room])
										{
											new_exit = cnt;
											exit_cnt++;
										}
									}
								}
							}

							if (exit_cnt != 1)
							{
								if (exit_cnt < 1)
								{
									if (ch->move > 15)
									{
										send_to_char("You stop traveling due to a dead end.\n\r", ch);
									}
									else
									{
										send_to_char("You stop traveling to rest.\n\r", ch);
									}
								}
								else
								{
									send_to_char("You stop traveling to pick directions.\n\r", ch);
								}
								ch->pcdata->travel		= -1;
								ch->pcdata->travel_from	= NULL;
							}
							else
							{
								ch->pcdata->travel		= new_exit;
								ch->pcdata->travel_from	= ch->in_room;

								ch_printf(ch, "You travel %s.\n\r", dir_name[new_exit]);
								move_char(ch, new_exit, TRUE);
							}
						}
					}
				}

				/*
					Check input
				*/
				if (d->incomm[0] != '\0')
				{
					if (IS_SET(d->mth->comm_flags, COMM_FLAG_NOINPUT))
					{
						REMOVE_BIT(d->mth->comm_flags, COMM_FLAG_NOINPUT);
						SET_BIT(d->mth->comm_flags, COMM_FLAG_COMMAND);
					}
					stop_idling(d);

					if (!IS_SET(d->mth->comm_flags, COMM_FLAG_DISCONNECT))
					{
						if (d->connected == CON_PLAYING)
						{
							interpret(d->character, d->incomm);
						}
						else if (d->connected == CON_EDITING)
						{
							edit_buffer(d->character, d->incomm);
						}
						else
						{
							if (nanny(d, d->incomm))
							{
								continue;
							}
						}
					}

				}
			}
		}

		close_timer(TIMER_PROCESS_INPUT);

		/*
			Prompt and Tactical.
		*/

		start_timer(TIMER_TACTICAL_UPD);

		if (mud->f_desc != NULL && !IS_SET(mud->flags, MUD_EMUD_BOOTING))
		{
			for (d = mud->f_desc ; d ; d = mud->update_gld)
			{
				mud->update_gld = d->next;

				*d->incomm = '\0';

				if ((IS_SET(d->mth->comm_flags, COMM_FLAG_COMMAND) || d->outtop > 0) && FD_ISSET(d->descriptor, &out_set))
				{
					if (!process_output(d, TRUE) || IS_SET(d->mth->comm_flags, COMM_FLAG_DISCONNECT))
					{
						d->outtop = 0;
						close_socket(d);
					}
				}
			}
		}


		if (mud->f_desc != NULL && !IS_SET(mud->flags, MUD_EMUD_BOOTING))
		{
			for (d = mud->f_desc ; d ; d = mud->update_gld)
			{
				mud->update_gld = d->next;

				if (!IS_SET(d->mth->comm_flags, COMM_FLAG_DISCONNECT) && d->character && d->connected >= CON_PLAYING && CH(d)->pcdata && IS_SET(CH(d)->pcdata->interp, INTERP_TACT_UPDATE))
				{
					vt100prompter(d->character);
				}
			}
		}

		close_timer(TIMER_TACTICAL_UPD);

		start_timer(TIMER_PROCESS_OUTPUT);

		if (mud->f_desc != NULL && !IS_SET(mud->flags, MUD_EMUD_BOOTING))
		{
			for (d = mud->f_desc ; d ; d = mud->update_gld)
			{
				mud->update_gld = d->next;

				if (IS_SET(d->mth->comm_flags, COMM_FLAG_DISCONNECT))
				{
					continue;
				}
				if (d->outtop)
				{
					if (d->outbuf[d->outtop - 1] != '\r')
					{
						d->outbuf[d->outtop++] = IAC;
						d->outbuf[d->outtop++] = EOR;
					}
					write_to_port(d);
				}
			}
		}

		close_timer(TIMER_PROCESS_OUTPUT);

		/*
			pause the loop
		*/

		gettimeofday(&now_time, NULL);

		if (now_time.tv_sec == last_time.tv_sec)
		{
			usec_gameloop = now_time.tv_usec - last_time.tv_usec;
		}
		else
		{
			usec_gameloop = 1000000 - last_time.tv_usec + now_time.tv_usec;
		}

		usec_gamewait = 1000000 / PULSE_PER_SECOND - usec_gameloop;

		stall_time.tv_usec = usec_gamewait;

		mud->total_io_ticks++;
		mud->total_io_exec  += usec_gameloop;
		mud->total_io_delay += usec_gamewait;

		if (usec_gamewait >= 0)
		{
			if (select(0, NULL, NULL, NULL, &stall_time) < 0)
			{
				perror("Game_loop: select: stall");
			}
		}
		else
		{
			/*
			log_printf("game_loop_unix: heartbeat violation of %lld usec", usec_gameloop - 1000000LL / PULSE_PER_SECOND);
			*/
		}
	}
	pop_call();
	return;
}


void init_descriptor (DESCRIPTOR_DATA *dnew, int desc)
{
	dnew->descriptor			= desc;
	dnew->connected			= CON_GET_NAME;
	dnew->connecting_time		= mud->current_time;
	dnew->port_size			= 10000;
	dnew->inlast				= STRDUPE(str_empty);
}

void new_descriptor(void)
{
	DESCRIPTOR_DATA *dnew, *dtmp;
	BAN_DATA *pban;
	struct sockaddr_in sock;
	int desc, sockbuf;
	socklen_t size;
	struct linger ld;

	size = sizeof(sock);

	push_call("new_descriptor(void)");

	getsockname(mud->control, (struct sockaddr *) &sock, &size);

	if ((desc = accept(mud->control, (struct sockaddr *) &sock, &size)) < 0)
	{
		perror("New_descriptor: accept");
		pop_call();
		return;
	}

	if (fcntl(desc, F_SETFL, O_NDELAY|O_NONBLOCK) == -1)
	{
		perror("new_descriptor: fcntl: O_NDELAY|O_NONBLOCK");
		pop_call();
		return;
	}

	sockbuf = 2048;

	if (setsockopt(desc, SOL_SOCKET, SO_RCVBUF, (char *) &sockbuf, sizeof(sockbuf)) < 0)
	{
		perror("new_socket: SO_RCVBUF");
	}

	ld.l_onoff  = 0;
	ld.l_linger = 100;

	if (setsockopt(desc, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld)) < 0)
	{
		perror("new_socket: SO_LINGER");
	}

	/*
		Cons a new descriptor.
	*/

	ALLOCMEM(dnew, DESCRIPTOR_DATA, 1);

	init_descriptor (dnew, desc);

	init_mth_socket(dnew);

	size = sizeof(sock);

	if (getpeername(desc, (struct sockaddr *) &sock, &size) < 0)
	{
		perror("new_descriptor: getpeername");
		dnew->host   = STRALLOC("(unknown)");
		SET_BIT(dnew->mth->comm_flags, COMM_FLAG_DISCONNECT);
	}
	else
	{
		int addr, address[4];
		char buf[MAX_INPUT_LENGTH];

		addr = ntohl(sock.sin_addr.s_addr);

		address[0] = (addr >> 24) & 0xFF ;
		address[1] = (addr >> 16) & 0xFF ;
		address[2] = (addr >>  8) & 0xFF ;
		address[3] = (addr      ) & 0xFF ;

		sprintf(buf, "%d.%d.%d.%d", address[0], address[1], address[2], address[3]);
		dnew->host = STRALLOC(buf);

		log_god_printf("New connect: %s D%d", dnew->host , dnew->descriptor);
	}

	/*
		Init descriptor data.
	*/

	for (dtmp = mud->f_desc ; dtmp ; dtmp = dtmp->next)
	{
		if (strcmp(dnew->host, dtmp->host) < 0)
		{
			INSERT_LEFT(dnew, dtmp, mud->f_desc, next, prev);
			break;
		}
	}

	if (dtmp == NULL)
	{
		LINK(dnew, mud->f_desc, mud->l_desc, next, prev);
	}

	mud->total_desc++;

	for (pban = mud->f_ban ; pban ; pban = pban->next)
	{
		if (!str_prefix(pban->name, dnew->host))
		{
			write_to_descriptor(dnew, "\n\rYour site has been banned from this Mud.\n\r\n\r", 0);
			log_god_printf("Ban prefix '%s' for '%s', denying access.", pban->name, dnew->host);
			close_socket(dnew);

			pop_call();
			return;
		}
	}

	if (mud->total_desc > MAX_LINKPERPORT)  /* Limit descriptors  */
	{
		char buf[MAX_STRING_LENGTH];

		sprintf(buf,"\n\rSorry, the limit of %d players online has been reached.\n\rPlease try back later.\n\r\n\r", MAX_LINKPERPORT);
		write_to_descriptor(dnew, buf, 0);
		close_socket(dnew);
		pop_call();
		return;
	}

	/*
		Inform the client about emud's telnet support
	*/

	/*
		Send the greeting.
	*/
	{
		char buf[40];

		sprintf(buf, "greeting%d", number_range(1, 8));

		force_help(dnew, buf);
	}
	pop_call();
	return;
}


void close_socket(DESCRIPTOR_DATA *dclose)
{
	CHAR_DATA *ch;
	DESCRIPTOR_DATA *dt;

	push_call("close_socket(%p)",dclose);

	for (dt = mud->f_desc ; dt ; dt = dt->next)
	{
		if (dt == dclose)
		{
			break;
		}
	}

	uninit_mth_socket(dclose);

	if (dt == NULL)
	{
		log_string("close_socket: unlinked desc called in close_socket");
		pop_call();
		return;
	}

	if (!IS_SET(dclose->mth->comm_flags, COMM_FLAG_DISCONNECT))
	{
		write_to_port(dclose);
	}

	if (dclose->snoop_by != NULL)
	{
		write_to_buffer(dclose->snoop_by, "Your victim has left the game.\n\r", 0);
		dclose->snoop_by = NULL;
	}

	for (dt = mud->f_desc ; dt ; dt = dt->next)
	{
		if (dt->snoop_by == dclose)
		{
			dt->snoop_by = NULL;
		}
	}

	if (dclose->character)
	{
		if (dclose->character->desc)
		{
			do_return(dclose->character, NULL);
		}
		ch = dclose->character;

		log_god_printf("Closing link to %s@%s D%d Connected %d.",
			ch->name,
			dclose->host,
			dclose->descriptor,
			dclose->connected);

		if (dclose->connected >= CON_PLAYING)
		{
			act("$n has lost $s link.", ch, NULL, NULL, TO_ROOM);
			ch->desc = NULL;
		}
		else
		{
			dclose->character = NULL;

			if (get_char_pvnum(ch->pcdata->pvnum) == NULL)
			{
				ch->desc = NULL;
				extract_char(ch);
			}
		}
	}

	dclose->outtop = 0;

	if (dclose == mud->update_gld)
	{
		mud->update_gld = dclose->next;
	}

	if ((dclose->prev == NULL && dclose != mud->f_desc)
	||  (dclose->next == NULL && dclose != mud->l_desc))
	{
		log_printf("UNLINK ERROR unlinking descriptor %d.", dclose->descriptor);
	}
	else
	{
		UNLINK(dclose, mud->f_desc, mud->l_desc, next, prev);
	}

	mud->total_desc--;

	close(dclose->descriptor);

	STRFREE(dclose->host);
	STRFREE(dclose->inlast);

	if (dclose->back_buf)
	{
		STRFREE(dclose->back_buf);
	}
	FREEMEM(dclose);

	pop_call();
	return;
}

bool read_from_descriptor(DESCRIPTOR_DATA *d)
{
	CHAR_DATA *ch;
	char bufin[MAX_INPUT_LENGTH];
	int nRead;

	push_call("read_from_descriptor(%p)",d);

	ch = d->original ? d->original : d->character;

	/*
		Hold horses if pending command already.
	*/

	if (d->incomm[0] != '\0')
	{
		pop_call();
		return TRUE;
	}

	if (ch && d->connected >= CON_PLAYING && ch->trust != ch->level)
	{
		ch->trust = ch->level;
	}

	/*
		Was there anything here to begin with ?
	*/

	if (d->back_buf)
	{
		pop_call();
		return(TRUE);
	}

	/*
		Check for overflow.
	*/

	if (d->intop > MAX_INPUT_LENGTH -100)
	{
		log_god_printf("%s input overflow!", d->host);
		write_to_descriptor(d, "\n\r*** PUT A LID ON IT!!! ***\n\rYou have just overflowed your buffer.  You may get back on the game.\n\r", 0);

		pop_call();
		return FALSE;
	}

	/*
		Snarf input.
	*/

	while (TRUE)
	{
		nRead = read(d->descriptor, bufin, MAX_INPUT_LENGTH);

		bufin[nRead] = 0;

		if (nRead > 0)
		{
			d->intop += translate_telopts(d, (unsigned char *)bufin, nRead, (unsigned char *) d->inbuf, d->intop);

			pop_call();
			return TRUE;
		}
		else if (nRead == 0)
		{
			pop_call();
			return FALSE;
		}
		else if (errno == EWOULDBLOCK)
		{
			break;
		}
		else
		{
			log_god_printf("Read_from_descriptor D%d@%s errno %d", d->descriptor, d->host, errno);
			pop_call();
			return FALSE;
		}
	}
	pop_call();
	return TRUE;
}

/*
	Transfer one line from input buffer to input line.
*/

void read_from_buffer(DESCRIPTOR_DATA *d)
{
	int i, k;
	CHAR_DATA *ch,*sh;

	/*
		Hold horses if pending command already.
	*/

	push_call("read_from_buffer(%p)",d);

	if (d->incomm[0] != '\0')
	{
		pop_call();
		return;
	}

	if (d->inbuf[0] == '\0')
	{
		if (d->intop > 0)
		{
			fprintf(stderr, "%12s: ITE: %3d %3d %3d %3d %3d %3d %3d %3d %3d\n",
				CH(d) ? CH(d)->name : "Nanny",
				d->inbuf[0], d->inbuf[1], d->inbuf[2],
				d->inbuf[3], d->inbuf[4], d->inbuf[5],
				d->inbuf[6], d->inbuf[7], d->inbuf[8]);

			memmove(&d->inbuf[0], &d->inbuf[1], MAX_INPUT_LENGTH - 1);

			d->intop--;
		}
		pop_call();
		return;
	}

	ch = d->original ? d->original : d->character;

	/*
		Look for at least one new line.
	*/

	if (d->intop < MAX_INPUT_LENGTH -10)
	{
		for (i = 0 ; d->inbuf[i] != '\n' ; i++)
		{
			if (i >= d->intop)
			{
				pop_call();
				return;
			}

			if (d->inbuf[i] == '\0')
			{
				log_god_printf("Read_from_buffer D%d@%s NULL byte in input string.", d->descriptor, d->host);

				write_to_buffer(d, "Input error.\n\r", 0);
				
				d->inbuf[0] = 0;
				d->intop = 0;

				pop_call();
				return;
			}
		}
	}

	for (i = k = 0 ; d->inbuf[i] != '\n' ; i++)
	{
		if (i >= MAX_INPUT_LENGTH - 20)
		{
			write_to_buffer(d, "Line too long.\n\r", 0);

			d->inbuf[i]    = '\n';
			d->inbuf[i+1]  = '\0';
			d->intop       = i+1;
			break;
		}

		if (d->inbuf[i] == '\b' && k > 0)
		{
			--k;
		}
		else
		{
			d->incomm[k++] = d->inbuf[i];
		}
	}

	/*
		Finish off the line.
	*/
	if (k == 0)
	{
		d->incomm[k++] = ' ';
	}

	d->incomm[k] = '\0';

	/*
		Do '!' substitution.
	*/

	if (d->connected >= CON_PLAYING && !IS_SET(ch->pcdata->interp, INTERP_ALIAS))
	{
		if (d->incomm[0] == '.' || d->incomm[0] == '!')
		{
			if (isalpha(d->incomm[1]))
			{
				str_cpy_max(d->incomm, ch->pcdata->back_buf[tolower(d->incomm[1]) - 'a'], MAX_INPUT_LENGTH);
			}
			else
			{
				strcpy(d->incomm, d->inlast);
			}
		}
		else
		{
			if (isalpha(d->incomm[0]))
			{
				RESTRING(d->inlast, d->incomm);

				STRFREE(ch->pcdata->back_buf[tolower(d->incomm[0]) - 'a']);
				ch->pcdata->back_buf[tolower(d->incomm[0]) - 'a'] = STRDUPE(d->inlast);
			}
		}
	}

	if (d->snoop_by && d->character)
	{
		sh = d->snoop_by->original ? d->snoop_by->original : d->snoop_by->character;

		if (sh && sh->desc && sh->desc->character == sh)
		{
			ch_printf_color(sh, "{168}%s {078}[{178}%s{078}]\n\r", ch->name, d->incomm);
		}
	}

	d->intop -= i + 1;

	memmove(d->inbuf, d->inbuf + i + 1, d->intop);

	d->inbuf[d->intop] = 0;

	pop_call();
	return;
}

void do_copyover (CHAR_DATA *ch, char * argument)
{
	FILE 		*fp;
	PLAYER_GAME	*gpl;
	DESCRIPTOR_DATA *d, *d_next;
	char buf[100], cport[10], ccontrol[10], cboot[20];

	push_call("do_copyover(%p,%p)",ch,argument);

	log_printf("COPYOVER by %s: port = %d, control = %d", ch->name, mud->port, mud->control);

	fp = my_fopen (COPYOVER_FILE, "w", FALSE);

	if (!fp)
	{
		perror(COPYOVER_FILE);
		ch_printf(ch, "Copyover file not writeable, aborted.\n\r");

		pop_call();
		return;
	}

	sprintf(buf, "\n\r *** COPYOVER by %s - please remain seated!\n\r", ch->name);

	save_world();

	log_printf("COPYOVER: Saving players...");

	for (gpl = mud->f_player ; gpl ; gpl = gpl->next)
	{
		save_char_obj(gpl->ch, NORMAL_SAVE);
	}

	log_printf("COPYOVER: Saving descriptors.");

	for (d = mud->f_desc ; d ; d = d_next)
	{
		d_next = d->next;

		if (!d->character || d->connected < 0)
		{
			write_to_descriptor (d, "\n\rSorry, we are rebooting. Come back in one minute.\n\r", 0);
			close_socket(d);
		}
		else
		{
			fprintf (fp, "D %d %s~ %s~\n",
				d->descriptor,
				CH(d)->name,
				d->host);

			uninit_mth_socket(d);

			write_to_descriptor(d, buf, 0);
		}
	}
	fprintf(fp, "$\n");
	my_fclose (fp);

	my_fclose(fpReserve);
	my_fclose(fpAppend);

	/*
		exec - descriptors are inherited
	*/

	sprintf(cport,      "%d", mud->port);
	sprintf(ccontrol,   "%d", mud->control);
	sprintf(cboot,     "%ld", mud->boot_time);

	execl(REAL_FILE, "lola", "-p", cport, "-c", ccontrol, "-b", cboot, (char *) NULL);

	perror ("do_copyover: execl");
	send_to_char ("Copyover FAILED!\n\r",ch);

	/*
		Here you might want to reopen fpReserve
	*/

	if ((fpReserve = my_fopen(NULL_FILE, "r", FALSE)) == NULL)
	{
		log_printf("do_copyover(): fpReserve -> Could not open %s to read.",NULL_FILE);
		perror(NULL_FILE);
		abort();
	}

	if ((fpAppend = my_fopen(NULL_FILE, "r", FALSE)) == NULL)
	{
		log_printf("do_copyover(): fpAppend -> Could not open %s to read.",NULL_FILE);
		perror(NULL_FILE);
		abort();
	}
	pop_call();
	return;
}

/* Recover from a copyover - load players */

void copyover_recover ()
{
	DESCRIPTOR_DATA *d;
	FILE *fp;
	char *name, letter;
	int desc;
	bool fOld;

	log_printf ("Copyover recovery initiated");

	fp = my_fopen (COPYOVER_FILE, "r", FALSE);

	/*
		there are some descriptors open which will hang forever then ?
	*/

	if (!fp)
	{
		perror ("copyover_recover:my_fopen");
		log_printf ("Copyover file not found. Exitting.\n\r");
		exit (1);
	}

	while (TRUE)
	{
		letter = fread_letter(fp);

		if (letter != 'D')
		{
			if (letter != '$')
			{
				log_printf("***WARNING*** copyover_recovery: bad format");
			}
			fread_to_eol(fp);
			break;
		}

		desc				= fread_number(fp);

		ALLOCMEM(d, DESCRIPTOR_DATA, 1);

		init_descriptor(d, desc);

		name			= fread_string(fp);
		d->host			= fread_string(fp);
		d->connected		= CON_COPYOVER_RECOVER;

		init_mth_socket(d);

		LINK(d, mud->f_desc, mud->l_desc, next, prev);

		mud->total_desc++;

		/* Now, find the pfile */

		fOld = load_char_obj (d, name);

		STRFREE(name);

		if (!fOld) /* Player file not found?! */
		{
			write_to_descriptor (d, "\n\rSomehow, your character was lost in the copyover. Sorry.\n\r", 0);
			close_socket(d);
		}
		else
		{
			if (!d->character->in_room)
			{
				d->character->in_room = get_room_index(ROOM_VNUM_TEMPLE);
			}

			d->connected = CON_PLAYING;

			add_char (d->character);
			add_player(d->character);

			char_to_room(d->character, d->character->pcdata->was_in_room);

			vt100prompt(d->character);

			enter_game(d->character);

			ch_printf(d->character, "%s", "\033[0;37mHave a nice day.\n\r");
		}
	}
	my_fclose(fp);

	unlink(COPYOVER_FILE);

	log_printf("Copyover recovery complete");
}

/*
	Low level output function.
*/

bool process_output(DESCRIPTOR_DATA *d, bool fPrompt)
{
	char buf[MAX_STRING_LENGTH];

	push_call("process_output(%p,%p)",d,fPrompt);

	if (d == NULL)
	{
		pop_call();
		return FALSE;
	}

	/*
		Bust a prompt.
	*/

	if (fPrompt && !IS_SET(mud->flags, MUD_EMUD_DOWN) && d->connected == CON_PLAYING)
	{
		CHAR_DATA *ch, *sh;

		ch = CH(d);
		sh = d->character;

		if (!is_desc_valid(sh))
		{
	  		pop_call();
			return FALSE;
		}

		if (!HAS_BIT(ch->pcdata->vt100_flags, VT100_INTERFACE))
		{
			if (IS_SET(ch->act, PLR_BLANK))
			{
				sprintf(buf, "%s\n\r", get_color_string(ch, COLOR_PROMPT, VT102_DIM));
			}
			else
			{
				strcpy(buf, "");
			}

			strcat(buf, prompt_return(sh, ch->pcdata->prompt_layout));

			SET_BIT(ch->pcdata->interp, INTERP_SCROLL);

			write_to_buffer(d, buf, 1000000);

			REMOVE_BIT(ch->pcdata->interp, INTERP_SCROLL);
		}
		else
		{
			vt100prompt(ch);
		}
	}
	pop_call();
	return TRUE;
}

/*
	The buffer that works with the page pauser
*/

int pager(DESCRIPTOR_DATA *d, const char *istr, int lng, char *ostr)
{
	int lines, pt, breakpt, lengt, cnt;
	char pag_buf[MAX_STRING_LENGTH], buf[MAX_INPUT_LENGTH], lcc[10];
	register char *ptt, *pto, *pti;

	push_call("pager(%p,%p,%p,%p)",d,istr,lng,ostr);

	if (d->connected < CON_PLAYING
	||  CH(d)->pcdata->last_command
	||  IS_SET(CH(d)->act, PLR_PAGER)
	||  IS_SET(CH(d)->pcdata->interp, INTERP_PAGE)
	||  IS_SET(CH(d)->pcdata->interp, INTERP_SCROLL))
	{
		REMOVE_BIT(CH(d)->pcdata->interp, INTERP_PAUSE);

		pop_call();
		return (str_cpy_max(ostr, istr, MAX_STRING_LENGTH));
	}

	for (ptt = (char *)istr, pt = 0, lines = 0 ; pt < lng ; pt++, ptt++)
	{
		if (*ptt == '\n')
		{
			lines++;
		}
	}

	breakpt = get_pager_breakpt(CH(d));

	if (lines <= breakpt+1)
	{
		REMOVE_BIT(CH(d)->pcdata->interp, INTERP_PAUSE);

		pop_call();
		return (str_cpy_max(ostr, istr, MAX_STRING_LENGTH));
	}

	SET_BIT(CH(d)->pcdata->interp, INTERP_PAUSE);

	pti = (char *)istr;
	pto = ostr;

	lcc[0] = '\0';

	for (pt = 0, lines = 0 ; lines < breakpt ; pt++, pti++, pto++)
	{
		*pto = *pti;

		if (*pti == '\033' && *(pti+1) == '[')
		{
			for (cnt = 0 ; *(pti+cnt) != 'm' && *(pti+cnt) != '\0' ; cnt++)
			{
				lcc[cnt] = *(pti+cnt);
			}
			lcc[cnt] = 'm';
			cnt++;
			lcc[cnt] = '\0';
		}
		if (*pti == '\n')
		{
			lines++;
		}
	}

	if (*pti == '\r')
	{
		*pto = *pti;
		pto++;
		pti++;
		pt++;
	}

	lengt = pt;

	*pto = '\0';

	for (pto = pag_buf, cnt = 0 ; lcc[cnt] != '\0' ; cnt++, pto++)
	{
		*pto = lcc[cnt];
	}

	for (; pt < lng ; pto++, pti++, pt++)
	{
		*pto = *pti;
	}
	*pto = '\0';

	if (!HAS_BIT(CH(d)->pcdata->vt100_flags, VT100_INTERFACE))
	{
		sprintf(buf, "%s --------------------------[Press Return to Continue]--------------------------%s\n\r", ansi_translate_text(CH(d), "{128}"), ansi_translate_text(CH(d), "{300}"));
		str_apd_max(ostr, buf, lengt, MAX_STRING_LENGTH);
	}
	STRFREE(CH(d)->pcdata->page_buf);
	CH(d)->pcdata->page_buf = STRALLOC(pag_buf);

	SET_BIT(CH(d)->pcdata->interp, INTERP_PAGE);

	pop_call();
	return(lengt);
}

/*
	The buffer used for the grep command.
*/

void scroll(DESCRIPTOR_DATA *d, const char *txi, int lng)
{
	PC_DATA *pch;
	char buf[MAX_STRING_LENGTH], *pti, *pto;

	pti = (char *) txi;
	pto = (char *) buf;

	push_call("scroll(%p,%p,%p)",d,txi,lng);

	if (d->connected != CON_PLAYING)
	{
		pop_call();
		return;
	}

	pch = CH(d)->pcdata;

	if (IS_SET(pch->interp, INTERP_SCROLL))
	{
		pop_call();
		return;
	}

	while (*pti)
	{
		if (pti[0] == '\n' && pti[1] == '\r')
		{
			if (pti[2] == '\033')
			{
				*pto = 0;

				pch->scroll_buf[pch->scroll_end] = STRALLOC(buf);

				pch->scroll_end++;

				if (pch->scroll_end >= MAX_SCROLL_BUF)
				{
					pch->scroll_beg = 1;
					pch->scroll_end = 0;
				}

				if (pch->scroll_buf[pch->scroll_end])
				{
					STRFREE(pch->scroll_buf[pch->scroll_end]);
				}

				pto = buf;
				pti += 2;
			}
			else if (pti[2] == 0)
			{
				*pto = 0;

				pch->scroll_buf[pch->scroll_end] = STRALLOC(buf);

				pch->scroll_end++;

				if (pch->scroll_end >= MAX_SCROLL_BUF)
				{
					pch->scroll_beg = 1;
					pch->scroll_end = 0;
				}

				if (pch->scroll_buf[pch->scroll_end])
				{
					STRFREE(pch->scroll_buf[pch->scroll_end]);
				}
				break;
			}
			else
			{
				*pto++ = *pti++;
			}
		}
		else
		{
			*pto++ = *pti++;
		}
	}


	pop_call();
	return;
}

char *ansi_strip(char *txi)
{
	register char *pti, *pto;

	push_call("ansi_strip(%p)",txi);

	pti = (char *)txi;
	pto = (char *)ansi_strip_txt;

	for (; *pti != '\0' ; pti++)
	{
		if (*pti == '\033' && *(pti+1) == '[')
		{
			while (*pti != 'm' && *pti != '\0')
			{
				pti++;
			}
			continue;
		}
		*pto = *pti;
		pto++;
	}
	*pto = '\0';

	pop_call();
	return (ansi_strip_txt);
}

/*
	Append onto an output buffer.
*/

void write_to_buffer(DESCRIPTOR_DATA *d, char *txt, int length)
{
	char buf[MAX_STRING_LENGTH];
	char txo[MAX_STRING_LENGTH];
	CHAR_DATA *ch, *sh;
	int size;

	push_call("write_to_buffer(%p,%p,%p)",d,txt,length);

	if (d == NULL || txt == NULL)
	{
		pop_call();
		return;
	}

	ch = CH(d);

	if (IS_SET(mud->flags, MUD_SKIPOUTPUT) && length != 1000000)
	{
		if (ch == NULL || IS_NPC(ch) || ch->level < MAX_LEVEL || !IS_SET(ch->act, PLR_WIZTIME))
		{
			pop_call();
			return;
		}
	}

	if (ch)
	{
		if (HAS_BIT(ch->pcdata->vt100_flags, VT100_INTERFACE) && d->connected >= CON_PLAYING && length != 1000000)
		{
			if (txt[0] == '\0')
			{
				pop_call();
				return;
			}
			size = UMIN(strlen(txt), MAX_STRING_LENGTH - 200);

			size = pager(d, txt, size, txo);

			if (length != 1000000)
			{
				scroll(d, txo, size);
			}

			buf[0] = 0;

			if (d->outtop == 0 && IS_SET(d->mth->comm_flags, COMM_FLAG_COMMAND))
			{
				strcat(buf, "\033[0K");
			}

			if (d->outtop == 0)
			{
				cat_sprintf(buf, "\0337\033[%d;1H%s%s\0338", ch->pcdata->screensize_v - 2, IS_SET(ch->act, PLR_BLANK) ? "\n\r" : "", txo);
			}
			else
			{
				cat_sprintf(buf, "\0337\033[%d;1H%s\0338", ch->pcdata->screensize_v - 2, txo);
			}

			if (IS_SET(ch->pcdata->interp, INTERP_PAUSE))
			{
				cat_sprintf(buf, "%s --------------------------[Press Return to Continue]--------------------------%s", ansi_translate_text(ch, "{128}"), get_color_string(ch, COLOR_PROMPT, VT102_DIM));
			}
		}
		else
		{
			if (!HAS_BIT(ch->pcdata->vt100_flags, VT100_INTERFACE) && d->connected >= CON_PLAYING && d->outtop == 0 && !IS_SET(d->mth->comm_flags, COMM_FLAG_COMMAND))
			{
				d->outbuf[0] = '\n';
				d->outbuf[1] = '\r';
				d->outbuf[2] = '\0';
				d->outtop = 2;
			}

			size = UMIN(strlen(txt), MAX_STRING_LENGTH - 100);

			size = pager(d, txt, size, txo);

			if (length != 1000000)
			{
				scroll(d, txo, size);
			}
			strcpy(buf, txo);
		}

		if (d->snoop_by && length != 1000000)
		{
			sh = CH(d->snoop_by);

			if (sh && sh->desc && sh->desc->character == sh)
			{
				if (IS_SET(CH(d)->pcdata->interp, INTERP_PAUSE))
				{
					send_to_char(txo, sh);
				}
				else
				{
					send_to_char(txt, sh);
				}
			}
		}
	}
	else
	{
		if (d->outtop == 0 && d->connected >= CON_PLAYING && !IS_SET(d->mth->comm_flags, COMM_FLAG_COMMAND))
		{
			d->outbuf[0] = '\n';
			d->outbuf[1] = '\r';
			d->outbuf[2] = '\0';
			d->outtop    = 2;
		}
		strcpy(buf, txt);
	}

	d->outtop = str_apd_max(d->outbuf, buf, d->outtop, MAX_STRING_LENGTH);

	pop_call();
	return;
}


bool write_to_descriptor(DESCRIPTOR_DATA *d, char *txt, int length)
{
	push_call("write_to_descriptor(%p,%p,%p)",d,txt,length);

	if (length == 0)
	{
		length = UMIN(strlen(txt), MAX_STRING_LENGTH - d->outtop - 10);
	}
	else
	{
		length = UMIN(length, MAX_STRING_LENGTH - d->outtop - 10);
	}
	memcpy(&d->outbuf[d->outtop], txt, length);

	d->outtop += length;

	write_to_port(d);

	pop_call();
	return TRUE;
}


/*
	Write text to the file descriptor utilizing the port size feature.
	Used to flush the outbuf buffer to the actual socket.
	Chaos 5/17/95
*/

void write_to_port(DESCRIPTOR_DATA *d)
{
	int nWrite, tWrite, nBlock, failure = FALSE;

	push_call("write_to_port(%p)",d);

	if (IS_SET(d->mth->comm_flags, COMM_FLAG_DISCONNECT))
	{
		pop_call();
		return;
	}

	if (d->mth->mccp2)
	{
		write_mccp2(d, d->outbuf, d->outtop);

		d->outtop = 0;

		pop_call();
		return;
	}

	for (tWrite = 0 ; tWrite < d->outtop ; tWrite += nWrite)
	{
		nBlock = UMIN(d->outtop - tWrite, d->port_size);

		if ((nWrite = write(d->descriptor, d->outbuf + tWrite, nBlock)) < 1)
		{
			if (errno != EAGAIN && errno != ENOSR)
			{
				log_god_printf("write_to_port D%d@%s", d->descriptor, d->host);
				dump_stack();
				SET_BIT(d->mth->comm_flags , COMM_FLAG_DISCONNECT);
			}
			else
			{
				failure = TRUE;
			}
			break;
		}
	}

	if (failure)
	{
		if (++d->timeout < 60)
		{
			log_god_printf("write_to_port: timeout D%d@%s", d->descriptor, d->host);
			SET_BIT(d->mth->comm_flags , COMM_FLAG_DISCONNECT);
		}
		else
		{
			pop_call();
			return;
		}
	}

	d->timeout = 0;

	mud->total_io_bytes += tWrite;

	d->outtop	 = 0;
	pop_call();
	return;
}

/*
	Scandum - 03-05-2002
*/

void display_empty_screen(DESCRIPTOR_DATA *d)
{
	push_call("display_empty_screen(%p)",d);

	if (d->character == NULL || !HAS_BIT(d->character->pcdata->vt100_flags, VT100_INTERFACE))
	{
		write_to_buffer(d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
		write_to_buffer(d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
		write_to_buffer(d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
		write_to_buffer(d, "\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r", 1000000);
	}
	else
	{
		write_to_buffer(d, "\033[2J", 1000000);
	}
	pop_call();
	return;
}

void display_class_selections(DESCRIPTOR_DATA *d)
{
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *ch;
	int iClass;

	push_call("display_class_selection(%p)",d);

	ch = d->character;

	strcpy(buf, "{038}You may choose from the following classes, or type help [class] to learn more:\n\r\n\r");
	write_to_buffer(d, (char *)ansi_translate_text(ch, buf), 1000000);

	strcpy(buf, "{168}     ");

	for (iClass = 0 ; iClass < MAX_CLASS ; iClass++)
	{
		if (strlen(buf) == 70)
		{
			strcat(buf, "\n\r{168}     ");
		}
		cat_sprintf(buf, "%-15s", class_table[iClass].name);
	}
	strcat(buf, "\n\r\n\r{038}Please choose a class: {118}");
	write_to_buffer(d, (char *) ansi_translate_text(ch, buf), 1000000);

	pop_call();
	return;
}

void display_class_details(DESCRIPTOR_DATA *d, int class)
{
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *ch;
	int iRace;

	push_call("display_class_details(%p,%p)",d,class);

	ch = d->character;

	display_empty_screen(d);

	strcpy(buf, class_table[class].name);
	strcat(buf, "intro");
	do_help(ch, buf);

	sprintf(buf, "\n\r{038}The following races can become %ss:\n\r\n\r{168}", class_table[class].name);

	write_to_buffer(d, (char *)ansi_translate_text(ch, buf), 1000000);

	sprintf(buf, "{168}     ");

	for (iRace = 0 ; iRace < MAX_RACE ; iRace++)
	{
		if (race_table[iRace].race_class[class] == 0)
		{
			continue;
		}

		if (ch->pcdata->reincarnation < race_table[iRace].enter || ch->pcdata->reincarnation > race_table[iRace].leave)
		{
			continue;
		}

		if (strlen(buf) == 70)
		{
			strcat(buf, "\n\r{168}     ");
		}
		cat_sprintf(buf, "%-15s", race_table[iRace].race_name);
	}
	strcat(buf, "\n\r\n\r");

	write_to_buffer(d, (char *)ansi_translate_text(ch, buf), 1000000);

	display_class_selections(d);

	pop_call();
	return;
}

void display_race_selections(DESCRIPTOR_DATA *d)
{
	char buf[MAX_INPUT_LENGTH];
	sh_int iRace;
	CHAR_DATA *ch;

	push_call("display_race_selections(%p)", d);

	ch = d->character;

	strcpy(buf, "\n\r{038}You may choose from the following races, or type help [race] to learn more:\n\r\n\r");
	write_to_buffer(d, (char *)ansi_translate_text(ch, buf), 1000000);

	strcpy(buf, "{168}     ");

	for (iRace = 0 ; iRace < MAX_RACE ; iRace++)
	{
		if (race_table[iRace].race_class[ch->class] == 0)
		{
			continue;
		}

		if (ch->pcdata->reincarnation < race_table[iRace].enter || ch->pcdata->reincarnation > race_table[iRace].leave)
		{
			continue;
		}

		if (strlen(buf) == 70)
		{
			strcat(buf, "\n\r{168}     ");
		}
		cat_sprintf(buf, "%-15s", race_table[iRace].race_name);
	}
	write_to_buffer(d, (char *)ansi_translate_text(ch, buf), 1000000);

	strcpy(buf, "\n\r\n\r{038}Please choose a race: {118}");
	write_to_buffer(d, (char *)ansi_translate_text(ch, buf), 1000000);

	pop_call();
	return;
}

void display_race_details(DESCRIPTOR_DATA *d, int race)
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *ch;
	int cnt;

	push_call("display_race_details(%p,%p)",d,race);

	ch = d->character;

	display_empty_screen(d);

	strcpy(buf, race_table[race].race_name);
	strcat(buf, "intro");
	do_help(ch, buf);

	strcpy(buf, "\n\r{038}Details:\n\r\n\r");
	write_to_buffer(d, (char *)ansi_translate_text(ch, buf), 1000000);

	buf[0] = 0;

	cat_sprintf(buf, "{068}  Speed: {138}");

	switch (race_table[race].max_speed)
	{
		case 0: strcat(buf, "Walk\n\r");    break;
		case 1: strcat(buf, "Normal\n\r");  break;
		case 2: strcat(buf, "Jog\n\r");     break;
		case 3: strcat(buf, "Run\n\r");     break;
		case 4: strcat(buf, "Haste\n\r");   break;
		case 5: strcat(buf, "Blazing\n\r"); break;
	}

	switch(race_table[race].vision)
	{
		case 0: strcat(buf, "{068} Vision: {138}Normal\n\r"); break;
		case 1: strcat(buf, "{068} Vision: {138}Night\n\r"); break;
		case 2: strcat(buf, "{068} Vision: {138}Dark\n\r"); break;
	}

	write_to_buffer(d, (char *)ansi_translate_text(ch, buf), 1000000);

	sprintf(buf, "{068}  Stats: {138}Str:{138}%+3d   {138}Dex:{138}%+3d   {138}Int:{138}%+3d   {138}Wis:{138}%+3d   {138}Con:{138}%+3d\n\r",
		race_table[race].race_mod[0],
		race_table[race].race_mod[1],
		race_table[race].race_mod[2],
		race_table[race].race_mod[3],
		race_table[race].race_mod[4]);

	write_to_buffer(d, (char *)ansi_translate_text(ch,buf), 1000000);

	sprintf(buf, "{068}  Bonus: {138}Health:{138}%+3d   {138}Mana:{138}%+3d   {138}Move:{138}%+3d\n\r",
		race_table[race].hp_mod,
		race_table[race].mana_mod,
		race_table[race].move_mod);

	write_to_buffer(d, (char *)ansi_translate_text(ch, buf), 1000000);

	sprintf(buf, "{068}Special: {138}%s\n\r", race_table[race].race_special);

	write_to_buffer(d, (char *)ansi_translate_text(ch,buf), 1000000);

	sprintf(buf, "{068}Classes: {138}");

	for (cnt = 0 ; cnt < MAX_CLASS ; cnt++)
	{
		if (race_table[race].race_class[cnt] == 1)
		{
			cat_sprintf(buf, "%s  ", class_table[cnt].name);
		}
	}
	strcat(buf, "\n\r");

	write_to_buffer(d, (char *)ansi_translate_text(ch,buf), 1000000);

	display_race_selections(d);

	pop_call();
	return;
}

/*
	Deal with sockets that haven't logged in yet.
*/

bool nanny(DESCRIPTOR_DATA *d, char *argument)
{
	char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH];
	BAN_DATA *pban;
	CHAR_DATA *ch;
	int iClass, iRace, cnt, count, fOld;

	push_call_format("nanny(%d,%s)",d->connected,argument);

	while (isspace(*argument))
	{
		argument++;
	}

	ch = d->character;

	nanny:

	switch (d->connected)
	{
		default:
			bug("Nanny: bad d->connected %d.", d->connected);
			close_socket(d);

			pop_call();
			return FALSE;

		case CON_GET_NAME:
			if (argument[0] == '\0')
			{
				close_socket(d);
				pop_call();
				return FALSE;
			}

			argument[0] = UPPER(argument[0]);

			if (!check_parse_name(argument, FALSE))
			{
				write_to_buffer(d, "\n\rIf you wish to create a new character, enter 'NEW' as your name.\n\r\n\rWho art thou: ", 1000000);

				pop_call();
				return FALSE;
			}

			ch = lookup_char(argument);

			d->character = NULL;
			fOld         = FALSE;

			remove_bad_desc_name(argument); /* kick out connections with this name */

			if (ch == NULL || ch->pcdata == NULL)
			{
				fOld = load_char_obj(d, argument);

				if (d->character == NULL)
				{
					write_to_buffer(d, "Your character is faulty.  Contact the Gods.\n\r", 0);
					close_socket(d);

					pop_call();
					return FALSE;
				}
				ch        = d->character;
				d->lookup = FALSE;

				if (IS_SET(mud->flags, MUD_WIZLOCK) && !IS_IMMORTAL(ch))
				{
					write_to_buffer(d, "The game is locked, try later.\n\r", 0);
					close_socket(d);

					pop_call();
					return FALSE;
				}

				if (ch->name == NULL || strcasecmp(ch->name, argument))
				{
					write_to_buffer(d, "Your character is faulty.  Contact the Gods.\n\r", 0);
					close_socket(d);

					pop_call();
					return FALSE;
				}
			}
			else
			{
				fOld         = TRUE;
				d->character = ch;
				d->lookup    = TRUE;
			}

			if (ch->name == NULL || ch->pcdata == NULL)
			{
				log_string("Found nullified character");
				close_socket(d);

				pop_call();
				return FALSE;
			}

			if (!strcasecmp(argument, "new"))
			{
				for (pban = mud->f_nban ; pban ; pban = pban->next)
				{
					if (!str_prefix(pban->name, d->host))
					{
						write_to_descriptor(d, "\n\rYou may not create new characters from this site.\n\r\n\rWho art thou: ", 0);

						pop_call();
						return FALSE;
					}
				}

				free_char(d->character);
				d->character = NULL;

				display_empty_screen(d);

				write_to_buffer(d, "Suitable names are an essential part of maintaining a role-playing environment.\n\r", 1000000);
				write_to_buffer(d, "Make sure to pick a name that suits a medieval fantasy theme, and that is not:\n\r\n\r", 1000000);
				write_to_buffer(d, " - Nonsensical, unpronouncable, ridiculous, profane, or derogatory.\n\r", 1000000);
				write_to_buffer(d, " - Contemporary or futuristic, for example 'John' or 'Robot'.\n\r", 1000000);
				write_to_buffer(d, " - Comprised of ranks or titles, for example 'Lady' or 'Sir'.\n\r", 1000000);
				write_to_buffer(d, " - Composed of singular descriptive nouns, adverbs or adjectives, as in\n\r   'Heart', 'Big', 'Flying', 'Death', or 'Broken'\n\r", 1000000);
				write_to_buffer(d, " - Any combination of singular descriptive nouns/adverbs or adjectives, for\n\r", 1000000);
				write_to_buffer(d, " - example 'Heartbound', 'SunStone', or 'Truesword'\n\r\n\r", 1000000);

				write_to_buffer(d, "If the name you select is not acceptable or otherwise unsuitable, you will be\n\rasked to pick another one.\n\r\n\r", 1000000);

				write_to_buffer(d, "Please choose a name for your character: ", 1000000);

				d->connected = CON_GET_NEW_NAME;

				pop_call();
				return FALSE;
			}

			if (fOld)
			{
				/*
					Old player
				*/

				if (IS_SET(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_DENIED))
				{
					log_god_printf("Denying access to %s@%s.", argument, d->host);

					write_to_buffer(d, "You are denied access.\n\r", 0);
					close_socket(d);
					pop_call();
					return FALSE;
				}

				send_echo_off(d);

				write_to_buffer(d, "Password: ", 1000000);
				d->connected = CON_GET_OLD_PASSWORD;

				pop_call();
				return FALSE;
			}
			else
			{
				write_to_buffer(d, "\n\rIf you wish to create a new character, enter 'NEW' as your name.\n\r\n\rWho art thou: ", 1000000);
				free_char(d->character);
				d->character = NULL;
				pop_call();
				return FALSE;
			}
			break;

		case CON_GET_NEW_NAME:
			if (!strcasecmp(argument, "new"))
			{
				write_to_buffer(d, "\n\rIllegal name, try another.\n\r\n\rName: ", 1000000);
				pop_call();
				return FALSE;
			}

			if (!check_parse_name(argument, TRUE))
			{
				write_to_buffer(d, "\n\rYou have chosen an illegal name.  Please try another.\n\r\n\rNew name: ", 1000000);
				pop_call();
				return FALSE;
			}

			remove_bad_desc_name(argument); /* kick out unconnected sessions with same name */

			if (load_char_obj(d, argument))
			{
				if (d->character != NULL)
				{
					free_char(d->character);
				}
				d->character = NULL;

				write_to_buffer(d, "\n\rYou have chosen a name that already exists.  Please try another.\n\r\n\rNew name: ", 1000000);

				pop_call();
				return FALSE;
			}
			else
			{
				send_echo_off(d);

				ch_printf_color(d->character, "\n\r{078}You must now choose a password.  This password must contain at least five\n\rcharacters, with at least one of them being a number.\n\r\n\rGive me a good password for %s: ", d->character->name);

				d->connected = CON_GET_NEW_PASSWORD;
			}
			break;

		case CON_GET_OLD_PASSWORD:
			if (ch->name == NULL || ch->pcdata == NULL)
			{
				log_string("Found nullified character");
				close_socket(d);
				pop_call();
				return FALSE;
			}
			if (encrypt(argument) != ch->pcdata->password && encrypt64(argument) != ch->pcdata->password)
			{
				ch_printf_color(ch, "\n\r{078}Wrong password.");

				if (d->lookup)
				{
					close_socket(d);
				}
				else
				{
					close_socket(d);
				}
				pop_call();
				return FALSE;
			}
			ch->pcdata->password = encrypt64(argument);

			/*
				Determine if it's a new guy over limit
			*/

			if (mud->total_plr >= MAX_LINKPERPORT && !d->lookup && !IS_IMMORTAL(ch))
			{
				ch_printf_color(ch, "\n\r\n\r{078}The game currently has the maximum amount of %d players online\n\rTry back in a few minutes.\n\r", MAX_LINKPERPORT);
				close_socket(d);
				pop_call();
				return FALSE;
			}
			send_echo_on(d);

			write_to_buffer(d, "\n\r", 0);

			if (check_reconnecting(d, ch))
			{
				log_god_printf("%s@%s has reconnected.  D%d", ch->name, d->host, d->descriptor);
				ch->desc = d;

				d->connected       = CON_PLAYING;
				d->character       = ch;
				d->incomm[0]       = 0;
				ch->pcdata->interp = 0;

				RESTRING(ch->pcdata->host, d->host);

				d->lookup = FALSE;

				if (HAS_BIT(ch->pcdata->vt100_flags, VT100_INTERFACE))
				{
					do_refresh(ch, "");
				}

				send_to_char("You've been reconnected.\n\r",  ch);

				pop_call();
				return TRUE;
			}
			else
			{
				ch = d->original ? d->original : d->character;
				ch->desc = d;

				if (ch->in_room == NULL)
				{
					ch->in_room = get_room_index(ROOM_VNUM_TEMPLE);
				}
				do_help(ch, "motd");

				d->connected = CON_READ_MOTD;

				d->lookup = FALSE;

				for (cnt = 0, count = 0 ; cnt < MAX_CLASS ; cnt++)
				{
					count += d->character->pcdata->mclass[cnt];
				}

				if (count != d->character->level && d->character->level < 97 && d->character->level > 10)
				{
					ch_printf_color(ch, "\n\r{078}Your player file has some serious flaws.  Please talk to the gods about this\n\rproblem.  You will not be able to use this character until it is fixed.\n\r");
					log_printf("%s has faulty multi-classed character.", d->character->name);

					close_socket(d);
				}
			}
			break;

		case CON_GET_NEW_PASSWORD:
				if (strlen(argument) < 5)
				{
					ch_printf_color(ch, "\n\r\n\r{078}Password must be at least five characters long.\n\r\n\rPassword: ");

					pop_call();
					return FALSE;
				}

				if (!is_valid_password(argument))
				{
					ch_printf_color(ch, "\n\r\n\r{078}That password is not acceptable, try again.\n\r\n\rPasswords may only contain letters (case sensitive), or numbers.\n\rAt least one number is required in the password.\n\r\n\rPassword: ");

					pop_call();
					return FALSE;
				}

				ch->pcdata->password = encrypt64(argument);

				ch_printf_color(ch, "\n\r\n\r{078}Please retype your password to confirm: ");

				d->connected = CON_CONFIRM_NEW_PASSWORD;
				break;

		case CON_CONFIRM_NEW_PASSWORD:
				if (encrypt64(argument) != ch->pcdata->password)
				{
					ch_printf_color(ch, "\n\r\n\r{078}The two passwords didn't match.\n\r\n\rPlease retype your password: ");
					d->connected = CON_GET_NEW_PASSWORD;

					pop_call();
					return FALSE;
				}

				send_echo_on(d);

				ch_printf_color(ch, "\n\r\n\r{078}Do you wish to use ANSI color? (Y/N) {118}");

				d->connected = CON_ANSI;
				break;

		case CON_ANSI:
			switch (tolower(*argument))
			{
				case 'y':
					ch->pcdata->vt100_flags = VT100_FAST|VT100_HIGHLIGHT|VT100_BOLD|VT100_BEEP|VT100_UNDERLINE|VT100_FLASH|VT100_REVERSE|VT100_ANSI;
					break;

				case 'n':
					break;

				default:
					ch_printf_color(ch, "\n\r{078}Please type Yes or No? {118}");

					pop_call();
					return FALSE;
			}

			display_empty_screen(d);

			ch_printf_color(ch, "{118}Your client does not support the VT100 protocol.{078}\033[1G\033[0K\n\r\n\r");

			ch_printf_color(ch, "{078}Lowlands boasts a specialized user interface based on the VT100\n\r");
			ch_printf_color(ch, "{078}terminal emulation protocol. It offers status bars at the top and\n\r");
			ch_printf_color(ch, "{078}bottom of the screen to display information about your character and\n\r");
			ch_printf_color(ch, "{078}surroundings. Clients known to support the VT100 protocol are Zmud,\n\r");
			ch_printf_color(ch, "{078}PuTTY, telnet, and TinTin++.\n\r\n\r");
			ch_printf_color(ch, "{078}Do you wish to use the VT100 interface? (Y/N) {118}");

			d->connected = CON_VT100;

			break;

		case CON_VT100:
			switch (tolower(*argument))
			{
				case 'y':
					SET_BIT(ch->pcdata->vt100_flags, VT100_INTERFACE);
					break;

				case 'n':
					break;

				default:
					ch_printf_color(ch, "\n\r{078}Please type Yes or No? {118}");

					pop_call();
					return FALSE;
			}

			reset_color(ch);
			display_empty_screen(d);

			ch_printf_color(ch, "\n\r{038}You may choose from the following genders:\n\r\n\r{138}    M{068} - Male\n\r{138}    F{068} - Female\n\r\n\r{038}Select your sex: {118}");

			d->connected = CON_GET_NEW_SEX;

			break;

		case CON_GET_NEW_SEX:
			switch (argument[0])
			{
				case 'm':
				case 'M':
					ch->sex = SEX_MALE;
					break;
				case 'f':
				case 'F':
					ch->sex = SEX_FEMALE;
					break;
				default:
					ch_printf_color(ch, "\n\r{018}That's not a sex.\n\r\n\r{038}Select your sex: {118}");
					write_to_buffer(d, (char *)ansi_translate_text(ch, buf), 1000000);

					pop_call();
					return FALSE;
			}
			ch->pcdata->actual_sex = ch->sex;

			display_empty_screen(d);
			display_class_selections(d);

			d->connected = CON_GET_NEW_CLASS;
			break;

		case CON_GET_NEW_CLASS:
				argument = one_argument(argument, buf2);

				if (!strcasecmp(buf2, "help"))
				{
					for (iClass = 0 ; iClass < MAX_CLASS ; iClass++)
					{
						if (!str_prefix(argument, class_table[iClass].name))
						{
							display_class_details(d, iClass);

							pop_call();
							return FALSE;
						}
					}
					strcpy(buf, "\n\r{018}No help on that topic.\n\r\n\r{038}Please choose a class: {118}");
					write_to_buffer(d, (char *)ansi_translate_text(ch,buf), 1000000);

					pop_call();
					return FALSE;
				}

				for (iClass = 0 ; iClass < MAX_CLASS ; iClass++)
				{
					if (!str_prefix(buf2, class_table[iClass].name))
					{
						break;
					}
				}

				if (iClass == MAX_CLASS)
				{
					strcpy(buf, "\n\r{018}That's not a class.\n\r\n\r{038}Please choose a class: {118}");
					write_to_buffer(d, (char *)ansi_translate_text(ch,buf), 1000000);

					pop_call();
					return FALSE;
				}

				ch->class = iClass;

				display_empty_screen(d);
				display_race_selections(d);

				d->connected = CON_GET_NEW_RACE;
				break;

		case CON_GET_NEW_RACE:
				argument = one_argument(argument, buf2);

				if (!strcasecmp(buf2, "help"))
				{
					for (iRace = 0 ; iRace < MAX_RACE; iRace++)
					{
						if (ch->pcdata->reincarnation < race_table[iRace].enter || ch->pcdata->reincarnation > race_table[iRace].leave)
						{
							continue;
						}

						if (!str_prefix(argument, race_table[iRace].race_name))
						{
							strcpy(buf, race_table[iRace].race_name);
							strcat(buf, "intro");
							do_help(ch, buf);

							display_race_details(d, iRace);



							pop_call();
							return FALSE;
						}
					}
					strcpy(buf, "\n\r{018}No help on that topic.\n\r\n\r{038}Please choose a race: {118}");
					write_to_buffer(d, (char *)ansi_translate_text(ch,buf), 1000000);

					pop_call();
					return FALSE;
				}

 				for (iRace = 0 ; iRace < MAX_RACE ; iRace++)
				{
					if (!str_prefix(buf2, race_table[iRace].race_name))
					{
						if (ch->pcdata->reincarnation < race_table[iRace].enter || ch->pcdata->reincarnation > race_table[iRace].leave)
						{
							continue;
						}
						break;
					}
				}

				if (iRace == MAX_RACE)
				{
					strcpy(buf, "\n\r{018}That's not a race.\n\r\n\r{038}Please choose a race: {118}");
					write_to_buffer(d, (char *)ansi_translate_text(ch,buf), 1000000);

					pop_call();
					return FALSE;
				}

				if (race_table[iRace].race_class[ch->class] == 0)
				{
					strcpy(buf, "\n\r\n\r{068}That combination of class and race is not allowed.\n\r{038}Please choose a race: {118}");
					write_to_buffer(d, (char *)ansi_translate_text(ch,buf), 1000000);

					pop_call();
					return FALSE;
				}

				ch->race = iRace;

/*				d->connected = CON_APPEAR_LOOK;

				display_appearance_selections(ch, APPEAR_LOOK);
*/

				process_appearance_selections(ch, -1, "");

				d->connected = CON_APPEAR_VERIFY;

				display_appearance_selections(ch, -1);

				break;

		case CON_APPEAR_LOOK:
			if (process_appearance_selections(ch, APPEAR_LOOK, argument))
			{
				d->connected = CON_APPEAR_SKINCOLOR;

				display_appearance_selections(ch, APPEAR_SKINCOLOR);
			}
			break;

		case CON_APPEAR_SKINCOLOR:
			if (process_appearance_selections(ch, APPEAR_SKINCOLOR, argument))
			{
				d->connected = CON_APPEAR_EYECOLOR;

				display_appearance_selections(ch, APPEAR_EYECOLOR);
			}
			break;

		case CON_APPEAR_EYECOLOR:
			if (process_appearance_selections(ch, APPEAR_EYECOLOR, argument))
			{
				d->connected = CON_APPEAR_HAIRCOLOR;

				display_appearance_selections(ch, APPEAR_HAIRCOLOR);
			}
			break;

		case CON_APPEAR_HAIRCOLOR:
			if (process_appearance_selections(ch, APPEAR_HAIRCOLOR, argument))
			{
				d->connected = CON_APPEAR_HAIRTYPE;

				display_appearance_selections(ch, APPEAR_HAIRTYPE);
			}
			break;

		case CON_APPEAR_HAIRTYPE:
			if (process_appearance_selections(ch, APPEAR_HAIRTYPE, argument))
			{
				d->connected = CON_APPEAR_HAIRLENGTH;

				display_appearance_selections(ch, APPEAR_HAIRLENGTH);
			}
			break;

		case CON_APPEAR_HAIRLENGTH:
			if (process_appearance_selections(ch, APPEAR_HAIRLENGTH, argument))
			{
				d->connected = CON_APPEAR_VERIFY;

				display_appearance_selections(ch, -1);
			}
			break;

		case CON_APPEAR_VERIFY:
			switch (*argument)
			{
				case 'n':
				case 'N':
					d->connected = CON_APPEAR_LOOK;

					display_appearance_selections(ch, APPEAR_LOOK);

					break;

				case 'y':
				case 'Y':
					d->connected = CON_REROLL;
					*argument = 0;
					goto nanny;

				default:
					display_appearance_selections(ch, -1);
					break;
			}
			break;

				
		case CON_REROLL:
			switch (*argument)
			{
				case 'n':
				case 'N':
					display_empty_screen(d);
					display_class_selections(d);

					d->connected = CON_GET_NEW_CLASS;
					break;

				case 'y':
				case 'Y':
					display_empty_screen(d);
					do_help(ch, "motd");
					d->connected = CON_READ_MOTD;
					break;
			}

			if (d->connected == CON_REROLL)
			{
				roll_race(ch);

				display_empty_screen(d);

				sprintf(buf, "{038}Stats for %s the %s %s:{068}\n\r\n\r",
					ch->name,
					race_table[ch->race].race_name,
					class_table[ch->class].name);

				write_to_buffer(d, (char *)ansi_translate_text(ch,buf), 1000000);

				sprintf(buf, "{068}Str: {138}%2d    {068}Dex: {138}%2d    {068}Int: {138}%2d    {068}Wis: {138}%2d    {068}Con: {138}%2d\n\r\n\r",
					ch->pcdata->perm_str,
					ch->pcdata->perm_dex,
					ch->pcdata->perm_int,
					ch->pcdata->perm_wis,
					ch->pcdata->perm_con);

				write_to_buffer(d, (char *)ansi_translate_text(ch, buf), 1000000);

				strcpy(buf, "{068}Languages Known: {178}");

				for (cnt = 0 ; cnt < MAX_LANGUAGE ; cnt++)
				{
					if (IS_SHIFT(ch->pcdata->language, cnt))
					{
						cat_sprintf(buf, "%s ", language_table[cnt].name);
					}
				}
				strcat(buf, "\n\r");

				write_to_buffer(d, (char *)ansi_translate_text(ch, buf), 1000000);

				strcpy(buf, "\n\r{038}Do you wish to keep this character?  ({138}Yes{038}, {138}No{038}): {118}");

				write_to_buffer(d, (char *)ansi_translate_text(ch,buf), 1000000);
			}
			break;

		case CON_READ_MOTD:
			if (new_notes(ch))
			{
				write_to_buffer(d, ansi_translate_text(ch, "{506}There are new notes.{088}\n\r\n\r"), 0);
			}

			if (ch->pcdata->pvnum == 0)
			{
				ch->pcdata->pvnum = find_free_pvnum();

				add_pvnum(ch);
				save_players();
			}

			add_char(ch);
			add_player(ch);

			if (get_room_index(ch->pcdata->recall) == NULL)
			{
				ch->pcdata->recall = ROOM_VNUM_TEMPLE;
			}

			if (get_room_index(ch->pcdata->death_room) == NULL)
			{
				ch->pcdata->death_room = ROOM_VNUM_TEMPLE;
			}

			d->connected = CON_PLAYING;

			if (ch->level == 0)
			{
				OBJ_DATA *obj;
				log_god_printf("New player %s@%s has entered the game.", ch->name, d->host);
				ch->level = 1;
				ch->hit   = ch->max_hit;
				ch->mana  = ch->max_mana;
				ch->move  = ch->max_move;

				ch->pcdata->mclass[ch->class] = 1;
				ch->pcdata->actual_max_hit  = ch->max_hit;
				ch->pcdata->actual_max_mana = ch->max_mana;
				ch->pcdata->actual_max_move = ch->max_move;

				ch->pcdata->honorific = ch->class * 8;

				sprintf(buf, "the %s", class_table[ch->class].name);
				set_title(ch, buf);

				obj = create_object(get_obj_index(OBJ_VNUM_SCHOOL_VEST), 0);
				obj_to_char(obj, ch);
				equip_char(ch, obj, WEAR_BODY);

				if (cflag(ch, CFLAG_SHIELD))
				{
					obj = create_object(get_obj_index(OBJ_VNUM_SCHOOL_SHIELD), 0);
					obj_to_char(obj, ch);
					equip_char(ch, obj, WEAR_SHIELD);
				}

				if (class_table[ch->class].weapon)
				{
					obj = create_object(get_obj_index(class_table[ch->class].weapon), 0);
					if (obj)
					{
						obj_to_char(obj, ch);
						equip_char(ch, obj, WEAR_WIELD);
					}
				}

				switch (ch->class)
				{
					case CLASS_RANGER:       ch->pcdata->learned[gsn_sword_fighting]  = 75; break;
					case CLASS_GLADIATOR:    ch->pcdata->learned[gsn_spear_fighting]  = 75; break;
					case CLASS_MARAUDER:     ch->pcdata->learned[gsn_dagger_fighting] = 75; break;
					case CLASS_NINJA:        ch->pcdata->learned[gsn_martial_arts]    = 75; break;
					case CLASS_ELEMENTALIST: ch->pcdata->learned[gsn_mace_fighting]   = 75; break;
					case CLASS_ILLUSIONIST:  ch->pcdata->learned[gsn_staff_fighting]  = 75; break;
					case CLASS_MONK:         ch->pcdata->learned[gsn_fisticuffs]      = 75; break;
					case CLASS_NECROMANCER:  ch->pcdata->learned[gsn_whip_fighting]   = 75; break;
				}

				char_to_room(ch, ROOM_VNUM_SCHOOL);

				if (HAS_BIT(ch->pcdata->vt100_flags, VT100_INTERFACE))
				{
					vt100prompt(ch);
				}
				ch->speed = get_max_speed(ch);
			}
			else
			{
				log_god_printf("%s@%s has entered the game.", ch->name, d->host);

				if (ch->level < LEVEL_IMMORTAL && room_is_private(room_index[ch->pcdata->was_in_room]))
				{
					char_to_room(ch, ROOM_VNUM_TEMPLE);
					send_to_char("The room you quit in is now private.\n\r", ch);
				}
				else
				{
					char_to_room(ch, ch->pcdata->was_in_room);
				}
			}
			enter_game(ch);
			break;
	}
	pop_call();
	return FALSE;
}

/*
	Parse a name for acceptability.
*/

bool check_parse_name(char *name , bool mobcheck)
{
	push_call("check_parse_name(%p,%p)",name,mobcheck);

	if (is_name(name, "new"))
	{
		pop_call();
		return TRUE;
	}

	/*	Reserved Words	*/

	if (is_name(name, "all auto immortal self god enemy clan target race class someone north east south west down open close hours trivia morph hit mana armor damage castle save bak del dmp hours"))
	{
		pop_call();
		return FALSE;
	}

	/*	Length restrictions.*/

	if (strlen(name) <  3)
	{
		pop_call();
		return FALSE;
	}

	if (strlen(name) > 12)
	{
		pop_call();
		return FALSE;
	}

	/*
		Alphanumerics only.
	*/

	{
		char *pc;

		for (pc = name ; *pc != '\0' ; pc++)
		{
			if ((*pc < 'a' || *pc > 'z') && (*pc < 'A' || *pc > 'Z'))
			{
				pop_call();
				return FALSE;
			}
		}

		/*
			No more names containing 'you' - Scandum 16-05-2002
		*/

		for (pc = name ; *(pc+2) != '\0' ; pc++)
		{
			if ((*(pc+0) == 'Y' || *(pc+0) == 'y')
			&&  (*(pc+1) == 'O' || *(pc+1) == 'o')
			&&  (*(pc+2) == 'U' || *(pc+2) == 'u'))
			{
				pop_call();
				return FALSE;
			}
		}
	}

	/*
		Don't allow players to name themself after mobs unless you
		make changes to mob_prog.c - Scandum
	*/

	if (mobcheck)
	{
		int vnum;

		for (vnum = 0 ; vnum < MAX_VNUM ; vnum++)
		{
			if (mob_index[vnum] && is_name(name, mob_index[vnum]->player_name))
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
	Look for link-dead player to reconnect.
*/

bool check_reconnecting(DESCRIPTOR_DATA *d, CHAR_DATA *ch)
{
	DESCRIPTOR_DATA *desc;

	push_call("check_reconnecting(%p,%p)",d,ch);

	if (d->lookup == FALSE)
	{
		pop_call();
		return FALSE;
	}

	for (desc = mud->f_desc ; desc ; desc = desc->next)
	{
		if (desc->lookup == FALSE && CH(desc) == ch)
		{
			desc->original = desc->character = NULL;
			SET_BIT(desc->mth->comm_flags, COMM_FLAG_DISCONNECT);
			break;
		}
	}

	ch->desc     = d;
	d->character = ch;
	d->original  = NULL;

	pop_call();
	return TRUE;
}

void enter_game(CHAR_DATA *ch)
{
	push_call("enter_game(%p)",ch);
/*
	if (ch->desc)
	{
		process_naws(ch->desc, ch->desc->cols, ch->desc->rows);
	}
*/
	show_who_can_see(ch, " has entered the game.\n\r");

	if (ch->pcdata->corpse != NULL)
	{
		obj_to_room(ch->pcdata->corpse, ch->pcdata->corpse_room);
	}

	if (ch->speed > get_max_speed(ch))
	{
		ch->speed = get_max_speed(ch);
	}

	if (ch->desc)
	{
		RESTRING(ch->pcdata->host, ch->desc->host);
	}

	if (IS_AFFECTED(ch, AFF_SLEEP))
	{
		ch->position = POS_SLEEPING;
		send_to_char("You are sleeping.\n\r" , ch);
	}
	else
	{
		ch->position = POS_STANDING;
		do_look(ch, "auto");
	}
	ch->pcdata->last_connect = mud->current_time;

	scan_object_for_dup(ch, ch->first_carrying);

	pop_call();
	return;
}

void do_lookup(CHAR_DATA *ch, char *arg)
{
	FIGHT_DATA		*fight;
	PET_DATA			*pet;
	ROOM_TIMER_DATA	*rtimer;

	int cnt, show = FALSE;

	push_call("do_lookup(%p,%p)",ch,arg);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (strcasecmp(arg, ""))
	{
		show = TRUE;
	}

	for (cnt = 0, fight = mud->f_fight ; fight ; fight = fight->next, cnt++)
	{
		if (show)
		{
			ch_printf(ch, "%s\n\r", fight->ch->name);
		}
	}
	ch_printf(ch, "     Fight list: %d\n\r", cnt);

	for (cnt = 0, pet = mud->f_pet ; pet ; pet = pet->next, cnt++)
	{
		if (show)
		{
			ch_printf(ch, "%s\n\r", pet->ch->name);
		}
	}

	ch_printf(ch, "       Pet list: %d\n\r", cnt);

	for (cnt = 0, rtimer = mud->f_room_timer ; rtimer ; rtimer = rtimer->next, cnt++)
	{
		if (show)
		{
			ch_printf(ch, "%d\n\r", rtimer->vnum);
		}
	}
	ch_printf(ch, "Room Timer list: %d\n\r", cnt);

	pop_call();
	return;
}


void remove_bad_desc_name(char *name)
{
	DESCRIPTOR_DATA *d;

	push_call("remove_bad_desc_name(%p)",name);

	for (d = mud->f_desc ; d ; d = d->next)
	{
		if (d->connected < CON_PLAYING && d->character)
		{
			if (!strcasecmp(d->character->name, name))
			{
				log_god_printf("failed connect: %s@%s", d->character->name, d->host);

				if (get_char_pvnum(d->character->pcdata->pvnum) == NULL)
				{
					d->character->desc = NULL;
					extract_char(d->character);
				}
				d->character = NULL;
				SET_BIT(d->mth->comm_flags, COMM_FLAG_DISCONNECT);
			}
		}
	}
	pop_call();
	return;
}

void check_bad_desc(CHAR_DATA *ch)
{
	DESCRIPTOR_DATA *d;

	push_call("check_bad_desc(%p)",ch);

	for (d = mud->f_desc ; d ; d = d->next)
	{
		if (d->connected < CON_PLAYING && d->character)
		{
			if (d->character == ch)
			{
				log_god_printf("failed connect: %s@%s", d->character->name, d->host);
				d->character = NULL;
				SET_BIT(d->mth->comm_flags, COMM_FLAG_DISCONNECT);
			}
		}
	}
	pop_call();
	return;
}

void stop_idling(DESCRIPTOR_DATA *d)
{
	push_call("stop_idling(%p)",d);

	if (d->character == NULL || d->connected < CON_PLAYING)
	{
		pop_call();
		return;
	}

	if (IS_SET(CH(d)->pcdata->interp, INTERP_AUTO))
	{
		pop_call();
		return;
	}

	CH(d)->timer = 0;

	if (CH(d)->in_room != room_index[ROOM_VNUM_LIMBO])
	{
		pop_call();
		return;
	}

	if (CH(d)->level < LEVEL_IMMORTAL && room_is_private(room_index[CH(d)->pcdata->was_in_room]))
	{
		char_to_room(CH(d), ROOM_VNUM_TEMPLE);
		send_to_char("The room you were idling in is now private.\n\r", CH(d));
	}
	else
	{
		char_to_room(CH(d), CH(d)->pcdata->was_in_room);
	}

	show_who_can_see(CH(d), " has returned from the void.\n\r");

	pop_call();
	return;
}

/*
	Write to one room - Scandum
*/

void send_to_room(char *txt, ROOM_INDEX_DATA *room)
{
	CHAR_DATA *ch;

	for (ch = room->first_person ; ch ; ch = ch->next_in_room)
	{
		send_to_char(justify(txt, get_page_width(ch)), ch);
	}
}

/*
	Write to one char.
*/

void send_to_char(const char *txt, CHAR_DATA *ch)
{
	char buf[MAX_STRING_LENGTH];
	bool foundyou;

	push_call("send_to_char(%p,%p)",txt,ch);

	if (txt == NULL || ch == NULL || ch->desc == NULL)
	{
		pop_call();
		return;
	}

	if (IS_SET(mud->flags, MUD_SKIPOUTPUT))
	{
		if (IS_NPC(ch) || ch->level < MAX_LEVEL || !IS_SET(ch->act, PLR_WIZTIME))
		{
			pop_call();
			return;
		}
	}

	if (txt[0] != '\033')
	{
		foundyou = scroll_you(ch->desc, txt, IS_SET(CH(ch->desc)->pcdata->vt100_flags, VT100_HIGHLIGHT));

		if (foundyou)
		{
			snprintf(buf, MAX_STRING_LENGTH-1, "%s%s", get_color_string(ch, COLOR_TEXT, VT102_BOLD), txt);
		}
		else
		{
			snprintf(buf, MAX_STRING_LENGTH-1, "%s%s", get_color_string(ch, COLOR_TEXT, VT102_DIM),  txt);
		}
		write_to_buffer(ch->desc, (char *)buf, 0);
	}
	else
	{
		write_to_buffer(ch->desc, (char *)txt, 0);
	}
	pop_call();
	return;
}

void send_to_char_color(char *txt, CHAR_DATA *ch)
{
	push_call("send_to_char_color(%p,%p)",txt,ch);

	send_to_char(ansi_translate_text(ch, txt), ch);

	pop_call();
	return;
}


#define NUM_FAKE_NAME 22

/*
	The primary output interface for formatted output.
*/

void act(const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type)
{
	static char * const he_she  [] = { "it",  "he",  "she" };
	static char * const him_her [] = { "it",  "him", "her" };
	static char * const his_her [] = { "its", "his", "her" };

	static char * const fake_name [] =
	{
		"no one in particular",		"a white rabbit",
		"Puff",					"a house plant",
		"a twinkie",				"your mother",
		"your alter ego",			"a buffer overflow",
		"a voice in your head",		"the wind",
		"Bubba",					"your father",
		"a lollipop",				"your mud client",
		"your index finger",		"a large worm",
		"the ground",				"a bad pointer reference",
		"your alter ego",			"a fortune cookie",
		"your shadow",				"the world"
	};

	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *to;
	CHAR_DATA *vch = (CHAR_DATA *) arg2;
	OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
	OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
	const char *str;
	const char *i;
	char *point;

	push_call("act(%p,%p,%p,%p,%p)",format,ch,arg1,arg2,type);

	/*
		Discard null messages generated by socials.
	*/

	if (format == NULL || format[0] == '\0')
	{
		pop_call();
		return;
	}

	if (IS_SET(mud->flags, MUD_SKIPOUTPUT))
	{
		if (IS_NPC(ch) || ch->level < MAX_LEVEL || !IS_SET(ch->act, PLR_WIZTIME))
		{
			pop_call();
			return;
		}
	}

	if (ch == NULL)
	{
		bug("Act: null ch.", 0);
		pop_call();
		return;
	}

	to = ch->in_room->first_person;

	if (type == TO_VICT)
	{
		if (vch == NULL)
		{
			bug("Act: null vch with TO_VICT.", 0);
			pop_call();
			return;
		}
		if (vch->in_room == NULL)
		{
			bug("Act: null victim room.", 0);
			pop_call();
			return;
		}
		to = vch->in_room->first_person;
	}

	for (; to != NULL ; to = to->next_in_room)
	{
		if (to->desc == NULL || !IS_AWAKE(to))
		{
			continue;
		}
		if (type == TO_CHAR && to != ch)
		{
			continue;
		}
		if (type == TO_VICT && (to != vch || to == ch))
		{
			continue;
		}
		if (type == TO_ROOM && to == ch)
		{
			continue;
		}
		if (type == TO_NOTVICT && (to == ch || to == vch))
		{
			continue;
		}

		point = buf;
		str = format;

		while (*str != '\0')
		{
			if (*str != '$')
			{
				*point++ = *str++;
				continue;
			}
			++str;

			if (arg1 == NULL && *str == 'p')
			{
				log_printf("Act: missing arg1 for code %d.", *str);
				i = "<ppp>";
			}
			else if (arg2 == NULL && *str >= 'A' && *str <= 'Z')
			{
				log_printf("Act: missing arg2 for code %d.", *str);
				i = "<A-Z>";
			}

			else	if (!IS_AFFECTED(to, AFF2_HALLUCINATE))
			{
				switch (*str)
				{
					default:
						log_printf("Act: act bad code %d.", *str);
						i = "<? ?>";
						break;
					case 't':
						i = (char *) arg1;
						break;
					case 'T':
						i = (char *) arg2;
						break;
					case 'n':
						i = get_name(ch);
						break;
					case 'N':
						i = get_name(vch);
						break;
					case 'e':
						i = he_she  [URANGE(0, ch->sex,  2)];
						break;
					case 'E':
						i = he_she  [URANGE(0, vch->sex, 2)];
						break;
					case 'm':
						i = him_her [URANGE(0, ch->sex,  2)];
						break;
					case 'M':
						i = him_her [URANGE(0, vch->sex, 2)];
						break;
					case 's':
						i = his_her [URANGE(0, ch->sex,  2)];
						break;
					case 'S':
						i = his_her [URANGE(0, vch->sex, 2)];
						break;
					case 'p':
						i = can_see_obj(to, obj1) ? obj1->short_descr : "something";
						break;
					case 'P':
						i = can_see_obj(to, obj2) ? obj2->short_descr : "something";
						break;
					case 'd':
						if (arg2 == NULL || ((char *) arg2)[0] == '\0')
						{
							i = "door";
						}
						else
						{
							i = arg2;
						}
						break;
					case '/':
						i = "\n\r";
						break;
				}
			}
			else /* THIS IS THE HALLUCINATE VERSION...DON'T PANIC -ORDER */
			{
				switch (*str)
				{
					default:
						log_printf("Act: act bad code %d.", *str);
						i = ">? ?<";
						break;
					case 't':
					case 'T':
					case 'n':
					case 'N':
					case 'p':
					case 'P':
					case 'd':
						i = fake_name[number_range(0, NUM_FAKE_NAME-1)];
						break;
					case 'e':
					case 'E':
						i = he_she  [number_range(0, 2)];
						break;
					case 'm':
					case 'M':
						i = him_her [number_range(0, 2)];
						break;
					case 's':
					case 'S':
						i = his_her [number_range(0, 2)];
						break;
				}
			}
			if (i == NULL)
			{
				log_string("hallucinate act: i == NULL");
				dump_stack();
				i = "nothing";
			}
			++str;
			while ((*point = *i) != '\0')
			{
				++point, ++i;
			}
		}
		*point++ = '\n';
		*point++ = '\r';
		*point++ = '\0';

		strcpy(buf, capitalize(buf));

		send_to_char_color(ansi_justify(buf, get_page_width(to)), to);
	}

	/*
		Placed act_prog triggering in a seperate loop. This way the act prog
		triggers after the message has been send. - Scandum
	*/

	if (type == TO_VICT)
	{
		to = vch->in_room->first_person;
	}
	else
	{
		to = ch->in_room->first_person;
	}

	for (; to ; to = to->next_in_room)
	{
		if (!MP_VALID_MOB(to) || !IS_SET(to->pIndexData->progtypes, ACT_PROG) || !IS_AWAKE(to))
		{
			continue;
		}
		if (type == TO_CHAR && to != ch)
		{
			continue;
		}
		if (type == TO_VICT && (to != vch || to == ch))
		{
			continue;
		}
		if (type == TO_ROOM && to == ch)
		{
			continue;
		}
		if (type == TO_NOTVICT && (to == ch || to == vch))
		{
			continue;
		}

		point = buf;
		str = format;
		while (*str != '\0')
		{
			if (*str != '$')
			{
				*point++ = *str++;
				continue;
			}
			++str;

			if (arg1 == NULL && *str == 'p')
			{
				log_printf("Act: missing arg1 for code %d.", *str);
				i = "<ppp>";
			}
			else if (arg2 == NULL && *str >= 'A' && *str <= 'Z')
			{
				log_printf("Act: missing arg2 for code %d.", *str);
				i = "<A-Z>";
			}

			switch (*str)
			{
				default:
					log_printf("Act: act bad code %d.", *str);
					i = "<? ?>";
					break;
				case 't':
					i = (char *) arg1;
					break;
				case 'T':
					i = (char *) arg2;
					break;
				case 'n':
					i = get_name(ch);
					break;
				case 'N':
					i = get_name(vch);
					break;
				case 'e':
					i = he_she  [URANGE(0, ch->sex,  2)];
					break;
				case 'E':
					i = he_she  [URANGE(0, vch->sex, 2)];
					break;
				case 'm':
					i = him_her [URANGE(0, ch->sex,  2)];
					break;
				case 'M':
					i = him_her [URANGE(0, vch->sex, 2)];
					break;
				case 's':
					i = his_her [URANGE(0, ch->sex,  2)];
					break;
				case 'S':
					i = his_her [URANGE(0, vch->sex, 2)];
					break;
				case 'p':
					i = obj1->short_descr;
					break;
				case 'P':
					i = obj2->short_descr;
					break;
				case 'd':
					if (arg2 == NULL || ((char *) arg2)[0] == '\0')
					{
						i = "door";
					}
					else
					{
						i = arg2;
					}
					break;
				case '/':
					i = "\n\r";
					break;
			}

			if (i == NULL)
			{
				log_string("act: i == NULL");
				i = "nothing";
			}
			++str;
			while ((*point = *i) != '\0')
			{
				++point, ++i;
			}
		}
		*point++ = '\n';
		*point++ = '\r';
		*point++ = '\0';

		buf[0]   = UPPER(buf[0]);

		mprog_act_trigger(buf, to, ch, obj1, vch);
	}
	pop_call();
	return;
}


void force_help(DESCRIPTOR_DATA *d, char *argument)
{
	AREA_DATA *pArea;
	HELP_DATA *pHelp;

	push_call("force_help(%p,%p)",d,argument);

	for (pArea = mud->f_area ; pArea ; pArea = pArea->next)
	{
		for (pHelp = pArea->first_help ; pHelp ; pHelp = pHelp->next)
		{
			if (is_name(argument, pHelp->keyword))
			{
				write_to_buffer(d, ansi_translate(pHelp->text), 1000000);
				pop_call();
				return;
			}
		}
	}
	pop_call();
	return;
}

bool is_desc_valid(CHAR_DATA *ch)
{
	push_call("is_desc_valid(%p)",ch);

	if (ch == NULL)
	{
		pop_call();
		return(FALSE);
	}

	if (ch->desc != NULL && ch->desc->character == ch)
	{
		pop_call();
		return(TRUE);
	}
	else
	{
		pop_call();
		return(FALSE);
	}
}


void add_player(CHAR_DATA *ch)
{
	PLAYER_GAME *gpl, *fpl;

	push_call("add_player(%p)",ch);

	if (get_char_pvnum(ch->pcdata->pvnum) != NULL)
	{
		log_printf("add_player: already listed as playing");
		dump_stack();
	}
	else
	{
		pvnum_index[ch->pcdata->pvnum]->ch = ch;
		REMOVE_BIT(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_DELETED);
	}

	ALLOCMEM(gpl, PLAYER_GAME, 1);
	gpl->ch = ch;

	mud->total_plr++;

	for (fpl = mud->f_player ; fpl ; fpl = fpl->next)
	{
		if (ch->level > fpl->ch->level)
		{
			continue;
		}
		if (ch->level < fpl->ch->level || ch->pcdata->played < fpl->ch->pcdata->played)
		{
			INSERT_LEFT(gpl, fpl, mud->f_player, next, prev);
			break;
		}
	}
	if (fpl == NULL)
	{
		LINK(gpl, mud->f_player, mud->l_player, next, prev);
	}

	pop_call();
	return;
}


void sub_player(CHAR_DATA *ch)
{
	PLAYER_GAME *fpl;

	push_call("sub_player(%p)",ch);

	for (fpl = mud->f_player ; fpl ; fpl = fpl->next)
	{
		if (fpl->ch == ch)
		{
			UNLINK(fpl, mud->f_player, mud->l_player, next, prev);
			FREEMEM(fpl);
			mud->total_plr--;
			break;
		}
	}

	if (get_char_pvnum(ch->pcdata->pvnum) != ch)
	{
		pop_call();
		return;
	}

	pvnum_index[ch->pcdata->pvnum]->ch = NULL;

	pop_call();
	return;
}


CHAR_DATA * start_partial_load(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *fch = NULL;
	int exists;
	DESCRIPTOR_DATA *d;

	push_call("start_partial_load(%p,%p)",ch,argument);

	exists = check_parse_name(argument, FALSE);

	if (exists)
	{
		ALLOCMEM(d, DESCRIPTOR_DATA, 1);
		d->original	= NULL;
		d->descriptor	= -999;  /* Special case for partial loads */
		exists		= load_char_obj(d, argument);
		fch			= d->character;
		fch->desc		= d;

		if (character_expiration(fch) < 0)
		{
			log_printf("partial_load: deleting expired player %s", argument);
			delete_player(fch);

			pop_call();
			return NULL;
		}
	}

	if (!exists)
	{
		if (ch)
		{
			ch_printf(ch, "The character named '%s' cannot be found.\n\r", argument);
		}
		if (fch)
		{
			clear_partial_load(fch);
		}
		pop_call();
		return NULL;
	}
	pop_call();
	return fch;
}

void clear_partial_load(CHAR_DATA *ch)
{
	DESCRIPTOR_DATA * d = ch->desc;

	push_call("clear_partial_load(%p)",ch);

	ch->desc = NULL;
	extract_char(ch);
	d->character = NULL;
	d->original  = NULL;
	FREEMEM(d);

	pop_call();
	return;
}

void do_finger(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *fch;
	char buf[MAX_STRING_LENGTH];
	char outbuf[MAX_STRING_LENGTH];
	char textBld[10], textDim[10], promBld[10], promDim[10];
	char *pt;
	int expire;
	int time_left;
	CRIME_DATA *pcd;
	sh_int rel_days, crcnt;
	sh_int rel_hrs;
	sh_int rel_mins;

	push_call("do_finger(%p,%p)",ch,argument);

	fch = start_partial_load(ch, argument);

	if (fch == NULL)
	{
		pop_call();
		return;
	}

	strcpy(textDim, get_color_string(ch, COLOR_TEXT,   VT102_DIM));
	strcpy(textBld, get_color_string(ch, COLOR_TEXT,   VT102_BOLD));
	strcpy(promDim, get_color_string(ch, COLOR_PROMPT, VT102_DIM));
	strcpy(promBld, get_color_string(ch, COLOR_PROMPT, VT102_BOLD));

	sprintf(outbuf, "%sInformation on %s%s\n\r\n\r", textBld, get_name(fch), fch->pcdata->title);

	cat_sprintf(outbuf, "%s\n\r", ansi_translate_text(ch, fch->description));

	cat_sprintf(outbuf, "%s%-12s%s[%2d ", textBld, capitalize(fch->name), promDim, fch->level);

	if (fch->level < MAX_LEVEL - 3)
	{
		cat_sprintf(outbuf, "%s", class_table[fch->class].who_name);
	}
	else switch (fch->level)
	{
		case MAX_LEVEL - 0: strcat(outbuf, "GOD"); break;
		case MAX_LEVEL - 1: strcat(outbuf, "ARC"); break;
		case MAX_LEVEL - 2: strcat(outbuf, "ANG"); break;
		case MAX_LEVEL - 3: strcat(outbuf, "LRD"); break;
	}

	if (fch->level >= LEVEL_IMMORTAL)
	{
		strcat(outbuf, " ---");
	}
	else
	{
		snprintf(buf, 5, " %.3s", race_table[fch->race].race_name);
		strcat(outbuf, buf);
	}
	cat_sprintf(outbuf, "]%s", promBld);

	cat_sprintf(outbuf, " %s%s:%s%2d %s%s:%s%2d %s%s:%s%2d %s%s:%s%2d %s%s:%s%2d %s%s:%s%2d %s%s:%s%2d %s%s:%s%2d\n\r",
		class_table[0].who_name, promDim, promBld, fch->pcdata->mclass[0],
		class_table[1].who_name, promDim, promBld, fch->pcdata->mclass[1],
		class_table[2].who_name, promDim, promBld, fch->pcdata->mclass[2],
		class_table[3].who_name, promDim, promBld, fch->pcdata->mclass[3],
		class_table[4].who_name, promDim, promBld, fch->pcdata->mclass[4],
		class_table[5].who_name, promDim, promBld, fch->pcdata->mclass[5],
		class_table[6].who_name, promDim, promBld, fch->pcdata->mclass[6],
		class_table[7].who_name, promDim, promBld, fch->pcdata->mclass[7]);


	cat_sprintf(outbuf, "%sAge: %d years    Played: %d hours    Reincarnated: %d times\n\r",
		textDim,
		get_age(fch),
		fch->pcdata->played/3600+fch->pcdata->previous_hours,
		fch->pcdata->reincarnation);

	expire = character_expiration(fch);

	if (ch->level == MAX_LEVEL || (expire < 60))
	{
		cat_sprintf(outbuf, "This player file will expire in %d days.\n\r", expire);
	}

	if (which_god(fch) != GOD_NEUTRAL && !IS_GOD(fch))
	{
			cat_sprintf(outbuf, "%s\n\r", god_table[which_god(fch)].finger_msg);
	}
	if (IS_SET(fch->act, PLR_OUTCAST))
	{
		cat_sprintf(outbuf, "%s is an Outcast!\n\r", get_name(fch));
	}
	else if (fch->pcdata->clan)
	{
		if (!strcasecmp(fch->name, fch->pcdata->clan->leader[0]))
		{
			cat_sprintf(outbuf, "%s is the founder of %s.\n\r", get_name(fch), fch->pcdata->clan_name);
		}
		else if (is_clan_leader(fch))
		{
			cat_sprintf(outbuf, "%s is a leader of %s.\n\r", get_name(fch), fch->pcdata->clan_name);
		}
		else
		{
			cat_sprintf(outbuf, "%s is a member of %s.\n\r", get_name(fch), fch->pcdata->clan_name);
		}
	}

	if (IS_SET(pvnum_index[fch->pcdata->pvnum]->flags, PVNUM_ARRESTED))
	{
		time_left = fch->pcdata->jailtime - (mud->current_time - fch->pcdata->jaildate);
		if (time_left > 0)
		{
			rel_days = time_left / 86400;
			rel_hrs  = time_left % 86400 / 3600;
			rel_mins = time_left % 86400 % 3600 / 60;

			cat_sprintf(outbuf, "Imprisoned in the dungeons for %d days, %d hours and %d minutes.\n\r", rel_days, rel_hrs, rel_mins);
		}
	}

	if (ch->level >= LEVEL_IMMORTAL)
	{
		if (fch->pcdata->first_record != NULL)
		{
			crcnt = 0;
			for (pcd = fch->pcdata->first_record ; pcd ; pcd = pcd->next)
			{
				if (pcd)
				{
					crcnt++;
				}
			}
			cat_sprintf(outbuf, "Has %d criminal records.\n\r",crcnt);
		}
	}

	if (FALSE && which_god(fch) != GOD_NEUTRAL)
	{
		cat_sprintf(outbuf, "\n\r Enemies Killed: %7d     Enemies that have killed %s: %7d"
		              "\n\rMonsters Killed: %7d    Monsters that have killed %s: %7d\n\r",
			fch->pcdata->history[HISTORY_KILL_EN],  fch->name, fch->pcdata->history[HISTORY_KILL_BY_EN],
			fch->pcdata->history[HISTORY_KILL_NPC], fch->name, fch->pcdata->history[HISTORY_KILL_BY_NPC]);
	}
	else
	{
		cat_sprintf(outbuf, "\n\rMonsters Killed: %7d    Monsters that have killed %s: %7d\n\r",
			fch->pcdata->history[HISTORY_KILL_NPC],	fch->name, fch->pcdata->history[HISTORY_KILL_BY_NPC]);
	}

	if (*fch->pcdata->mail_address != '\0')
	{
		cat_sprintf(outbuf, "Internet Email address: %s%s\n\r", fch->pcdata->mail_address, textDim);
	}

	if (*fch->pcdata->html_address != '\0')
	{
		strcpy(buf, fch->pcdata->html_address);
		for (pt = buf ; *pt != '\0' ; pt++)
		{
			if (*pt == '*')
			{
				*pt = '~';
			}
		}
		cat_sprintf(outbuf, "Internet Home Page: %s%s\n\r", buf, textDim);
	}

	if (fch->pcdata->castle && fch->pcdata->castle->cost)
	{
		cat_sprintf(outbuf, "Has a castle that costs %s,000 gold coins to build.\n\r", tomoney(fch->pcdata->castle->cost));
	}

	if (IS_GOD(ch))
	{
		if (!lookup_char(argument))
		{
			if (fch->pcdata->last_time > 0)
			{
				cat_sprintf(outbuf, "%sLast time in the realm: %s\n\r", promDim, get_time_string(fch->pcdata->last_time));
			}
			cat_sprintf(outbuf, "%sInternet computer last on: %s\n\r", promDim, fch->pcdata->host);
		}
		else if (is_desc_valid(fch))
		{
			cat_sprintf(outbuf, "%sInternet computer on: %s\n\r", promDim, fch->pcdata->host);
		}
	}
	send_to_char(outbuf, ch);

	clear_partial_load(fch);

	pop_call();
	return;
}

bool is_valid_password(char *pass)
{
	char *pt;
	bool found_number;
	bool good_char;

	push_call("is_valid_password(%p)",pass);

	for (found_number = FALSE, pt = pass ; *pt != '\0' ; pt++)
	{
		good_char = FALSE;
		if (*pt >= 'a' && *pt <= 'z')
			good_char = TRUE;
		if (*pt >= 'A' && *pt <= 'Z')
			good_char = TRUE;
		if (*pt >= '0' && *pt <= '9')
		{
			good_char = TRUE;
			found_number = TRUE;
		}
		if (!good_char)
		{
			pop_call();
			return(FALSE);
		}
	}
	if (!found_number && strlen(pass) < 8)
	{
		pop_call();
		return(FALSE);
	}
	else
	{
		pop_call();
		return(TRUE);
	}
}

void do_prompt(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];

	push_call("do_prompt(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (HAS_BIT(ch->pcdata->vt100_flags, VT100_INTERFACE))
	{
		vt100off(ch);
		send_to_char("Prompt mode enabled.\n\r", ch);

		pop_call();
		return;
	}

	str_cpy_max(buf, argument, 160);

	smash_tilde(buf);

	if (buf[0] == '\0')
	{
		send_to_char("Set your prompt to what?\n\r", ch);

		pop_call();
		return;
	}

	switch (buf[0])
	{
		case '.':
			strcpy(buf, "");
			break;
		case '1':
			sprintf(buf, "{078}<$hhp $mm $vmv $Xxp> {300}");
			break;
		case '2':
			sprintf(buf, "{078}<$c$h{078}hp $c$m{078}m $c$v{078}mv $c$X{078}xp> {300}");
			break;
		case '3':
			sprintf(buf, "{078}<$hhp $mm $vmv $Xxp> <$e> ($l) ($L) {300}");
			break;
		case '4':
			sprintf(buf, "{078}<$c$h{078}hp $c$m{078}m $c$v{078}mv $c$X{078}xp> <{178}$e{078}> ($c$l{078}) ($c$L{078}) ");
			break;
	}

	RESTRING(ch->pcdata->prompt_layout, buf);
	ch_printf(ch, "Prompt set to: %s\n\r", buf);

	pop_call();
	return;
}

char *get_gradient(bool gradient, int current, int max)
{
	int percent;

	push_call("get_gradient(%p,%p,%p)",gradient,current,max);

	if (gradient == FALSE)
	{
		pop_call();
		return "";
	}

	percent = 100 * UMAX(0, current) / UMAX(1, max);

	switch (percent / 25)
	{
		case 0:
			pop_call();
			return "{118}";
		case 1:
		case 2:
			pop_call();
			return "{138}";
	}
	pop_call();
	return "{128}";
}

char *prompt_return(CHAR_DATA *ch, char *layout)
{
	AFFECT_DATA *paf;
	EXIT_DATA *pexit;
	static char prompt_buffer[MAX_INPUT_LENGTH];
	char tbuf[MAX_INPUT_LENGTH];
	char *pti, *pto;
	bool last_was_str, gradient;
	int door;
	struct tm clk;

	push_call("prompt_return(%p,%p)",ch,layout);

	if (!IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL && ch->pcdata->editmode != MODE_NONE && ch->pcdata->subprompt && ch->pcdata->subprompt[0] != '\0')
	{
		sprintf (prompt_buffer, "{138}<{158}%s{138}> {078}", ch->pcdata->subprompt);
	}
	else if (layout == NULL || *layout == '\0')
	{
		sprintf(prompt_buffer,  "%s<%dhp %dm %dmv %dxp> ", get_color_string(ch, COLOR_PROMPT, VT102_DIM), ch->hit, ch->mana, ch->move, exp_level(ch->class, ch->level) - (IS_NPC(ch) ? 0 : ch->pcdata->exp));
	}
	else
	{
		last_was_str = FALSE;
		gradient     = FALSE;
		pti  = layout;
		strcpy(prompt_buffer, get_color_string(ch, COLOR_PROMPT, VT102_DIM));
		pto  = prompt_buffer;
		pto += strlen(prompt_buffer);

		while(*pti != '\0')
		{
			if (last_was_str)
			{
				last_was_str = FALSE;

				switch (*pti++)
				{
					case 'h':
						sprintf(tbuf, "%s%d", get_gradient(gradient, ch->hit, ch->max_hit), ch->hit);
						strcpy(pto, tbuf);
						pto += strlen(tbuf);
						break;
					case 'H':
						sprintf(tbuf, "%s%d", get_gradient(gradient, ch->hit, ch->max_hit), ch->max_hit);
						strcpy(pto, tbuf);
						pto += strlen(tbuf);
						break;

					case 'v':
						sprintf(tbuf, "%s%d", get_gradient(gradient, ch->move, ch->max_move), ch->move);
						strcpy(pto, tbuf);
						pto += strlen(tbuf);
						break;
					case 'V':
						sprintf(tbuf, "%s%d", get_gradient(gradient, ch->move, ch->max_move), ch->max_move);
						strcpy(pto, tbuf);
						pto += strlen(tbuf);
						break;
					case 'm':
						sprintf(tbuf, "%s%d", get_gradient(gradient, ch->mana, ch->max_mana), ch->mana);
						strcpy(pto, tbuf);
						pto += strlen(tbuf);
						break;
					case 'M':
						sprintf(tbuf, "%s%d", get_gradient(gradient, ch->mana, ch->max_mana), ch->max_mana);
						strcpy(pto, tbuf);
						pto += strlen(tbuf);
						break;
					case 'x':
						sprintf(tbuf, "%s%d", get_gradient(gradient, exp_level(ch->class, ch->level) - (IS_NPC(ch) ? 0 : ch->pcdata->exp), exp_level(ch->class, ch->level) - exp_level(ch->class, ch->level-1)), IS_NPC(ch) ? 0 : ch->pcdata->exp);
						strcpy(pto, tbuf);
						pto += strlen(tbuf);
						break;
					case 'X':
						sprintf(tbuf, "%s%d", get_gradient(gradient, exp_level(ch->class, ch->level) - (IS_NPC(ch) ? 0 : ch->pcdata->exp), exp_level(ch->class, ch->level) - exp_level(ch->class, ch->level-1)), exp_level(ch->class, ch->level) - (IS_NPC(ch) ? 0 : ch->pcdata->exp));
						strcpy(pto, tbuf);
						pto += strlen(tbuf);
						break;
					case 'l':
						sprintf(tbuf, "%s%d", get_gradient(gradient, ch->hit, ch->max_hit), 100 * ch->hit / UMAX(1, ch->max_hit));
						strcpy(pto, tbuf);
						pto += strlen(tbuf);
						break;
					case 'L':
						if (who_fighting(ch))
						{
							sprintf(tbuf, "%s%d", get_gradient(gradient, ch->fighting->who->hit, ch->fighting->who->max_hit), 100 * ch->fighting->who->hit / UMAX(1, ch->fighting->who->max_hit));
						}
						else
						{
							sprintf(tbuf, "--");
						}
						strcpy(pto, tbuf);
						pto += strlen (tbuf);
						break;
					case 'g':
						sprintf(tbuf, "%s%d", get_gradient(gradient, ch->gold, ch->level * 1000000), ch->gold);
						strcpy(pto, tbuf);
						pto += strlen(tbuf);
						break;
					case 'a':
						sprintf(tbuf, "%s%d", get_gradient(gradient, ch->alignment + 1000, 2000), ch->alignment);
						strcpy(pto, tbuf);
						pto += strlen(tbuf);
						break;
					case 's':
						switch (ch->speed)
						{
							case 0: *pto = 'W'; break;
							case 1: *pto = 'N'; break;
							case 2: *pto = 'J'; break;
							case 3: *pto = 'R'; break;
							case 4: *pto = 'H'; break;
							case 5: *pto = 'B'; break;
						}
						pto++;
						break;

					case 'S':
						strcpy(tbuf, language_table[UNSHIFT(CH(ch->desc)->pcdata->speak)].name);
						strcpy(pto, tbuf);
						pto += strlen(tbuf);
						break;
						
					case 'f':
						for (paf = ch->first_affect ; paf ; paf = paf->next)
						{
							*pto = skill_table[paf->type].name[0] - 32;	pto++;
							*pto = skill_table[paf->type].name[1];		pto++;
						}
						break;
					case 'e':
						if (can_see_in_room(ch, ch->in_room))
						{
							for (door = 0 ; door < 6 ; door++)
							{
								if ((pexit = get_exit(ch->in_room->vnum, door)) != NULL
								&&   !IS_SET(pexit->flags, EX_CLOSED)
								&&  (!IS_SET(ch->in_room->room_flags, ROOM_SMOKE)	|| can_see_smoke(ch))
								&&  (!IS_SET(pexit->flags, EX_HIDDEN) || can_see_hidden(ch))
								&&   can_use_exit(ch, pexit))
								{
									switch (door)
									{
										case 0: *pto = 'N'; pto++; break;
										case 1: *pto = 'E'; pto++; break;
										case 2: *pto = 'S'; pto++; break;
										case 3: *pto = 'W'; pto++; break;
										case 4: *pto = 'U'; pto++; break;
										case 5: *pto = 'D'; pto++; break;
									}
								}
							}
						}
						break;
					case 'w':
						ch->wait ? (*pto = '*') : (*pto = ' ');
						pto++;
						break;
					case 't':
						sprintf(tbuf, "%s", tocivilian(mud->time_info->hour));
						strcpy(pto, tbuf);
						pto += strlen(tbuf);
						break;
					case 'T':
						clk = *localtime (&mud->current_time);
						if (CH(ch->desc)->pcdata->clock > 99)
						{
							sprintf(tbuf, "%2d:%02d", (clk.tm_hour + (CH(ch->desc)->pcdata->clock % 100)) % 24, clk.tm_min);
						}
						else
						{
							sprintf(tbuf, "%2d:%02d %s", clk.tm_hour % 12 != 0 ? clk.tm_hour % 12 : 12, clk.tm_min, clk.tm_hour >= 12 ? "pm" : "am");
						}
						if (IS_SET(CH(ch->desc)->act, PLR_WIZTIME))
						{
							sprintf(tbuf, "%02d:%02d", clk.tm_sec, (int) (get_game_usec() % 1000000 / 10000));
						}
						strcpy(pto, tbuf);
						pto += strlen(tbuf);
						break;
					case '$':
						*pto = '$';
						pto++;
						break;
					case '/':
						strcpy(pto, "\n\r");
						pto += 2;
						break;
					case 'c':
						gradient = TRUE;
						continue;
						break;
					default:
						*pto = '$';
						pto ++;
						*pto = *pti;
						pto ++;
						break;
				}
				gradient = FALSE;
			}
			else
			{
				if (*pti != '$')
				{
					*pto = *pti;
					pto++;
				}
				else
				{
					last_was_str = TRUE;
				}
				pti++;
			}
		}
		*pto = '\0';
	}

	pop_call();
	return (ansi_translate_text(ch, prompt_buffer));
}


int cat_sprintf(char *dest, const char *fmt, ...)
{
	char buf[MAX_STRING_LENGTH];
	int size;
	va_list args;

	va_start(args, fmt);
	size = vsprintf(buf, fmt, args);
	va_end(args);

	strcat(dest, buf);

	return size;
}

int cat_snprintf(char *dest, size_t leng, char *fmt, ...)
{
	char buf[MAX_STRING_LENGTH];
	int size;
	va_list args;

	va_start(args, fmt);
	size = vsprintf(buf, fmt, args);
	va_end(args);

	strncat(dest, buf, leng);

	return UMIN(size, leng);
}

void ch_printf(CHAR_DATA *ch, const char *fmt, ...)
{
	char buf[MAX_STRING_LENGTH];
	va_list args;

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	send_to_char(buf, ch);
}

void ch_printf_color(CHAR_DATA *ch, const char *fmt, ...)
{
	char buf[MAX_STRING_LENGTH];
	va_list args;

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	send_to_char_color(buf, ch);
}


void log_printf (char * fmt, ...)
{
	char buf [MAX_STRING_LENGTH];
	va_list args;

	va_start (args, fmt);
	vsprintf (buf, fmt, args);
	va_end (args);

	log_string (buf);
}

void log_build_printf (int vnum, char * fmt, ...)
{
	char buf [MAX_STRING_LENGTH];
	va_list args;

	va_start (args, fmt);
	vsprintf (buf, fmt, args);
	va_end (args);

	log_build_string (vnum, buf);
}

void log_god_printf (char * fmt, ...)
{
	char buf [MAX_STRING_LENGTH];
	va_list args;

	va_start (args, fmt);
	vsprintf (buf, fmt, args);
	va_end (args);

	log_god_string (buf);
}

bool scroll_you(DESCRIPTOR_DATA *d, const char *txi, bool youcheck)
{
	register char *pti;

	push_call("scroll_you(%p,%p,%p)",d,txi,youcheck);

	if (!youcheck)
	{
		pop_call();
		return(FALSE);
	}

	if (!IS_SET(CH(d)->pcdata->vt100_flags, VT100_BOLD))
	{
		pop_call();
		return FALSE;
	}

	if (d->connected != CON_PLAYING)
	{
		pop_call();
		return(FALSE);
	}

	if (strlen(txi) < 3)
	{
		pop_call();
		return FALSE;
	}

	for (pti = (char *) txi ; *(pti+2) != '\0' ; pti++)
	{
		if ((*(pti+0) == 'Y' || *(pti+0) == 'y')
		&&  (*(pti+1) == 'O' || *(pti+1) == 'o')
		&&  (*(pti+2) == 'U' || *(pti+2) == 'u'))
		{
			pop_call();
			return TRUE;
		}
	}
	pop_call();
	return FALSE;
}


/*
	Days till character expires.  less than 0 is dead.
*/

int character_expiration(CHAR_DATA *ch)
{
	int pdays, edays;

	push_call("character_expiration(%p)",ch);

	if (ch->pcdata->last_time == 0)
	{
		pop_call();
		return 1;
	}

	pdays = ch->pcdata->reincarnation * 730 + ch->level * 7 + 65;
	edays = (mud->current_time - ch->pcdata->last_time) / (60 * 60 * 24);

	pop_call();
	return (pdays - edays);
}


void scan_object_for_dup(CHAR_DATA *ch , OBJ_DATA *obj)
{
	OBJ_DATA *ref;
	int ref_ind;

	push_call("scan_object_for_dup(%p,%p)",ch,obj);

	while (obj)
	{
		ref_ind = obj->obj_ref_key % MAX_OBJ_REF;

		for (ref = obj_ref_list[ref_ind].first ; ref ; ref = ref->next_ref)
		{
			if (ref != obj && ref->obj_ref_key == obj->obj_ref_key)
			{
				if (IS_SET(ref->extra_flags, ITEM_NOT_VALID))
				{
					continue;
				}
				log_printf("OBJ DUPE %p [%d] carried by %s",  obj, obj->pIndexData->vnum, ch->name);

				if (ref->carried_by)
				{
					log_printf("OBJ DUPE %p [%d] carried by %s", ref, ref->pIndexData->vnum, ref->carried_by->name);
				}
				else if (ref->in_obj && ref->in_obj->carried_by)
				{
					log_printf("OBJ DUPE %p [%d] carried by %s inside [%d]", ref, ref->pIndexData->vnum, ref->in_obj->carried_by->name, ref->in_obj->pIndexData->vnum);
				}
				else
				{
					log_printf("OBJ DUPE %p [%d] carried by UNKNOWN", ref, ref->pIndexData->vnum);
				}
				SET_BIT(obj->extra_flags, ITEM_NOT_VALID);
				break;
			}
		}

		if (obj->first_content != NULL)
		{
			scan_object_for_dup(ch, obj->first_content);
		}
		obj = obj->next_content;
	}
	pop_call();
	return;
}

void add_obj_ref_hash(OBJ_DATA *obj)
{
	int ref_ind;

	push_call("add_obj_ref_hash(%p)",obj);

	if (obj->obj_ref_key == 0)
	{
		obj->obj_ref_key = obj_ref_key++;
	}

	ref_ind = obj->obj_ref_key % MAX_OBJ_REF;

	LINK(obj, obj_ref_list[ref_ind].first, obj_ref_list[ref_ind].last, next_ref, prev_ref);

	pop_call();
	return;
}


void rem_obj_ref_hash(OBJ_DATA *obj)
{
	int ref_ind;

	push_call("rem_obj_ref_hash(%p)",obj);

	ref_ind = obj->obj_ref_key % MAX_OBJ_REF;

	UNLINK(obj, obj_ref_list[ref_ind].first, obj_ref_list[ref_ind].last, next_ref, prev_ref);

	pop_call();
	return;
}


void wait_state(CHAR_DATA *ch, int npulse)
{
	push_call("wait_state(%p,%p)",ch,npulse);

	if (ch->desc)
	{
		ch->wait = UMAX(ch->wait, npulse);

		vt100prompt(ch);
	}
	pop_call();
	return;
}


void do_maillog(CHAR_DATA *ch, char *argument)
{
	FILE *logfp;
	char pathname[MAX_INPUT_LENGTH], sysbuf[MAX_INPUT_LENGTH];

	push_call("do_maillog(%p,%p)",ch,argument);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	if (ch->level < MAX_LEVEL)
	{
		send_to_char("You're not allowed to mail the logs.\n\r",ch);
		pop_call();
		return;
	}

	if (strstr(ch->pcdata->mail_address, "@") == NULL)
	{
		send_to_char("You don't have a valid email address set-up.\n\r",ch);
		pop_call();
		return;
	}

	/*
		actual mail sending should happen here...
	*/

	if (argument[0] == '\0')
	{
		sprintf(sysbuf, "mail -s 'Your request for log %s' %s < %s", logfilename, ch->pcdata->mail_address, logfilename);
		system(sysbuf);

		ch_printf(ch, "log %s sent to %s\n\r", logfilename, ch->pcdata->mail_address);
		pop_call();
		return;
	}
	else
	{
		sprintf(pathname, "../log/%d.log", atoi(argument));

		logfp = my_fopen(pathname, "r", FALSE);

		if (logfp == NULL)
		{
			send_to_char("There is no logfile with that number, sorry\n\r", ch);
			pop_call();
			return;
		}

		sprintf(sysbuf, "mail -s 'Your request for log %d.log'  %s < ../log/%d.log", atoi(argument), ch->pcdata->mail_address, atoi(argument));
		system(sysbuf);

		ch_printf(ch, "log %d.log sent to mail-address %s\n\r", atoi(argument), ch->pcdata->mail_address);
		pop_call();
		return;
	}
}


void do_llog(CHAR_DATA *ch, char *argument)
{
	int lines;
	char syscmd[100];
	FILE *fp;
	char buf[ MAX_STRING_LENGTH ];
	char letter;
	char *pt;

	push_call("do_llog(%p,%p)",ch,argument);

	lines = URANGE(20, atoi(argument), 200);

	close_reserve();

	ch_printf_color(ch, "{128}Log file used is %s\n\r", logfilename);

	sprintf(syscmd, "/usr/bin/tail -%d %s > data/llog.out", lines, logfilename);
	system(syscmd);

	fp = my_fopen("data/llog.out", "r",FALSE);

	if (fp == NULL)
	{
		send_to_char("No llog generated.\n\r", ch);
		open_reserve();
		pop_call();
		return;
	}

	*buf		= '\0';
	pt		= buf;
	letter	= getc(fp);

	while (letter != EOF)
	{
		if (letter != '\r')
		{
			*pt = letter;
			pt++;
		}

		if (letter == '\n')
		{
			*pt = '\r';	pt++;
			*pt = '{'; 	pt++;
			*pt = '1'; 	pt++;
			*pt = '7'; 	pt++;
			*pt = '8'; 	pt++;
			*pt = '}'; 	pt++;
		}
		letter = getc(fp);
	}

	*pt = '\0';

	ch_printf_color(ch, "{178}%s", buf);

	my_fclose(fp);
	open_reserve();

	unlink("data/llog.out");

	pop_call();
	return;
}


