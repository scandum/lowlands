/***************************************************************************
 * Lola  1.0 by Igor van den Hoven.                                        *
 *                                                                         *
 * Emud  3.0 by Igor van den Hoven.                                        *
 ***************************************************************************/

#include "mud.h"

void get_char_him        args( ( CHAR_DATA *ch, CHAR_DATA *victim, char *result ) );
void get_char_his        args( ( CHAR_DATA *ch, CHAR_DATA *victim, char *result ) );
void get_char_he         args( ( CHAR_DATA *ch, CHAR_DATA *victim, char *result ) );
void get_char_looks      args( ( CHAR_DATA *ch, CHAR_DATA *victim, char *result ) );
void get_char_race       args( ( CHAR_DATA *ch, CHAR_DATA *victim, char *result ) );
void get_char_skin_color args( ( CHAR_DATA *ch, CHAR_DATA *victim, char *result ) );
void get_char_eye_color  args( ( CHAR_DATA *ch, CHAR_DATA *victim, char *result ) );
void get_char_hair       args( ( CHAR_DATA *ch, CHAR_DATA *victim, char *result ) );
void get_char_hair_type  args( ( CHAR_DATA *ch, CHAR_DATA *victim, char *result ) );
void get_char_hair_color args( ( CHAR_DATA *ch, CHAR_DATA *victim, char *result ) );
 
typedef void DO_DYN args( ( CHAR_DATA *ch, CHAR_DATA *victim, char *result ) );

struct dyn_type
{
	char       * name;
	DO_DYN     * exec;
};

const struct dyn_type dynamic_table[] =
{
	{	"his",                get_char_his          },
	{       "him",                get_char_him          },
	{       "he",                 get_char_he           },
	{       "looks",              get_char_looks        },
	{       "race",               get_char_race         },
	{	"skin_color",         get_char_skin_color   },
	{	"eye_color",          get_char_eye_color    },
	{       "hair",               get_char_hair         },
	{       "hair_type",          get_char_hair_type    },
	{       "hair_color",         get_char_hair_color   },
	{	"",                   NULL                  }
};

ROOM_INDEX_DATA *dyn_room;
CHAR_DATA *dyn_char;
char dyn_out[MAX_STRING_LENGTH];
long long dyn_arg[4];

char *get_arg_in_brackets(char *string, char *result)
{
	char *pti, *pto;
	int nest = 1;

	pti = string;
	pto = result;

	pti++;

	while (*pti)
	{
		if (*pti == '[')
		{
			nest++;
		}
		else if (*pti == ']')
		{
			nest--;

			if (nest == 0)
			{
				break;
			}
		}
		*pto++ = *pti++;
	}

	if (*pti != 0)
	{
		pti++;
	}
	*pto = 0;

	return pti;
}

void get_dynamic_content(CHAR_DATA *ch, CHAR_DATA *victim, char *string, char *result)
{
	int cnt;

	push_call("get_dynamic_content(%p,%p,%p,%p)",ch,victim,string,result);

	for (cnt = 0 ; *dynamic_table[cnt].name ; cnt++)
	{
		if (!strcmp(string, dynamic_table[cnt].name))
		{
			dynamic_table[cnt].exec (ch, victim, result);

			pop_call();
			return;
		}
	}
	sprintf(result, "[%s]", string);

	pop_call();
	return;
}

char *get_dynamic_player_description(CHAR_DATA *ch, CHAR_DATA *victim, char *string)
{
	static char result[MAX_STRING_LENGTH];
	char *pti, *pto, buf[MAX_INPUT_LENGTH];

	pti = string;
	pto = result;

	while (*pti)
	{
		if (*pti == '[')
		{
			pti = get_arg_in_brackets(pti, buf);

			get_dynamic_content(ch, victim, buf, pto);

			while (*pto) pto++;
		}
		else
		{
			*pto++ = *pti++;
		}
	}
	*pto = 0;

	return result;
}

char *get_dyn_position()
{
	char * const dynamic_pos_earth [] =
	{
		"lying deadly wounded on",
		"lying mortaly wounded on",
		"lying incapitated on",
		"lying stunned on",
		"sleeping on",
		"resting on",
		"sitting on",
		"fighting on",
		"standing on"
	};

	char * const dynamic_pos_water [] =
	{
		"floating above",
		"sailing through",
		"swimming in"
	};

	if (CAN_FLY(dyn_char))
	{
		return dynamic_pos_water[0];
	}

	if (IS_SET(sector_table[dyn_room->sector_type].flags, SFLAG_SWIM))
	{
		OBJ_DATA *boat;

		for (boat = dyn_char->first_carrying ; boat ; boat = boat->next_content)
		{
			if (boat->item_type == ITEM_BOAT)
			{
				return dynamic_pos_water[1];
			}
		}
		return dynamic_pos_water[2];
	}
	else
	{
		return dynamic_pos_earth[dyn_char->position];
	}
}

void process_dyn_sector()
{
	strcat(dyn_out, sector_table[dyn_room->sector_type].sector_name);
}

void process_dyn_test()
{
	cat_sprintf(dyn_out, "%lld %lld %lld %lld", dyn_arg[0], dyn_arg[1], dyn_arg[2], dyn_arg[3]);
}

void process_dyn_sector_climate()
{
	char * const dynamic_sector_climate[SECT_MAX][9] =
	{
		{
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			""
		},
		{
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			""
		},

		{
			"frozen wasteland",
			"desolated wasteland",
			"cracked wasteland",
			"misty plain",
			"green plain",
			"arid plain",
			"damp field",
			"sodden field",
			"verdant field"
		},

		{
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			""
		},
		{
			"frigid rock hill",
			"dusty rock hill",
			"arid rock hill",
			"frosty hill",
			"lush hill",
			"flourishing hill",
			"frigid sand slope",
			"boggy sand slope",
			"muddy sand slope"
		},
		{
			"ice-capped mountain",
			"towering mountain",
			"rocky mountain",
			"windswept mountain",
			"soaring mountain",
			"balmy mountain",
			"waterlogged mountain",
			"slippery mountain",
			"steamy mountain"
		},
		{
			"icebound lake",
			"shallow lake",
			"tepid lake",
			"iceblue lake",
			"azure lake",
			"clouded lake",
			"foggy lake",
			"billowing lake",
			"murky lake"
		},
		{
			"glacial river",
			"sluggish river",
			"murky river",
			"steel grey river",
			"winding river",
			"swampy river",
			"swollen stream",
			"swirling stream",
			"hazy stream"
		},
		{
			"arctic ocean",
			"cold ocean",
			"azure ocean",
			"leaden ocean",
			"rippling ocean",
			"silken ocean",
			"chilly ocean",
			"floroush ocean",
			"torrid ocean"
		},
		{
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			""
		},
		{
			"frigid sand plain",
			"barren sand plain",
			"parched sand plain",
			"frozen desert",
			"sandy desert",
			"sultry desert",
			"damp desert",
			"fertile desert",
			"sweltering desert"
		},
		{
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			""
		},
		{
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			""
		},
		{
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			""
		},
		{
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			""
		},
		{
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			""
		},
		{
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			""
		},
		{
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			""
		},
		{
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			""
		},
		{
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			""
		},
		{
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			""
		},
		{
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			""
		}
	};
	if (dynamic_sector_climate[0][0])
	{
		return;
	}
}

char * const dyn_road_exits [] =
{
	"a dirt road that has come to a dead end",
	"a dirt road",
	"a dirt intersection",
	"a dirt crossroads"
};

/*
	SKY 0...3 - WIND 0...1 - TEMP 0...1
*/

char * const dyn_road_sky_wind_temp [] =
{
	"dust blowing softly in the breeze",
	"small puddles muddying the ground",
	"soft raindrops creating muddy puddles",
	"the pouring rain overflowing the puddles muddying the ground",

	"a strong wind swirling dust around",
	"a strong wind rippling small puddles on the ground",
	"soft raindrops creating muddy puddles",
	"the pouring rain overflowing the puddles muddying the ground",


	"feathery strokes of frost covering the ground",
	"a layer of snow covering the ground",
	"the falling snow covering the ground in a white blanket",
	"the falling snow creating enormous drifts on the ground",

	"a strong wind swirling powdery snow around",
	"powdery snow blowing white in the harsh wind",
	"the falling snow covering the ground in a white blanket",
	"the falling snow creating enormous drifts on the ground"
};

/*
	SUN 0...3		SKY 0...3		TEMPERATURE 0...1
*/

char * const dyn_sun_sky_temp [] =
{
	"your surroundings lit only by stars",
	"the rising sun revealing a perfect sky",
	"a canopy of azure blue sky above you, the sun shining warmly",
	"the colors of the sky melting into deep crimson and gold as the sun sets",

	"a few stars barely peeking through the clouded skies above",
	"the sun painting the clouded sky in shades of deep blue and pink as it rises",
	"the clouds lowering over your head in shades of grey and lavender",
	"the sun sinking into a band of red and amber clouds as it sets",

	"the rain falling inky and black in the night",
	"the rain greeting the sun as it peeks above the horizon",
	"silvery rain falling from a dark grey sky",
	"the rain growing purple as the sun sets for the day",

	"the dark velvet of the sky only illuminated by the sudden flash of lightning",
	"a brilliant blue bolt of lightning greeting the sun as it cautiously rises",
	"the leaden light surrounding you brightened by occasional flashes of lightning",
	"an enormous bolt of lightning striking as the sun lowers its head for the night",


	"the stars sparkling like diamonds in a midnight blue sky",
	"the light of the stars giving way to the arrival of the sun",
	"the sky a brilliant, cloudless blue",
	"the peacock blue sky giving way to the pinks and golds of the setting sun",

	"the sky clouded, with only the barest light from a few rogue stars lighting your way",
	"the sky painted in broad stripes of red and orange as the sun struggles to be seen through the clouds",
	"the clouds obscuring the warmth of the sun, reducing it to a mere aura of light",
	"the sun gratefully dipping below the horizon, leaving the clouds to the night",

	"whispers of snowflakes falling from the night sky",
	"the sun rising from its sleep to reveal the lacy beauty of falling snow",
	"the silence surrounding you broken only by the soft fall of snow from the clouds above",
	"the sun setting, pulling a blanket of pink over the snow falling silently around you",

	"the blackness of the sky illuminated by the swiftly falling snow",
	"the gold of the sun obscured by the rapidly falling snow",
	"the world transformed into a startling blue white, the only sound the constant fall of snow",
	"the heavy snow falling in shades of pink and orange as the sun lowers its golden curtain"
};


char * const dyn_season [] =
{
	"The cool, crisp smell of winter gives the air a chilling bite",
	"The scent of tender plants opening to the spring fills the air",
	"The heady scent of larkspur and hyacinth perfume the air",
	"The secret, nostalgic scent of fallen leaves surrounds you"
};

char * const dyn_ocean_season [] =
{
	"Lonely icebergs pierce through the depths, keeping watch on the distant horizon",
	"A pair of dolphins leap from the water before diving back under to safety",
	"Seals twist and wriggle before diving beneath the surface for more fish",
	"Whales sing their mournful songs as they spray tall geysers of white water into the air"
};

/*
	SKY 0...3 - TEMP 0...1
*/

char * const dyn_road_sky_temp [] =
{
	"Well worn wagon tracks are dug into the cracked ground.",
	"Well worn wagon tracks are dug into the damp ground.",
	"Well worn wagon tracks are dug into the muddy ground.",
	"Well worn wagon tracks are dug into the mud.",

	"Well worn wagon tracks are frozen into the ground.",
	"Wagon tracks are ground into the snowy slush.",
	"Frozen wagon tracks are almost obscured by the new fallen snow.",
	"A thick blanket of unblemished snow covers the ground."
};

/*
	SUN 0...3 - SKY 0...4 - TEMP 0...1
*/

char * const dyn_ocean_sun_sky_temp []	=
{
	"a silver ocean, stars reflected in its black depths",
	"an indigo ocean, the sunrise dancing over the waves",
	"a cerulean blue ocean, its surface sparkling with a thousand diamonds",
	"a crimson ocean, the sun setting beneath the horizon",

	"a black ocean, its inky depths untouched by light",
	"a dark grey ocean, the soft pink of sunrise barely showing behind thick clouds",
	"a steel grey ocean, the clouds lowering over its onyx flecked surface",
	"a darkening ocean, the orange haze of sunset coloring the clouds bove",

	"a coal black ocean, dark blue rain piercing its inky surface",
	"a leaden ocean, the sunrise coloring the silvery rain a soft pink",
	"a windswept ocean of teal green, dark silver rain falling from heavy clouds above",
	"a burnt orange ocean, the rain transmuting from silver to dark gold, dancing on its surface",

	"a churning black ocean, heavy gusts of wind blowing the rain in frenzied circles",
	"a blood red ocean, the sunrise revealing angry clouds unleashing their fury over the dark surface",
	"a dark grey ocean, mirroring darker grey clouds above as they mercilessly release their pent up fury",
	"a jade colored ocean, the sunset hiding the storm's malevolence",


	"a silver ocean, stars reflected in its black depths",
	"an indigo ocean, sunrise dancing over the waves",
	"a cerulean blue ocean, its surface sparkling with a thousand diamonds",
	"a crimson ocean, the sun setting beneath the horizon",

	"a black ocean, its inky depths untouched by light",
	"a dark grey ocean, the soft pink of sunrise barely showing behind thick clouds",
	"a steel grey ocean, the clouds lowering over its onyx flecked surface",
	"a darkening ocean, the orange haze of sunset coloring the clouds bove",

	"an ebony ocean, argent flakes of snow whispering down from the night skies",
	"a slate ocean, the obscured sunrise turning the downy flakes of snow into alabaster",
	"a pale grey ocean, pearl white snowflakes casting themselves down upon its surface",
	"a deep purple ocean, flakes of snow falling from a scarlet sky",

	"an ocean the color of jet, heavy, ice-blue snow falling from the skies",
	"a jet black ocean, ice blue snow heavily falling from the skies",
	"a shadowy grey ocean, heavy snows flying down from leaden skies",
	"a deep amethyst ocean, snow falling in a blinding tempest",
};

char * const dyn_ocean_wind_1 [] =
{
	"A barely noticable",
	"A soft",
	"A light",
	"A slight",
	"A gentle",
	"A steady",
	"A strong",
	"A sudden",
	"An angry",
	"A raging",
};

char * const dyn_ocean_wind_2 [] =
{
	"northerly",	"northeasterly",	"easterly",	"southeasterly",
	"southerly",	"southwesterly",	"westerly",	"northwesterly"
};

char * const dyn_ocean_wind_3 [] =
{
	"wind leaves the glass-like surface of the water untouched",
	"wind blows across the waters, leaving gentle ripples in its wake",
	"breeze causes the waters to flutter gently",
	"zephyr blows across the ocean, causing the water to foam",
	"flow of air gusts across the ocean creating small whitecaps",
	"wind kisses the surrounding ocean causing waves to rise",
	"wind blows across the ocean as the waves roll",
	"squall blows in as the waters churn in response",
	"gale flares up as waves violently surge upward",
	"typhoon scours the ocean as unforgiving titanic waves savagely reign"
};

int get_sky( ROOM_INDEX_DATA *room )
{
	return (room->area->weather_info->sky);
}

int get_sun( ROOM_INDEX_DATA *room )
{
	return (mud->sunlight);
}

int get_wind( ROOM_INDEX_DATA *room )
{
	return (room->area->weather_info->wind_speed < 6 ? 0 : 1);
}

int get_wind_speed( ROOM_INDEX_DATA *room )
{
	return URANGE(0, room->area->weather_info->wind_speed, 9);
}

int get_wind_dir (ROOM_INDEX_DATA *room )
{
	return room->area->weather_info->wind_dir;
}

int get_frost( ROOM_INDEX_DATA *room )
{
	return (room->area->weather_info->temperature > 0 ? 0 : 1);
}

int get_season()
{
	return (mud->time_info->month / 4);
}

int count_exits( ROOM_INDEX_DATA *room )
{
	int door, cnt;

	for (door = cnt = 0 ; door < 6 ; door++)
	{
		if (get_exit(room->vnum, door))
		{
			cnt++;
		}
	}
	return URANGE(0, cnt - 1, 3);
}

char *get_dyn_road_exits( ROOM_INDEX_DATA *room )
{
	return dyn_road_exits[count_exits(room)];
}

char *get_dyn_road_sky_wind_temp( ROOM_INDEX_DATA *room )
{
	return dyn_road_sky_wind_temp[get_sky(room) + get_wind(room) * 4 + get_frost(room) * 8];
}

char *get_dyn_sun_sky_temp( ROOM_INDEX_DATA *room )
{
	return dyn_sun_sky_temp[get_sun(room) + get_sky(room) * 4 + get_frost(room) * 16];
}

char *get_dyn_ocean_sun_sky_temp( ROOM_INDEX_DATA *room )
{
	return dyn_ocean_sun_sky_temp[get_sun(room) + get_sky(room) * 4 + get_frost(room) * 16];
}

char *get_dyn_ocean_wind_1( ROOM_INDEX_DATA *room )
{
	return dyn_ocean_wind_1[get_wind_speed(room)];
}

char *get_dyn_ocean_wind_2( ROOM_INDEX_DATA *room )
{
	return dyn_ocean_wind_2[get_wind_dir(room)];
}

char *get_dyn_ocean_wind_3( ROOM_INDEX_DATA *room )
{
	return dyn_ocean_wind_3[get_wind_speed(room)];
}

char *get_dyn_season( )
{
	return dyn_season[get_season()];
}

char *get_dyn_ocean_season( )
{
	return dyn_ocean_season[get_season()];
}

char *get_dyn_road_sky_temp( ROOM_INDEX_DATA *room )
{
	return dyn_road_sky_temp[get_sky(room) + get_frost(room) * 4];
}

char *get_dynamic_description( CHAR_DATA *ch )
{
	static char dyn_buf[MAX_INPUT_LENGTH];

	push_call("get_dynamic_description(%p)",ch);

	dyn_char = ch;
	dyn_room = ch->in_room;


	switch (ch->in_room->sector_type)
	{
		case SECT_ROAD:
			sprintf(dyn_buf, "You are %s %s, %s, %s.  %s %s.%s%s",
				get_dyn_position(ch),
				get_dyn_road_exits(ch->in_room),
				get_dyn_road_sky_wind_temp(ch->in_room),
				get_dyn_sun_sky_temp(ch->in_room),
				get_dyn_road_sky_temp(ch->in_room),
				get_dyn_season(),
				ch->in_room->description[0] != '\0' ? "  " : "\n\r",
				ch->in_room->description);
			pop_call();
			return dyn_buf;

		case SECT_OCEAN:
			sprintf(dyn_buf, "You are %s %s.  %s %s %s.  %s.%s%s",
				get_dyn_position(ch),
				get_dyn_ocean_sun_sky_temp(ch->in_room),
				get_dyn_ocean_wind_1(ch->in_room),
				get_dyn_ocean_wind_2(ch->in_room),
				get_dyn_ocean_wind_3(ch->in_room),
				get_dyn_ocean_season(),
				ch->in_room->description[0] != '\0' ? "  " : "\n\r",
				ch->in_room->description);
			pop_call();
			return dyn_buf;
		default:
			pop_call();
			return ch->in_room->description;
	}
	pop_call();
	return "Incorrect sector type for dynamic descriptions.";
}

/*
	Character description
*/

char * char_looks_male [] =
{
	"hideously ugly",
	"rather ugly",
	"unattractive",
	"somewhat plain looking",
	"good looking",
	"very good looking",
	"very handsome",
	"extremely handsome",
	""
};

char *char_looks_female [] =
{
	"hideously ugly",
	"rather ugly",
	"unattractive",
	"somewhat plain looking",
	"quite cute",
	"very cute",
	"very pretty",
	"extremely pretty",
	""
};

char * char_skin_color [] =
{
	"pale white",
	"creamy white",
	"lightly tanned",
	"darkly tanned",

	"light brown",
	"dark brown",
	"light ebony",
	"dark ebony",

	"cerulean blue",
	"indigo blue",
	"prussian blue",

	"stone grey",
	"slate grey",
	"coal grey",

	"hazel green",
	"olive green",
	"moss green",

	"copper brown",
	"rust brown",
	"umber brown",
	""
};

char * char_eye_color [] =
{
	"light green",
	"dark green",
	"pale blue",
	"sky blue",
	"light brown",
	"dark brown",
	"light grey",
	"dark grey",
	"coal black",
	"pitch black",

	"silver",
	"golden",
	"blood red",
	"amber",
	"turquoise",
	"steel grey",
	"violet",
	""
};

char * char_hair_color [] =
{
	"silvery white",
	"platinum blond",
	"golden blond",
	"dark blond",
	"light auburn",
	"dark auburn",
	"light brown",
	"dark brown",
	"light grey",
	"dark grey",
	"raven black",
	""
};

char * char_hair_type [] =
{
	"brittle",
	"soft",
	"shiny",
	"oily",
	"greasy",
	"straight",
	"wavy",
	"curly",
	"coarse",
	"tangled",
	""
};


char * char_hair_length [] =
{
	"a completely bald head",
	"a shaven head",
	"a shaven head covered with [hair_color] stubble",
	"close-cropped [hair_color] hair",
	"short [hair_type] [hair_color] hair",
	"[hair_type] [hair_color] hair which reaches [his] shoulders",
	"[hair_type] [hair_color] hair which reaches down to the middle of [his] back",
	"[hair_type] [hair_color] hair which reaches down to [his] waist",
	""
};

void get_char_him(CHAR_DATA *ch, CHAR_DATA *victim, char *result)
{
	switch (ch->sex)
	{
		case SEX_MALE:
			strcpy(result, "him");
			break;
		case SEX_FEMALE:
			strcpy(result, "her");
			break;
		default:
			strcpy(result, "it");
			break;
	}
	if (ch == victim)
	{
		strcpy(result, "yourself");
	}
}

void get_char_his(CHAR_DATA *ch, CHAR_DATA *victim, char *result)
{
	switch (ch->sex)
	{
		case SEX_MALE:
			strcpy(result, "his");
			break;
		case SEX_FEMALE:
			strcpy(result, "her");
			break;
		default:
			strcpy(result, "its");
			break;
	}
	if (ch == victim)
	{
		strcpy(result, "your");
	}
}

void get_char_he(CHAR_DATA *ch, CHAR_DATA *victim, char *result)
{
	switch (ch->sex)
	{
		case SEX_MALE:
			strcpy(result, "he");
			break;
		case SEX_FEMALE:
			strcpy(result, "she");
			break;
		default:
			strcpy(result, "it");
			break;
	}
	if (ch == victim)
	{
		strcpy(result, "you");
	}
}

void get_char_looks(CHAR_DATA *ch, CHAR_DATA *victim, char *result)
{
	if (ch->sex == SEX_FEMALE)
	{
		strcpy(result, char_looks_female[ch->pcdata->appearance[APPEAR_LOOK]]);
	}
	else
	{
		strcpy(result, char_looks_male[ch->pcdata->appearance[APPEAR_LOOK]]);
	}
}

void get_char_race(CHAR_DATA *ch, CHAR_DATA *victim, char *result)
{
	strcpy(result, lower_all(race_table[ch->race].race_name));
}

void get_char_skin_color(CHAR_DATA *ch, CHAR_DATA *victim, char *result)
{
	strcpy(result, char_skin_color[ch->pcdata->appearance[APPEAR_SKINCOLOR]]);
}

void get_char_eye_color(CHAR_DATA *ch, CHAR_DATA *victim, char *result)
{
	strcpy(result, char_eye_color[ch->pcdata->appearance[APPEAR_EYECOLOR]]);
}

void get_char_hair(CHAR_DATA *ch, CHAR_DATA *victim, char *result)
{
	strcpy(result, char_hair_length[ch->pcdata->appearance[APPEAR_HAIRLENGTH]]);
}

void get_char_hair_color(CHAR_DATA *ch, CHAR_DATA *victim, char *result)
{
	strcpy(result, char_hair_color[ch->pcdata->appearance[APPEAR_HAIRCOLOR]]);
}

void get_char_hair_type(CHAR_DATA *ch, CHAR_DATA *victim, char *result)
{
	strcpy(result, char_hair_type[ch->pcdata->appearance[APPEAR_HAIRTYPE]]);
}

char *get_char_description(CHAR_DATA *ch)
{
	static char desc[MAX_INPUT_LENGTH];

	sprintf(desc, "[he] is [looks], with [skin_color] skin, [eye_color] eyes and [hair].");

	return get_dynamic_player_description(ch, NULL, desc);
}


/*
	You see [looks] [race], with [eye_color] eyes and [hair].
*/

void display_appearance_selections(CHAR_DATA *ch, int appearance)
{
	char buf[MAX_INPUT_LENGTH];
	int cnt;
	char **table;

	push_call("display_appearance_selections(%p,%p)",ch,appearance);
	
	if (ch->desc->connected < CON_PLAYING)
	{
		display_empty_screen(ch->desc);
	}

	if (appearance == -1)
	{
		sprintf(buf, "\n\r\n\r{038}Your appearance is as following:");
		cat_sprintf(buf, "\n\r\n\r{168}%s", capitalize(get_char_description(ch)));
		cat_sprintf(buf, "\n\r\n\r{038}Do you wish to keep this appearance?  ({138}Yes{038}, {138}No{038}): {118}");

		ch_printf_color(ch, "%s", justify(get_dynamic_player_description(ch, NULL, buf), 80));

		pop_call();
		return;
	}

	switch (appearance)
	{
		case APPEAR_LOOK:
			table = ch->sex == SEX_FEMALE ? char_looks_female : char_looks_male;
			strcpy(buf, "\n\r\n\r{038}You may choose from the following looks:\n\r\n\r");
			break;
		case APPEAR_SKINCOLOR:
			table = char_skin_color;
			strcpy(buf, "\n\r\n\r{038}You may choose from the following skin colors:\n\r\n\r");
			break;
		case APPEAR_EYECOLOR:
			table = char_eye_color;
			strcpy(buf, "\n\r\n\r{038}You may choose from the following eye colors:\n\r\n\r");
			break;
		case APPEAR_HAIRCOLOR:
			table = char_hair_color;
			strcpy(buf, "\n\r\n\r{038}You may choose from the following hair colors:\n\r\n\r");
			break;
		case APPEAR_HAIRTYPE:
			table = char_hair_type;
			strcpy(buf, "\n\r\n\r{038}You may choose from the following hair types:\n\r\n\r");
			break;
		case APPEAR_HAIRLENGTH:
			table = char_hair_length;
			strcpy(buf, "\n\r\n\r{038}You may choose from the following hair lengths:\n\r\n\r");
			break;
	}

	for (cnt = 0 ; *table[cnt] ; cnt++)
	{
		if (cnt >= race_table[ch->race].min_appearance[appearance] && cnt <= race_table[ch->race].max_appearance[appearance])
		{
			cat_sprintf(buf, "{168} %2d   %s\n\r", cnt, get_dynamic_player_description(ch, NULL, table[cnt]));
		}
	}

	switch (appearance)
	{
		case APPEAR_LOOK:
			strcat(buf, "\n\r{038}Please select your look:{118} ");
			break;
		case APPEAR_SKINCOLOR:
			strcat(buf, "\n\r{038}Please select your skin color:{118} ");
			break;
		case APPEAR_EYECOLOR:
			strcat(buf, "\n\r{038}Please select your eye color:{118} ");
			break;
		case APPEAR_HAIRCOLOR:
			strcat(buf, "\n\r{038}Please select your hair color:{118} ");
			break;
		case APPEAR_HAIRTYPE:
			strcat(buf, "\n\r{038}Please select your hair type:{118} ");
			break;
		case APPEAR_HAIRLENGTH:
			strcat(buf, "\n\r{038}Please select your hair length:{118} ");
			break;
	}

	ch_printf_color(ch, "%s", buf);

	pop_call();
	return;
}

int process_appearance_selections(CHAR_DATA *ch, int appearance, char *arg)
{
	push_call("process_appearance_selections(%p,%p,%p)",ch,appearance,arg);

	if (appearance == -1)
	{
		for (appearance = 0 ; appearance < APPEAR_MAX ; appearance++)
		{
			ch->pcdata->appearance[appearance] = number_range(race_table[ch->race].min_appearance[appearance], race_table[ch->race].max_appearance[appearance]);
		}
		pop_call();
		return TRUE;
	}

	if (!is_number(arg) || atoi(arg) < race_table[ch->race].min_appearance[appearance] || atoi(arg) > race_table[ch->race].max_appearance[appearance])
	{
		ch_printf_color(ch, "\n\r{018}That's not a valid index.\n\r\n\r{038}Please choose a number between %d and %d: {118}", race_table[ch->race].min_appearance[appearance], race_table[ch->race].max_appearance[appearance]);

		pop_call();
		return FALSE;
	}

	ch->pcdata->appearance[appearance] = atoi(arg);

	pop_call();
	return TRUE;
}

