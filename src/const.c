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

#include "mud.h"


const	struct	god_type 		god_table		[MAX_GOD]		=
{
	{
		"the Gods",
		"The sands of time slowly swirl until your return.",
		"This player likes living a peaceful life.",
		"You receive %d experience points.",
		"The favour of the Gods makes you stronger.",
		"The gods have retrieved your soul from oblivion.",
		"The gods have forsaken you.",
		"Raw energy flows from $N's corpse, seeping into you.",
		"Raw energy flows from $N's corpse, seeping into $n.",
		"%s has recalled from the blazing anger of %s!",
		"{138}",
		" ",
		DAM_NONE,
		0, 0, 0
	},

	{
		"Chaos",
		"The fires below simmer restlessly as Lord Chaos bades you farewell.",
		"This player is a follower of Chaos.",
		"The forces of entropy triumph, you gain %d experience points.",
		"The warmth of fire spreads through your body.",
		"Lord Chaos has retrieved your soul from the fires below.",
		"The Lord of Chaos has forsaken you.",
		"Bolts of eratic fire soar from $N's corpse, engulfing you.",
		"Bolts of eratic fire soar from $N's corpse, engulfing $n.",
		"%s has recalled from the eratic frenzy of %s!",
		"{118}",
		"C",
		DAM_FIRE,
		1, 1, 1
	},

	{
		"Order",
		"Peaceful waters flow as Lord Order bades you farewell and awaits your return.",
		"This player is a follower of Order.",
		"The forces of harmony rejoice, you gain %d experience points.",
		"The warmth of harmony spreads through your body.",
		"Lord Order has retrieved your soul from oblivion.",
		"The Lord of Order has forsaken you.",
		"Bolts of ice rush from $N's corpse, melting into you.",
		"Bolts of ice rush from $N's corpse, melting into $n.",
		"%s has recalled from the righteous fury of %s!",
		"{168}",
		"O",
		DAM_COLD,
		1, 1, 1
	}
};

const	struct	sector_type	sector_table	[SECT_MAX] =
{
	{	"inside",
		2,
		7 + 0 * 8 + 0 * 128,
		SFLAG_INDOORS|SFLAG_NOWEATHER,
		50,
		25
	},

	{
		"city",
		2,
		5 + 7 * 8 + 0 * 128,
		SFLAG_NONE,
		50,
		30
	},
	{
		"field",
		3,
		0 + 3 * 8 + 0 * 128,
		SFLAG_NONE,
		50,
		25
	},
	{
		"forest",
		4,
		1 + 2 * 8 + 0 * 128,
		SFLAG_NONE,
		25,
		50
	},
	{
		"hills",
		5,
		0 + 7 * 8 + 0 * 128,
		SFLAG_NONE,
		50,
		40
	},
	{
		"mountain",
		6,
		4 + 7 * 8 + 0 * 128,
		SFLAG_NONE,
		50,
		40
	},
	{
		"lake",
		4,
		4 + 6 * 8 + 0 * 128,
		SFLAG_SWIM,
		50,
		25
	},
	{
		"river",
		4,
		7 + 6 * 8 + 0 * 128,
		SFLAG_SWIM,
		50,
		25
	},
	{
		"ocean",
		6,
		6 + 4 * 8 + 0 * 128,
		SFLAG_SWIM,
		50,
		25
	},
	{	"air",
		1,
		0 + 6 * 8 + 0 * 128,
		SFLAG_NONE,
		50,
		25
	},
	{	"desert",
		8,
		1 + 3 * 8 + 0 * 128,
		SFLAG_NONE,
		50,
		30
	},
	{
		"lava",
		10,	3 + 1 * 8 + 0 * 128,
		SFLAG_NONE,
		100,
		50
	},
	{
		"ethereal",
		1,
		2 + 0 * 8 + 0 * 128,
		SFLAG_NOWEATHER,
		100,
		50
	},
	{
		"astral",
		1,
		3 + 0 * 8 + 0 * 128,
		SFLAG_NOWEATHER,
		100,
		50
	},
	{
		"underwater",
		10,
		4 + 0 * 8 + 0 * 128,
		SFLAG_NOWEATHER,
		-100,
		75
	},
	{
		"underground",
		3,
		5 + 0 * 8 + 0 * 128,
		SFLAG_INDOORS|SFLAG_NOWEATHER,
		-50,
		50
	},
	{
		"deepearth",
		3,
		1 + 0 * 8 + 0 * 128,
		SFLAG_INDOORS|SFLAG_NOWEATHER,
		-50,
		50
	},
	{
		"road",
		1,
		3 + 7 * 8 + 0 * 128,
		SFLAG_NONE,
		50,
		25
	},
	{
		"swamp",
		6,
		2 + 3 * 8 + 0 * 128,
		SFLAG_NONE,
		25,
		50
	},
	{
		"beach",
		4,
		7 + 3 * 8 + 0 * 128,
		SFLAG_NONE,
		50,
		25
	},
	{
		"tundra",
		3,
		1 + 7 * 8 + 0 * 128,
		SFLAG_NONE,
		50,
		30
	},
	{
		"edge",
		10,
		0 + 0 * 8 + 1 * 128,
		SFLAG_NONE,
		0,
		25
	}
};

const struct language_type language_table [MAX_LANGUAGE] =
{
	{
		"Human",
		"Hum",
		LANGFLAG_NONE
	},
	{
		"Elven",
		"Elv",
		LANGFLAG_NONE
	},
	{
		"Drow",
		"Dro",
		LANGFLAG_NONE
	},
	{
		"Dwarven",
		"Dwa",
		LANGFLAG_NONE
	},
	{
		"Gnomish",
		"Gno",
		LANGFLAG_NONE
	},
	{
		"Orcish",
		"Orc",
		LANGFLAG_NONE
	},
	{
		"Ogrish",
		"Ogr",
		LANGFLAG_NONE
	},
	{
		"Halfling",
		"Hal",
		LANGFLAG_NONE
	},
	{
		"Demonic",
		"Dem",
		LANGFLAG_CLASS
	},
	{
		"Archaic",
		"Arc",
		LANGFLAG_CLASS
	},
	{
		"Runic",
		"Run",
		LANGFLAG_CLASS
	},
	{
		"Templar",
		"Tem",
		LANGFLAG_CLASS
	},
	{
		"Dragonian",
		"Dra",
		LANGFLAG_CLASS
	},
	{
		"Entropian",
		"Ent",
		LANGFLAG_CHAOS
	},
	{
		"Equilibrium",
		"Equ",
		LANGFLAG_ORDER
	}
};
	
const struct race_type race_table [MAX_RACE] =
{
	{
		"Human",
		{0,0,0,0,0},
		{1,1,0,1,0,1,0,1},
		 SPEED_JOG,    MOVE_MEDIUM, VISION_NORMAL,
		 0,	 0,	 0,
		"Swim.",
		"Humans are the most numerous, plainest and flexible race in the realms.",
		 0,   5,
		 165, 6,
		 0,
		 0,
		 0,
		 0,
		 RSPEC_SWIM|RSPEC_FASTHEAL,
		 BODY_FOOT|BODY_HAND|BODY_LEG|BODY_ARM|BODY_HEAD|BODY_TORSO|BODY_HIP|BODY_MOUTH,
		 LANG_HUMAN,
		{0, 0, 0, 0, 0, 0},
		{7, 5, 9, 9, 9, 7}
	},

	{
		"Elf",
		{-1,1,1,1,-2},
		{1,0,1,1,0,1,1,0},
		 SPEED_RUN,    MOVE_LIGHT,  VISION_NIGHT,
		-2,  1,  1,
		"Enhanced reviving.",
		"Tall, slender creatures with an affinity for the woods and nature, Elves may lack physical stature, but have superior agility, both in thought and movement.",
		 0,		2,
		 150,	6,
		 DAM_MAGIC,
		 0,
		 0,
		 DAM_LIFE,
		 RSPEC_FASTREVIVE,
		 BODY_FOOT|BODY_HAND|BODY_LEG|BODY_ARM,
		 LANG_ELVEN,
		{2, 0, 0, 0, 0, 1},
		{7, 3,16, 9, 9, 7}
	},

	{
		"Drow",
		{0,1,0,-1,0},
		{0,0,1,1,1,1,0,1},
		 SPEED_RUN,    MOVE_MEDIUM, VISION_DARK,
		-1,  0,  1,
		"Glob of darkness.",
		"The dark cousin of the elf, the Drow live underground in a vast network of caverns and cities. They are lightning fast and posses dark powers.",
		 0,		2,
		 160,	6,
		 DAM_MAGIC,
		 0,
		 0,
		 DAM_LIGHT,
		 RSPEC_NONE,
		 BODY_FOOT|BODY_HAND|BODY_LEG|BODY_ARM,
		 LANG_DROW,
		{2, 6,10, 0, 0, 1},
		{7,10,16, 1, 9, 7}
	},

	{
		"Dwarf",
		{1,-1,-1,-1,2},
		{1,1,1,0,1,0,1,0},
		 SPEED_JOG, MOVE_HEAVY,   VISION_DARK,
		 1,  0,  3,
		"Magic resistance.",
		"Dwarves are relatively short, stocky and powerfully built creatures. Long lived and extremely sturdy, they are famed for their hardiness.",
		 0,		2,
		 130,	4,
		 DAM_MAGIC|DAM_PSIONIC|DAM_EVIL|DAM_POISON,
		 0,
		 DAM_PIERCE,
		 DAM_LIGHT,
		 RSPEC_MAGIC_DEFENSE,
		 BODY_FOOT|BODY_HAND|BODY_LEG|BODY_ARM|BODY_HEAD|BODY_TORSO,
		 LANG_DWARVEN,
		{0, 0, 0, 0, 0, 0},
		{5, 3, 9, 9, 9, 7}
	},

	{
		"Gnome",
		{0,-1,1,1,0},
		{1,1,0,0,1,1,1,0},
		 SPEED_JOG, MOVE_HEAVY,   VISION_NORMAL,
		-1,  2,  -1,
		"Forage for food.",
		"A short and quirky race, gnomes are not the mightiest of races, though they are well versed in their studies of knowledges, magics and religions.",
		 0,		2,
		 65,		2,
		 DAM_MAGIC,
		 0,
		 0,
		 DAM_LIFE,
		 RSPEC_FORAGE,
		 BODY_FOOT|BODY_HAND,
		 LANG_GNOMISH,
		{0, 0, 0, 0, 0, 0},
		{5, 3, 9, 9, 9, 7}
	},

	{
		"Orc",
		{1,1,-2,-1,1},
		{0,1,1,1,0,0,1,1},
		 SPEED_JOG,   MOVE_LIGHT,   VISION_NIGHT,
		 1,  -2,  2,
		"Street fighting.",
		"Large and dull of wit, with short but broad bodies, their poor mental dexterity is offset by their iron constitution and impressive physical strength.",
		 0,		2,
		 180,	6,
		 DAM_THRUST,
		 0,
		 DAM_HOLY,
		 0,
		 RSPEC_BRAWLING,
		 BODY_FOOT|BODY_HAND|BODY_LEG|BODY_ARM|BODY_HEAD|BODY_MOUTH,
		 LANG_ORCISH,
		{0,11,10, 0, 0, 0},
		{3,19,16, 9, 9, 6}
	},

	{
		"Ogre",
		{2,1,-2,-3,2},
		{1,1,0,0,1,0,1,1},
		 SPEED_JOG,   MOVE_HEAVY,    VISION_NIGHT,
		 2,  -4,  1,
		"Intimidation.",
		"Huge and dull of wit, with short but broad bodies, their poor mental dexterity is offset by their iron constitution and impressive physical strength.",
		 0,     2,
		 180,	6,
		 DAM_THRUST,
		 0,
		 DAM_HOLY,
		 0,
		 RSPEC_INTIMIDATE,
		 BODY_FOOT|BODY_HAND|BODY_LEG|BODY_ARM|BODY_HEAD|BODY_MOUTH,
		 LANG_OGRISH,
		{0,11,10, 0, 0, 0},
		{3,19,16, 9, 9, 6}
	},

	{
		"Hobbit",
		{-1,1,0,0,1},
		{0,0,1,1,1,1,0,1},
		 SPEED_JOG, MOVE_LIGHT,     VISION_NORMAL,
		 0,  0,  2,
		"Sneaking barefoot.",
		"Hobbit's have furry feet.",
		 0,		2,
		 90,		3,
		 DAM_MAGIC,
		 0,
		 DAM_LIFE,
		 0,
		 RSPEC_NONE,
		 BODY_FOOT|BODY_HAND|BODY_LEG|BODY_ARM,
		 LANG_HALFLING,
		{0, 0, 0, 0, 0, 0},
		{7, 5, 9, 9, 9, 7}
	},

	{
		"Werewolf",
		{2,1,-2,-1,2},
		{0,1,0,0,1,1,0,1},
		 3,	 2,	2,
		 2,	-2,	0,
		"Undead, Ability to morph into a wolf.",
		"Werewolfs are humans that can change into a wolf.",
		 3,		4,
	 	 200,	6,
		 DAM_SLICE|DAM_BASH|DAM_REND|DAM_THRUST,
		 0,
		 DAM_LIGHT|DAM_FIRE,
		 0,
		 RSPEC_UNDEAD|RSPEC_VAMPIRIC,
		 BODY_HAND|BODY_FOOT|BODY_LEG|BODY_ARM|BODY_HEAD,
		 LANG_HUMAN,
		{0, 0, 0, 0, 0, 0},
		{7, 7, 9, 9, 9, 7}
	},

	{
		"Demon",
		{3,0,-3,0,3},
		{0,1,1,0,1,0,1,0},
		 2,	 2,	 2,
		 4,	-4,	-1,
		"Natural ability for flight and curse.",
		"Demons are cruel.",
		 3,		4,
		 225,	7,
		 0,
		 DAM_FIRE,
		 DAM_COLD,
		 DAM_COLD,
		 RSPEC_UNDEAD|RSPEC_FLYING|RSPEC_BRAWLING,
		 BODY_CLAW|BODY_WING|BODY_FOOT|BODY_MOUTH|BODY_LEG|BODY_ARM,
		 LANG_DEMONIC,
		{0,11,10, 0, 0, 0},
		{7,19,16, 9, 9, 7}
	},

	{
		"Wraith",
		{-1,-1,3,3,-1},
		{1,1,1,1,0,0,0,0},
		 1,	4,	 0,
		-2,	5,	-1,
		"Natural ability to passdoor and resist magic.",
		"Wraits are undead mages.",
		 3,		4,
		 90,		6,
		 DAM_PSIONIC,
		 DAM_MAGIC,
		 0,
		 DAM_LIGHT|DAM_FIRE,
		 RSPEC_UNDEAD|RSPEC_VAMPIRIC|RSPEC_MAGIC_DEFENSE|RSPEC_PASS_DOOR,
		 BODY_EYE,
		 LANG_ARCHAIC,
		{0, 0, 0, 0, 0, 0},
		{7, 7, 9, 9, 9, 7}
	},

	{
		"Troll",
		{5,-1,-4,-1,5},
		{0,0,0,1,1,1,0,1},
		 2,	 4,	 1,
		 4,	-3,	 0,
		"Fast Regenerate.",
		"Uhm...",
		 3,		4,
		 400,	8,
		 DAM_BASH|DAM_THRUST,
		 0,
		 0,
		 DAM_FIRE|DAM_LIGHT,
		 RSPEC_INTIMIDATE|RSPEC_AGE_DR|RSPEC_FASTHEAL|RSPEC_FASTREVIVE|RSPEC_BRAWLING,
		 BODY_HAND|BODY_FOOT|BODY_LEG|BODY_ARM|BODY_HEAD|BODY_TORSO,
		 LANG_OGRISH,
		{0,11,10, 0, 0, 0},
		{3,19,16, 9, 9, 0}
	},

	{
		"Vampire",
		{1,1,0,1,1},
		{0,1,0,0,1,0,1,0},
		 3,	 0,	 2,
		 3,	 1,	 0,
		"Regenerate by drinking victim's blood",
		"Vampires are some kind of Dracula (or is it the other way around ?)",
		 3,		4,
		 165,	6,
		 DAM_THRUST|DAM_BASH,
		 0,
		 0,
		 DAM_LIGHT|DAM_FIRE,
		 RSPEC_UNDEAD|RSPEC_FLYING|RSPEC_VAMPIRIC|RSPEC_AGE_HR,
		 BODY_HAND|BODY_FOOT|BODY_LEG|BODY_ARM|BODY_HEAD,
		 LANG_HUMAN,
		{0, 0, 0, 0, 0, 0},
		{7, 7, 9, 9, 9, 7}
	},

	{
		"Titan",
		{3,-3,3,0,3},
		{0,1,1,1,0,0,0,1},
		 2,	 1,	 0,
		 4,	 2,	-4,
		"Aging Hitroll.",
		"Stood up against the gods, now are less mighty than they were.",
		 5,		6,
		 500,	10,
		 DAM_MAGIC,
		 DAM_PSIONIC,
		 0,
		 DAM_LIFE,
		 RSPEC_AGE_HR,
		 BODY_HAND|BODY_FOOT|BODY_LEG|BODY_ARM|BODY_HEAD|BODY_TORSO|BODY_HIP,
		 LANG_OGRISH,
		{0, 0, 0, 0, 0, 0},
		{7, 7, 9, 9, 9, 7}
	},

	{
		"Ent",
		{2,0,2,0,6},
		{0,0,0,0,0,0,0,0},
		 4,	1,	1,
		 4,	2,	2,
		"They grew from the earth.",
		"Ents are the champions of Gaia",
		 5,		6,
		 1000,	16,
		 DAM_LIFE,
		 DAM_LIFE,
		 0,
		 0,
		 RSPEC_INTIMIDATE|RSPEC_EASY_FLEE|RSPEC_MAGIC_DEFENSE|RSPEC_FASTHEAL|RSPEC_FASTREVIVE|RSPEC_AGE_AC|RSPEC_AGE_HR|RSPEC_AGE_DR,
		 BODY_BRANCH|BODY_TRUNK|BODY_ROOT,
		 LANG_RUNIC,
		{0, 0, 0, 0, 0, 0},
		{7, 7, 9, 9, 9, 7}
	},
	{
		"Wyvern",
		{2,2,2,2,2},
		{0,0,0,0,0,0,0,0},
		 4,	1,	1,
		 4,	2,	2,
		"Fearsome winged creatures.",
		"Dragons like to hang out in the void.",
		 5,		6,
		 1000,	16,
		 DAM_LIFE,
		 DAM_LIFE,
		 0,
		 0,
		 RSPEC_FLYING|RSPEC_INTIMIDATE|RSPEC_EASY_FLEE|RSPEC_MAGIC_DEFENSE,
		 BODY_BRANCH|BODY_TRUNK|BODY_ROOT,
		 LANG_DRACONIC,
		{0, 0, 0, 0, 0, 0},
		{7, 7, 9, 9, 9, 7}
	}
};


const struct class_type class_table [MAX_CLASS] =
{
	{
		"Ran", "Ranger", OBJ_VNUM_SCHOOL_SWORD,
		91, 95,
		15, 20,
		 1,  3,
		"Basic, but deadly fighters who love the outdoors.",
		CFLAG_SHIELD|CFLAG_EAT|CFLAG_QUAFF,
		APPLY_CON, APPLY_STR, 2, 2
	},

	{
		"Gla", "Gladiator", OBJ_VNUM_SCHOOL_SPEAR,
		87, 90,
		12, 17,
		 1,  5,
		"The famous and deadly arena fighters.",
		CFLAG_QUAFF|CFLAG_EAT,
		APPLY_DEX, APPLY_CON, 2, 2
	},

	{
		"Mar", "Marauder", OBJ_VNUM_SCHOOL_DAGGER,
		93, 97,
		13, 18,
		 1,  7,
		"Weak fighters with a killer backstab.",
		CFLAG_SHIELD|CFLAG_QUAFF|CFLAG_EAT,
		APPLY_DEX, APPLY_CON, 3, 1
	},

	{
		"Nin", "Ninja", 0,
		90, 93,
		11, 16,
		 2,  8,
		"The ninja shows up to kill when least expected.",
		CFLAG_QUAFF,
		APPLY_DEX, APPLY_STR, 3, 1
	},

	{
		"Ele", "Elementalist", OBJ_VNUM_SCHOOL_MACE,
		90, 95,
		10, 15,
		 4, 12,
		"Elemental magic wielders capable of ranged and area spells.",
		CFLAG_SHIELD|CFLAG_MANA,
		APPLY_WIS, APPLY_DEX, 2, 2
	},

	{
		"Ill", "Illusionist", OBJ_VNUM_SCHOOL_STAFF,
		95, 97,
		 9, 14,
		 6, 14,
		"Illusionists are subtle and quick to anger.",
		CFLAG_MANA,
		APPLY_INT, APPLY_WIS, 3, 1
	},

	{
		"Mon", "Monk",	0,
		88, 93,
		10, 16,
		 3, 11,
		"Divine spellcasters and healers who know how to throw a punch.",
		CFLAG_SHIELD|CFLAG_QUICK_FLEE|CFLAG_MANA,
		APPLY_STR, APPLY_CON, 3, 1
	},

	{
		"Nec", "Necromancer", OBJ_VNUM_SCHOOL_WHIP,
		96, 99,
		 8, 14,
		 7, 15,
		"Evil darkside magicians who master unholy magic.",
		CFLAG_MANA,
		APPLY_WIS, APPLY_INT, 3, 1
	}
};

const float class_eq_table [MAX_CLASS] [MAX_WEAR] =
{
	{1, 1, 1, 1, 1, 3.0, 2.0, 2.0, 1.0, 1.0, 1.5, 1.5, 2.0, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 2.0, 2.0, 1.5, 1.0, 1.0, 0.5, 0.0, 2.0, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 3.0, 2.0, 2.0, 1.0, 1.0, 1.0, 1.0, 2.0, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 2.0, 1.5, 1.5, 0.5, 1.0, 1.0, 0.0, 3.0, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 3.0, 2.0, 2.0, 1.0, 1.0, 1.0, 1.0, 2.0, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 2.0, 1.5, 2.0, 1.0, 0.5, 0.5, 0.0, 2.0, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 3.0, 2.0, 2.0, 1.0, 0.5, 1.0, 1.0, 2.0, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 2.0, 2.0, 2.0, 1.0, 0.5, 1.0, 1.0, 2.0, 1, 1, 1, 1, 1, 1, 1}
};


/*
	hitroll, damroll, carry, wield
*/

const struct str_app_type str_app [MAX_STAT] =
{
	{ -5, -4,    0,  0 }, /*  0 */
	{ -5, -4,    3,  1 },
	{ -3, -2,    3,  2 },
	{ -3, -1,   10,  3 },
	{ -2, -1,   25,  4 },
	{ -2, -1,   55,  5 }, /*  5 */
	{ -1,  0,   80,  6 },
	{ -1,  0,   90,  7 },
	{  0,  0,  100,  8 },
	{  0,  0,  100,  9 },
	{  0,  0,  115, 10 }, /* 10 */
	{  0,  0,  115, 11 },
	{  0,  0,  140, 12 },
	{  0,  0,  140, 13 },
	{  0,  1,  170, 14 },
	{  1,  1,  200, 15 }, /* 15 */
	{  1,  2,  240, 16 },
	{  2,  3,  290, 18 },
	{  2,  4,  350, 20 },
	{  3,  5,  420, 22 },
	{  3,  6,  500, 24 }, /* 20 */
	{  4,  7,  590, 26 },
	{  5,  7,  690, 28 },
	{  6,  8,  700, 30 },
	{  7,  8,  820, 32 },
	{  8,  9,  950, 34 }, /* 25 */
	{  9,  9, 1090, 36 },
	{ 10, 10, 1240, 38 },
	{ 11, 10, 1400, 40 },
	{ 12, 11, 1600, 42 },
	{ 13, 11, 1800, 44 }, /* 30 */
	{ 14, 12, 2000, 46 },
	{ 15, 12, 2200, 48 },
	{ 16, 13, 2400, 50 },
	{ 17, 13, 2600, 52 },
	{ 18, 14, 2800, 54 }, /* 35 */
	{ 19, 14, 3000, 56 },
	{ 20, 15, 3200, 58 },
	{ 21, 15, 3400, 60 },
	{ 22, 16, 3600, 62 },
	{ 23, 16, 3800, 64 }, /* 40 */
	{ 24, 17, 4000, 66 },
	{ 25, 17, 4200, 68 },
	{ 26, 18, 4400, 70 },
	{ 27, 18, 4600, 72 },
	{ 28, 19, 4800, 74 }, /* 45 */
	{ 29, 19, 5000, 76 },
	{ 30, 20, 5200, 78 },
	{ 31, 20, 5400, 80 },
	{ 32, 21, 5800, 82 },
	{ 33, 21, 6000, 84 }  /* 50 */
};

/*
	learn, mana
*/

const struct int_app_type int_app [MAX_STAT] =
{
	{  1, -5 }, /*  0 */
	{  2, -5 }, /*  1 */
	{  3, -4 },
	{  4, -4 }, /*  3 */
	{  5, -3 },
	{  6, -3 }, /*  5 */
	{  7, -3 },
	{  8, -2 },
	{  9, -2 },
	{ 10, -2 },
	{ 11, -1 }, /* 10 */
	{ 12, -1 },
	{ 13, -1 },
	{ 14, -1 },
	{ 15,  0 },
	{ 16,  0 }, /* 15 */
	{ 17,  0 },
	{ 18,  0 },
	{ 19,  1 }, /* 18 */
	{ 20,  1 },
	{ 21,  1 }, /* 20 */
	{ 22,  1 },
	{ 23,  2 },
	{ 24,  2 },
	{ 25,  2 },
	{ 26,  2 }, /* 25 */
	{ 27,  3 },
	{ 28,  3 },
	{ 29,  3 },
	{ 30,  4 },
	{ 31,  4 }, /* 30 */
	{ 32,  4 },
	{ 33,  5 },
	{ 34,  5 },
	{ 35,  5 },
	{ 36,  6 }, /* 35 */
	{ 37,  6 },
	{ 38,  6 },
	{ 39,  7 },
	{ 40,  7 },
	{ 41,  8 }, /* 40 */
	{ 42,  8 },
	{ 43,  9 },
	{ 44,  9 },
	{ 45, 10 },
	{ 46, 11 }, /* 45 */
	{ 47, 12 },
	{ 48, 13 },
	{ 49, 14 },
	{ 50, 15 },
	{ 51, 16 }  /* 50 */
};

/*
	practices, mana regen
*/

const struct wis_app_type wis_app [MAX_STAT] =
{
	{  0,   1 }, /* 0 */
	{  0,   3 },
	{  0,   6 },
	{  0,  10 },
	{  0,  15 },
	{  0,  21 }, /*  5 */
	{  0,  26 },
	{  0,  30 },
	{  0,  32 },
	{  1,  32 },
	{  1,  33 }, /* 10 */
	{  1,  34 },
	{  2,  35 },
	{  2,  37 },
	{  2,  39 },
	{  3,  41 }, /* 15 */
	{  3,  44 },
	{  4,  47 },
	{  4,  51 },
	{  5,  55 },
	{  5,  60 }, /* 20 */
	{  6,  66 },
	{  6,  72 },
	{  7,  79 },
	{  7,  87 },
	{  8,  95 }, /* 25 */
	{  8, 103 },
	{  9, 112 },
	{  9, 121 },
	{ 10, 131 },
	{ 10, 141 }, /* 30 */
	{ 11, 152 },
	{ 11, 163 },
	{ 12, 175 },
	{ 12, 187 },
	{ 13, 200 }, /* 35 */
	{ 13, 213 },
	{ 14, 227 },
	{ 14, 241 },
	{ 15, 256 },
	{ 15, 271 }, /* 40 */
	{ 16, 287 },
	{ 16, 303 },
	{ 17, 320 },
	{ 17, 337 },
	{ 18, 355 }, /* 45 */
	{ 18, 373 },
	{ 19, 392 },
	{ 19, 411 },
	{ 20, 432 },
	{ 20, 452 }  /* 50 */
};

/*
	defense, skill bonus, level bonus, regen
*/

const struct dex_app_type dex_app [MAX_STAT] =
{
	{   75, -10,  1,   1 }, /* 0 */
	{   70,  -9,  2,   3 },
	{   65,  -8,  3,   6 },
	{   50,  -7,  3,   9 },
	{   55,  -6,  4,  13 },
	{   50,  -5,  4,  17 }, /* 5 */
	{   45,  -4,  5,  22 },
	{   40,  -3,  5,  27 },
	{   35,  -3,  6,  33 },
	{   30,  -2,  6,  39 },
	{   25,  -2,  7,  48 }, /* 10 */
	{   20,  -1,  7,  55 },
	{   15,  -1,  8,  63 },
	{   10,   0,  8,  71 },
	{    5,   0,  9,  80 },
	{    0,   0,  9,  89 }, /* 15 */
	{   -5,   1, 10,  99 },
	{  -10,   1, 10, 109 },
	{  -15,   1, 10, 119 },
	{  -20,   1, 11, 130 },
	{  -25,   2, 11, 141 }, /* 20 */
	{  -30,   2, 11, 152 },
	{  -35,   2, 12, 164 },
	{  -40,   3, 12, 176 },
	{ - 45,   3, 12, 188 },
	{ - 50,   3, 13, 201 }, /* 25 */
	{ - 55,   4, 13, 214 },
	{ - 60,   4, 13, 227 },
	{ - 65,   4, 14, 241 },
	{ - 70,   5, 14, 255 },
	{ - 75,   5, 14, 269 }, /* 30 */
	{ - 80,   6, 14, 283 },
	{ - 85,   6, 15, 298 },
	{ - 90,   7, 15, 313 },
	{ - 95,   7, 15, 328 },
	{ -100,   8, 15, 343 }, /* 35 */
	{ -105,   8, 16, 359 },
	{ -110,   9, 16, 375 },
	{ -115,   9, 16, 391 },
	{ -120,  10, 16, 407 },
	{ -125,  10, 17, 421 }, /* 40 */
	{ -130,  11, 17, 438 },
	{ -135,  11, 17, 455 },
	{ -140,  12, 17, 472 },
	{ -145,  12, 18, 490 },
	{ -150,  13, 18, 508 }, /* 45 */
	{ -155,  13, 18, 526 },
	{ -160,  14, 19, 545 },
	{ -165,  14, 19, 564 },
	{ -170,  15, 20, 584 },
	{ -175,  15, 20, 604 } /* 50 */
};

/*
	level bonus, health regen
*/

const struct con_app_type con_app [MAX_STAT] =
{
	{ -6,   1 },/*  0 */
	{ -6,   2 },
	{ -5,   3 },
	{ -5,   6 },
	{ -4,   9 },
	{ -4,  12 }, /*  5 */
	{ -3,  15 },
	{ -3,  18 },
	{ -2,  21 },
	{ -2,  23 },
	{ -1,  25 }, /* 10 */
	{ -1,  26 },
	{  0,  27 },
	{  0,  27 },
	{  1,  28 },
	{  1,  29 }, /* 15 */
	{  2,  30 },
	{  2,  32 },
	{  3,  34 },
	{  3,  37 },
	{  4,  40 }, /* 20 */
	{  4,  44 },
	{  5,  49 },
	{  5,  54 },
	{  6,  60 },
	{  6,  66 }, /* 25 */
	{  7,  73 },
	{  7,  80 },
	{  8,  88 },
	{  8,  96 },
	{  9, 105 }, /* 30 */
	{  9, 114 },
	{ 10, 124 },
	{ 10, 134 },
	{ 11, 145 },
	{ 12, 157 }, /* 35 */
	{ 12, 169 },
	{ 13, 182 },
	{ 13, 195 },
	{ 14, 209 },
	{ 14, 225 }, /* 40 */
	{ 15, 240 },
	{ 15, 255 },
	{ 16, 271 },
	{ 16, 287 },
	{ 17, 304 }, /* 45 */
	{ 17, 321 },
	{ 18, 339 },
	{ 18, 357 },
	{ 19, 376 },
	{ 19, 395 }  /* 50 */
};



/*
 * Liquid properties.
 * Used in world.obj.
 */

const	struct	liq_type	liq_table	[LIQ_MAX]	=
{
	{ "water",               "clear",               {  0, 0, 10 } },  /*  0 */
	{ "beer",                "amber",               {  2, 0,  5 } },
	{ "wine",                "rose",                {  5, 1,  5 } },
	{ "ale",                 "brown",               {  2, 1,  5 } },
	{ "dark ale",            "dark",                {  1, 1,  5 } },

	{ "whisky",              "golden",              {  6, 0,  2 } },  /*  5 */
	{ "lemonade",            "pink",                {  0, 0,  8 } },
	{ "firewater",           "boiling",             { 10, 0,  0 } },
	{ "moonshine",           "golden",              {  8, 0,  2 } },
	{ "slime",               "green",               {  0, 1, -5 } },

	{ "milk",                "white",               {  0, 2,  6 } },  /* 10 */
	{ "tea",                 "tan",                 {  0, 0,  6 } },
	{ "coffee",              "black",               {  0, 0,  6 } },
	{ "blood",               "red",                 {  0, 3, -1 } },
	{ "salt water",          "clear",               {  0, 0, -3 } },

	{ "cola",                "cherry",              {  0, 0,  6 } },  /* 15 */
	{ "orange juice",        "orange",              {  0, 1,  6 } },
	{ "brandy",              "golden",              {  6, 0,  2 } },
	{ "icewine",             "purple",              {  4, 0,  4 } },
	{ "rum",                 "amber",               {  7, 0,  2 } },

	{ "vodka",               "clear",               {  7, 0,  2 } },  /* 20 */
	{ "champagne",           "golden",              {  5, 1,  5 } }
};


char * dir_name [] =
{
	"north", "east", "south", "west", "up", "down"
};

char * dir_name_short [] =
{
	"n", "e", "s", "w", "u", "d"
};

sh_int rev_dir [] =
{
	DIR_SOUTH, DIR_WEST, DIR_NORTH, DIR_EAST, DIR_DOWN, DIR_UP
};

char * wind_dir_name [] =
{
	"northern", "northeastern", "eastern", "southeastern",
	"southern", "southwestern", "western", "northwestern"
};

/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */

#define SLOT(n)	n

const struct body_type body_table [MAX_BODY] =
{
	{
		"head", "%s's head",
		"%s's head lies here.",
		"%s's head is slowly staring into space.",
		"head butt",
		"$n's head slowly falls off.",
		DAM_BASH
	},

	{
		"jaw", "%s's jaw bone",
		"%s's jaw bone lies here.",
		"%s's jaw bone has a few scraps of meat left on it.",
		"bite",
		"$n's jaw drops to the floor.",
		DAM_PIERCE
	},

	{
		"eye eyeball", "%s's eye",
		"%s's eyeball lies here.",
		"%s's eyeball looks very peircing.",
		"evil eye",
		"$n's eyeball pops out.",
		DAM_EVIL
	},

	{
		"heart", "%s's heart",
		"The torn-out heart of %s rots here.",
		"%s's heart does not have too many maggots on it.",
		"shoulder slam",
		"$n's heart is torn from $s chest.",
		DAM_BASH
	},

	{
		"hip", "%s's hip",
		"%s's hip is reeking here.",
		"%s's hip has some meat left on it.",
		"hip slam",
		"$n falls to pieces.",
		DAM_BASH
	},

	{
		"leg", "%s's leg",
		"%s's bloody leg rests on the ground.",
		"%s's leg is black, blue, and red.",
		"knee",
		"$n kicks $s leg off.",
		DAM_THRUST
	},

	{
		"arm", "%s's arm",
		"%s's arm is resting on the ground.",
		"%s's arm looks very strong.",
		"elbow slam",
		"$n's arm seems to have been unattached.",
		DAM_THRUST
	},

	{
		"wing", "%s's wing",
		"%s's wing rests here.",
		"%s's wing has a few cuts in it.",
		"wing slap",
		"$n is flying nowhere.",
		DAM_THRUST
	},

	{
		"tail", "%s's tail",
		"%s's tail is laying on the ground.",
		"%s's tail looks only slightly edible.",
		"tail slap",
		"$n can no longer wag $s tail.",
		DAM_BASH
	},

	{
		"tentacle", "%s's tentacle",
		"%s's tentacle rests here.",
		"%s's tentacle looks very slimy.",
		"tentacle whip",
		"The stump of $n's tentacle spurts blood.",
		DAM_REND
	},

	{
		"horn", "%s's horn",
		"%s's horn rests here.",
		"%s's horn has a bit of meat stuck to it.",
		"horn jab",
		"$n's horn bumps you as it falls to the floor.",
		DAM_PIERCE
	},

	{
		"claw", "%s's claw",
		"%s's claw is here.",
		"%s's claw seems to have gotten quite a bit of use.",
		"claw",
		"The claw of $n gets clipped.",
		DAM_REND
	},

	{
		"hand", "%s's hand",
		"%s's hand sits on the ground here.",
		"%s's hand is in no shape to be reattached.",
		"punch",
		"$n gives you the finger, right before losing $s hand.",
		DAM_THRUST
	},

	{
		"foot", "%s's foot",
		"%s's foot stands here.",
		"%s's foot is only slightly smelly.",
		"kick",
		"$n stumbles, as $s foot falls off.",
		DAM_THRUST
	},

	{
		"branch", "%s's branch",
		"%s's branch lies broken here.",
		"%s's branch is broken and splintered.",
		"branch sweep",
		"$n's branch crashes to the ground.",
		DAM_REND
	},

	{
		"root", "%s's root",
		"%s's root is tangled here.",
		"%s's root has been ripped, bent and torn.",
		"root slap",
		"$n's root shakes out of the ground.",
		DAM_THRUST
	},

	{
		"twig", "%s's twig",
		"%s's twig lies trampled here.",
		"%s's twig is twisted and stripped from most of its leaves.",
		"twig rake",
		"$n's twig snaps and falls to ground.",
		DAM_REND
	},

	{
		"trunk", "%s's trunk",
		"%s's trunk is rotting here.",
		"%s's knotted trunk is covered in rough bark.",
		"trunk crash",
		"$n's trunk splinters as it falls.",
		DAM_BASH
	},

	{
		"pincers", "%s's pincers",
		"%s's pincers lay here.",
		"%s's pincers still look sharp.",
		"savage bite",
		"$n's pincers have bitten the dust.",
		DAM_PIERCE
	},

	{
		"spinnaret", "%s's spinnaret",
		"%s's spinnaret is gathering dust here.",
		"%s's spinnaret can spin no more webs.",
		"caustic web",
		"$n's spinnaret has spun its last web and falls off.",
		DAM_ACID
	},

	{
		"beak",	"%s's beak",
		"%s's sharp beak lies here.",
		"%s's beak opens dangerously.",
		"peck",
		"$n's beak is pecking no more.",
		DAM_PIERCE
	}
};

/*
	table wherein the spells are defined,
	modified by Manwe.
	mod 1. : added levels for new classes. 10/10/1999
	mod 2. : added a flag to determine wether skill or spell 12/10/1999
	mod 3. : added flag to see which god you should follow ??/??/1999
	mod 4. : added new spells/skills

	WAR  GLA  MAR  NIN  DRU  SOR  SHA  WLC
*/

const struct skill_type skill_table [MAX_SKILL] =
{
	{
		"-1 slot",                    { 99, 99, 99, 99, 99, 99, 99, 99 },
		NULL,                         NULL,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT( -1),
		0,                            0,
		"",
		"!-1 slot!",
		"!-1 slot!"
	},

	{
		"acid blast",                 { 99, 99, 99, 99, 99, 31, 99, 99 },
		spell_acid_blast,             NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_ACID,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT( 70),
		20,                           40,
		"acid blast",
		"!acid blast!",
		"!acid blast!"
	},

	{
		"acid breath",                { 99, 99, 99, 99, 99, 99, 99, 99 },
		spell_acid_breath,            NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_ACID,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(200),
		30,                           40,
		"blast of acid",
		"!acid breath!",
		"!acid breath!"
	},

	{
		"acupunch",                   { 99, 99, 99, 13, 99, 99, 99, 99 },
		NULL,                         &gsn_acupunch,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_THRUST,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(  0),
		16,                           60,
		"acupunch",
		"!acupunch!",
		"!acupunch!"
	},

	{
		"ambush",                     { 99, 99, 99, 44, 99, 99, 99, 99 },
		NULL,                         &gsn_ambush,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		8,                            40,
		"",
		"!ambush!",
		"!ambush!"
	},

	{
		"anatomy",                    { 99, 99, 41, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_anatomy,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!anatomy!",
		"!anatomy!"
	},

	{
		"animate dead",               { 99, 99, 99, 99, 99, 99, 99, 39 },
		spell_animate_dead,           NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_EVIL,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(617),
		50,                           80,
		"",
		"!animate dead!",
		"!animate dead!"
	},

	{
		"animate object",             { 99, 99, 99, 99, 33, 99, 99, 99 },
		spell_animate_object,         NULL,
		FSKILL_SPELL,                 SFOL_DUNNO,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_OBJ_INV,                  POS_STANDING,
		SLOT(670),
		75,                           80,
		"",
		"!animate object!",
		"!animate object!"
	},

	{
		"anti-magic shell",           { 99, 99, 99, 99, 99, 91, 99, 99 },
		spell_anti_magic_shell,       &gsn_anti_magic_shell,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_SELF,                POS_STANDING,
		SLOT(650),
		80,                           120,
		"",
		"The shimmering glow fades from about your body.",
		"!anti-magic shell!"
	},

	{
		"armor",                      { 99, 99, 99, 99,  2, 99, 99, 99 },
		spell_armor,                  NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(  1),
		5,                            40,
		"",
		"Your armor returns to its mundane value.",
		"!armor!"
	},

	{
		"armor usage",                { 99, 99, 16, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_armor_usage,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!armor usage!",
		"!armor usage!"
	},

	{
		"assassinate",                { 99, 99, 99, 36, 99, 99, 99, 99 },
		NULL,                         &gsn_assassinate,
		FSKILL_CMD,                   SFOL_ALL,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		24,                           60,
		"",
		"!assassinate!",
		"!assassinate!"
	},

	{
		"astral projection",          { 99, 99, 99, 99, 99, 28, 99, 99 },
		spell_astral_projection,      NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(634),
		45,                           80,
		"",
		"You step back into your body.",
		"!astral projection!"
	},

	{
		"attack",                     { 99, 99, 14, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_attack,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!attack!",
		"!attack!"
	},

	{
		"axe fighting",               { 20, 37, 99, 99, 22, 99, 56, 99 },
		NULL,                         &gsn_axe_fighting,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!axe fighting!",
		"!axe fighting!"
	},

	{
		"backstab",                   { 99, 99,  3, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_backstab,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_PIERCE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		16,                           40,
		"backstab",
		"!backstab!",
		"!backstab!"
	},

	{
		"banish",                     { 99, 99, 99, 99, 99, 99, 99, 38 },
		spell_banish,                 NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(618),
		80,                           40,
		"banish spell",
		"!banish!",
		"!banish!"
	},

	{
		"bargain",                    { 99, 99, 11, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_bargain,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!bargain!",
		"!bargain!"
	},

	{
		"bash",                       { 81, 99, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_bash,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_BASH,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(  0),
		50,                           60,
		"bash",
		"!bash!",
		"!bash!"
	},

	{
		"beast",                      { 99, 99, 99, 99, 99, 17, 99, 99 },
		spell_beast,                  NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_FIGHTING,
		SLOT(605),
		20,                           40,
		"",
		"!beast!",
		"!beast!"
	},

	{
		"benediction",                { 99, 99, 99, 99, 99, 99, 61, 99 },
		spell_benediction,            NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(640),
		45,                           80,
		"",
		"The powerful blessing of the gods fades away.",
		"!benediction!"
	},

	{
		"berserk",                    { 51, 99, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_berserk,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_FIGHTING,
		SLOT(805),
		24,                           40,
		"berserk",
		"You start to feel more rational.",
		"!berserk!"
	},

	{
		"black aura",                 { 99, 99, 99, 99, 99, 99, 99, 71 },
		spell_black_aura,             &gsn_black_aura,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_UNHOLY,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(667),
		75,                           80,
		"black aura spell",
		"The black aura around your body fades.",
		"!black aura!"
	},

	{
		"bless",                      { 99, 99, 99, 99, 99, 99,  2, 99 },
		spell_bless,                  NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(  3),
		15,                           40,
		"",
		"Your blessing fades away.",
		"!bless!"
	},

	{
		"blindness",                  { 99, 99, 99, 99, 99, 20, 99, 99 },
		spell_blindness,              &gsn_blindness,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_EVIL,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(  4),
		5,                            40,
		"spell",
		"Your vision returns.",
		"!blindness!"
	},

	{
		"block area",                 { 99, 99, 99, 99, 40, 99, 99, 99 },
		spell_block_area,             NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(601),
		25,                           40,
		"",
		"!block area!",
		"!block area!"
	},

	{
		"blood frenzy",               { 15, 15, 25, 25, 99, 99, 35, 99 },
		NULL,                         &gsn_blood_frenzy,
		FSKILL_NONE,                  SFOL_DUNNO,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_FIGHTING,
		SLOT(807),
		24,                           60,
		"bloodfrenzy",
		"Your blood stops boiling.",
		"!blood frenzy!"
	},

	{
		"brandish",                   { 99, 99, 99, 99,  4, 12,  9,  7 },
		NULL,                         &gsn_brandish,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_FIGHTING,
		SLOT(  0),
		4,                            60,
		"",
		"!brandish!",
		"!brandish!"
	},

	{
		"brawling",                   { 99, 99, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_brawling,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_THRUST,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!brawling!",
		"!brawling!"
	},

	{
		"breath water",               { 99, 99, 99, 99, 14, 99, 99, 99 },
		spell_breath_water,           NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(635),
		15,                           40,
		"breath water",
		"You can no longer breath water.",
		"!breath water!"
	},

	{
		"brew potion",                { 99, 99, 99, 99, 46, 99, 99, 99 },
		spell_brew_potion,            NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_SELF,                POS_STANDING,
		SLOT(656),
		10,                           40,
		"",
		"!brew potion!",
		"!brew potion!"
	},

	{
		"burning hands",              { 99, 99, 99, 99,  2, 99, 99, 99 },
		spell_burning_hands,          NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_FIRE,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(  5),
		10,                           40,
		"burning hands",
		"!burning hands!",
		"!burning hands!"
	},

	{
		"call lightning",             { 99, 99, 99, 99, 26, 99, 99, 99 },
		spell_call_lightning,         NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_LIGHTNING,
		TAR_IGNORE,                   POS_FIGHTING,
		SLOT(  6),
		30,                           40,
		"lightning bolt",
		"!call lightning!",
		"!call lightning!"
	},

	{
		"camp",                       { 26, 99, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_camp,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_RESTING,
		SLOT(  0),
		50,                           160,
		"",
		"!camp!",
		"!camp!"
	},

	{
		"cause critical",             { 99, 99, 99, 99, 99, 99, 13, 99 },
		spell_cause_critical,         NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT( 63),
		15,                           40,
		"spell",
		"!cause critical!",
		"!cause critical!"
	},

	{
		"cause light",                { 99, 99, 99, 99, 99, 99,  3, 99 },
		spell_cause_light,            NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT( 62),
		5,                            40,
		"spell",
		"!cause light!",
		"!cause light!"
	},

	{
		"cause serious",              { 99, 99, 99, 99, 99, 99,  8, 99 },
		spell_cause_serious,          NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT( 64),
		10,                           40,
		"spell",
		"!cause serious!",
		"!cause serious!"
	},

	{
		"chain lightning",            { 99, 99, 99, 99, 99, 36, 99, 99 },
		spell_chain_lightning,        NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_LIGHTNING,
		TAR_IGNORE,                   POS_FIGHTING,
		SLOT(686),
		45,                           40,
		"chain lightning",
		"!chain lightning!",
		"!chain lightning!"
	},

	{
		"change sex",                 { 99, 99, 99, 99, 99, 11, 99, 99 },
		spell_change_sex,             NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT( 82),
		15,                           40,
		"",
		"Your body feels familiar again.",
		"!change sex!"
	},

	{
		"charm person",               { 99, 99, 99, 99, 99, 99, 99, 20 },
		spell_charm_person,           &gsn_charm_person,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_PSIONIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(  7),
		25,                           40,
		"charm spell",
		"You regain your free will.",
		"!charm person!"
	},

	{
		"chill touch",                { 99, 99, 99, 99, 7, 99, 99, 99 },
		spell_chill_touch,            NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_COLD,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(  8),
		10,                           40,
		"chilling touch",
		"You feel less cold.",
		"!chill touch!"
	},

	{
		"circle",                     { 99, 99, 74, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_circle,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_PIERCE,
		TAR_IGNORE,                   POS_FIGHTING,
		SLOT(  0),
		60,                           60,
		"circle",
		"!circle!",
		"!circle!"
	},

	{
		"claw fighting",              { 99, 99, 99, 10, 99, 99, 64, 99 },
		NULL,                         &gsn_claw_fighting,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!claw fighting!",
		"!claw fighting!"
	},

	{
		"clear path",                 { 19, 99, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_clear_path,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(803),
		24,                           20,
		"",
		"You stop clearing paths.",
		"!clear path!"
	},

	{
		"climb",                      { 99, 99, 33, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_climb,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		5,                            20,
		"",
		"!climb!",
		"!climb!"
	},

	{
		"color spray",                { 99, 99, 99, 99, 99, 19, 99, 99 },
		spell_color_spray,            NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_LIGHT,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT( 10),
		15,                           40,
		"color spray",
		"!color spray!",
		"!color spray!"
	},

	{
		"confusion",                  { 99, 99, 99, 99, 99, 38, 99, 99 },
		spell_confusion,              NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_PSIONIC,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(637),
		30,                           60,
		"confusion spell",
		"Your eyes refocus.",
		"!confusion!"
	},

	{
		"continual light",            { 99, 99, 99, 99, 99, 99, 99, 99 },
		spell_continual_light,        NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_STANDING,
		SLOT( 57),
		7,                            40,
		"",
		"!continual light!",
		"!continual light!"
	},

	{
		"control weather",            { 99, 99, 99, 99, 16, 99, 99, 99 },
		spell_control_weather,        NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_STANDING,
		SLOT( 11),
		25,                           40,
		"",
		"!control weather!",
		"!control weather!"
	},

	{
		"create food",                { 99, 99, 99, 99, 99, 99,  6, 99 },
		spell_create_food,            NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_LIFE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT( 12),
		5,                            40,
		"",
		"!create food!",
		"!create food!"
	},

	{
		"create spring",              { 99, 99, 99, 99, 3, 99, 99, 99 },
		spell_create_spring,          NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_LIFE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT( 80),
		20,                           40,
		"",
		"!create spring!",
		"!create spring!"
	},

	{
		"cripple",                    { 99, 99, 49, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_cripple,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_THRUST,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(808),
		24,                           40,
		"low kick",
		"You are no longer crippled.",
		"!cripple!"
	},

	{
		"critical hit",               { 99, 99, 99, 41, 99, 99, 99, 99 },
		NULL,                         &gsn_critical_hit,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_REND,
		TAR_IGNORE,                   POS_FIGHTING,
		SLOT(806),
		0,                            0,
		"critical hit",
		"Your wounds stop bleeding.",
		"!critical hit!"
	},

	{
		"cure blindness",             { 99, 99, 99, 99, 99, 99,  4, 99 },
		spell_cure_blindness,         NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_GOOD,
		TAR_CHAR_DEFENSIVE,           POS_FIGHTING,
		SLOT( 14),
		5,                            40,
		"",
		"!cure blindness!",
		"!cure blindness!"
	},

	{
		"cure critical",              { 99, 99, 99, 99, 99, 99, 13, 99 },
		spell_cure_critical,          NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_LIFE,
		TAR_CHAR_DEFENSIVE,           POS_FIGHTING,
		SLOT( 15),
		20,                           40,
		"",
		"!cure critical!",
		"!cure critical!"
	},

	{
		"cure light",                 { 99, 99, 99, 99, 99, 99,  3, 99 },
		spell_cure_light,             NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_LIFE,
		TAR_CHAR_DEFENSIVE,           POS_FIGHTING,
		SLOT( 16),
		10,                           40,
		"",
		"!cure light!",
		"!cure light!"
	},

	{
		"cure poison",                { 99, 99, 99, 99, 99, 99, 15, 99 },
		spell_cure_poison,            NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_LIFE,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT( 43),
		5,                            40,
		"",
		"!cure poison!",
		"!cure poison!"
	},

	{
		"cure serious",               { 99, 99, 99, 99, 99, 99,  8, 99 },
		spell_cure_serious,           NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_LIFE,
		TAR_CHAR_DEFENSIVE,           POS_FIGHTING,
		SLOT( 61),
		15,                           40,
		"",
		"!cure serious!",
		"!cure serious!"
	},

	{
		"curse",                      { 99, 99, 99, 99, 99, 99, 99, 30 },
		spell_curse,                  &gsn_curse,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_EVIL,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT( 17),
		20,                           40,
		"curse spell",
		"The curse wears off.",
		"!curse!"
	},

	{
		"dagger fighting",            {  9, 14,  1, 23, 99,  6, 99, 12 },
		NULL,                         &gsn_dagger_fighting,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!dagger fighting!",
		"!dagger fighting!"
	},

	{
		"demon",                      { 99, 99, 99, 99, 99, 99, 99, 13 },
		spell_demon,                  NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(604),
		90,                           40,
		"",
		"!demon!",
		"!demon!"
	},

	{
		"detect evil",                { 99, 99, 99, 99, 99, 99, 11, 99 },
		spell_detect_evil,            &gsn_detect_evil,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_SELF,                POS_STANDING,
		SLOT( 18),
		5,                            40,
		"",
		"The red outlines fade from your vision.",
		"!detect evil!"
	},

	{
		"detect forgery",             { 99, 99, 39, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_detect_forgery,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!detect forgery!",
		"!detect forgery!"
	},

	{
		"detect good",                { 99, 99, 99, 99, 99, 99, 25, 99 },
		spell_detect_good,            &gsn_detect_good,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_SELF,                POS_STANDING,
		SLOT(665),
		5,                            40,
		"",
		"The gold outlines fade from your vision.",
		"!detect good!"
	},

	{
		"detect hidden",              { 99, 99, 99, 99, 99,  8, 99, 99 },
		spell_detect_hidden,          NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_SELF,                POS_STANDING,
		SLOT( 44),
		5,                            40,
		"",
		"You feel less aware of your surroundings.",
		"!detect hidden!"
	},

	{
		"detect invis",               { 99, 99, 99, 99, 99,  3, 99, 99 },
		spell_detect_invis,           NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_SELF,                POS_STANDING,
		SLOT( 19),
		5,                            40,
		"",
		"You no longer see invisible objects.",
		"!detect invis!"
	},

	{
		"detect poison",              { 99, 99, 99, 99, 99, 99, 12, 99 },
		spell_detect_poison,          NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_OBJ_INV,                  POS_STANDING,
		SLOT( 21),
		5,                            40,
		"",
		"!detect poison!",
		"!detect poison!"
	},

	{
		"disarm",                     { 99, 99,  9, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_disarm,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_FIGHTING,
		SLOT(  0),
		32,                           40,
		"",
		"!disarm!",
		"!disarm!"
	},

	{
		"disguise",                   { 99, 99, 99, 25, 99, 99, 99, 99 },
		NULL,                         &gsn_disguise,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_RESTING,
		SLOT(  0),
		4,                            20,
		"",
		"!disguise!",
		"!disguise!"
	},

	{
		"dispel evil",                { 99, 99, 99, 99, 99, 99, 16, 99 },
		spell_dispel_evil,            NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_GOOD,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT( 22),
		15,                           40,
		"dispel evil",
		"!dispel evil!",
		"!dispel evil!"
	},

	{
		"dispel good",                { 99, 99, 99, 99, 99, 99, 37, 99 },
		spell_dispel_good,            NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_EVIL,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(622),
		20,                           40,
		"dispel",
		"!dispel good!",
		"!dispel good!"
	},

	{
		"dispel magic",               { 99, 99, 99, 99, 99, 99, 99,  8 },
		spell_dispel_magic,           NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_OBJ_CHAR_DEF,             POS_FIGHTING,
		SLOT( 59),
		15,                           40,
		"",
		"!dispel magic!",
		"!dispel magic!"
	},

	{
		"dispel undead",              { 99, 99, 99, 99, 99, 99, 19, 99 },
		spell_dispel_undead,          NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_HOLY,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(621),
		20,                           40,
		"dispel",
		"!dispel undead!",
		"!dispel undead!"
	},

	{
		"distract",                   { 99, 99, 19, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_distract,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		24,                           20,
		"",
		"!distract!",
		"!distract!"
	},

	{
		"divert",                     { 99, 99, 99,  9, 99, 99, 99, 99 },
		NULL,                         &gsn_divert,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_FIGHTING,
		SLOT(  0),
		32,                           40,
		"",
		"!divert!",
		"!divert!"
	},

	{
		"divine inspiration",         { 99, 99, 99, 99, 99, 24, 99, 99 },
		spell_divine_inspiration,     NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(666),
		75,                           80,
		"",
		"Your divine inspiration abruptly fades, leaving you sad and depressed.",
		"!divine inspiration!"
	},

	{
		"dodge",                      { 99, 99,  6, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_dodge,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!dodge!",
		"!dodge!"
	},

	{
		"door bash",                  { 36, 99, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_door_bash,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_BASH,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		60,                           60,
		"door bash",
		"!door bash!",
		"!door bash!"
	},

	{
		"drain",                      { 99, 99, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_drain,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_LIFE,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(  0),
		16,                           60,
		"vampiric bite",
		"!drain!",
		"!drain!"
	},

	{
		"dual wield",                 { 99, 91, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_dual_wield,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!dual wield!",
		"!dual wield!"
	},

	{
		"earthbind",                  { 99, 99, 99, 99, 99, 99, 99, 33 },
		spell_earthbind,              &gsn_earthbind,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(673),
		35,                           60,
		"earthbind",
		"You are no longer bound to the earth.",
		"!earthbind!"
	},

	{
		"earthen spirit",             { 99, 99, 99, 99, 36, 99, 99, 99 },
		spell_earthen_spirit,         NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(678),
		25,                           40,
		"",
		"The spirit of the earth recedes.",
		"!earthen spirit!"
	},

	{
		"earthquake",                 { 99, 99, 99, 99, 30, 99, 99, 99 },
		spell_earthquake,             NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_VIBE,
		TAR_IGNORE,                   POS_FIGHTING,
		SLOT( 23),
		15,                           40,
		"earthquake",
		"!earthquake!",
		"!earthquake!"
	},

	{
		"elemental",                  { 99, 99, 99, 99, 71, 99, 99, 99 },
		spell_elemental,              NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_SELF,                POS_STANDING,
		SLOT(657),
		10,                           80,
		"",
		"!elemental!",
		"!elemental!"
	},

	{
		"enchant weapon",             { 99, 99, 99, 99, 99, 99, 99, 24 },
		spell_enchant_weapon,         NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_OBJ_INV,                  POS_STANDING,
		SLOT( 24),
		100,                          80,
		"",
		"!enchant weapon!",
		"!enchant weapon!"
	},

	{
		"endurance",                  { 15, 99, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_endurance,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_FIGHTING,
		SLOT(  0),
		0,                            0,
		"",
		"!endurance!",
		"!endurance!"
	},

	{
		"energy bolt",                { 99, 99, 99, 99, 99, 99, 99,  2 },
		spell_energy_bolt,            NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_LIGHTNING,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(679),
		5,                            40,
		"energy bolt",
		"!energy bolt!",
		"!energy bolt!"
	},

	{
		"energy drain",               { 99, 99, 99, 99, 99, 99, 99, 17 },
		spell_energy_drain,           NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_LIFE,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT( 25),
		25,                           40,
		"energy drain",
		"!energy drain!",
		"!energy drain!"
	},

	{
		"energy shift",               { 99, 99, 99, 99, 99, 99, 99, 32 },
		spell_energy_shift,           NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_FIGHTING,
		SLOT(627),
		20,                           80,
		"",
		"!energy shift!",
		"!energy shift!"
	},

	{
		"enhance object",             { 99, 99, 99, 99, 28, 99, 99, 99 },
		spell_enhance_object,         NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_OBJ_INV,                  POS_STANDING,
		SLOT(619),
		80,                           80,
		"",
		"!enhance object!",
		"!enhance object!"
	},

	{
		"enhanced damage",            { 11, 21, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_enhanced_damage,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!enhanced damage!",
		"!enhanced damage!"
	},

	{
		"enhanced heal",              { 99, 99, 99, 99, 99, 99, 35, 99 },
		spell_enhanced_heal,          NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(615),
		10,                           80,
		"",
		"You no longer heal easily.",
		"!enhanced heal!"
	},

	{
		"enhanced rest",              { 99, 99, 99, 99, 34, 99, 99, 99 },
		spell_enhanced_rest,          NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(614),
		10,                           80,
		"",
		"You no longer rest easily.",
		"!enhanced rest!"
	},

	{
		"enhanced revive",            { 99, 99, 99, 99, 99, 99, 99, 35 },
		spell_enhanced_revive,        NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(616),
		10,                           80,
		"",
		"You no longer revive easily.",
		"!enhanced revive!"
	},

	{
		"envision",                   { 99, 99, 99, 99, 99, 99, 99, 55 },
		spell_envision,               NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(683),
		500,                          40,
		"",
		"!envision!",
		"!envision!"
	},

	{
		"ethereal travel",            { 99, 99, 99, 99, 99, 99, 99, 28 },
		spell_ethereal_travel,        NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(633),
		35,                           80,
		"",
		"You become solid.",
		"!ethereal travel!"
	},

	{
		"faerie fire",                { 99, 99, 99, 99, 99, 14, 99, 99 },
		spell_faerie_fire,            &gsn_faerie_fire,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_LIGHT,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT( 72),
		5,                            40,
		"faerie fire",
		"The pink aura around you fades away.",
		"!faerie fire!"
	},

	{
		"faerie fog",                 { 99, 99, 99, 99, 99, 16, 99, 99 },
		spell_faerie_fog,             NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_LIGHT,
		TAR_IGNORE,                   POS_STANDING,
		SLOT( 73),
		12,                           40,
		"faerie fog",
		"!faerie fog!",
		"!faerie fog!"
	},

	{
		"farheal",                    { 99, 99, 99, 99, 99, 99, 91, 99 },
		spell_farheal,                NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(642),
		200,                          120,
		"",
		"!farheal!",
		"!farheal!"
	},

	{
		"fear",                       { 99, 99, 99, 99, 99, 99, 99, 41 },
		spell_fear,                   &gsn_fear,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_EVIL,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(685),
		30,                           40,
		"fear spell",
		"You feel less frightened.",
		"!fear!"
	},

	{
		"feast",                      { 99, 99, 99, 99, 99, 99, 34, 99 },
		spell_feast,                  NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_SELF,                POS_STANDING,
		SLOT(624),
		75,                           80,
		"",
		"!feast!",
		"!feast!"
	},

	{
		"fifth attack",               { 99, 81, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_fifth_attack,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!fifth attack!",
		"!fifth attack!"
	},

	{
		"fire breath",                { 99, 99, 99, 99, 99, 99, 99, 99 },
		spell_fire_breath,            NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_FIRE,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(201),
		45,                           80,
		"blast of flame",
		"!fire breath!",
		"!fire breath!"
	},

	{
		"fireball",                   { 99, 99, 99, 99, 21, 99, 99, 99 },
		spell_fireball,               NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_FIRE,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT( 26),
		15,                           40,
		"fireball",
		"!fireball!",
		"!fireball!"
	},

	{
		"fireshield",                 { 99, 99, 99, 99, 73, 99, 99, 99 },
		spell_fireshield,             &gsn_fireshield,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_SELF,                POS_STANDING,
		SLOT(658),
		50,                           80,
		"fireshield",
		"The flames around your body wink out.",
		"!fireshield!"
	},

	{
		"firewalk",                   { 99, 99, 99, 99, 67, 99, 99, 99 },
		spell_firewalk,               NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(664),
		30,                           80,
		"",
		"You are no longer fire proof.",
		"!firewalk!"
	},

	{
		"first strike",               { 99, 49, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_first_strike,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!first strike!",
		"!first strike!"
	},

	{
		"fisticuffs",                 { 99, 99, 99, 99, 99, 99,  1, 99 },
		NULL,                         &gsn_fisticuffs,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_THRUST,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"fist blow",
		"!fisticuffs!",
		"!fisticuffs!"
	},

	{
		"flail fighting",             { 33, 23, 38, 99, 30, 99, 77, 99 },
		NULL,                         &gsn_flail_fighting,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!flail fighting!",
		"!flail fighting!"
	},

	{
		"flame blade",                { 99, 99, 99, 99, 99, 56, 99, 99 },
		spell_flame_blade,            &gsn_flame_blade,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_FIRE,
		TAR_OBJ_INV,                  POS_STANDING,
		SLOT(687),
		500,                          200,
		"flame",
		"!flame blade!",
		"!flame blade!"
	},

	{
		"flamestrike",                { 99, 99, 99, 99, 99, 99, 99, 99 },
		spell_flamestrike,            NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_FIRE,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT( 65),
		20,                           40,
		"flamestrike",
		"!flamestrike!",
		"!flamestrike!"
	},

	{
		"flamewave",                  { 99, 99, 99, 99, 51, 99, 99, 99 },
		spell_flamewave,              NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_FIRE,
		TAR_IGNORE,                   POS_FIGHTING,
		SLOT(682),
		20,                           40,
		"flamewave",
		"!flamewave!",
		"!flamewave!"
	},

	{
		"flash powder",               { 99, 99, 99, 61, 99, 99, 99, 99 },
		NULL,                         &gsn_flash_powder,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!flash powder!",
		"!flash powder!"
	},

	{
		"fly",                        { 99, 99, 99, 99,  9, 99, 99, 99 },
		spell_fly,                    &gsn_fly,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT( 56),
		10,                           60,
		"",
		"You slowly float to the ground.",
		"!fly!"
	},

	{
		"forge",                      { 99, 99, 53, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_forge,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            240,
		"",
		"!forge!",
		"!forge!"
	},

	{
		"fourth attack",              { 66, 71, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_fourth_attack,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!fourth attack!",
		"!fourth attack!"
	},

	{
		"frost breath",               { 99, 99, 99, 99, 99, 99, 99, 99 },
		spell_frost_breath,           NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_COLD,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(202),
		45,                           80,
		"blast of frost",
		"!frost breath!",
		"!frost breath!"
	},

	{
		"gas breath",                 { 99, 99, 99, 99, 99, 99, 99, 99 },
		spell_gas_breath,             NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_POISON,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(203),
		45,                           80,
		"blast of gas",
		"!gas breath!",
		"!gas breath!"
	},

	{
		"gate",                       { 99, 99, 99, 99, 99, 60, 99, 99 },
		spell_gate,                   NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_STANDING,
		SLOT( 83),
		50,                           40,
		"",
		"!gate!",
		"!gate!"
	},

	{
		"giant strength",             { 99, 99, 99, 99, 99, 99, 14, 99 },
		spell_giant_strength,         NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT( 39),
		20,                           40,
		"",
		"You feel weaker.",
		"!giant strength!"
	},

	{
		"glance",                     { 99, 99, 99, 28, 99, 99, 99, 99 },
		NULL,                         &gsn_glance,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!glance!",
		"!glance!"
	},

	{
		"globe of darkness",          { 99, 99, 99, 99, 99, 99, 99, 99 },
		spell_globe_of_darkness,      NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_FIGHTING,
		SLOT(672),
		75,                           80,
		"",
		"!globe of darkness!",
		"!globe of darkness!"
	},

	{
		"gouge",                      { 99, 99, 46, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_gouge,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_THRUST,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(  0),
		20,                           40,
		"gouge",
		"Your vision returns.",
		"!gouge!"
	},

	{
		"greater backstab",           { 99, 99, 71, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_greater_backstab,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!greater backstab!",
		"!greater backstab!"
	},

	{
		"greater hunt",               { 61, 99, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_greater_hunt,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!greater hunt!",
		"!greater hunt!"
	},

	{
		"greater peek",               { 99, 99, 20, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_greater_peek,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!greater peek!",
		"!greater peek!"
	},

	{
		"greater pick",               { 99, 99, 25, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_greater_pick,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!greater pick!",
		"!greater pick!"
	},

	{
		"greater stealth",            { 99, 99, 99, 91, 99, 99, 99, 99 },
		NULL,                         &gsn_greater_stealth,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!greater stealth!",
		"!greater stealth!"
	},

	{
		"greater track",              { 46, 99, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_greater_track,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!greater track!",
		"!greater track!"
	},

	{
		"guard",                      { 99, 99, 99, 66, 99, 99, 99, 99 },
		NULL,                         &gsn_guard,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!guard!",
		"!guard!"
	},

	{
		"hallucinate",                { 99, 99, 99, 99, 99, 22, 99, 99 },
		spell_hallucinate,            NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_PSIONIC,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(631),
		25,                           40,
		"hallucinate spell",
		"Reality shifts back into focus.",
		"!hallucinate!"
	},

	{
		"harm",                       { 99, 99, 99, 99, 99, 99, 22, 99 },
		spell_harm,                   NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT( 27),
		35,                           40,
		"harm spell",
		"!harm!",
		"!harm!"
	},

	{
		"haste",                      { 99, 99, 99, 99, 13, 99, 99, 99 },
		spell_haste,                  NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(613),
		10,                           80,
		"",
		"You slow down.",
		"!haste!"
	},

	{
		"head butt",                  { 99, 99, 99, 99, 99, 99, 26, 99 },
		NULL,                         &gsn_head_butt,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_BASH,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(  0),
		20,                           60,
		"head butt",
		"!head butt!",
		"!head butt!"
	},

	{
		"heal",                       { 99, 99, 99, 99, 99, 99, 20, 99 },
		spell_heal,                   NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_LIFE,
		TAR_CHAR_DEFENSIVE,           POS_FIGHTING,
		SLOT( 28),
		50,                           40,
		"",
		"!heal!",
		"!heal!"
	},

	{
		"hide",                       { 99, 99, 99,  2, 99, 99, 99, 99 },
		NULL,                         &gsn_hide,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(800),
		16,                           20,
		"",
		"!hide!",
		"!hide!"
	},

	{
		"high explosive",             { 99, 99, 99, 99, 99, 99, 99, 99 },
		spell_high_explosive,         NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(502),
		0,                            40,
		"high explosive ammo",
		"!high explosive!",
		"!high explosive!"
	},

	{
		"holy word",                  { 99, 99, 99, 99, 99, 99, 99, 99 },
		spell_holy_word,              NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_HOLY,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(644),
		17,                           40,
		"holy word",
		"!holy word!",
		"!holy word!"
	},

	{
		"homonculous",                { 99, 99, 99, 99, 99, 99, 99, 18 },
		spell_homonculous,            NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(603),
		90,                           40,
		"",
		"!homonculous!",
		"!homonculous!"
	},

	{
		"hunt",                       { 13, 99, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_hunt,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(804),
		24,                           20,
		"",
		"You stop hunting for today.",
		"!hunt!"
	},

	{
		"ice arrow",                  { 99, 99, 99, 52, 99, 99, 99, 99 },
		spell_ice_arrow,              NULL,
		FSKILL_SPELL,                 SFOL_DUNNO,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(677),
		10,                           10,
		"",
		"!ice arrow!",
		"!ice arrow!"
	},

	{
		"ice layer",                  { 99, 99, 99, 99, 83, 99, 99, 99 },
		spell_ice_layer,              NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(680),
		75,                           80,
		"",
		"!ice layer!",
		"!ice layer!"
	},

	{
		"icicle armor",               { 99, 99, 99, 99, 53, 99, 99, 99 },
		spell_icicle_armor,           &gsn_icicle_armor,
		FSKILL_SPELL,                 SFOL_DUNNO,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_SELF,                POS_STANDING,
		SLOT(681),
		75,                           40,
		"icicle armor",
		"Your icicle armor slides back into your body.",
		"!icicle armor!"
	},

	{
		"identify",                   { 99, 99, 99, 99, 99, 76, 99,  5 },
		spell_identify,               NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_OBJ_INV,                  POS_STANDING,
		SLOT( 53),
		12,                           20,
		"",
		"!identify!",
		"!identify!"
	},

	{
		"illusion",                   { 99, 99, 99, 99, 99, 35, 99, 99 },
		spell_illusion,               NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(629),
		75,                           80,
		"illusion",
		"!illusion!",
		"!illusion!"
	},

	{
		"impale",                     { 99, 63, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_impale,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_PIERCE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"impaling thrust",
		"!impale!",
		"!impale!"
	},

	{
		"improved invis",             { 99, 99, 99, 99, 99, 51, 99, 99 },
		spell_improved_invis,         &gsn_improved_invis,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_SELF,                POS_STANDING,
		SLOT(646),
		45,                           80,
		"",
		"The shroud of invisibility fades from about your body.",
		"!improved invis!"
	},

	{
		"induction",                  { 99, 99, 99, 99, 32, 99, 99, 99 },
		spell_induction,              NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_SELF,                POS_STANDING,
		SLOT(628),
		20,                           10,
		"",
		"!induction!",
		"!induction!"
	},

	{
		"infravision",                { 99, 99, 99, 99, 99, 21, 99, 99 },
		spell_infravision,            NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT( 77),
		5,                            40,
		"",
		"You no longer see in the dark.",
		"!infravision!"
	},

	{
		"invigorate",                 { 99, 99, 99, 99, 41, 99, 99, 99 },
		spell_invigorate,             NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(645),
		20,                           80,
		"",
		"!invigorate!",
		"!invigorate!"
	},

	{
		"invis",                      { 99, 99, 99, 99, 99, 18, 99, 99 },
		spell_invis,                  &gsn_invis,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_OBJ_CHAR_DEF,             POS_STANDING,
		SLOT( 29),
		5,                            60,
		"",
		"You are no longer invisible.",
		"!invis!"
	},

	{
		"kick",                       {  2, 99, 2, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_kick,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_THRUST,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(  0),
		16,                           40,
		"kick",
		"!kick!",
		"!kick!"
	},

	{
		"knife",                      { 99, 99, 99, 33, 99, 99, 99, 99 },
		NULL,                         &gsn_knife,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_PIERCE,
		TAR_IGNORE,                   POS_FIGHTING,
		SLOT(  0),
		24,                           80,
		"knife",
		"!knife!",
		"!knife!"
	},

	{
		"lightning breath",           { 99, 99, 99, 99, 99, 99, 99, 99 },
		spell_lightning_breath,       NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_LIGHTNING,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(204),
		45,                           80,
		"blast of lightning",
		"!lightning breath!",
		"!lightning breath!"
	},

	{
		"lightning reflexes",         { 99, 99, 99, 87, 99, 99, 99, 99 },
		NULL,                         &gsn_lightning_reflexes,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_THRUST,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!lightning reflexes!",
		"!lightning reflexes!"
	},

	{
		"locate object",              { 99, 99, 99, 99, 99, 10, 99, 99 },
		spell_locate_object,          NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_STANDING,
		SLOT( 31),
		20,                           120,
		"",
		"!locate object!",
		"!locate object!"
	},

	{
		"lock lock",                  { 99, 99, 13, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_lock_lock,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            10,
		"",
		"!lock lock!",
		"!lock lock!"
	},

	{
		"mace fighting",              { 42, 99, 99, 99,  1, 99, 28, 99 },
		NULL,                         &gsn_mace_fighting,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!mace fighting!",
		"!mace fighting!"
	},

	{
		"mage blast",                 { 99, 99, 99, 99, 99, 99, 99, 45 },
		spell_mage_blast,             &gsn_mage_blast,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_PSIONIC,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(636),
		45,                           40,
		"mage blast",
		"You feel less vulnerable.",
		"!mage blast!"
	},

	{
		"mage shield",                { 99, 99, 99, 99, 99, 99, 99, 25 },
		spell_mage_shield,            NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_SELF,                POS_STANDING,
		SLOT(620),
		80,                           40,
		"",
		"The shimmering glow fades from about your body.",
		"!mage shield!"
	},

	{
		"magic mirror",               { 99, 99, 99, 99, 99, 71, 99, 99 },
		spell_magic_mirror,           &gsn_magic_mirror,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_SELF,                POS_STANDING,
		SLOT(669),
		150,                          80,
		"",
		"The magical mirror around your body shatters.",
		"!magic mirror!"
	},

	{
		"magic missile",              { 99, 99, 99, 99, 99,  2, 99, 99 },
		spell_magic_missile,          NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT( 32),
		5,                            13,
		"magic missile",
		"!magic missile!",
		"!magic missile!"
	},

	{
		"make poison",                 { 99, 99, 99, 47, 99, 99, 99, 99 },
		NULL,                         &gsn_make_poison,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_POISON,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		16,                           80,
		"poison",
		"!make poison!",
		"!make poison!"
	},

	{
		"mana shackles",              { 99, 99, 99, 99, 99, 99, 99, 37 },
		spell_mana_shackles,          &gsn_mana_shackles,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_LIGHTNING,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(671),
		50,                           80,
		"mana shackles spell",
		"Your mana shackles disentangle and fall away.",
		"!mana shackles!"
	},

	{
		"mana shield",                { 99, 99, 99, 99, 99, 99, 99, 91 },
		spell_mana_shield,            &gsn_mana_shield,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_SELF,                POS_STANDING,
		SLOT(668),
		150,                          80,
		"",
		"The blue aura around your body fades.",
		"!mana shield!"
	},

	{
		"martial arts",               { 99, 99, 99,  1, 99, 99, 99, 99 },
		NULL,                         &gsn_martial_arts,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_THRUST,
		TAR_CHAR_OFFENSIVE,           POS_STANDING,
		SLOT(  0),
		0,                            0,
		"martial arts",
		"!martial arts!",
		"!martial arts!"
	},

	{
		"mass",                       { 99, 99, 99, 99, 99, 46, 44, 99 },
		NULL,                         &gsn_mass,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            40,
		"",
		"!mass!",
		"!mass!"
	},

	{
		"mass invis",                 { 99, 99, 99, 99, 99, 22, 99, 99 },
		spell_mass_invis,             NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_STANDING,
		SLOT( 69),
		20,                           80,
		"",
		"!mass invis!",
		"!mass invis!"
	},

	{
		"metamorphose liquids",       { 99, 99, 99, 99, 52, 99, 99, 99 },
		spell_metamorphose_liquids,   NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_OBJ_INV,                  POS_STANDING,
		SLOT(663),
		15,                           40,
		"",
		"!metamorphose liquids!",
		"!metamorphose liquids!"
	},

	{
		"mindblast",                  { 99, 99, 99, 99, 99, 99, 99, 99 },
		spell_mindblast,              NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_PSIONIC,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT( 47),
		15,                           40,
		"mindblast",
		"!mindblast!",
		"!mindblast!"
	},

	{
		"mirror image",               { 99, 99, 99, 99, 99, 26, 99, 99 },
		spell_mirror_image,           NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(630),
		80,                           80,
		"mirror image",
		"Your images fold back into you.",
		"!mirror image!"
	},

	{
		"mount",                      { 22, 99, 48, 82, 34, 64, 99, 36 },
		NULL,                         &gsn_mount,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		16,                           40,
		"",
		"!mount!",
		"!mount!"
	},

	{
		"muffle",                     { 99, 99, 99, 39, 99, 99, 99, 99 },
		NULL,                         &gsn_muffle,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!muffle!",
		"!muffle!"
	},

	{
		"nightmare",                  { 99, 99, 99, 99, 99, 89, 99, 99 },
		spell_nightmare,              &gsn_nightmare,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_PSIONIC,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(653),
		60,                           40,
		"spell",
		"Your panic recedes and your breathing returns to normal.",
		"!nightmare!"
	},

	{
		"notice",                     { 18, 99, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_notice,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		8,                            20,
		"notice",
		"!notice!",
		"!notice!"
	},

	{
		"object rape",                { 99, 99, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_object_rape,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!object rape!",
		"!object rape!"
	},

	{
		"paralyzing embrace",         { 99, 99, 99, 99, 99, 99, 99, 99 },
		spell_paralyzing_embrace,     NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_PSIONIC,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT( 48),
		10,                           40,
		"paralyzing embrace",
		"Your grasp on reality feels strong again.",
		"!paralyzing embrace!"
	},

	{
		"parry",                      { 99,  3, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_parry,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!parry!",
		"!parry!"
	},

	{
		"pass door",                  { 99, 99, 99, 99, 24, 99, 99, 99 },
		spell_pass_door,              NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_SELF,                POS_STANDING,
		SLOT( 74),
		20,                           40,
		"",
		"You feel solid again.",
		"!pass door!"
	},

	{
		"pass without trace",         { 99, 99, 99, 72, 99, 99, 99, 99 },
		NULL,                         &gsn_pass_without_trace,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!pass without trace!",
		"!pass without trace!"
	},

	{
		"peek",                       { 99, 99,  2, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_peek,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!peek!",
		"!peek!"
	},

	{
		"perception",                 { 75, 99, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_perception,
		FSKILL_NONE,                  SFOL_DUNNO,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_FIGHTING,
		SLOT(  0),
		0,                            0,
		"",
		"!perception!",
		"!perception!"
	},

	{
		"petrifying touch",           { 99, 99, 99, 99, 99, 99, 99, 99 },
		spell_petrifying_touch,       NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_LIFE,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT( 30),
		10,                           40,
		"petrifying touch",
		"Your skin feels less hard.",
		"!petrifying touch!"
	},

	{
		"phantasm",                   { 99, 99, 99, 99, 99, 23, 99, 99 },
		spell_phantasm,               NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_FIGHTING,
		SLOT(607),
		30,                           40,
		"",
		"!phantasm!",
		"!phantasm!"
	},

	{
		"pick lock",                  { 99, 99, 10, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_pick_lock,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		8,                            20,
		"",
		"!pick lock!",
		"!pick lock!"
	},

	{
		"plant",                      { 99, 99, 31, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_plant,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		8,                            10,
		"",
		"!plant!",
		"!plant!"
	},

	{
		"poison",                     { 99, 99, 99, 99, 99, 99, 99, 16 },
		spell_poison,                 &gsn_poison,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_POISON,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT( 33),
		10,                           40,
		"poison spell",
		"You feel less sick.",
		"!poison!"
	},

	{
		"polymorph",                  { 99, 99, 99, 99, 91, 99, 99, 99 },
		NULL,                         &gsn_polymorph,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_FIGHTING,
		SLOT(809),
		600,                          80,
		"",
		"!polymorph!",
		"!polymorph!"
	},

	{
		"possess",                    { 99, 99, 99, 99, 99, 99, 99, 81 },
		spell_possess,                NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_PSIONIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(651),
		25,                           80,
		"possess spell",
		"Your mind is wrenched back to it's rightful home.",
		"!possess!"
	},

	{
		"precision",                  { 99, 99, 87, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_precision,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_THRUST,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!precision!",
		"!precision!"
	},

	{
		"protection evil",            { 99, 99, 99, 99, 99, 99, 17, 99 },
		spell_protection_evil,        NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_SELF,                POS_STANDING,
		SLOT( 34),
		5,                            40,
		"",
		"You feel less protected from evil.",
		"!protection evil!"
	},

	{
		"protection good",            { 99, 99, 99, 99, 99, 99, 30, 99 },
		spell_protection_good,        NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_EVIL,
		TAR_CHAR_SELF,                POS_STANDING,
		SLOT(661),
		12,                           40,
		"",
		"You feel less protected from good.",
		"!protection good!"
	},

	{
		"psionic shockwave",          { 99, 99, 99, 99, 99, 99, 99, 99 },
		spell_psionic_shockwave,      NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_PSIONIC,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(205),
		45,                           80,
		"psionic shockwave",
		"!psionic shockwave!",
		"!psionic shockwave!"
	},

	{
		"quick draw",                 { 99, 99, 99, 56, 99, 99, 99, 99 },
		NULL,                         &gsn_quick_draw,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!quick draw!",
		"!quick draw!"
	},

	{
		"quicken",                    { 99, 99, 99, 99, 99, 81, 99, 99 },
		spell_quicken,                NULL,
		FSKILL_SPELL,                 SFOL_DUNNO,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_SELF,                POS_STANDING,
		SLOT(674),
		50,                           40,
		"",
		"Time moves more swiftly as your thoughts slow.",
		"!quicken!"
	},

	{
		"range cast",                 { 99, 99, 99, 99, 12, 99, 99, 99 },
		NULL,                         &gsn_range_cast,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            40,
		"",
		"!range cast!",
		"!range cast!"
	},

	{
		"rearm",                      { 23, 13, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_rearm,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!rearm!",
		"!rearm!"
	},

	{
		"recharge",                   { 99, 99, 99, 99, 99, 99, 99, 59 },
		spell_recharge,               NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_OBJ_INV,                  POS_STANDING,
		SLOT(648),
		50,                           40,
		"",
		"!recharge!",
		"!recharge!"
	},

	{
		"recite",                     { 99, 99, 99, 99,  6,  4,  7,  3 },
		NULL,                         &gsn_recite,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_FIGHTING,
		SLOT(  0),
		4,                            60,
		"",
		"!recite!",
		"!recite!"
	},

	{
		"refresh",                    { 99, 99, 99, 99,  8, 99, 99, 99 },
		spell_refresh,                NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT( 81),
		12,                           40,
		"refresh",
		"!refresh!",
		"!refresh!"
	},

	{
		"remove curse",               { 99, 99, 99, 99, 99, 99, 21, 99 },
		spell_remove_curse,           NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_GOOD,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT( 35),
		5,                            40,
		"",
		"!remove curse!",
		"!remove curse!"
	},

	{
		"remove fear",                { 99, 99, 99, 99, 99, 99, 29, 99 },
		spell_remove_fear,            &gsn_remove_fear,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_EVIL,
		TAR_CHAR_DEFENSIVE,           POS_FIGHTING,
		SLOT(623),
		40,                           40,
		"spell",
		"You are no longer fearless.",
		"!remove fear!"
	},

	{
		"riposte",                    { 99, 26, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_riposte,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!riposte!",
		"!riposte!"
	},

	{
		"rescue",                     {  7, 99, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_rescue,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_FIGHTING,
		SLOT(  0),
		36,                           40,
		"",
		"!rescue!",
		"!rescue!"
	},

	{
		"resilience",                 { 77, 99, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_resilience,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_THRUST,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!resilience!",
		"!resilience!"
	},

	{
		"restore",                    { 99, 99, 99, 99, 99, 99, 45, 99 },
		spell_restore,                NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_FIGHTING,
		SLOT(625),
		20,                           80,
		"",
		"!restore!",
		"!restore!"
	},

	{
		"rift",                       { 99, 99, 99, 99, 99, 99, 99, 27 },
		spell_rift,                   NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(608),
		250,                          80,
		"",
		"!rift!",
		"!rift!"
	},

	{
		"righteous fury",             { 99, 99, 99, 99, 99, 99, 73, 99 },
		spell_righteous_fury,         NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(641),
		75,                           80,
		"",
		"Your righteous fury abruptly leaves you, you feel drained and weak.",
		"!righteous fury!"
	},

	{
		"rip",                        { 99, 99, 99, 99, 99, 99, 99, 23 },
		spell_rip,                    NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(609),
		250,                          80,
		"",
		"!rip!",
		"!rip!"
	},

	{
		"sanctify",                   { 99, 99, 99, 99, 99, 99, 53, 99 },
		spell_sanctify,               NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(638),
		75,                           80,
		"",
		"!sanctify!",
		"!sanctify!"
	},

	{
		"sanctuary",                  { 99, 99, 99, 99, 99, 99, 27, 99 },
		spell_sanctuary,              NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_HOLY,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT( 36),
		75,                           40,
		"",
		"The white aura around your body fades.",
		"!sanctuary!"
	},

	{
		"scroll mastery",             { 99, 99, 99, 99, 99, 99, 99, 31 },
		NULL,                         &gsn_scroll_mastery,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"scroll mastery",
		"!scroll mastery!"
	},

	{
		"second attack",              {  4,  7,  8, 18, 99, 99, 99, 99 },
		NULL,                         &gsn_second_attack,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!second attack!",
		"!second attack!"
	},

	{
		"shade",                      { 99, 99, 99, 99, 99,  9, 99, 99 },
		spell_shade,                  NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(606),
		50,                           40,
		"",
		"!shade!",
		"!shade!"
	},

	{
		"shadow",                     { 99, 99, 99,  7, 99, 99, 99, 99 },
		NULL,                         &gsn_shadow,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		8,                            20,
		"",
		"!shadow!",
		"!shadow!"
	},

	{
		"shield",                     { 99, 99, 99, 99, 99, 99, 99, 15 },
		spell_shield,                 NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT( 67),
		12,                           60,
		"",
		"Your force shield shimmers then fades away.",
		"!shield!"
	},

	{
		"shield block",               { 16, 99, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_shield_block,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!shield block!",
		"!shield block!"
	},

	{
		"shocking grasp",             { 99, 99, 99, 99, 99, 99, 99, 99 },
		spell_shocking_grasp,         NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_LIGHTNING,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT( 37),
		10,                           40,
		"shocking grasp",
		"!shocking grasp!",
		"!shocking grasp!"
	},

	{
		"shoot",                      { 45, 99, 99,  8, 99, 99, 99, 99 },
		NULL,                         &gsn_shoot,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_PIERCE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		8,                            80,
		"shot",
		"!shoot!",
		"!shoot!"
	},

	{
		"sleep",                      { 99, 99, 99, 99, 99, 99, 99, 34 },
		spell_sleep,                  &gsn_sleep,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT( 38),
		15,                           40,
		"",
		"You feel less tired.",
		"!sleep!"
	},

	{
		"slow",                       { 99, 99, 99, 99, 47, 99, 99, 99 },
		spell_slow,                   &gsn_slow,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(655),
		10,                           40,
		"slow spell",
		"You speed up.",
		"!slow!"
	},

	{
		"smoke",                      { 99, 99, 99, 99, 99, 61, 99, 99 },
		spell_smoke,                  NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(654),
		80,                           40,
		"smoke",
		"!smoke!",
		"!smoke!"
	},

	{
		"snake dart",                 { 99, 99, 99, 99, 99, 99, 99, 99 },
		spell_snake_dart,             NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_POISON,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT( 48),
		15,                           40,
		"snake dart",
		"!snake dart!",
		"!snake dart!"
	},

	{
		"snatch",                     { 99, 99, 23, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_snatch,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_RESTING,
		SLOT(  0),
		4,                            10,
		"",
		"!snatch!",
		"!snatch!"
	},

	{
		"sneak",                      { 99, 99, 36,  3, 99, 99, 99, 99 },
		NULL,                         &gsn_sneak,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(801),
		8,                            20,
		"",
		"!sneak!",
		"!sneak!"
	},

	{
		"song of the seas",           { 99, 99, 99, 99, 19, 99, 99, 99 },
		spell_song_of_the_seas,       NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(676),
		25,                           40,
		"spell",
		"You feel unsure and guideless as the song of the seas dies down inside you.",
		"!song of the seas!"
	},

	{
		"soothing touch",             { 99, 99, 99, 99, 99, 99, 81, 99 },
		spell_soothing_touch,         NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_FIGHTING,
		SLOT(639),
		150,                          80,
		"",
		"!soothing touch!",
		"!soothing touch!"
	},

	{
		"spear fighting",             { 54,  1, 24, 51, 99, 99, 99, 99 },
		NULL,                         &gsn_spear_fighting,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!spear fighting!",
		"!spear fighting!"
	},

	{
		"spy",                        { 99, 99, 99, 31, 99, 99, 99, 99 },
		NULL,                         &gsn_spy,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		8,                            40,
		"",
		"!spy!",
		"!spy!"
	},

	{
		"stability",                  { 99, 99, 99, 99, 27, 99, 99, 99 },
		spell_stability,              NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_RESTING,
		SLOT(632),
		10,                           40,
		"",
		"You no longer feel the strength of the earth.",
		"!stability!"
	},

	{
		"staff fighting",             { 99, 32, 99, 76, 11,  1, 99, 22 },
		NULL,                         &gsn_staff_fighting,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!staff fighting!",
		"!staff fighting!"
	},

	{
		"steal",                      { 99, 99,  7, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_steal,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		16,                           40,
		"",
		"!steal!",
		"!steal!"
	},

	{
		"stealth",                    { 99, 99, 99, 26, 99, 99, 99, 99 },
		NULL,                         &gsn_stealth,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(802),
		16,                           20,
		"",
		"!stealth!",
		"!stealth!"
	},

	{
		"steelhand",                  { 99, 99, 99, 99, 99, 99, 99, 99 },
		spell_steelhand,              NULL,
		FSKILL_SPELL,                 SFOL_DUNNO,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(665),
		15,                           80,
		"",
		"Your hands become flesh again.",
		"!steelhand!"
	},

	{
		"stone fist",                 { 99, 99, 99, 99, 37, 99, 99, 99 },
		spell_stone_fist,             &gsn_stone_fist,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(  9),
		25,                           60,
		"",
		"Your fists feel soft again.",
		"!stone fist!"
	},

	{
		"stone skin",                 { 99, 99, 99, 99, 15, 99, 99, 99 },
		spell_stone_skin,             NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT( 66),
		50,                           60,
		"",
		"Your skin feels soft again.",
		"!stone skin!"
	},

	{
		"strangle",                   { 99, 99, 99, 53, 99, 99, 99, 99 },
		NULL,                         &gsn_strangle,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		50,                           40,
		"",
		"!strangle!",
		"!strangle!"
	},

	{
		"summon",                     { 99, 99, 99, 99, 99, 99, 99, 64 },
		spell_summon,                 NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_STANDING,
		SLOT( 76),
		50,                           40,
		"summon spell",
		"!summon!",
		"!summon!"
	},

	{
		"sword fighting",             {  1,  4, 17, 99, 99, 25, 99, 99 },
		NULL,                         &gsn_sword_fighting,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!sword fighting!",
		"!sword fighting!"
	},

	{
		"tame",                       { 72, 99, 99, 99, 48, 99, 99, 99 },
		NULL,                         &gsn_tame,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		64,                           80,
		"",
		"!tame!",
		"!tame!"
	},

	{
		"teleport",                   { 99, 99, 99, 99, 27, 99, 99, 99 },
		spell_teleport,               NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_SELF,                POS_FIGHTING,
		SLOT(  2),
		35,                           40,
		"",
		"!teleport!",
		"!teleport!"
	},

	{
		"third attack",               { 14, 17, 61, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_third_attack,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!third attack!",
		"!third attack!"
	},

	{
		"throw",                      { 99, 99,  4, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_throw,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_PIERCE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		8,                            80,
		"throw",
		"!throw!",
		"!throw!"
	},

	{
		"tongues",                    { 99, 99, 99, 99, 10, 99, 99, 99 },
		spell_tongues,                NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(610),
		10,                           20,
		"",
		"You no longer speak in tongues!",
		"!tongues!"
	},

	{
		"torrid balm",                { 99, 99, 99, 99, 99, 99, 99, 41 },
		spell_torrid_balm,            NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_FIRE,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(675),
		100,                          60,
		"torrid balm",
		"The smell of seared flesh dissipates as the torrid balm evaporates.",
		"!torrid balm!"
	},

	{
		"totem",                      { 99, 99, 99, 99, 99, 99, 38, 99 },
		spell_totem,                  NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(688),
		100,                          80,
		"",
		"!totem!",
		"!totem!"
	},

	{
		"track",                      { 17, 99, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_track,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		8,                            40,
		"track",
		"!track!",
		"!track!"
	},

	{
		"transport",                  { 99, 99, 99, 99, 99, 99, 99, 76 },
		spell_transport,              NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(649),
		50,                           40,
		"",
		"!transport!",
		"!transport!"
	},

	{
		"tremor",                     { 99, 99, 99, 99, 43, 99, 99, 99 },
		spell_tremor,                 NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_VIBE,
		TAR_IGNORE,                   POS_FIGHTING,
		SLOT(626),
		45,                           40,
		"tremor",
		"!tremor!",
		"!tremor!"
	},

	{
		"trip",                       { 99, 99, 22, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_trip,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_THRUST,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(  0),
		24,                           60,
		"trip",
		"!trip!",
		"!trip!"
	},

	{
		"truesight",                  { 99, 99, 99, 99, 99, 99, 99, 99 },
		spell_truesight,              NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_SELF,                POS_STANDING,
		SLOT(647),
		50,                           60,
		"",
		"The clarity in your vision fades away.",
		"!truesight!"
	},

	{
		"tumble",                     { 99, 99, 99, 35, 99, 99, 99, 99 },
		NULL,                         &gsn_tumble,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!tumble!",
		"!tumble!"
	},

	{
		"unbarring ways",             { 99, 99, 99, 99, 99, 99, 68, 99 },
		spell_unbarring_ways,         NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(659),
		10,                           80,
		"",
		"!unbarring ways!",
		"!unbarring ways!"
	},

	{
		"understand",                 { 99, 99, 99, 99,  5, 99, 99, 99 },
		spell_understand,             NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_DEFENSIVE,           POS_STANDING,
		SLOT(611),
		10,                           20,
		"",
		"You lose all understanding!",
		"!understand!"
	},

	{
		"unholy word",                { 99, 99, 99, 99, 99, 99, 99, 99 },
		spell_unholy_word,            NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_UNHOLY,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(643),
		17,                           40,
		"unholy word",
		"!unholy word!",
		"!unholy word!"
	},

	{
		"vampiric touch",             { 99, 99, 99, 99, 99, 99, 99, 52 },
		spell_vampiric_touch,         &gsn_vampiric_touch,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_LIFE,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(660),
		35,                           40,
		"vampiric touch",
		"!vampiric touch!",
		"!vampiric touch!"
	},

	{
		"ventriloquate",              { 99, 99, 99, 99, 99,  7, 99, 99 },
		spell_ventriloquate,          NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_PSIONIC,
		TAR_IGNORE,                   POS_STANDING,
		SLOT( 41),
		5,                            40,
		"",
		"!ventriloquate!",
		"!ventriloquate!"
	},

	{
		"voice",                      { 99, 99, 99, 17, 99, 99, 99, 99 },
		NULL,                         &gsn_voice,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_RESTING,
		SLOT(  0),
		0,                            4,
		"",
		"!voice!",
		"!voice!"
	},

	{
		"warmth",                     { 99, 99, 99, 99, 99, 99, 99, 26 },
		NULL,                         &gsn_warmth,
		FSKILL_NONE,                  SFOL_DUNNO,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"warmth",
		"!warmth!"
	},

	{
		"waterburst",                 { 99, 99, 99, 99, 99, 99, 99, 99 },
		spell_waterburst,             NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_BASH,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT( 46),
		15,                           40,
		"waterburst",
		"!waterburst!",
		"!waterburst!"
	},

	{
		"weaken",                     { 99, 99, 99, 99, 99, 99, 99,  4 },
		spell_weaken,                 NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_LIFE,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT( 68),
		20,                           40,
		"weaken spell",
		"You feel stronger.",
		"!weaken!"
	},

	{
		"whip fighting",              { 99, 18, 43, 42, 99, 99, 99,  1 },
		NULL,                         &gsn_whip_fighting,
		FSKILL_NONE,                  SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(  0),
		0,                            0,
		"",
		"!whip fighting!",
		"!whip fighting!"
	},

	{
		"whirl",                      { 99, 62, 99, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_whirl,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_THRUST,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT(  0),
		32,                           60,
		"whirl",
		"!whirl!",
		"!whirl!"
	},

	{
		"windblast",                  { 99, 99, 99, 99, 99, 99, 99, 99 },
		spell_windblast,              NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_VIBE,
		TAR_CHAR_OFFENSIVE,           POS_FIGHTING,
		SLOT( 45),
		15,                           40,
		"windblast",
		"!windblast!",
		"!windblast!"
	},

	{
		"winged call",                { 99, 99, 99, 99, 99, 54, 99, 99 },
		spell_winged_call,            NULL,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_STANDING,
		SLOT(684),
		150,                          40,
		"",
		"!winged call!",
		"!winged call!"
	},

	{
		"withdraw",                   { 99, 99, 26, 99, 99, 99, 99, 99 },
		NULL,                         &gsn_withdraw,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_NONE,
		TAR_IGNORE,                   POS_FIGHTING,
		SLOT(  0),
		0,                            0,
		"",
		"!withdraw!",
		"!withdraw!"
	},

	{
		"word of recall",             { 99, 99, 99, 99, 26, 99, 99, 99 },
		spell_word_of_recall,         &gsn_word_of_recall,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_CHAR_SELF,                POS_RESTING,
		SLOT( 42),
		5,                            40,
		"",
		"!word of recall!",
		"!word of recall!"
	},

	{
		"write spell",                { 99, 99, 99, 99, 99, 99, 99, 19 },
		spell_write_spell,            &gsn_write_spell,
		FSKILL_SPELL,                 SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_RESTING,
		SLOT(602),
		5,                            40,
		"",
		"!write spell!",
		"!write spell!"
	},

	{
		"zap",                        { 99, 99, 99, 99, 99,  4, 99,  6 },
		NULL,                         &gsn_zap,
		FSKILL_CMD,                   SFOL_NONE,
		RSKILL_NONE,                  DAM_MAGIC,
		TAR_IGNORE,                   POS_FIGHTING,
		SLOT(  0),
		4,                            60,
		"zap",
		"!zap!",
		"!zap!"
	}
};

const	struct	topic_data topic_table	[MAX_TOPIC]	=
{
	{"Announcements",			 5},
	{"Game Changes and Ideas", 	 5},
	{"Area Creators", 			 5},
	{"Bugs and Typos",			 5},
	{"Complaints", 			 5},
	{"Chat",					 5},
	{"Private Note Boards",		 5}
};

const struct bitvector_type bitvector_table [MAX_BITVECTOR] =
{
	{ "ACT_IS_NPC",                   BV01},
	{ "ACT_SENTINEL",                 BV02},
	{ "ACT_SCAVENGER",                BV03},
	{ "ACT_DRUNK",                    BV04},
	{ "ACT_MOUNT",                    BV05},
	{ "ACT_AGGRESSIVE",               BV06},
	{ "ACT_STAY_SECTOR",              BV07},
	{ "ACT_WIMPY",                    BV08},
	{ "ACT_PET",                      BV09},
	{ "ACT_TRAIN",                    BV10},
	{ "ACT_PRACTICE",                 BV11},
	{ "ACT_WEAK",                     BV12},
	{ "ACT_SMART",                    BV13},
	{ "ACT_ONE_FIGHT",                BV14},
	{ "ACT_NO_ORDER",                 BV15},
	{ "ACT_RIDE",                     BV16},
	{ "ACT_BODY",                     BV17},
	{ "ACT_RACE",                     BV18},
	{ "ACT_UNDEAD",                   BV19},
	{ "ACT_ELEMENTAL",                BV20},
	{ "ACT_CLAN_GUARD",               BV21},
	{ "ACT_CLAN_HEALER",              BV22},
	{ "ACT_WILL_DIE",                 BV23},
	{ "ACT_NO_DAMAGE",                BV24},
	{ "ACT_CLASS",                    BV25},

	{ "AFF_BLIND",                    BV01},
	{ "AFF_INVISIBLE",                BV02},
	{ "AFF_IMP_INVISIBLE",            BV03},
	{ "AFF_DETECT_INVIS",             BV04},
	{ "AFF_RESERVED_B",               BV05},
	{ "AFF_DETECT_HIDDEN",            BV06},
	{ "AFF_TRUESIGHT",                BV07},
	{ "AFF_SANCTUARY",                BV08},
	{ "AFF_RESERVED_C",               BV09},
	{ "AFF_INFRARED",                 BV10},
	{ "AFF_CURSE",                    BV11},
	{ "AFF_UNDERSTAND",               BV12},
	{ "AFF_POISON",                   BV13},
	{ "AFF_PROTECT_EVIL",             BV14},
	{ "AFF_PROTECT_GOOD",             BV15},
	{ "AFF_SNEAK",                    BV16},
	{ "AFF_HIDE",                     BV17},
	{ "AFF_SLEEP",                    BV18},
	{ "AFF_CHARM",                    BV19},
	{ "AFF_FLYING",                   BV20},
	{ "AFF_PASS_DOOR",                BV21},
	{ "AFF_STEALTH",                  BV22},
	{ "AFF_CLEAR",                    BV23},
	{ "AFF_HUNT",                     BV24},
	{ "AFF_TONGUES",                  BV25},
	{ "AFF_ETHEREAL",                 BV26},
	{ "AFF_HASTE",                    BV27},
	{ "AFF_STABILITY",                BV28},

	{ "AFLAG_NODEBUG",                BV01},
	{ "AFLAG_NOTELEPORT",             BV02},
	{ "AFLAG_NOGOHOME",               BV03},
	{ "AFLAG_NORECALL",               BV04},
	{ "AFLAG_NOCASTLE",               BV05},
	{ "AFLAG_NORIP",                  BV06},
	{ "AFLAG_FREEQUIT",               BV07},
	{ "AFLAG_NOSUMMON",               BV08},
	{ "AFLAG_AUTOSAVE",               BV09},
	{ "AFLAG_MODIFIED",               BV10},
	{ "AFLAG_WEATHER",                BV11},

	{ "APPLY_NONE",                      0},
	{ "APPLY_STR",                       1},
	{ "APPLY_DEX",                       2},
	{ "APPLY_INT",                       3},
	{ "APPLY_WIS",                       4},
	{ "APPLY_CON",                       5},
	{ "APPLY_SEX",                       6},
	{ "APPLY_RACE",                      7},
	{ "APPLY_LEVEL",                     8},
	{ "APPLY_AGE",                       9},
	{ "APPLY_HEIGHT",                   10},
	{ "APPLY_WEIGHT",                   11},
	{ "APPLY_MANA",                     12},
	{ "APPLY_HIT",                      13},
	{ "APPLY_MOVE",                     14},
	{ "APPLY_AC",                       17},
	{ "APPLY_HITROLL",                  18},
	{ "APPLY_DAMROLL",                  19},
	{ "APPLY_SAVING_PARA",              20},
	{ "APPLY_SAVING_ROD",               21},
	{ "APPLY_SAVING_PETRI",             22},
	{ "APPLY_SAVING_BREATH",            23},
	{ "APPLY_SAVING_SPELL",             24},

	{ "BODY_HEAD",                    BV01},
	{ "BODY_MOUTH",                   BV02},
	{ "BODY_EYE",                     BV03},
	{ "BODY_TORSO",                   BV04},
	{ "BODY_HIP",                     BV05},
	{ "BODY_LEG",                     BV06},
	{ "BODY_ARM",                     BV07},
	{ "BODY_WING",                    BV08},
	{ "BODY_TAIL",                    BV09},
	{ "BODY_TENTACLE",                BV10},
	{ "BODY_HORN",                    BV11},
	{ "BODY_CLAW",                    BV12},
	{ "BODY_HAND",                    BV13},
	{ "BODY_FOOT",                    BV14},
	{ "BODY_BRANCH",                  BV15},
	{ "BODY_ROOT",                    BV16},
	{ "BODY_TWIG",                    BV17},
	{ "BODY_TRUNK",                   BV18},
	{ "BODY_PINCERS",                 BV19},
	{ "BODY_SPINNARET",               BV20},
	{ "BODY_BEAK",                    BV21},

	{ "CLASS_RANGER",                    0},
	{ "CLASS_GLADIATOR",                 1},
	{ "CLASS_MARAUDER",                  2},
	{ "CLASS_NINJA",                     3},
	{ "CLASS_ELEMENTALIST",              4},
	{ "CLASS_ILLUSIONIST",               5},
	{ "CLASS_MONK",                      6},
	{ "CLASS_NECROMANCER",               7},

	{ "CONT_CLOSEABLE",               BV01},
	{ "CONT_PICKPROOF",               BV02},
	{ "CONT_CLOSED",                  BV03},
	{ "CONT_LOCKED",                  BV04},
	{ "CONT_MAGICAL_LOCK",            BV05},
	{ "CONT_TRAPPED_LOCK",            BV06},
	{ "CONT_MECHANICAL_LOCK",         BV07},
	{ "CONT_SMALL_LOCK",              BV08},
	{ "CONT_BIG_LOCK",                BV08},
	{ "CONT_EASY_PICK",               BV10},
	{ "CONT_HARD_PICK",               BV11},

	{ "DIR_NORTH",                       0},
	{ "DIR_EAST",                        1},
	{ "DIR_SOUTH",                       2},
	{ "DIR_WEST",                        3},
	{ "DIR_UP",                          4},
	{ "DIR_DOWN",                        5},

	{ "DOOR_OPEN",                       0},
	{ "DOOR_CLOSED",                     1},
	{ "DOOR_CLOSED_LOCKED",              2},

	{ "EX_ISDOOR",                    BV01},
	{ "EX_CLOSED",                    BV02},
	{ "EX_LOCKED",                    BV03},
	{ "EX_HIDDEN",                    BV04},
	{ "EX_RIP",                       BV05},
	{ "EX_PICKPROOF",                 BV06},
	{ "EX_BASHPROOF",                 BV07},
	{ "EX_MAGICPROOF",                BV08},
	{ "EX_BASHED",                    BV09},
	{ "EX_UNBARRED",                  BV10},
	{ "EX_BACKDOOR",                  BV11},
	{ "EX_CLAN_BACKDOOR",             BV12},
	{ "EX_PASSPROOF",                 BV13},
	{ "EX_MAGICAL_LOCK",              BV14},
	{ "EX_TRAPPED_LOCK",              BV15},
	{ "EX_MECHANICAL_LOCK",           BV16},
	{ "EX_SMALL_LOCK",                BV17},
	{ "EX_BIG_LOCK",                  BV18},
	{ "EX_EASY_PICK",                 BV19},
	{ "EX_HARD_PICK",                 BV20},

	{ "FLAG_CLASS_RANGER",            BV01},
	{ "FLAG_CLASS_GLADIATOR",         BV02},
	{ "FLAG_CLASS_MARAUDER",          BV03},
	{ "FLAG_CLASS_NINJA",             BV04},
	{ "FLAG_CLASS_ELEMENTALIST",      BV05},
	{ "FLAG_CLASS_ILLUSIONIST",       BV06},
	{ "FLAG_CLASS_MONK",              BV07},
	{ "FLAG_CLASS_NECROMANCER",       BV08},

	{ "FURNITURE_SLEEP_IN",           BV01},
	{ "FURNITURE_SLEEP_ON",           BV02},
	{ "FURNITURE_REST_IN",            BV03},
	{ "FURNITURE_REST_ON",            BV04},
	{ "FURNITURE_SIT_IN",             BV05},
	{ "FURNITURE_SIT_ON",             BV06},
	{ "FURNITURE_STAND_IN",           BV07},
	{ "FURNITURE_STAND_ON",           BV08},

	{ "GATE_RANDOM",                  BV01},
	{ "GATE_GOWITH",                  BV02},
	{ "GATE_NOFLEE",                  BV03},
	{ "GATE_STEP_THROUGH",            BV04},
	{ "GATE_STEP_INTO",               BV05},
	{ "GATE_RANDOM_AREA",             BV06},

	{ "GOD_NEUTRAL",                     0},
	{ "GOD_CHAOS",                       1},
	{ "GOD_ORDER",                       2},

	{ "ITEM_FLAG_GLOW",               BV01},
	{ "ITEM_FLAG_HUM",                BV02},
	{ "ITEM_FLAG_DARK",               BV03},
	{ "ITEM_FLAG_LOCK",               BV04},
	{ "ITEM_FLAG_EVIL",               BV05},
	{ "ITEM_FLAG_INVIS",              BV06},
	{ "ITEM_FLAG_MAGIC",              BV07},
	{ "ITEM_FLAG_NODROP",             BV08},
	{ "ITEM_FLAG_BLESS",              BV09},
	{ "ITEM_FLAG_ANTI_GOOD",          BV10},
	{ "ITEM_FLAG_ANTI_EVIL",          BV11},
	{ "ITEM_FLAG_ANTI_NEUTRAL",       BV12},
	{ "ITEM_FLAG_NOREMOVE",           BV13},
	{ "ITEM_FLAG_INVENTORY",          BV14},
	{ "ITEM_FLAG_BURNING",            BV15},
	{ "ITEM_FLAG_NOT_VALID",          BV16},
	{ "ITEM_FLAG_AUTO_ENGRAVE",       BV17},
	{ "ITEM_FLAG_ETHEREAL",           BV19},
	{ "ITEM_FLAG_MOUNT",              BV20},
	{ "ITEM_FLAG_MODIFIED",           BV21},
	{ "ITEM_FLAG_MYTHICAL",           BV22},
	{ "ITEM_FLAG_EPICAL",             BV23},

	{ "ITEM_TYPE_NOTHING",               0},
	{ "ITEM_TYPE_LIGHT",                 1},
	{ "ITEM_TYPE_SCROLL",                2},
	{ "ITEM_TYPE_WAND",                  3},
	{ "ITEM_TYPE_STAFF",                 4},
	{ "ITEM_TYPE_WEAPON",                5},
	{ "ITEM_TYPE_TREASURE",              8},
	{ "ITEM_TYPE_ARMOR",                 9},
	{ "ITEM_TYPE_POTION",               10},
	{ "ITEM_TYPE_FURNITURE",            12},
	{ "ITEM_TYPE_TRASH",                13},
	{ "ITEM_TYPE_CONTAINER",            15},
	{ "ITEM_TYPE_DRINK_CON",            17},
	{ "ITEM_TYPE_KEY",                  18},
	{ "ITEM_TYPE_FOOD",                 19},
	{ "ITEM_TYPE_MONEY",                20},
	{ "ITEM_TYPE_BOAT",                 22},
	{ "ITEM_TYPE_CORPSE_NPC",           23},
	{ "ITEM_TYPE_CORPSE_PC",            24},
	{ "ITEM_TYPE_FOUNTAIN",             25},
	{ "ITEM_TYPE_PILL",                 26},
	{ "ITEM_TYPE_PORTAL",               27},
	{ "ITEM_TYPE_ARTIFACT",             28},
	{ "ITEM_TYPE_LOCKPICK",             29},
	{ "ITEM_TYPE_AMMO",                 30},
	{ "ITEM_TYPE_TOTEM",                31},
	{ "ITEM_TYPE_PLANT",                32},

	{ "ITEM_WEAR_TAKE",               BV01},
	{ "ITEM_WEAR_FINGER",             BV02},
	{ "ITEM_WEAR_NECK",               BV03},
	{ "ITEM_WEAR_BODY",               BV04},
	{ "ITEM_WEAR_HEAD",               BV05},
	{ "ITEM_WEAR_LEGS",               BV06},
	{ "ITEM_WEAR_FEET",               BV07},
	{ "ITEM_WEAR_HANDS",              BV08},
	{ "ITEM_WEAR_ARMS",               BV09},
	{ "ITEM_WEAR_SHIELD",             BV10},
	{ "ITEM_WEAR_ABOUT",              BV11},
	{ "ITEM_WEAR_WAIST",              BV12},
	{ "ITEM_WEAR_WRIST",              BV13},
	{ "ITEM_WEAR_WIELD",              BV14},
	{ "ITEM_WEAR_HOLD",               BV15},
	{ "ITEM_WEAR_HEART",              BV16},

	{ "LIQ_WATER",                       0},
	{ "LIQ_BEER",                        1},
	{ "LIQ_WINE",                        2},
	{ "LIQ_ALE",                         3},
	{ "LIQ_DARKALE",                     4},
	{ "LIQ_WHISKY",                      5},
	{ "LIQ_LEMONADE",                    6},
	{ "LIQ_FIREWATER",                   7},
	{ "LIQ_MOONSHINE",                   8},
	{ "LIQ_SLIME",                       9},
	{ "LIQ_MILK",                       10},
	{ "LIQ_TEA",                        11},
	{ "LIQ_COFFEE",                     12},
	{ "LIQ_BLOOD",                      13},
	{ "LIQ_SALTWATER",                  14},
	{ "LIQ_COKE",                       15},
	{ "LIQ_ORANGEJUICE",                16},
	{ "LIQ_BRANDY",                     17},
	{ "LIQ_ICEWINE",                    18},
	{ "LIQ_RUM",                        19},
	{ "LIQ_VODKA",                      20},
	{ "LIQ_CHAMPAGNE",                  21},

	{ "OAPPLY_HIT",                      1},
	{ "OAPPLY_MOVE",                     2},
	{ "OAPPLY_MANA",                     3},
	{ "OAPPLY_ALIGNMENT",                4},

	{ "OIF_WEAR_LOC",                    8},
	{ "OIF_TIME_OF_DAY",                 9},
	{ "OIF_WEATHER",                    10},
	{ "OIF_RANDOM_PERCENT",             11},
	{ "OIF_USER_PERCENT_HITPT",         12},
	{ "OIF_USER_PERCENT_MANA",          13},
	{ "OIF_USER_PERCENT_MOVE",          14},
	{ "OIF_USER_SECTOR",                15},
	{ "OIF_USER_ALIGNMENT",             16},
	{ "OIF_USER_GOLD",                  17},
	{ "OIF_USER_CLASS",                 18},
	{ "OIF_USER_WHICH_GOD",             19},
	{ "OIF_USER_AREA",                  20},
	{ "OIF_USER_LEVEL",                 21},
	{ "OIF_USER_POSITION",              22},
	{ "OIF_USER_RACE",                  23},
	{ "OIF_USER_SEX",                   24},
	{ "OIF_USER_ROOM_NUM",              25},
	{ "OIF_USER_FIGHTING",              26},

	{ "OPROG_ECHO",                      0},
	{ "OPROG_GOD_COMMAND",               1},
	{ "OPROG_GOD_ARGUMENT",              2},
	{ "OPROG_COMMAND",                   3},
	{ "OPROG_ARGUMENT",                  4},
	{ "OPROG_IF_HAS_OBJECT",             5},
	{ "OPROG_IF",                        6},
	{ "OPROG_JUNK",                      7},
	{ "OPROG_QUEST_SET",                 8},
	{ "OPROG_QUEST_ADD",                 9},
	{ "OPROG_OBJECT_QUEST_IF",          10},
	{ "OPROG_PLAYER_QUEST_IF",          11},
	{ "OPROG_APPLY",                    12},

	{ "PICK_MAGICAL_LOCK",            BV01},
	{ "PICK_TRAPPED_LOCK",            BV02},
	{ "PICK_MECHANICAL_LOCK",         BV03},
	{ "PICK_SMALL_LOCK",              BV04},
	{ "PICK_BIG_LOCK",                BV05},
	{ "PICK_EASY_PICK",               BV06},
	{ "PICK_HARD_PICK",               BV07},

	{ "POISONED_FALSE",                  0},
	{ "POISONED_TRUE",                   1},

	{ "POS_DEAD",                        0},
	{ "POS_MORTAL",                      1},
	{ "POS_INCAP",                       2},
	{ "POS_STUNNED",                     3},
	{ "POS_SLEEPING",                    4},
	{ "POS_RESTING",                     5},
	{ "POS_SITTING",                     6},
	{ "POS_FIGHTING",                    7},
	{ "POS_STANDING",                    8},

	{ "RACE_HUMAN",                      0},
	{ "RACE_ELF",                        1},
	{ "RACE_DROW",                       2},
	{ "RACE_DWARF",                      3},
	{ "RACE_GNOME",                      4},
	{ "RACE_ORC",                        5},
	{ "RACE_OGRE",                       6},
	{ "RACE_HOBBIT",                     7},
	{ "RACE_WEREWOLF",                   8},
	{ "RACE_DEMON",                      9},
	{ "RACE_WRAITH",                    10},
	{ "RACE_TROLL",                     11},
	{ "RACE_VAMPIRE",                   12},
	{ "RACE_TITAN",                     13},
	{ "RACE_ENT",                       14},
	{ "RACE_DRAGON",                    15},

	{ "ROOM_DARK",                    BV01},
	{ "ROOM_SMOKE",                   BV02},
	{ "ROOM_NO_MOB",                  BV03},
	{ "ROOM_INDOORS",                 BV04},
	{ "ROOM_GLOBE",                   BV05},
	{ "ROOM_NO_GOHOME",               BV06},
	{ "ROOM_CLAN_DONATION",           BV07},
	{ "ROOM_ALTAR_N",                 BV08},
	{ "ROOM_BANK",                    BV09},
	{ "ROOM_PRIVATE",                 BV10},
	{ "ROOM_SAFE",                    BV11},
	{ "ROOM_SOLITARY",                BV12},
	{ "ROOM_PET_SHOP",                BV13},
	{ "ROOM_NO_RECALL",               BV14},
	{ "ROOM_RIP",                     BV15},
	{ "ROOM_BLOCK",                   BV16},
	{ "ROOM_NO_SAVE",                 BV17},
	{ "ROOM_MORGUE",                  BV18},
	{ "ROOM_ALLOW_WAR",               BV19},
	{ "ROOM_ALLOW_GLA",               BV20},
	{ "ROOM_ALLOW_MAR",               BV21},
	{ "ROOM_ALLOW_NIN",               BV22},
	{ "ROOM_ALLOW_DRU",               BV23},
	{ "ROOM_ALLOW_SOR",               BV24},
	{ "ROOM_ALLOW_SHA",               BV25},
	{ "ROOM_ALLOW_WLC",               BV26},
	{ "ROOM_IS_CASTLE",               BV27},
	{ "ROOM_IS_ENTRANCE",             BV28},
	{ "ROOM_NOTE_BOARD",              BV29},
	{ "ROOM_NO_CASTLE",               BV30},
	{ "ROOM_NO_RIP",                  BV31},
	{ "ROOM_MAZE",                    BV32},
	{ "ROOM_ICE",                     BV33},
	{ "ROOM_DYNAMIC",                 BV34},
	{ "ROOM_WILDERNESS",              BV35},

	{ "SECT_INSIDE",                     0},
	{ "SECT_CITY",                       1},
	{ "SECT_FIELD",                      2},
	{ "SECT_FOREST",                     3},
	{ "SECT_HILLS",                      4},
	{ "SECT_MOUNTAIN",                   5},
	{ "SECT_LAKE",                       6},
	{ "SECT_RIVER",                      7},
	{ "SECT_OCEAN",                      8},
	{ "SECT_AIR",                        9},
	{ "SECT_DESERT",                    10},
	{ "SECT_LAVA",                      11},
	{ "SECT_ETHEREAL",                  12},
	{ "SECT_ASTRAL",                    13},
	{ "SECT_UNDER_WATER",               14},
	{ "SECT_UNDER_GROUND",              15},
	{ "SECT_DEEP_EARTH",                16},
	{ "SECT_ROAD",                      17},
	{ "SECT_SWAMP",                     18},
	{ "SECT_BEACH",                     19},
	{ "SECT_TUNDRA",                    20},
	{ "SECT_EDGE",                      21},

	{ "SEX_NEUTRAL",                     0},
	{ "SEX_MALE",                        1},
	{ "SEX_FEMALE",                      2},

	{ "TRIG_COMMAND",                 BV01},
	{ "TRIG_VOID",                    BV02},
	{ "TRIG_UNKNOWN",                 BV03},
	{ "TRIG_TICK",                    BV04},
	{ "TRIG_DAMAGE",               BV05},
	{ "TRIG_HIT",                     BV06},
	{ "TRIG_WEAR",                    BV07},
	{ "TRIG_REMOVE",               BV08},
	{ "TRIG_SACRIFICE",               BV09},
	{ "TRIG_DROP",                    BV10},
	{ "TRIG_GET",                     BV11},
	{ "TRIG_ROOM_COMMAND",          BV12},
	{ "TRIG_ROOM_UNKNOWN",          BV13},
	{ "TRIG_WEAR_COMMAND",          BV14},
	{ "TRIG_WEAR_UNKNOWN",            BV15},

	{ "WEAPONTYPE_WEAPON",               0},
	{ "WEAPONTYPE_SWORD",                1},
	{ "WEAPONTYPE_DAGGER",               2},
	{ "WEAPONTYPE_AXE",                  3},
	{ "WEAPONTYPE_MACE",                 4},
	{ "WEAPONTYPE_STAFF",                5},
	{ "WEAPONTYPE_WHIP",                 6},
	{ "WEAPONTYPE_FLAIL",                7},
	{ "WEAPONTYPE_SPEAR",                8},
	{ "WEAPONTYPE_CLAW",                 9},
	{ "WEAPONTYPE_SHORTBOW",            10},
	{ "WEAPONTYPE_LONGBOW",             11},
	{ "WEAPONTYPE_CROSSBOW",            12},
	{ "WEAPONTYPE_BLOWPIPE",            13},

	{ "WEAPON_SLICE",                    1},
	{ "WEAPON_STAB",                     2},
	{ "WEAPON_SLASH",                    3},
	{ "WEAPON_WHIP",                     4},
	{ "WEAPON_CLAW",                     5},
	{ "WEAPON_BLAST",                    6},
	{ "WEAPON_POUND",                    7},
	{ "WEAPON_CRUSH",                    8},
	{ "WEAPON_GREP",                     9},
	{ "WEAPON_BITE",                    10},
	{ "WEAPON_PIERCE",                  11},
	{ "WEAPON_CHOP",                    12},

	{ "WEAR_NONE",                      -1},
	{ "WEAR_LIGHT",                      0},
	{ "WEAR_FINGER_L",                   1},
	{ "WEAR_FINGER_R",                   2},
	{ "WEAR_NECK_A",                     3},
	{ "WEAR_NECK_B",                     4},
	{ "WEAR_BODY",                       5},
	{ "WEAR_HEAD",                       6},
	{ "WEAR_LEGS",                       7},
	{ "WEAR_FEET",                       8},
	{ "WEAR_HANDS",                      9},
	{ "WEAR_ARMS",                      10},
	{ "WEAR_SHIELD",                    11},
	{ "WEAR_ABOUT",                     12},
	{ "WEAR_WAIST",                     13},
	{ "WEAR_WRIST_L",                   14},
	{ "WEAR_WRIST_R",                   15},
	{ "WEAR_WIELD",                     16},
	{ "WEAR_DUAL_WIELD",                17},
	{ "WEAR_HOLD",                      18},
	{ "WEAR_HEART",                     19},
};
