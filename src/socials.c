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

void do_socials( CHAR_DATA *ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	int iSocial, col;

	push_call("do_socials(%p,%p)",ch,argument);

	for (col = buf[0] = iSocial = 0 ; *social_table[iSocial].name ; iSocial++)
	{
		if (col % 6 == 0)
		{
			strcat(buf, get_color_string(ch, COLOR_TEXT, VT102_DIM));
		}
		cat_snprintf(buf, 13, "%-13s", social_table[iSocial].name);
		if (++col % 6 == 0)
		{
			strcat(buf, "\n\r");
		}
	}
	if (col % 6 != 0)
	{
		strcat(buf, "\n\r");
	}
	send_to_char(buf, ch);

	pop_call();
	return;
}

bool check_social( CHAR_DATA *ch, char *command, char *argument )
{
	int cmd;

	push_call("check_social(%p,%p,%p)",ch,command,argument);

	if ((cmd = find_social(command)) == -1)
	{
		pop_call();
		return FALSE;
	}

	interp_social(ch, cmd, argument);

	pop_call();
	return TRUE;
}

void interp_social( CHAR_DATA *ch, int cmd, char *argument )
{
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	push_call("interp_social(%p,%p,%p)",ch,cmd,argument);

	switch (ch->position)
	{
		case POS_DEAD:
			send_to_char( "Lie still; you are DEAD.\n\r", ch );
			pop_call();
			return;

		case POS_INCAP:
		case POS_MORTAL:
			send_to_char( "You are hurt far too bad for that.\n\r", ch );
			pop_call();
			return;

		case POS_STUNNED:
			send_to_char( "You are too stunned to do that.\n\r", ch );
			pop_call();
			return;

		case POS_SLEEPING:
			send_to_char( "In your dreams, or what?\n\r", ch );
			pop_call();
			return;
	}

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (*arg1 == 0)
	{
		act(social_table[cmd].self_no_vict, ch, social_table[cmd].type, NULL, TO_CHAR);
		act(social_table[cmd].room_no_vict, ch, social_table[cmd].type, NULL, TO_ROOM);

		pop_call();
		return;
	}

	if (*arg2)
	{
		victim = get_char_room(ch, arg2);
	}
	else
	{
		victim = get_char_room(ch, arg1);
	}

	if (victim == NULL)
	{
		if (*arg2)
		{
			send_to_char("They aren't here.\n\r", ch);
		}
		else
		{
			act(social_table[cmd].self_no_vict, ch, arg1, NULL, TO_CHAR);
			act(social_table[cmd].room_no_vict, ch, arg1, NULL, TO_ROOM);
		}
	}
	else if (victim == ch)
	{
		act(social_table[cmd].self_to_self, ch, *arg2 ? arg1 : social_table[cmd].type, victim, TO_CHAR);
		act(social_table[cmd].self_to_room, ch, *arg2 ? arg1 : social_table[cmd].type, victim, TO_ROOM);
	}
	else
	{
		act(social_table[cmd].vict_to_self, ch, *arg2 ? arg1 : social_table[cmd].type, victim, TO_CHAR);
		act(social_table[cmd].vict_to_room, ch, *arg2 ? arg1 : social_table[cmd].type, victim, TO_NOTVICT);
		act(social_table[cmd].vict_to_vict, ch, *arg2 ? arg1 : social_table[cmd].type, victim, TO_VICT);

		mprog_social_trigger(social_table[cmd].name, victim, ch);
/*
		{
			if (!MP_VALID_MOB(ch) && MP_VALID_MOB(victim))
			{
				switch (number_bits(2))
				{
					case 0:
						check_social_fast(victim, cmd, ch->name);
						break;
					case 1:
						check_social(victim, "slap", ch->name);
						break;
					case 2:
						check_social(victim, "ponder", ch->name);
						break;
				}
			}
		}
*/
	}
	pop_call();
	return;
}

/*
	The social table.
*/

const struct social_type social_table [] =
{
	{
		"beam",
		"broadly",
		"You beam $t.",
		"$n beams $t.",
		"You beam $t at $N.",
		"$n beams $t at $N.",
		"$n beams $t at you.",
		"You beam $t, obviously pleased with yourself.",
		"$n beams $t, obviously pleased with $mself."
	},

	{
		"beckon",
		NULL,
		"You beckon for everyone to follow.",
		"$n beckons for everyone to follow.",
		"You beckon for $M to follow.",
		"$n beckons $N to follow.",
		"$n beckons for you to follow.",
		"You beckon to your shadow to follow.",
		"$n beckons to $s shadow to follow."
	},

	{
		"blink",
		"utter disbelief",
		"You blink in $t.",
		"$n blinks in $t.",
		"You blink at $M in $t.",
		"$n blinks at $N in $t.",
		"$n blinks at you in $t.",
		"You blink repeatedly in $t.",
		"$n blinks repeatedly in $t."
	},

	{
		"blush",
		"deeply",
		"You blush $t.",
		"$n blushes $t.",
		"You blush $t at $N.",
		"$n blushes $t at $N.",
		"$n blushes $t at you.",
		"You blush $t your actions.",
		"$n blushes $t at $s actions."
	},

	{
		"bonk",
		"angrily",
		"You glare around $t for someone to bonk.",
		"$n glares around $t for someone to bonk.",
		"You bonk $M on the head for being such a moron.",
		"$n bonks $N on the head for being such a moron.",
		"$n bonks you on the head for being such a moron.",
		"You $t bonk yourself on the head and grimace in pain.",
		"$n $t bonks $mself on the head and grimaces in pain."
	},

	{
		"bow",
		"deeply",
		"You bow $t, flourishing your cape.",
		"$n bows deeply, flourishing $s cape.",
		"You bow $t before $M.",
		"$n bows $t before $N.",
		"$n bows $t before you.",
		"You fold up like a jack knife and kiss your own toes.",
		"$n folds up like a jack knife and kisses $s own toes."
	},

	{
		"burp",
		"loudly",
		"You burp $t.",
		"$n burps $t.",
		"You burp loudly at $M.",
		"$n burps loudly at $N.",
		"$n burps loudly at you.",
		"You burp $t for your own entertainment.",
		"$n burps $t for $s own entertainment."
	},

	{
		"bye",
		NULL,
		"You say goodbye to everyone in the room.",
		"$n says goodbye to everyone in the room, including you.",
		"You say goodbye to all in the room, and especially to $N.",
		"$n says goodbye to everyone in the room, including you.",
		"$n says goodbye to everyone in the room, and especially to you.",
		"You say goodbye to everyone in the room.",
		"$n says goodbye to everyone in the room, including $mself."
	},

	{
		"cackle",
		"gleefully",
		"You throw back your head and cackle $t.",
		"$n throws back $s head and cackles $t.",
		"You cackle gleefully at $M",
		"$n cackles gleefully at $N.",
		"$n cackles gleefully at you.",
		"You cackle $t at yourself.",
		"$n cackles $t at $mself."
	},

	{
		"cap",
		"gallantly",
		"You tip your cap $t.",
		"$n tips $s cap $t.",
		"You tip your cap to $M $t.",
		"$n tips $s cap $t for $N.",
		"$n tips $s cap to you $t.",
		"You tip your cap over your head for a nap.",
		"$n tips $s cap over $s head for a nap."
	},

	{
		"cheek",
		"gently",
		"Kiss whose cheek?",
		NULL,
		"You lean forward and kiss $M $t on the cheek.",
		"$n leans forward and kisses $N $t on the cheek.",
		"$n leans forward and kisses you $t on the cheek.",
		"Kiss whose cheek?",
		NULL
	},

	{
		"cheer",
		"wildly",
		"You cheer $t.",
		"$n cheers $t.",
		"You cheer $t for $N.",
		"$n cheers $t for $N.",
		"$n cheers $t for you.",
		"You cheer $t for yourself since no one else does.",
		"$n cheers $t for $mself since no one else does."
	},

	{
		"choke",
		"violently",
		"Choke whom?",
		NULL,
		"You grab $M by the neck and shake $t.",
		"$n grabs $N by the neck and shakes $t.",
		"$n grabs you by the neck and starts shaking $t.",
		"You grab your own neck and start shaking $t.",
		"$n grabs $s own neck and starts shaking violently."
	},

	{
		"chuckle",
		"politely",
		"You chuckle $t.",
		"$n chuckles $t.",
		"You chuckle $t at $S actions.",
		"$n chuckles $t at $N's actions.",
		"$n chuckles $t at your actions.",
		"You chuckle $t at your own joke, since no one else did.",
		"$n chuckles $t at $s own joke, since no one else did."
	},

	{
		"clap",
		"loudly",
		"You show your approval by clapping your hands together $t.",
		"$n shows $s approval by clapping $s hands together $t.",
		"You clap $t at $S performance.",
		"$n claps $t at $N's performance.",
		"$n claps $t at your performance.",
		"You clap $t at your own performance.",
		"$n claps $t at $s own performance."
	},

	{
		"comfort",
		NULL,
		"You look around for someone to comfort you.",
		"$n needs someone to comfort $m.",
		"You comfort $M.",
		"$n comforts $N.",
		"$n comforts you.",
		"You make an attempt to comfort yourself.",
		"$n has no one to comfort $m but $mself."
	},

	{
		"cough",
		"loudly",
		"You cough $t.",
		"$n coughs $t.",
		"You cough $t at $M.",
		"$n coughs $t at $N.",
		"$n coughs $t at you.",
		"You cough $t.",
		"$n coughs $t."
	},

	{
		"cringe",
		"terror",
		"You cringe in $t.",
		"$n cringes in $t.",
		"You cringe away from $M in $t.",
		"$n cringes away from $N in $t.",
		"$n cringes away from you in $t.",
		"You cringe in $t.",
		"$n cringes in $t."
	},

	{
		"cry",
		"sadly",
		"You cry $t.",
		"$n cries $t.",
		"You cry on $S shoulder.",
		"$n cries on $N's shoulder.",
		"$n cries on your shoulder.",
		"You cry to yourself.",
		"$n sobs quietly to $mself."
	},

	{
		"cuddle",
		"warmly",
		"Cuddle whom?",
		NULL,
		"You cuddle $M $t.",
		"$n cuddles $N $t.",
		"$n cuddles you $t.",
		"Cuddle whom?",
		NULL
	},

	{
		"curse",
		"loudly",
		"You swear $t.",
		"$n swears $t",
		"You swear $t at $M.",
		"$n swears $t at $N.",
		"$n swears $t at you.",
		"You swear $t at yourself.",
		"$n swears $t at $mself."
	},

	{
		"curtsey",
		"gracefully",
		"You curtsey $t.",
		"$n curtseys $t.",
		"You curtsey $t to $M.",
		"$n curtseys $t to $N.",
		"$n curtseys $t for you.",
		"You curtsey $t.",
		"$n curtseys $t."
	},

	{
		"embrace",
		"warm and loving",
		"Embrace whom?",
		NULL,
		"You hold $N in a $t embrace.",
		"$n holds $N in a $t embrace.",
		"$n holds you in a $t embrace.",
		"Embrace whom?",
		NULL
	},

	{
		"eyebrow",
		"curiously",
		"You raise an eyebrow $t.",
		"$n raises an eyebrow $t.",
		"You raise your eyebrow $t at $M.",
		"$n raises an eyebrow $t at $N.",
		"$n raises an eyebrow $t at you.",
		"You make weird faces, practising your eyebrow raise skills.",
		"$n makes weird faces while practising $s eyebrow raising."
	},

	{
		"frown",
		"deeply",
		"You frown $t.",
		"$n frowns $t.",
		"You frown $t at $M.",
		"$n frowns $t at $N.",
		"$n frowns $t at you.",
		"You frown $t while lost in thought.",
		"$n frowns $t while lost in thought."
	},

	{
		"gasp",
		"astonishment",
		"You gasp in $t.",
		"$n gasps in $t.",
		"You gasp in $t at $N.",
		"$n gasps in $t at $N.",
		"$n gasps in $t at you.",
		"You look up and gasp in $t.",
		"$n looks up and gasps in $t."
	},

	{
		"giggle",
		"coyly",
		"You giggle $t.",
		"$n giggles $t.",
		"You giggle $t at $N.",
		"$n giggles $t at $N.",
		"$n giggles $t at you.",
		"You giggle $t at $mself.",
		"$n giggles $t at $mself."
	},

	{
		"glare",
		"icily",
		"You glare around $t.",
		"$n glares around $t.",
		"You glare $t at $N.",
		"$n glares $t at $N.",
		"$n glares $t at you.",
		"You glare $t.",
		"$n glares $t."
	},

	{
		"grin",
		"evilly",
		"You grin $t.",
		"$n grins $t.",
		"You grin $t at $M.",
		"$n grins $t at $N.",
		"$n grins $t at you.",
		"You grin $t at your own thoughts.",
		"$n grins $t at $s own thoughts."
	},

	{
		"groan",
		"quietly",
		"You groan $t.",
		"$n groans $t.",
		"You groan $t at $M.",
		"$n groans $t at $N.",
		"$n groans $t at you.",
		"You groan $t at yourself.",
		"$n groans $t at $mself."
	},

	{
		"grovel",
		"",
		"You fall down on your knees and grovel.",
		"$n falls down on $s knees and grovels.",
		"You fall down and grovel before $m on your knees.",
		"$n falls down and grovels before $N on $s knees.",
		"$n falls down and grovels before you on $s knees.",
		"You fall down on your knees and grovel.",
		"$n falls down on $s knees and grovels."
	},

	{
		"growl",
		"angrily",
		"You glare around and growl $t.",
		"$n glares around and growls $t.",
		"You glare $t at $N.",
		"$n growls $t at $N.",
		"$n growls $t at you.",
		"You growl $t.",
		"$n growls $t."
	},

	{
		"grumble",
		"disgruntledly",
		"You grumble $t.",
		"$n grumbles $t.",
		"You grumble $t to $M.",
		"$n grumbles $t to $N.",
		"$n grumbles $t to you.",
		"You grumble $t under your breath.",
		"$n grumbles $t under $s breath."
	},

	{
		"hug",
		"tightly",
		"Hug whom?",
		NULL,
		"You hug $M $t.",
		"$n hugs $N $t.",
		"$n hugs you $t.",
		"You hug yourself $t.",
		"$n hugs $mself $t."
	},

	{
		"kiss",
		"softly",
		"Kiss whom?",
		NULL,
		"You kiss $M $t.",
		"$n kisses $N $t.",
		"$n kisses you $t.",
		"Kiss whom?",
		NULL
	},

	{
		"laugh",
		"cheerfully",
		"You laugh $t.",
		"$n laughs $t.",
		"You laugh $t at $N.",
		"$n laughs $t at $N.",
		"$n laughs $t at you.",
		"You laugh $t at your own thoughts.",
		"$n laughs $t at $s own thoughts."
	},

	{
		"moan",
		"loudly",
		"You moan $t.",
		"$n moans $t.",
		"You moan $t at $N.",
		"$n moans $t at $N.",
		"$n moans $t at you.",
		"You moan $t at yourself.",
		"$n moans $t at $mself."
	},

	{
		"nod",
		"solemnly",
		"You nod $t.",
		"$n nods $t.",
		"You nod $t at $N.",
		"$n nods $t at $N.",
		"$n nods $t at you.",
		"You nod $t to yourself.",
		"$n nods $t to $mself."
	},

	{
		"nudge",
		"gently",
		"Nudge whom?",
		NULL,
		"You nudge $M $t.",
		"$n nudges $N $t.",
		"$n nudges you $t.",
		"You nudge yourself $t, trying to stay awake.",
		"$n nudges $mself $t, trying to stay awake."
	},

	{
		"pat",
		"smugly",
		"Pat whom?",
		NULL,
		"You $t pat $N on $S head.",
		"$n $t pats $N on $S head.",
		"$n $t pats you on your head.",
		"You $t pat yourself on your back for a job well done.",
		"$n $t pats $mself on the back for a job well done."
	},

	{
		"peer",
		"quizzically",
		"You peer abount $t.",
		"$n peers about $t.",
		"You peer at $M $t.",
		"$n peers at $N $t.",
		"$n peers at you $t.",
		"You peer about $t.",
		"$n peers about $t."
	},

	{
		"point",
		"accusingly",
		"Point at whom?",
		NULL,
		"You point $t at $N.",
		"$n points $t at $N.",
		"$n points $t at you.",
		"You point $t at yourself.",
		"$n points $t at $mself."
	},

	{
		"ponder",
		"deeply",
		"You sit down and think $t.",
		"$n sits down and thinks $t.",
		"You sit down before $N and think $t.",
		"$n sits down before $N and thinks $t.",
		"$n sits down before you and thinks $t.",
		"You sit down and think $t.",
		"$n sits down and thinks $t."
	},

	{
		"pout",
		"sadly",
		"You pout $t.",
		"$n pouts $t.",
		"You pout $t at $M.",
		"$n pouts $t at $N.",
		"$n pouts $t at you.",
		"You pout $t to yourself.",
		"$n pouts $t to $mself."
	},

	{
		"pray",
		"fervently",
		"You fall down praying $t to the powers that be.",
		"$n falls down praying $t to the powers that be.",
		"You fall down on your knees praying $t before $N.",
		"$n falls down on $s knees praying $t before $N.",
		"$n falls down on $s knees praying $t before you.",
		"You fall down praying $t to the powers that be.",
		"$n falls down praying $t to the powers that be."
	},

	{
		"rofl",
		"hysterically",
		"You roll around on the floor laughing $t.",
		"$n rolls around on the floor laughing $t.",
		"You roll around on the floor laughing $t at $M.",
		"$n rolls around on the floor laughing $t at $N.",
		"$n rolls around on the floor laughing $t at you.",
		"You roll around on the floor laughing $t.",
		"$n rolls around on the floor laughing $t."
	},

	{
		"roll",
		"disgust",
		"You roll your eyes in $t.",
		"$n rolls $s eyes in $t.",
		"You roll your eyes at $M in $t.",
		"$n rolls $s eyes at $N in $t.",
		"$n rolls $s eyes at you in $t.",
		"You roll your eyes in $t.",
		"$n rolls $s eyes in $t."
	},

	{
		"ruffle",
		"playfully",
		"Ruffly whom?",
		NULL,
		"You ruffle $S hair playfully.",
		"$n ruffles $N's hair playfully.",
		"$n ruffles your hair playfully.",
		"You $t ruffle your hair.",
		"$n $t ruffles $s hair."
	},

	{
		"scream",
		"angrily",
		"You scream $t.",
		"$n screams $t.",
		"You scream $t at $M.",
		"$n screams $t at $N.",
		"$n screams $t at you.",
		"You scream $t at yourself.",
		"$n screams $t at $mself."
	},

	{
		"shake",
		"firmly",
		"You shake your head $t.",
		"$n shakes $s head $t.",
		"You shake $S hand $t.",
		"$n shakes $N's hand $t.",
		"$n shakes your hand $t.",
		"You shake your head $t.",
		"$n shakes $s head $t."
	},

	{
    		"shawl",
    		"diabolically",
	    	"You slam your head against the wall, laughing $t.",
    		"$n slams $s head against the wall, laughing $t.",
	    	"You slam your head against the wall, laughing $t at $N.",
    		"$n slams $s head against the wall, laughing $t at $N.",
    		"$n slams $s head against the wall, laughing $t at you.",
	    	"You slam your head against the wall, laughing $t at your own joke.",
    		"$n slams $s head against the wall, laughing $t at $s own joke."
	},

	{
		"shrug",
		"helplessly.",
		"You shrug $t.",
		"$n shrugs $t.",
		"You shrug $t at $M.",
		"$n shrugs $t at $N.",
		"$n shrugs $t at you.",
		"You shrug $t to yourself.",
		"$n shrugs $t to $mself."
	},

	{
		"sigh",
		"softly",
		"You sigh $t.",
		"$n sighs $t.",
		"You sigh $t at $M.",
		"$n sighs $t at $N.",
		"$n sighs $t at you.",
		"You sigh $t at yourself.",
		"$n sighs at $mself."
	},

	{
		"slap",
		"angrily",
		"Slap whom?",
		NULL,
		"You $t slap $M on the face.",
		"$n $t slaps $N on the face.",
		"$n $t slaps you on your face.",
		"You slap yourself repeatedly.",
		"$n slaps $mself repeatedly."
	},

	{
		"smile",
		"happily",
		"You smile $t.",
		"$n smiles $t.",
		"You smile $t at $M.",
		"$n smiles $t at $N.",
		"$n smiles $t at you.",
		"You smile $t at yourself.",
		"$n smiles $t at $mself."
	},

	{
		"smirk",
		"broadly",
		"You smirk $t.",
		"$n smirks $t.",
		"You smirk $t at $M.",
		"$n smirks $t at $N.",
		"$n smirks $t at you.",
		"You smirk $t at yourself.",
		"$n smirks $t at $mself."
	},

	{
		"snap",
		"impatiently",
		"You snap your fingers $t.",
		"$n snaps $s fingers $t.",
		"You snap you fingers at $M $t.",
		"$n snaps $s fingers at $N $t.",
		"$n snaps $s fingers at you $t.",
		"You snap your fingers $t.",
		"$n snaps $s fingers $t."
	},

	{
		"snicker",
		"softly",
		"You snicker $t.",
		"$n snickers $t.",
		"You snicker $t at $M.",
		"$n snickers $t at $N.",
		"$n snickers $t at you.",
		"You snicker $t at your own thoughts.",
		"$n snickers $t at $s own thoughts."
	},

	{
		"sniff",
		"sadly",
		"You sniff $t.",
		"$n sniffs $t.",
		"You sniff $t at $m.",
		"$n sniffs $t at $N.",
		"$n sniffs $t at you.",
		"You sniff $t.",
		"$n sniffs $t."
	},

	{
		"stare",
		"dreamily",
		"Stare at whom?",
		NULL,
		"You stare $t at $N.",
		"$n stares $t at $N.",
		"$n stares $t at you.",
		"Stare at whom?",
		NULL
	},

	{
    		"sweep",
    		"long and deeply",
	    	"Sweep whom into your arms?",
    		NULL,
    		"You sweep $N in your arms and kiss $M $t.",
    		"$n sweeps $N in $s arms and kisses $M $t.",
	    	"$n sweeps you in $s arms and kisses you $t.",
	    	"Oh behave!",
	    	NULL
	},

	{
		"tap",
		"impatiently",
		"You tap your foot $t.",
		"$n taps his foot $t.",
		"You $t tap your foot at $M.",
		"$n $t taps $s foot at $N.",
		"$n $t taps $s foot at you.",
		"You tap your foot $t.",
		"$n taps his foot $t."
	},

	{
		"thank",
		"heartily",
		"Thank whom?",
		NULL,
		"You thank $M $t.",
		"$n thanks $N $t.",
		"$n thanks you $t.",
		"Thank whom?",
		NULL
	},

	{
		"twiddle",
		"patiently",
		"You twiddle your thumbs $t.",
		"$n twiddles $s thumbs $t.",
		"You $t twiddle your thumbs while waiting for $M.",
		"$n $t twiddles $s thumbs while waiting for $N.",
		"$n $t twiddles $s thumbs while waiting for you.",
		"You twiddle your thumbs $t.",
		"$n twiddles $s thumbs $t."
	},

	{
		"wave",
		"merrily",
		"You wave $t at everyone in the room.",
		"$n waves $t at everyone in the room.",
		"You wave $t at $N.",
		"$n waves $t at $N.",
		"$n waves $t at you.",
		"You wave $t.",
		"$n waves $t."
	},
		
	{
		"wink",
		"suggestively",
		"You wink $t.",
		"$n winks $t.",
		"You wink $t at $N.",
		"$n winks $t at $N.",
		"$n winks $t at you.",
		"You wink $t.",
		"You wink $t."
	},

	{
		"yawn",
		"tiredly",
		"You yawn $t.",
		"$n yawns $t.",
		"You yawn $t at $M.",
		"$n yawns $t at $N.",
		"$n yawns $t at you.",
		"You yawn $t.",
		"$n yawns $t."
	},

	{
		"",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	}
};
