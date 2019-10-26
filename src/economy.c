/***************************************************************************
 * Lola  1.0 by Igor van den Hoven.                                        *
 ***************************************************************************/

#include "mud.h"

int obj_worth(OBJ_INDEX_DATA *objIndex)
{
	int cost, lvl_neg, lvl_pos;

	obj_level(objIndex, &lvl_neg, &lvl_pos);

	number_seed(objIndex->vnum);

	cost = number_range(5, 10) + number_range(75, 125) * lvl_pos / 10;
	
	switch (objIndex->item_type)
	{
		case ITEM_MONEY:
		case ITEM_TREASURE:
			cost = objIndex->value[0];
			break;
	}

	return cost;
}

int mob_worth(MOB_INDEX_DATA *mobIndex)
{
	int level;

	if (mobIndex->gold == 0)
	{
		return 0;
	}

	level = mobIndex->level;

	if (HAS_BIT(mobIndex->affected_by, AFF_SANCTUARY))
	{
		level += level / 5;
	}

	return number_range(1, 10) + level;
}
