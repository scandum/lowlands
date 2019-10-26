/***************************************************************************
 * Lola  1.0 by Igor van den Hoven.                                        *
 *                                                                         *
 * Emud  3.0 by Igor van den Hoven.                                        *
 ***************************************************************************/

#include "mud.h"

/*
	mathematical expression interpreter - Scandum
*/

#define EXP_VARIABLE         0
#define EXP_STRING           1
#define EXP_OPERATOR         2

#define EXP_PR_DICE          0
#define EXP_PR_INTMUL        1
#define EXP_PR_INTADD        2
#define EXP_PR_BITSHIFT      3
#define EXP_PR_LOGLTGT       4
#define EXP_PR_LOGCOMP       5
#define EXP_PR_BITAND        6
#define EXP_PR_BITXOR        7
#define EXP_PR_BITOR         8
#define EXP_PR_LOGAND        9
#define EXP_PR_LOGXOR       10
#define EXP_PR_LOGOR        11
#define EXP_PR_VAR          12
#define EXP_PR_LVL          13


MATH_DATA *mathnode_s;
MATH_DATA *mathnode_e;
MATH_DATA *mathnode_n;


long long mathexp(CHAR_DATA *ch, const char *str)
{
	MATH_DATA *node;

	push_call("mathexp(%p,%p)",ch,str);

	if (mathexp_tokenize(ch, str) == FALSE)
	{
		if (mud->f_math == NULL)
		{
			if (IS_NPC(ch))
			{
				log_build_printf(ch->pIndexData->vnum, "mathexp: invalid input");
			}
			else
			{
				ch_printf(ch, "mathexp: invalid input: ");
			}
		}
		pop_call();
		return 0;
	}

	node = mud->f_math;

	while (node->prev || node->next)
	{
		if (node->next == NULL || atoi(node->next->str1) < atoi(node->str1))
		{
			mathexp_level(ch, node);

			node = mud->f_math;
		}
		else
		{
			node = node->next;
		}
	}
	pop_call();
	return tintoi(node->str3);
}


void add_mathnode(char *buf1, char *buf2, char *buf3)
{
	MATH_DATA *node;

	push_call("add_mathnode(void)");

	ALLOCMEM(node, MATH_DATA, 1);

	node->str1 = STRALLOC(buf1);
	node->str2 = STRALLOC(buf2);
	node->str3 = STRALLOC(buf3);

	LINK(node, mud->f_math, mud->l_math, next, prev);

	pop_call();
	return;
}

void del_mathnode(MATH_DATA *node)
{
	push_call("del_mathnode(%p)",node);

	STRFREE(node->str1);
	STRFREE(node->str2);
	STRFREE(node->str3);

	UNLINK(node, mud->f_math, mud->l_math, next, prev);

	FREEMEM(node);

	pop_call();
	return;
}

#define MATH_NODE(buffercheck, priority, newstatus)  \
{                                                    \
	if (buffercheck && pta == buf3)              \
	{                                            \
		return FALSE;                        \
	}                                            \
	*pta = 0;                                    \
	sprintf(buf1, "%02d", level);                \
	sprintf(buf2, "%02d", priority);             \
	add_mathnode(buf1, buf2, buf3);              \
	status = newstatus;                          \
	pta = buf3;                                  \
}


long long mathexp_tokenize(CHAR_DATA *ch, const char *str)
{
	char buf1[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH], buf3[MAX_INPUT_LENGTH], *pti, *pta;
	int level, status;

	push_call("mathexp_tokenize(%p,%p)",ch,str);

	level  = 0;
	status = EXP_VARIABLE;

	pta = (char *) buf3;
	pti = (char *) str;

	while (mud->f_math)
	{
		del_mathnode(mud->f_math);
	}

	mud->f_math = mud->l_math = NULL;

	while (*pti)
	{
		switch (status)
		{
			case EXP_VARIABLE:
				switch (*pti)
				{
					case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
						*pta++ = *pti++;
						break;

					case '!':
					case '~':
					case '+':
					case '-':
						if (pta != buf3)
						{
							MATH_NODE(FALSE, EXP_PR_VAR, EXP_OPERATOR);
						}
						else
						{
							*pta++ = *pti++;
						}
						break;

					case '"':
						if (pta != buf3)
						{
							if (IS_NPC(ch))
							{
								log_build_printf(ch->pIndexData->vnum, "mathexp: \" found inside integer");
							}
							else
							{
								ch_printf(ch, "mathexp: \" found inside integer: ");
							}
							pop_call();
							return FALSE;
						}
						*pta++ = *pti++;
						status = EXP_STRING;
						break;

					case '(':
						if (pta != buf3)
						{
							if (IS_NPC(ch))
							{
								log_build_printf(ch->pIndexData->vnum, "mathexp: ( found inside integer");
							}
							else
							{
								ch_printf(ch, "mathexp: ( found inside integer: ");
							}
							pop_call();
							return FALSE;
						}
						*pta++ = *pti++;
						MATH_NODE(TRUE, EXP_PR_LVL, EXP_VARIABLE);
						level++;
						break;

					case ' ':
						pti++;
						break;

					default:
						MATH_NODE(TRUE, EXP_PR_VAR, EXP_OPERATOR);
						break;
				}
				break;

			case EXP_STRING:
				switch (*pti)
				{
					case '"':
						*pta++ = *pti++;
						MATH_NODE(FALSE, EXP_PR_VAR, EXP_OPERATOR);
						break;

					default:
						*pta++ = *pti++;
						break;
				}
				break;

			case EXP_OPERATOR:
				switch (*pti)
				{
					case ' ':
						pti++;
						break;

					case ')':
						*pta++ = *pti++;
						level--;
						MATH_NODE(TRUE, EXP_PR_LVL, EXP_OPERATOR);
						break;

					case 'd':
						*pta++ = *pti++;
						MATH_NODE(FALSE, EXP_PR_DICE, EXP_VARIABLE);
						break;

					case '*':
					case '/':
					case '%':
						*pta++ = *pti++;
						MATH_NODE(FALSE, EXP_PR_INTMUL, EXP_VARIABLE);
						break;

					case '+':
					case '-':
						*pta++ = *pti++;
						MATH_NODE(FALSE, EXP_PR_INTADD, EXP_VARIABLE);
						break;

					case '<':
						*pta++ = *pti++;

						switch (*pti)
						{
							case '<':
								*pta++ = *pti++;
								MATH_NODE(FALSE, EXP_PR_BITSHIFT, EXP_VARIABLE);
								break;

							case '=':
								*pta++ = *pti++;
								MATH_NODE(FALSE, EXP_PR_LOGLTGT, EXP_VARIABLE);
								break;

							default:
								MATH_NODE(FALSE, EXP_PR_LOGLTGT, EXP_VARIABLE);
								break;
						}
						break;

					case '>':
						*pta++ = *pti++;

						switch (*pti)
						{
							case '>':
								*pta++ = *pti++;
								MATH_NODE(FALSE, EXP_PR_BITSHIFT, EXP_VARIABLE);
								break;

							case '=':
								*pta++ = *pti++;
								MATH_NODE(FALSE, EXP_PR_LOGLTGT, EXP_VARIABLE);
								break;

							default:
								MATH_NODE(FALSE, EXP_PR_LOGLTGT, EXP_VARIABLE);
								break;
						}
						break;

					case '&':
						*pta++ = *pti++;

						switch (*pti)
						{
							case '&':
								*pta++ = *pti++;
								MATH_NODE(FALSE, EXP_PR_LOGAND, EXP_VARIABLE);
								break;

							default:
								MATH_NODE(FALSE, EXP_PR_BITAND, EXP_VARIABLE);
								break;
						}
						break;

					case '^':
						*pta++ = *pti++;

						switch (*pti)
						{
							case '^':
								*pta++ = *pti++;
								MATH_NODE(FALSE, EXP_PR_LOGXOR, EXP_VARIABLE);
								break;

							default:
								MATH_NODE(FALSE, EXP_PR_BITXOR, EXP_VARIABLE);
								break;
						}
						break;

					case '|':
						*pta++ = *pti++;

						switch (*pti)
						{
							case '|':
								*pta++ = *pti++;
								MATH_NODE(FALSE, 7, EXP_VARIABLE);
								break;

							default:
								*pta++ = *pti++;
								MATH_NODE(FALSE, 5, EXP_VARIABLE);
								break;
						}
						break;

					case '=':
					case '!':
						*pta++ = *pti++;
						switch (*pti)
						{
							case '=':
								*pta++ = *pti++;
								MATH_NODE(FALSE, EXP_PR_LOGCOMP, EXP_VARIABLE);
								break;

							default:
								if (IS_NPC(ch))
								{
									log_build_printf(ch->pIndexData->vnum, "mathexp: unknown operator: %c%c", pti[0], pti[1]);
								}
								else
								{
									ch_printf(ch, "mathexp: unknown operator: %c%c: ", pti[0], pti[1]);
								}
								pop_call();
								return FALSE;
						}
						break;

					default:
						if (IS_NPC(ch))
						{
							log_build_printf(ch->pIndexData->vnum, "mathexp: unknown operator: %c", *pti);
						}
						else
						{
							ch_printf(ch, "mathexp: unknown operator: %c: ", *pti);
						}
						pop_call();
						return FALSE;
				}
				break;
		}
	}

	if (level != 0)
	{
		if (IS_NPC(ch))
		{
			log_build_printf(ch->pIndexData->vnum, "mathexp: unmatched parentheses, level: %d", level);
		}
		else
		{
			ch_printf(ch, "mathexp: unmatched parentheses, level: %d: ", level);
		}
		pop_call();
		return FALSE;
	}

	if (status != EXP_OPERATOR)
	{
		MATH_NODE(TRUE, EXP_PR_VAR, EXP_OPERATOR);
	}
	pop_call();
	return TRUE;
}


void mathexp_level(CHAR_DATA *ch, MATH_DATA *node)
{
	int priority;

	mathnode_e = node;

	while (node->prev)
	{
		if (atoi(node->prev->str1) < atoi(node->str1))
		{
			break;
		}
		node = node->prev;
	}

	mathnode_s = node;

	for (priority = 0 ; priority < EXP_PR_VAR ; priority++)
	{
		for (node = mathnode_s ; node ; node = node->next)
		{
			if (atoi(node->str2) == priority)
			{
				mathexp_compute(ch, node);
			}
			if (node == mathnode_e)
			{
				break;
			}
		}
	}

	node = mathnode_s;

	while (node->prev && node->next)
	{
		if (atoi(node->prev->str2) == EXP_PR_LVL && atoi(node->next->str2) == EXP_PR_LVL)
		{
			STRFREE(node->str1);
			node->str1 = STRALLOC(node->next->str1);

			del_mathnode(node->next);
			del_mathnode(node->prev);
		}
		else
		{
			break;
		}
	}
	return;
}

void mathexp_compute(CHAR_DATA *ch, MATH_DATA *node)
{
	char temp[MAX_INPUT_LENGTH];
	long long value;

	switch (node->str3[0])
	{
		case 'd':
			if (tintoi(node->next->str3) <= 0)
			{
				ch_printf(ch, "mathexp: invalid dice: ");
				value = 0;
			}
			else
			{
				value = tindice(node->prev->str3, node->next->str3);
			}
			break;
		case '*':
			value = tintoi(node->prev->str3) * tintoi(node->next->str3);
			break;
		case '/':
			if (tintoi(node->next->str3) == 0)
			{
				ch_printf(ch, "mathexp: division zero: ");
				value = tintoi(node->prev->str3) / 1;
			}
			else
			{
				value = tintoi(node->prev->str3) / tintoi(node->next->str3);
			}
			break;
		case '%':
			if (tintoi(node->next->str3) == 0)
			{
				ch_printf(ch, "mathexp: division zero: ");
				value = tintoi(node->prev->str3) / 1;
			}
			else
			{
				value = tintoi(node->prev->str3) % tintoi(node->next->str3);
			}
			break;
		case '+':
			value = tintoi(node->prev->str3) + tintoi(node->next->str3);
			break;
		case '-':
			value = tintoi(node->prev->str3) - tintoi(node->next->str3);
			break;
		case '<':
			switch (node->str3[1])
			{
				case '=':
					value = tincmp(node->prev->str3, node->next->str3) <= 0;
					break;
				case '<':
					value = tintoi(node->prev->str3) << tintoi(node->next->str3);
					break;
				default:
					value = tincmp(node->prev->str3, node->next->str3) < 0;
					break;
			}
			break;
		case '>':
			switch (node->str3[1])
			{
				case '=':
					value = tincmp(node->prev->str3, node->next->str3) >= 0;
					break;
				case '>':
					value = tintoi(node->prev->str3) >> tintoi(node->next->str3);
					break;
				default:
					value = tincmp(node->prev->str3, node->next->str3) > 0;
					break;
			}
			break;

		case '&':
			switch (node->str3[1])
			{
				case '&':
					value = tintoi(node->prev->str3) && tintoi(node->next->str3);
					break;
				default:
					value = tintoi(node->prev->str3) & tintoi(node->next->str3);
					break;
			}
			break;
		case '^':
			switch (node->str3[1])
			{
				case '^':
					value = ((tintoi(node->prev->str3) || tintoi(node->next->str3)) && (!tintoi(node->prev->str3) != !tintoi(node->next->str3)));
					break;

				default:
					value = tintoi(node->prev->str3) ^ tintoi(node->next->str3);
					break;
			}
			break;
		case '|':
			switch (node->str3[1])
			{
				case '|':
					value = tintoi(node->prev->str3) || tintoi(node->next->str3);
					break;
				default:
					value = tintoi(node->prev->str3) | tintoi(node->next->str3);
					break;
			}
			break;
		case '=':
			value = (tincmp(node->next->str3, node->prev->str3) == 0);
			break;
		case '!':
			value = (tincmp(node->next->str3, node->prev->str3) != 0);
			break;
		default:
			value = 0;
			break;
	}

	if (node->prev == mathnode_s)
	{
		mathnode_s = node;
	}

	if (node->next == mathnode_e)
	{
		mathnode_e = node;
	}

	del_mathnode(node->next);
	del_mathnode(node->prev);

	sprintf(temp, "%d", EXP_PR_VAR);
	STRFREE(node->str2);
	node->str2 = STRALLOC(temp);

	sprintf(temp, "%lld", value);
	STRFREE(node->str3);
	node->str3 = STRALLOC(temp);
}

long long tintoi(const char *str)
{
	switch (str[0])
	{
		case '!':
			return !atoll(&str[1]);

		case '~':
			return ~atoll(&str[1]);

		case '+':
			return +atoll(&str[1]);

		case '-':
			return -atoll(&str[1]);

		default:
			return atoll(str);
	}
}

long long tincmp(const char *left, const char *right)
{
	switch (left[0])
	{
		case '"':
			return strcmp(left, right);

		default:
			return tintoi(left) - tintoi(right);
	}
}

long long tindice(const char *left, const char *right)
{
	long long cnt, numdice, sizedice, sum;

	numdice  = tintoi(left);
	sizedice = tintoi(right);

	for (cnt = sum = 0 ; cnt < numdice ; cnt++)
	{
		sum += lrand48() % sizedice + 1;
	}
	return sum;
}
