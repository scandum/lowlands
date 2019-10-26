/***************************************************************************
 * Lola  1.0 by Igor van den Hoven.                                        *
 *                                                                         *
 * Emud  3.0 by Igor van den Hoven.                                        *
 ***************************************************************************/

#include "mud.h"

/*
	Generate a kinky hash index - Scandum
*/
/*
int str_leng_alloc;

unsigned int get_hash( char *str)
{
	unsigned int h;

	for (h = str_leng_alloc = 0 ; *str != 0 ; str++, str_leng_alloc++)
	{
		h -= (4 - (h >> 7)) * *str;
	}

	h = (h + str_leng_alloc) % MAX_STR_HASH;

	return h;
}
*/

char *str_alloc( char *str )
{
	return strdup(str);
}

char *str_dupe( char *str )
{
	return strdup(str);
}

void str_free( char *str )
{
	push_call("str_free(%p)",str);

	if (str == NULL)
	{
		pop_call();
		return;
	}

	free(str);

	pop_call();
	return;
}
