/***************************************************************************
 * Lola  1.0 by Igor van den Hoven.                                        *
 *                                                                         *
 * Emud  3.0 by Igor van den Hoven.                                        *
 ***************************************************************************/

#include <dirent.h>
#include "mud.h"

/*
	Scandum 28-04-2003
*/

int purger_dir1	= 0;
int purger_dir2	= 0;
int purger_index	= 0;

struct dirent **purger_list;

void start_purger()
{
	push_call("start_purger(void)");

	if (IS_SET(mud->flags, MUD_PURGER))
	{
		log_printf("start_purger: already running");
		pop_call();
		return;
	}

	SET_BIT(mud->flags, MUD_PURGER);

	purger_index	= -1;
	purger_dir1	= 0;
	purger_dir2	= 0;

	log_printf("start_purger: purger started");

	pop_call();
	return;
}


void update_purger()
{
	CHAR_DATA *pch;
	char dir_buf[20];

	push_call("update_purger(void)");

	if (purger_index <= -1)
	{
		if (purger_dir2 >= 26)
		{
			purger_dir1++;
			purger_dir2 = 0;
		}

		if (purger_dir1 >= 26)
		{
			log_printf("update_purger: scan completed");

			REMOVE_BIT(mud->flags, MUD_PURGER);

			free(purger_list);

			pop_call();
			return;
		}
		/*
		if (purger_dir2 == 0)
		{
			log_printf("update_purger: scanning dir %c", 'a' + purger_dir1);
		}
		*/
		sprintf(dir_buf, "%s/%c/%c", PLAYER_DIR, 'a' + purger_dir1, 'a' + purger_dir2);

		purger_index = scandir(dir_buf, &purger_list, 0, alphasort);

		purger_dir2++;

		pop_call();
		return;
	}

	if (--purger_index <= -1)
	{
		pop_call();
		return;
	}

	strcpy(dir_buf, capitalize_name(purger_list[purger_index]->d_name));

	pch = lookup_char(dir_buf);

	if (pch == NULL)
	{
		pch = start_partial_load(NULL, dir_buf);

		if (pch)
		{
			clear_partial_load(pch);
		}
	}
	free(purger_list[purger_index]);

	pop_call();
	return;
}
