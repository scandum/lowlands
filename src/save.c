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

#include <sys/stat.h>
#include "mud.h"

/*
	Array of containers read for proper re-nesting of objects.
*/

#define MAX_NEST	10
static	OBJ_DATA *	rgObjNest	[MAX_NEST];

/*
	The object version number is saved in the pfiles, if someone logs
	in with a non matching version number all the objects carried by
	that person will be purged, if you ever need to use this it would
	be smart to also add the version check to the quest bits of players,
	so the kids can get some new gear - Scandum
*/

/*
	Local functions.
*/

void			fwrite_char		args( ( CHAR_DATA *ch, FILE *fp ) );
void			fwrite_obj		args( ( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, char iNest ) );
void			fread_char		args( ( CHAR_DATA *ch, FILE *fp ) );
void			fread_obj			args( ( CHAR_DATA *ch, FILE *fp ) );
bool			is_valid_file		args( ( CHAR_DATA *ch, FILE *fp ) );
void			fwrite_poison_data( POISON_DATA *, FILE *);
POISON_DATA	*fread_poison_data( FILE *);

long long encrypt( char *str )
{
	long long h;

	push_call("encrypt(%p)",str);

	for (h = 0 ; *str != 0 ; str++)
	{
		h += (4 + (h << 7)) * *str;
	}

	pop_call();
	return h;
}

long long encrypt64( char *str )
{
	long long h;

	push_call("encrypt64(%p)",str);

	for (h = 0 ; *str != 0 ; str++)
	{
		h = (h << 7) + *str;
	}

	pop_call();
	return h;
}

void	add_pvnum( CHAR_DATA *ch )
{
	PVNUM_DATA *pvnum;

	push_call("add_pvnum(%p)",ch);

	if (ch->pcdata->pvnum && pvnum_index[ch->pcdata->pvnum] == NULL)
	{
		ALLOCMEM(pvnum, PVNUM_DATA, 1);

		pvnum->name = STRALLOC(capitalize_name(ch->name));
		pvnum->date = mud->current_time;

		pvnum_index[ch->pcdata->pvnum] = pvnum;
	}
	pop_call();
	return;
}

int find_free_pvnum()
{
	int cnt;

	push_call("find_free_pvnum()");

	for (cnt = 100 ; cnt < MAX_PVNUM / 2 ; cnt++)
	{
		if (pvnum_index[cnt] == NULL)
		{
			pop_call();
			return cnt;
		}
	}
	log_printf("find_free_pvnum: no free pvnum index found: removing deleted players");

	for (cnt = 100 ; cnt < MAX_PVNUM ; cnt++)
	{
		if (pvnum_index[cnt] == NULL)
		{
			continue;
		}
		if (IS_SET(pvnum_index[cnt]->flags, PVNUM_DELETED))
		{
			STRFREE(pvnum_index[cnt]->name);
			FREEMEM(pvnum_index[cnt]);
		}
	}

	for (cnt = 100 ; cnt < MAX_PVNUM ; cnt++)
	{
		if (pvnum_index[cnt] == NULL)
		{
			pop_call();
			return cnt;
		}
	}
	log_printf("find_free_pvnum: no free pvnum index found");

	pop_call();
	return 0;
}

/*
	Save a character and inventory.
*/

void save_char_obj(CHAR_DATA *ch, int which_type)
{
	char strsave[MAX_INPUT_LENGTH], strtemp[MAX_INPUT_LENGTH];
	char logfile[250];
	char cap_name[30];
	FILE *fp, *logfp;
	ROOM_INDEX_DATA *troom;
	bool IS_DESC;

	push_call("save_char_obj(%p,%p)",ch,which_type);

	if (IS_NPC(ch))
	{
		pop_call();
		return;
	}

	IS_DESC = is_desc_valid( ch );

	if ( IS_DESC && ch->desc->original != NULL )
	{
		send_to_char( "You can't save out of body!\n\r", ch);
		pop_call();
		return;
	}

	troom = ch->in_room;

	if (troom == NULL)
	{
		log_printf("Save_char_obj: char was not in a room");

		pop_call();
		return;
	}

	check_most(ch);

	if (IS_SET(troom->room_flags, ROOM_NO_SAVE))
	{
		char_from_room( ch );
		char_to_room( ch, ROOM_VNUM_TEMPLE );
	}

	strcpy( cap_name, capitalize_name( ch->name ) );

	if (which_type == NORMAL_SAVE)
	{
		sprintf(strsave,"%s/%c/%c/%s",     PLAYER_DIR, tolower(cap_name[0]), tolower(cap_name[1]), cap_name);
		sprintf(strtemp,"%s/%c/%c/temp.%s",PLAYER_DIR, tolower(cap_name[0]), tolower(cap_name[1]), cap_name);
	}
	else
	{
		sprintf(strsave, "%s/%c/%c/bak/%s",      PLAYER_DIR, tolower(cap_name[0]), tolower(cap_name[1]), cap_name);
		sprintf(strtemp, "%s/%c/%c/bak/temp.%s", PLAYER_DIR, tolower(cap_name[0]), tolower(cap_name[1]), cap_name);
	}

	close_reserve();
/*
	Save char to "temp.name" then rename to "name" after all done for safety
*/
	sprintf(logfile, "%s/%c/%c/%s.log", PLAYER_DIR, tolower(cap_name[0]), tolower(cap_name[1]), cap_name);

	logfp = my_fopen(logfile,"w",FALSE);

	if (logfp == NULL)
	{
		log_printf("Could _NOT_ open logfile to log the saving of %s\n\r.",cap_name);
		print_errno(errno);
	}

	if (remove( strtemp ) != 0)
	{
		if (errno != ENOENT) /* if there was no temp file there's no prob */
		{
			log_printf("save_char_obj: could not remove %s.", strtemp);
			print_errno(errno);
		}
	}

	fp = my_fopen( strtemp, "w", FALSE ) ;

	if (fp == NULL )
	{
		log_printf("Save_char_obj: could not my_fopen file.");
		perror( strsave );
		send_to_char( "Through some wierd game error, your character did not save.\n\r", ch);
		open_reserve();

		pop_call();
		return;
	}

	if (logfp) /* if the logfile is opened correctly */
	{
		fprintf(logfp, "Removed old backup and opened a new tempfile successfully.\n");
	}

	{
		fwrite_char( ch, fp );

		if (logfp)
		{
			fprintf(logfp,"Wrote character information successfully. (fwrite_char)\n");
		}

		if (ch->pcdata->corpse && get_room_index(ch->pcdata->corpse_room))
		{
			obj_to_char(ch->pcdata->corpse, ch);
		}

		if (ch->first_carrying != NULL)
		{
			if (logfp)
			{
				fprintf(logfp,"Starting to write ch->first_carrying (%p) to file.\n",ch->first_carrying);
			}
			fwrite_obj( ch, ch->first_carrying, fp, 0 );
		}
		if (ch->pcdata->corpse && get_room_index(ch->pcdata->corpse_room))
		{
			obj_to_room(ch->pcdata->corpse, ch->pcdata->corpse_room);
		}
		fprintf(fp, "#END %s\n", ch->name );
		if (logfp)
		{
			fprintf(logfp,"Save successfull\n");
		}
	}

	if (ftell(fp) < (long)100)
	{
		my_fclose(fp );
		send_to_char("Oops, file system full! tell one of the Gods!\n\r(Hypnos, Discard, Gaia, Demise or Manwe)\n\r",ch);
		log_string("File system full!!!\n\r");
	}
	else
	{
		my_fclose(fp );

		fp = my_fopen( strtemp, "r",FALSE ) ;

		if (!is_valid_file(ch, fp))
		{
			my_fclose(fp );
			log_printf("SAVE not valid for %s", ch->name );
			log_printf("Filesystem unstable, while saving %s", strtemp);
			send_to_char( "The file system has become unstable.  Please inform the Gods.\n\r", ch);
			remove( strtemp );
		}
		else
		{
			my_fclose(fp );

			if (remove(strsave) != 0)
			{
				if (errno != ENOENT)
				{
					log_printf("save_char_obj: (2nd) could not remove %s.", strsave);
					print_errno(errno);
				}
			}

			if (rename( strtemp, strsave ) != 0)
			{
				log_printf("Could not rename %s to %s.", strtemp, strsave);
				print_errno(errno);
			}
		}
	}

	if (logfp)
	{
		fprintf(logfp,"All done right.\n");
		fflush(logfp);
		my_fclose(logfp);
		remove( logfile );
	}

	open_reserve();

	ch->pcdata->last_saved = mud->current_time;

	/* restore char to proper room if in NO_SAVE room */

	if (ch->in_room != troom)
	{
		char_from_room(ch);
		char_to_room(ch, troom->vnum);
	}

	pop_call();
	return;
}

void fix_object_bits(CHAR_DATA *ch, OBJ_DATA *obj)
{
	push_call("fix_object_bits(%p,%p)",ch,obj);

	while (obj != NULL)
	{
		if (obj->first_content != NULL)
		{
			fix_object_bits(ch, obj->first_content);
		}

		if (obj->wear_loc != WEAR_NONE && obj->level > reinc_eq_level(ch))
		{
			unequip_char(ch, obj);
		}
		if (!IS_SET(obj->extra_flags, ITEM_MODIFIED))
		{
			obj = obj->next_content;
			continue;
		}
		if (IS_SET(obj->pIndexData->extra_flags, ITEM_MAGIC)
		&& !IS_SET(obj->extra_flags,ITEM_MAGIC))
		{
			SET_BIT(obj->extra_flags,ITEM_MAGIC);
		}
		if (IS_SET(obj->pIndexData->extra_flags, ITEM_ANTI_GOOD)
		&& !IS_SET(obj->extra_flags, ITEM_ANTI_GOOD))
		{
			SET_BIT(obj->extra_flags,ITEM_ANTI_GOOD);
		}
		if (IS_SET(obj->pIndexData->extra_flags,ITEM_ANTI_EVIL)
		&& !IS_SET(obj->extra_flags,ITEM_ANTI_EVIL))
		{
			SET_BIT(obj->extra_flags,ITEM_ANTI_EVIL);
		}
		if (IS_SET(obj->pIndexData->extra_flags,ITEM_ANTI_NEUTRAL)
		&& !IS_SET(obj->extra_flags,ITEM_ANTI_NEUTRAL))
		{
			SET_BIT(obj->extra_flags, ITEM_ANTI_NEUTRAL);
		}
		if (IS_SET(obj->pIndexData->extra_flags, ITEM_INVENTORY)
		&& !IS_SET(obj->extra_flags,ITEM_INVENTORY))
		{
			SET_BIT(obj->extra_flags,ITEM_INVENTORY);
		}
		obj = obj->next_content;
	}
	pop_call();
	return;
}

void fwrite_char( CHAR_DATA *ch, FILE *fp )
{
	AFFECT_DATA *paf;
	CRIME_DATA *pcr;
	int sn;
	sh_int cnt;

	push_call("fwrite_char(%p,%p)",ch,fp);

	fprintf(fp, "#PLAYER\n");

	fprintf(fp, "Version  %d\n",  PLAYER_VERSION);
	fprintf(fp, "Login    %s~\n", ch->pcdata->host);

	fprintf(fp, "S_Descr  %s~\n", ch->short_descr);
	fprintf(fp, "L_Descr  %s~\n", ch->long_descr);
	fprintf(fp, "E_Descr  %s~\n", fixer(ch->description));

	if (ch->pcdata->first_record)
	{
		for (pcr = ch->pcdata->first_record ; pcr ; pcr = pcr->next)
		{
			fprintf(fp, "Crime_Data\n");
			fprintf(fp, "%s~\n", fixer(pcr->crime_record));
			fprintf(fp, "%s~\n", pcr->arrester);
			fprintf(fp, "%d\n",  pcr->date);
			fprintf(fp, "%d\n",  pcr->level);
			fprintf(fp, "%d\n",  pcr->jailtime);
			fprintf(fp, "%d\n",  pcr->released);
			fprintf(fp, "%s~\n", pcr->releaser);
		}
	}
	fprintf(fp, "Level     %d\n", ch->level);
	fprintf(fp, "Sex       %d\n", ch->sex);
	fprintf(fp, "ActualSex %d\n", ch->pcdata->actual_sex);
	fprintf(fp, "Class     %d\n", ch->class);
	fprintf(fp, "Race      %d\n", ch->race);

	if (get_room_index(ch->pcdata->was_in_room) && IS_SET(ch->in_room->room_flags, ROOM_NO_SAVE))
	{
		fprintf(fp, "Room     %u\n",	ch->pcdata->was_in_room);
	}
	else
	{
		fprintf(fp, "Room     %u\n", ch->in_room->vnum);
	}

	if (ch->pcdata->corpse)
	{
		fprintf(fp, "C_Room   %u\n", ch->pcdata->corpse_room);
	}

	fprintf(fp, "LastRoom %d\n",   ch->pcdata->last_real_room);
	fprintf(fp, "Language %d\n",   ch->pcdata->language);
	fprintf(fp, "Speak    %d\n",   ch->pcdata->speak);
	fprintf(fp, "Played   %d\n",   ch->pcdata->played);
	fprintf(fp, "K_Played %d\n",   ch->pcdata->killer_played);
	fprintf(fp, "O_Played %d\n",   ch->pcdata->outcast_played);

	fprintf(fp, "HpMaMv   %d %d %d %d %d %d\n", ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move);
	fprintf(fp, "Actuals  %d %d %d\n", ch->pcdata->actual_max_hit, ch->pcdata->actual_max_mana, ch->pcdata->actual_max_move);
	fprintf(fp, "Gold     %d\n",   ch->gold);
	fprintf(fp, "Account  %d\n",   ch->pcdata->account);
	fprintf(fp, "Exp      %d\n",   ch->pcdata->exp);
	fprintf(fp, "Act      %lld\n", ch->act);
	fprintf(fp, "Affects1 %lld\n", ch->affected_by);
	fprintf(fp, "Affects2 %lld\n", ch->affected2_by);
	fprintf(fp, "Practice %d\n",   ch->pcdata->practice);
	fprintf(fp, "Align    %d\n",   ch->alignment);
	fprintf(fp, "Channel  %d\n",   ch->pcdata->channel);
	fprintf(fp, "Morph    %d\n",   ch->pcdata->morph_points);
	fprintf(fp, "Trivia   %d\n",   ch->pcdata->trivia_points);

	fprintf(fp, "Wimpy    %d\n",	ch->pcdata->wimpy		);

	if (ch->pcdata->clan && ch->pcdata->clan_name[0] != '\0')
	{
		fprintf(fp, "ClanName %s~\n", ch->pcdata->clan_name	);
	}
	if (ch->pcdata->clan_pledge[0] != '\0')
	{
		fprintf(fp, "ClanPled %s~\n", ch->pcdata->clan_pledge	);
	}
	if (ch->pcdata->clan_position != 0)
	{
		fprintf(fp, "ClanPosi %d\n", ch->pcdata->clan_position);
	}
	if (ch->pcdata->last_connect != mud->current_time)
	{
		fprintf(fp, "LastTime %d\n",		(int) mud->current_time	);

	}
	else
	{
		fprintf(fp, "LastTime %d\n",		ch->pcdata->last_time	);
	}

	fprintf(fp, "LastImprove %d\n",        ch->pcdata->last_improve);
	fprintf(fp, "Playtime %d\n",		(int) mud->current_time - ch->pcdata->last_connect);
	fprintf(fp, "Jailtime %d\n",		ch->pcdata->jailtime	);
	fprintf(fp, "Jaildate %d\n",		ch->pcdata->jaildate	);
	fprintf(fp, "MailAddy %s~\n",	ch->pcdata->mail_address	);
	fprintf(fp, "HtmlAddy %s~\n",	ch->pcdata->html_address	);
	fprintf(fp, "Password %lld\n",	ch->pcdata->password	);
	fprintf(fp, "Bamfin   %s~\n",	ch->pcdata->bamfin		);
	fprintf(fp, "Bamfout  %s~\n",	ch->pcdata->bamfout		);
	fprintf(fp, "Title    %s~\n",	ch->pcdata->title		);
	fprintf(fp, "Prefix   %d\n",		ch->pcdata->honorific	);
	fprintf(fp, "AttrPerm %d %d %d %d %d\n",	ch->pcdata->perm_str, ch->pcdata->perm_int, ch->pcdata->perm_wis, ch->pcdata->perm_dex, ch->pcdata->perm_con);
	fprintf(fp, "AttrMod  %d %d %d %d %d\n",	ch->pcdata->mod_str,  ch->pcdata->mod_int,  ch->pcdata->mod_wis,  ch->pcdata->mod_dex,  ch->pcdata->mod_con );
	fprintf(fp, "DrFuThAi %d %d %d %d\n",		ch->pcdata->condition[0], ch->pcdata->condition[1], ch->pcdata->condition[2], ch->pcdata->condition[3]);

	for (cnt = 0 ; cnt < MAX_CLASS ; cnt++)
	{
		fprintf(fp, "Mclass   %d %d\n",	cnt, ch->pcdata->mclass[cnt]	);
	}

	for (sn = 0 ; sn < MAX_SKILL ; sn++)
	{
		if (skill_table[sn].name != NULL && ch->pcdata->learned[sn] > 0)
		{
			if (multi(ch, sn) != -1)
			{
				fprintf(fp, "Skill    %d '%s'\n", ch->pcdata->learned[sn], skill_table[sn].name );
			}
		}
	}

	for (sn = 0 ; sn < MAX_AREA ; sn++)
	{
		if (ch->pcdata->quest[sn])
		{
			fprintf(fp, "Qstb %d %3d ", MAX_QUEST_BYTES, sn);

			for (cnt = 0 ; cnt < MAX_QUEST_BYTES ; cnt++)
			{
				fprintf(fp, "%d ", ch->pcdata->quest[sn][cnt]);
			}
			fprintf(fp, "\n");
		}
	}

	fprintf(fp, "Speed    %d\n",       ch->speed);
	fprintf(fp, "Reinc    %d\n",       ch->pcdata->reincarnation);
	fprintf(fp, "Prev_hrs %d\n",       ch->pcdata->previous_hours);
	fprintf(fp, "Tact_v   %d\n",       ch->pcdata->tactical_size_v);
	fprintf(fp, "Tact_f   %d\n",       ch->pcdata->tactical_flags);
	fprintf(fp, "Tact_c   %d\n",       ch->pcdata->tactical_size_c);
	fprintf(fp, "Tact_i   %s~\n",      ch->pcdata->tactical_index);

	if (IS_IMMORTAL(ch))
	{
		if (ch->pcdata->a_range_lo && ch->pcdata->a_range_hi)
		{
			fprintf(fp, "A_Range  %d %d\n", ch->pcdata->a_range_lo, ch->pcdata->a_range_hi);
		}
	}

	fprintf(fp, "Pvnum    %d\n",       ch->pcdata->pvnum);
	fprintf(fp, "Size_v   %d\n",       ch->pcdata->screensize_v );
	fprintf(fp, "Size_h   %d\n",       ch->pcdata->screensize_h );
	fprintf(fp, "Vt100_F  %d\n",       ch->pcdata->vt100_flags );
	fprintf(fp, "Portsize %d\n",       ch->pcdata->port_size );
	fprintf(fp, "Prompt   %s~\n",      ch->pcdata->prompt_layout);
	fprintf(fp, "Spam     %d\n",       ch->pcdata->spam );
	fprintf(fp, "Clock    %d\n",       ch->pcdata->clock);
	fprintf(fp, "Recall   %d\n",       ch->pcdata->recall);
	fprintf(fp, "Death    %d\n",       ch->pcdata->death_room);
	fprintf(fp, "Whichgod %d\n",       ch->pcdata->god);
	fprintf(fp, "Blocking %s~\n",      ch->pcdata->block_list);
	fprintf(fp, "AutoComm %s~\n",      ch->pcdata->auto_command);
	fprintf(fp, "AutoMode %d\n",       ch->pcdata->auto_flags);

	for (cnt = 0 ; cnt < 6 ; cnt++)
	{
		fprintf(fp, "History  %d %d\n",	cnt, ch->pcdata->history[cnt]);
	}

	fprintf(fp, "LastNote %d\n",		ch->pcdata->last_note );

	for (cnt = 0 ; cnt < MAX_TOPIC ; cnt++)
	{
		fprintf(fp, "Topic    %d %d\n",	cnt, ch->pcdata->topic_stamp[cnt]);
	}

	for (cnt = 0 ; cnt < COLOR_MAX ; cnt++)
	{
		fprintf(fp, "Color    %2d %2d\n",	cnt, ch->pcdata->color[cnt]);
	}

	for (cnt = 0 ; cnt < APPEAR_MAX ; cnt++)
	{
		fprintf(fp, "Appear   %2d %2d\n", cnt, ch->pcdata->appearance[cnt]);
	}

	for (cnt = 0 ; cnt < MAX_ALIAS ; cnt++)
	{
		if (ch->pcdata->alias[cnt][0]!='\0')
		{
			fprintf(fp, "Alias    %s~%s~\n", ch->pcdata->alias_c[cnt], ch->pcdata->alias[cnt]);
		}
	}

/*
	castle stuff
*/
	if (ch->pcdata->castle != NULL)
	{
		fprintf(fp, "Cstl_ent    %d\n",ch->pcdata->castle->entrance);
		fprintf(fp, "Cstl_dor    %d\n",ch->pcdata->castle->door_room);
		fprintf(fp, "Cstl_dir    %d\n",ch->pcdata->castle->door_dir);
		fprintf(fp, "Cstl_has    %d\n",ch->pcdata->castle->has_backdoor);
		fprintf(fp, "Cstl_cst    %d\n",ch->pcdata->castle->cost);
		fprintf(fp, "Cstl_nrm    %d\n",ch->pcdata->castle->num_rooms);
		fprintf(fp, "Cstl_nmo    %d\n",ch->pcdata->castle->num_mobiles);
		fprintf(fp, "Cstl_nob    %d\n",ch->pcdata->castle->num_objects);
	}

	for (paf = ch->first_affect ; paf != NULL ; paf = paf->next)
	{
		if (paf->type < 0 || paf->type >= MAX_SKILL)
		{
			log_printf("save_char_obj: bad affect type: %d", paf->type);
			continue;
		}
		fprintf(fp, "AffectData '%s' %d %d %d %d %lld\n", skill_table[paf->type].name,	paf->duration,	paf->modifier,	paf->location,	paf->bittype, paf->bitvector);
	}

	fprintf(fp, "Critical %d\n",	ch->critical_hit_by	);

	if (ch->poison != NULL)
	{
		fwrite_poison_data( ch->poison, fp );
	}

	fprintf(fp, "PK_ATTACKS %d\n", MAX_PK_ATTACKS );

	for (cnt = 0 ; cnt < MAX_PK_ATTACKS ; cnt++)
	{
		fprintf(fp, "%d %d %s~\n", ch->pcdata->last_pk_attack_time[cnt], ch->pcdata->last_pk_attack_pvnum[cnt], ch->pcdata->last_pk_attack_name[cnt]);
	}

	fprintf(fp, "End\n\n" );
	pop_call();
	return;
}

void load_clan_pit( FILE *fp )
{
	OBJ_INDEX_DATA *pit;
	char letter, loop, *word;

	push_call("fread_clanpit(void)");

	word         = fread_word(fp);
	pit          = obj_index[atol(word)];
	rgObjNest[0] = pit->first_instance;

	for (loop = TRUE ; loop ; )
	{
		letter = fread_letter(fp);

		if (letter != '#')
		{
			log_printf("fread_clan_pit: # not found. Word was '%c%s'", letter, fread_word(fp));
			break;
		}

		word = fread_word(fp);

		switch (word[0])
		{
			case 'O':
				fread_obj(NULL, fp);
				break;

			case 'E':
				loop = FALSE;
				break;

			default:
				log_printf("fread_clan_pit: bad section.");
				loop = FALSE;
				break;
		}
	}
	pop_call();
	return;
}

void save_clan_pit(CLAN_DATA *clan, FILE *fp )
{
	ROOM_INDEX_DATA *location;
	OBJ_DATA *pit;

	push_call("save_clan_pits(void)");

	if ((location = get_room_index(clan->store)) == NULL)
	{
		pop_call();
		return;
	}

	for (pit = location->first_content ; pit ; pit = pit->next_content)
	{
		if (pit->pIndexData->creator_pvnum == clan->founder_pvnum && pit->item_type == ITEM_CONTAINER)
		{
			break;
		}
	}
	if (pit == NULL)
	{
		pop_call();
		return;
	}
	fprintf(fp, "#PIT %d\n\n", pit->pIndexData->vnum);

	fwrite_obj(NULL, pit->first_content, fp, 1);

	fprintf(fp, "#END\n");

	pop_call();
	return;
}

/*
	Write an object and its first_content.

	Increased speed, no more naughty recursion, objects in unsaved
	containers go to inventory - Scandum april 2002
*/

void fwrite_obj( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, char iNest )
{
	AFFECT_DATA *paf;

	push_call("fwrite_obj(%p,%p,%p,%p)",ch,obj,fp,iNest);

	while (obj)
	{
		if ((ch != NULL && obj_eq_level(ch, obj) - 5 > ch->level)
		||  obj->item_type == ITEM_KEY
		||  IS_SET(obj->extra_flags, ITEM_NOT_VALID))
		{
			if (obj->first_content != NULL)
			{
				fwrite_obj(ch, obj->first_content, fp, iNest);
			}
			obj = obj->next_content;
			continue;
		}
		else
		{
			fprintf(fp, "#OBJECT\n" );

			if (iNest)
			{
				fprintf(fp, "Nest %d\n", iNest);
			}
			else if (obj->wear_loc != WEAR_NONE)
			{
				fprintf(fp, "WearLoc %s\n", broken_bits(obj->wear_loc, "WEAR_", TRUE));
			}

			if (!IS_SET(obj->extra_flags, ITEM_MODIFIED) && obj->first_content == NULL	&& obj->item_type != ITEM_CONTAINER)
			{
				fprintf(fp, "BasicVnum %d\n", obj->pIndexData->vnum);
			}
			else
			{
				/*
					Compact, latest mode for non-basic objects
				*/

				fprintf(fp, "Compact\n");

				fprintf(fp, "%d\n", obj->pIndexData->vnum);

				if (IS_OBJ_STAT(obj, ITEM_FORGERY))
				{
					REMOVE_BIT(obj->extra_flags,ITEM_FORGERY);
					fprintf(fp, "%d ", obj->extra_flags);
					SET_BIT(obj->extra_flags,ITEM_FORGERY);
				}
				else
				{
					fprintf(fp, "%d ", obj->extra_flags);
				}

				fprintf(fp, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
					obj->wear_flags,
					obj->item_type,
					obj->weight,
					obj->level,
					obj->timer,
					obj->cost,
					obj->owned_by,
					obj->value[0],
					obj->value[1],
					obj->value[2],
					obj->value[3],
					obj->value[4],
					obj->value[5],
					obj->value[6],
					obj->value[7]);

				if (obj->obj_quest)
				{
					int cnt;

					fprintf(fp, "%d ", MAX_QUEST_BYTES);
					for (cnt = 0 ; cnt < MAX_QUEST_BYTES ; cnt++)
					{
						fprintf(fp, "%d ", obj->obj_quest[cnt]);
					}
					fprintf(fp, "\n");
				}
				else
				{
					fprintf(fp, "0\n");
				}

				/*
					End of Compact mode, Emud only checks non basic objects - Scandum
				*/

				if (!IS_SET(obj->extra_flags, ITEM_FORGERY))
				{
					if (obj->name != obj->pIndexData->name)
					{
						fprintf(fp, "Name %s~\n", obj->name);
					}

					if (obj->short_descr != obj->pIndexData->short_descr)
					{
						fprintf(fp, "Short %s~\n", obj->short_descr);
					}

					if (obj->long_descr != obj->pIndexData->long_descr)
					{
						fprintf(fp, "Long %s~\n", obj->long_descr);
					}

					if (obj->description != obj->pIndexData->description)
					{
						fprintf(fp, "Desc %s~\n", obj->description);
					}
				}

				switch ( obj->item_type )
				{
					case ITEM_POTION:
					case ITEM_SCROLL:
					case ITEM_PILL:
						if (obj->value[1] > 0)
						{
							fprintf(fp, "Spell 1 '%s'\n", skill_table[obj->value[1]].name );
						}
						if ( obj->value[2] > 0 )
						{
							fprintf(fp, "Spell 2 '%s'\n", skill_table[obj->value[2]].name );
						}
						if ( obj->value[3] > 0 )
						{
							fprintf(fp, "Spell 3 '%s'\n", skill_table[obj->value[3]].name );
						}
						break;

					case ITEM_STAFF:
					case ITEM_WAND:
						if ( obj->value[3] > 0 )
						{
							fprintf(fp, "Spell 3 '%s'\n", skill_table[obj->value[3]].name );
						}
						break;
				}
				for (paf = obj->first_affect ; paf ; paf = paf->next )
				{
					if (paf->type < 0 || paf->type >= MAX_SKILL)
					{
						log_printf("fwrite_obj: Bad affect: %d", paf->type);
						continue;
					}
					fprintf(fp, "AffectData '%s' %d %d %d %d %lld\n", skill_table[paf->type].name, paf->duration, paf->modifier, paf->location, paf->bittype, paf->bitvector );
				}
				if (obj->poison != NULL)
				{
					fwrite_poison_data( obj->poison, fp );
				}
			}
			fprintf(fp, "RefKey %lld\nEnd\n\n", obj->obj_ref_key);
		}

		if (obj->first_content != NULL)
		{
			fwrite_obj( ch, obj->first_content, fp, iNest + 1 );
		}
		obj = obj->next_content;
	}
	pop_call();
	return;
}

/*
	Load a char and inventory into a new ch structure.
*/

bool load_char_obj(DESCRIPTOR_DATA *d, char *name)
{
	char strsave[MAX_INPUT_LENGTH], buf[200], buf1[200], buf2[200];
	OBJ_DATA	*obj;
	CHAR_DATA *ch;
	FILE *fp;
	bool found, loop;
	sh_int cnt;

	push_call("load_char_obj(%p,%p) (%s)",d,name,name);

	if (*name == 0)
	{
		pop_call();
		return FALSE;
	}

	ALLOCMEM(ch, CHAR_DATA, 1);

	clear_char(ch);

	ALLOCMEM(ch->pcdata, PC_DATA, 1);

	d->character                  = ch;

	ch->desc                      = d;

	ch->name                      = STRALLOC(capitalize(name));
	ch->short_descr               = STRALLOC( "" );
	ch->description               = STRALLOC( "" );
	ch->long_descr                = STRALLOC( "" );

	ch->pcdata->host              = STRALLOC( "" );
	ch->pcdata->mail_address      = STRALLOC( "" );
	ch->pcdata->html_address      = STRALLOC( "" );
	ch->pcdata->clan_name         = STRALLOC( "" );
	ch->pcdata->block_list        = STRALLOC( "" );
	ch->pcdata->title             = STRALLOC( "" );
	ch->pcdata->auto_command      = STRALLOC( "" );
	ch->pcdata->bamfin            = STRALLOC( "" );
	ch->pcdata->bamfout           = STRALLOC( "" );

	ch->pcdata->practice          = 21;
	ch->pcdata->travel            = -1;

	if (d->descriptor != -999)
	{
		ch->pcdata->last_time              = mud->current_time;
	}

	ch->act                            = PLR_AUTOEXIT|PLR_AUTOSAC|PLR_AUTO_SPLIT;
	ch->pcdata->channel                = CHANNEL_CHAT|CHANNEL_PLAN|CHANNEL_TALK|CHANNEL_TRIVIA|CHANNEL_MORPH;
	ch->pcdata->condition[COND_THIRST] = 48;
	ch->pcdata->condition[COND_FULL]   = 48;
	ch->pcdata->condition[COND_AIR]    = 10;

	ch->pcdata->spam                   = 1024;

	ch->pcdata->tactical_size_v        = 6;
	ch->pcdata->tactical_size_c        = 7;
	ch->pcdata->tactical_flags         = TACTICAL_TOP|TACTICAL_INDEX|TACTICAL_EXPTNL;
	sprintf(buf, "%c%c", toupper(name[0]), tolower(name[1]));
	ch->pcdata->tactical_index         = STRALLOC(buf);

	ch->pcdata->edit_buf               = STRDUPE(str_empty);
	ch->pcdata->prompt_layout          = STRALLOC( "{078}<$c$h{078}hp $c$m{078}m $c$v{078}mv $c$X{078}xp> <{178}$e{078}> ($c$l{078}) ($c$L{078}) {300}" );
	ch->pcdata->subprompt              = STRDUPE(str_empty);
	ch->pcdata->tracking               = STRDUPE(str_empty);
	ch->pcdata->ambush                 = STRDUPE(str_empty);
	ch->desc->port_size                = 8192;
	ch->pcdata->port_size              = 8192;
	ch->pcdata->death_room             = ROOM_VNUM_SCHOOL;

	ch->pcdata->clan_pledge            = STRDUPE(str_empty);
	ch->pcdata->god                    = -1;
	ch->pcdata->screensize_h           = 80;
	ch->pcdata->screensize_v           = 24;
	ch->pcdata->recall                 = ROOM_VNUM_SCHOOL;

	for (cnt = 0 ; cnt < COLOR_MAX ; cnt++)
	{
		ch->pcdata->color[cnt] = 7 + 8 * 10;
	}

	for (cnt = 0 ; cnt < MAX_PK_ATTACKS ; cnt++)
	{
		ch->pcdata->last_pk_attack_name[cnt] = STRDUPE(str_empty);
	}

	/*
		the following is allocated for file reading purposes, gets
		deallocated if the player has no castle, need to load for finger too
	*/

	ALLOCMEM(ch->pcdata->castle, CASTLE_DATA, 1);
	ch->pcdata->castle->door_room		= -1;
	ch->pcdata->castle->door_dir		= -1;
	ch->pcdata->last_real_room		= ROOM_VNUM_TEMPLE;

	for (cnt = 0 ; cnt < MAX_ALIAS ; cnt++)
	{
		ch->pcdata->alias[cnt]	= STRDUPE(str_empty);
		ch->pcdata->alias_c[cnt]	= STRDUPE(str_empty);
	}

	for (cnt = 0 ; cnt < 26 ; cnt++)
	{
		ch->pcdata->back_buf[cnt] = STRALLOC("");
	}

	found = FALSE;

	close_reserve();

	/*
		Move to a/a player dir structure for seek speed - Scandum
	*/

	sprintf(buf1, "%s/%c/%s", PLAYER_DIR, tolower(name[0]), capitalize_name(name));
	sprintf(buf2, "%s/%c/%c/%s", PLAYER_DIR, tolower(name[0]), tolower(name[1]), capitalize_name(name));

	if ((fp = my_fopen(buf1, "r", TRUE)) != NULL)
	{
		rename(buf1, buf2);

		my_fclose(fp);
	}

	sprintf(strsave, "%s/%c/%c/%s", PLAYER_DIR, tolower(name[0]), tolower(name[1]), capitalize_name(name));

	fp = my_fopen(strsave, "r" , TRUE);

	if (fp != NULL)
	{
		int iNest;

		for (iNest = 0 ; iNest < MAX_NEST ; iNest++)
		{
			rgObjNest[iNest] = NULL;
		}
		if (d->descriptor != -999  )
		{
			log_god_printf("Loading character file: %s D%d", name, d->descriptor);
		}

		if (!is_valid_file(ch, fp))
		{
			log_printf("Erasing invalid file: %s", name);

			delete_player(ch);

			pop_call();
			return(load_char_obj(d, name));
		}

		found = TRUE;
		loop  = TRUE;

		while (loop)
		{
			char letter;
			char *word;

			letter = fread_letter(fp );
			if ( letter == '*' )
			{
				fread_to_eol(fp );
				continue;
			}

			if ( letter != '#' )
			{
				log_printf("Load_char_obj: # not found.  word was '%c%s'", letter, fread_word(fp ));
				loop = FALSE;
			}

			word = fread_word(fp );

			switch (word[0])
			{
				case 'O':
					fread_obj(ch, fp);
					break;

				case 'P':
					fread_char (ch, fp);

					if (d->descriptor == -999)
					{
						loop = FALSE;
					}
					if (strcasecmp(ch->name, name))
					{
						log_printf("Incorrect name. #END on %s", name );
						loop = FALSE;
					}
					break;

				case 'E':
					loop = FALSE;
					break;

				default:
					log_printf("Load_char_obj: bad section.");
					loop = FALSE;
					break;
			}
		}

		if (ch->pcdata->corpse_room)
		{
			for (obj = ch->first_carrying ; obj ; obj = obj->next_content)
			{
				if (obj->item_type == ITEM_CORPSE_PC)
				{
					ch->pcdata->corpse = obj;
				}
			}
		}

		if (fp != NULL)
		{
			my_fclose(fp );
		}
	}

	if (ch->pcdata->castle && ch->pcdata->castle->num_rooms == 0)
	{
		FREEMEM(ch->pcdata->castle);
		ch->pcdata->castle = NULL;
	}

	open_reserve();

	/*
		Fix up a few flags -    Chaos 10/1/95
	*/

	ch->trust = ch->level;

	ch->pcdata->god = ch->pcdata->god % MAX_GOD;

	if (d->descriptor == -999)
	{
		pop_call();
		return found;
	}

	ch->pcdata->last_saved = mud->current_time;

	if (strlen(ch->pcdata->tactical_index) < 2)
	{
		sprintf(buf, "%c%c", toupper(ch->name[0]), tolower(ch->name[1]));
		STRFREE(ch->pcdata->tactical_index);
		ch->pcdata->tactical_index = STRALLOC(buf);
	}

	if (ch->level >= MAX_LEVEL)
	{
		if (!IS_GOD(ch))
		{
			SET_BIT(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_DENIED);
		}
		else
		{
			REMOVE_BIT(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_DENIED);
		}
	}

	/*
		Deny players with screwed up levels  - Chaos  3/7/99
	*/

	if (ch->level < MAX_LEVEL)
	{
		int cnt2;
		for (cnt = 0, cnt2 = 0 ; cnt < MAX_CLASS ; cnt++)
		{
			cnt2 += ch->pcdata->mclass[cnt];
		}
		if (ch->level != cnt2)
		{
			SET_BIT(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_DENIED);
		}
	}

	/*
		Check for illegal items, fix bits - Order 7/3/1995
	*/

	fix_object_bits(ch, ch->first_carrying);

	char_reset( ch );

	ch->pcdata->god = -1;
	ch->pcdata->god = which_god(ch);

	fix_language(ch);

	if (ch->level >= 90)
	{
		knight_adjust_hpmnmv( ch );
	}

	/*
		Check for old character   -  Chaos 10/30/95
	*/

	if (ch->level > 0 && character_expiration(ch) < 0)
	{
		delete_player(ch);

		log_printf("Erasing old char %s", name);
		pop_call();
		return( load_char_obj( d, name ) );
	}
	pop_call();
	return found;
}

/*
	Read in a char.
*/

void fread_char( CHAR_DATA *ch, FILE *fp )
{
	char *word;
	char *line;
	bool fMatch;
	sh_int cnt, tst;
	int x1, x2, x3, x4, x5;

	push_call("fread_char(%p,%p)",ch,fp);

	if (ch == NULL)
	{
		log_printf("Attempt to load a NULL character.");
		pop_call();
		return;
	}

	if (ch->pcdata == NULL)
	{
		log_printf("Attempting to load a player with no pcdata!\n");
		pop_call();
		return;
	}

	for ( ; ; )
	{
		word   = feof(fp ) ? "End" : fread_word(fp );

		fMatch = FALSE;

		switch (UPPER(word[0]))
		{
			case '*':
				fMatch = TRUE;
				fread_to_eol(fp );
				break;

			case 'A':
				AKEY("Appear",      ch->pcdata->appearance,   fread_number(fp));
				NKEY("Account",     ch->pcdata->account,      fread_number(fp));
				NKEY("Act",         ch->act,                  fread_number(fp));
				NKEY("ActualSex",   ch->pcdata->actual_sex,   fread_number(fp));
				NKEY("Affects1",    ch->affected_by,          fread_number(fp));
				NKEY("Affects2",    ch->affected2_by,         fread_number(fp));
				NKEY("Align",       ch->alignment,            fread_number(fp));
				NKEY("AutoMode",    ch->pcdata->auto_flags,   fread_number(fp));
				SKEY("AutoComm",    ch->pcdata->auto_command, fread_string(fp));

				if (!strcasecmp(word, "AffectData"))
				{
					AFFECT_DATA *paf;
					int sn;

					ALLOCMEM(paf, AFFECT_DATA, 1);
					sn = skill_lookup(fread_word(fp));

					if (sn < 0 || sn > MAX_SKILL)
					{
						bug("fread_char: unknown skill.");
					}
					else
					{
						paf->type = sn;
					}
					paf->duration	= fread_number(fp );
					paf->modifier	= fread_number(fp );
					paf->location	= fread_number(fp );
					paf->bittype	= fread_number(fp );
					paf->bitvector	= fread_number(fp );
					LINK(paf, ch->first_affect, ch->last_affect, next, prev );
					fMatch = TRUE;
					break;
				}

				if (!strcasecmp(word, "AffectData1"))
				{
					AFFECT_DATA *paf;
					int sn;

					ALLOCMEM(paf, AFFECT_DATA, 1);
					sn = skill_lookup(fread_word(fp));

					if (sn < 0 || sn > MAX_SKILL)
					{
						bug("fread_char: unknown skill.");
					}
					else
					{
						paf->type = sn;
					}
					paf->duration	= fread_number(fp );
					paf->modifier	= fread_number(fp );
					paf->location	= fread_number(fp );
					paf->bittype	= fread_number(fp );
					paf->bitvector	= fread_number(fp );
					LINK(paf, ch->first_affect, ch->last_affect, next, prev );
					fMatch = TRUE;
					break;
				}

				if ( !strcasecmp( word, "AttrMod"  ) )
				{
					line = fread_line(fp );
					x1=x2=x3=x4=x5=0;
					sscanf( line, "%d %d %d %d %d", &x1, &x2, &x3, &x4, &x5 );
					ch->pcdata->mod_str = x1;
					ch->pcdata->mod_int = x2;
					ch->pcdata->mod_wis = x3;
					ch->pcdata->mod_dex = x4;
					ch->pcdata->mod_con = x5;
					fMatch = TRUE;
					break;
				}

				if ( !strcasecmp( word, "AttrPerm" ) )
				{
					line = fread_line(fp );
					x1=x2=x3=x4=x5=13;
					sscanf( line, "%d %d %d %d %d",  &x1, &x2, &x3, &x4, &x5 );
					ch->pcdata->perm_str = x1;
					ch->pcdata->perm_int = x2;
					ch->pcdata->perm_wis = x3;
					ch->pcdata->perm_dex = x4;
					ch->pcdata->perm_con = x5;
					fMatch = TRUE;
					break;
				}

				if ( !strcasecmp( word, "Actuals" ) )
				{
					ch->pcdata->actual_max_hit	= fread_number(fp );
					ch->pcdata->actual_max_mana	= fread_number(fp );
					ch->pcdata->actual_max_move	= fread_number(fp );
					fMatch = TRUE;
					break;
				}

				if ( !strcmp( word, "Alias" ) )
				{
					tst=0;
					for (cnt = 0 ; cnt < MAX_ALIAS && tst == 0 ; cnt++)
					{
						if (ch->pcdata->alias[cnt][0]=='\0')
						{
							tst=1;
							STRFREE (ch->pcdata->alias[cnt] );
							STRFREE (ch->pcdata->alias_c[cnt] );
							ch->pcdata->alias_c[cnt]=fread_string(fp ) ;
							ch->pcdata->alias[cnt]=fread_string(fp ) ;
						}
					}
					if (tst==0)
					{
						char *ptx1;
						ptx1 = fread_string(fp );
						STRFREE (ptx1 );
						ptx1 = fread_string(fp );
						STRFREE (ptx1 );
					}
					fMatch = TRUE;
					break;
				}

				if (!strcmp( word, "A_Range" ) )
				{
					ch->pcdata->a_range_lo = fread_number(fp);
					ch->pcdata->a_range_hi = fread_number(fp);
					fMatch = TRUE;
					break;
				}
				break;

			case 'B':
				SKEY("Bamfin",      ch->pcdata->bamfin,           fread_string(fp));
				SKEY("Bamfout",     ch->pcdata->bamfout,          fread_string(fp));
				SKEY("Blocking",    ch->pcdata->block_list,       fread_string(fp));
				break;

			case 'C':
				AKEY("Color",       ch->pcdata->color,            fread_number(fp));
				SKEY("ClanName",    ch->pcdata->clan_name,        fread_string(fp));
				SKEY("ClanPled",    ch->pcdata->clan_pledge,      fread_string(fp));
				NKEY("Channel",     ch->pcdata->channel,          fread_number(fp));
				NKEY("ClanPosi",    ch->pcdata->clan_position,    fread_number(fp));
				NKEY("Class",       ch->class,                    fread_number(fp));
				NKEY("Critical",    ch->critical_hit_by,          fread_number(fp));
				NKEY("Clock",       ch->pcdata->clock,            fread_number(fp));
				NKEY("C_Room",      ch->pcdata->corpse_room,      fread_number(fp));

				if (ch->pcdata->castle != NULL)
				{
					NKEY("Cstl_ent",	ch->pcdata->castle->entrance,		fread_number(fp));
					NKEY("Cstl_dor",	ch->pcdata->castle->door_room,	fread_number(fp));
					NKEY("Cstl_dir",	ch->pcdata->castle->door_dir,		fread_number(fp));
					NKEY("Cstl_has",	ch->pcdata->castle->has_backdoor,	fread_number(fp));
					NKEY("Cstl_cst",	ch->pcdata->castle->cost,		fread_number(fp));
					NKEY("Cstl_nrm",	ch->pcdata->castle->num_rooms,	fread_number(fp));
					NKEY("Cstl_nmo",	ch->pcdata->castle->num_mobiles,	fread_number(fp));
					NKEY("Cstl_nob",	ch->pcdata->castle->num_objects,	fread_number(fp));
				}

				if (!strcasecmp(word, "Crime_Data"))
				{
					CRIME_DATA *pcr;

					ALLOCMEM(pcr, CRIME_DATA, 1);

					pcr->crime_record	= fread_string(fp);
					pcr->arrester		= fread_string(fp);
					pcr->date			= fread_number(fp);
					pcr->level		= fread_number(fp);
					pcr->jailtime		= fread_number(fp);
					pcr->released		= fread_number(fp);
					pcr->releaser		= fread_string(fp);

					LINK(pcr, ch->pcdata->first_record, ch->pcdata->last_record, next, prev );
					fMatch = TRUE;
					break;
				}
				break;

			case 'D':
				NKEY("Damroll",		ch->damroll,			fread_number(fp));
				NKEY("Death",			ch->pcdata->death_room,	fread_number(fp));
				SKEY("Domain",			ch->desc->host,		fread_string(fp));

				if ( !strcasecmp( word, "DrFuTh" ) )
				{
					line = fread_line(fp );
					sscanf( line, "%d %d %d", &x1, &x2, &x3 );
					ch->pcdata->condition[0] = x1;
					ch->pcdata->condition[1] = x2;
					ch->pcdata->condition[2] = x3;
					fMatch = TRUE;
					break;
				}
				if ( !strcasecmp( word, "DrFuThAi" ) )
				{
					line = fread_line(fp );
					sscanf( line, "%d %d %d %d", &x1, &x2, &x3, &x4 );
					ch->pcdata->condition[0] = x1;
					ch->pcdata->condition[1] = x2;
					ch->pcdata->condition[2] = x3;
					ch->pcdata->condition[3] = x4;
					fMatch = TRUE;
					break;
				}
				break;

			case 'E':
				SKEY("E_Descr",		ch->description,		fread_string(fp));
				NKEY("EQ_DR",			ch->pcdata->eqdamroll,	fread_number(fp));
				NKEY("EQ_HR",			ch->pcdata->eqhitroll,	fread_number(fp));
				NKEY("EQ_AC",			cnt,					fread_number(fp));
				NKEY("EQ_SP",			ch->pcdata->eqsaves,	fread_number(fp));

				if ( !strcasecmp( word, "End" ) )
				{
					/*
						ENDLOAD     Chaos  10/11/93
					*/

//					REMOVE_BIT(ch->act, PLR_TELOPTS|PLR_COMBINE|PLR_TELNET_GA|PLR_SILENCE|PLR_MUTE|PLR_LOG|PLR_DENY|PLR_FREEZE|PLR_CHAT|PLR_PLAN|PLR_BATTLE|PLR_VICTIM_LIST|PLR_FOS|PLR_ARRESTED);

					if (ch->pcdata->clan_name[0] != '\0')
					{
						if (get_clan(ch->pcdata->clan_name) != NULL)
						{
							ch->pcdata->clan = get_clan(ch->pcdata->clan_name);
						}
						else
						{
							RESTRING(ch->pcdata->clan_name, "");
							ch->pcdata->clan = NULL;
						}
					}

					if (ch->pcdata->screensize_h < 80)
					{
						ch->pcdata->screensize_h = 80;
					}

					if (ch->pcdata->screensize_v < MAX_TACTICAL_ROW)
					{
						ch->pcdata->screensize_v = MAX_TACTICAL_ROW;
					}
					
					ch->desc->port_size		= ch->pcdata->port_size;
					ch->position			= POS_STANDING;

					for (cnt = 0 ; cnt < MAX_ALIAS ; cnt++)
					{
						if (ch->pcdata->alias[cnt] == NULL)
						{
							ch->pcdata->alias[cnt] = STRDUPE(str_empty);
						}
						if (ch->pcdata->alias_c[cnt] == NULL)
						{
							ch->pcdata->alias_c[cnt] = STRDUPE(str_empty);
						}
					}

					if (ch->hit < 0)
					{
						ch->hit = 1;
					}
					ch->trust = ch->level;

					if (ch->level < LEVEL_IMMORTAL)
					{
						if (IS_SET(ch->act, PLR_WIZINVIS))
						{
							REMOVE_BIT(ch->act, PLR_WIZINVIS);
						}
						if (IS_SET(ch->act, PLR_WIZCLOAK))
						{
							REMOVE_BIT(ch->act, PLR_WIZCLOAK);
						}
						if (IS_AFFECTED(ch, AFF2_CAMPING))
						{
							REMOVE_BIT(ch->affected2_by, 0-AFF2_CAMPING);
						}
						if (IS_SET(ch->act, PLR_HOLYLIGHT))
						{
							REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
						}
						if (IS_SET(ch->act, PLR_BUILDLIGHT))
						{
							REMOVE_BIT(ch->act, PLR_BUILDLIGHT);
						}
					}

					if (ch->pcdata->castle && ch->pcdata->castle->entrance > 0 && ch->pcdata->castle->entrance < ROOM_VNUM_CASTLE)
					{
						ch->pcdata->castle->entrance	+= 60000;
						ch->pcdata->castle->door_room = -1;
						ch->pcdata->castle->door_dir	= -1;
					}

					if (ch->act < 0)
					{
						ch->act = 0;
					}

					add_pvnum(ch);

					REMOVE_BIT(pvnum_index[ch->pcdata->pvnum]->flags, PVNUM_DELETED);

					if (get_room_index(ch->pcdata->recall) == NULL)
					{
						ch->pcdata->recall = ROOM_VNUM_TEMPLE;
					}
					if (get_room_index(ch->pcdata->death_room) == NULL)
					{
						ch->pcdata->death_room = ROOM_VNUM_TEMPLE;
					}
					if (get_room_index(ch->pcdata->was_in_room) == NULL)
					{
						ch->pcdata->was_in_room = ROOM_VNUM_TEMPLE;
					}
					ch->in_room = get_room_index(ch->pcdata->was_in_room);
					pop_call();
					return;
				}

				NKEY("Exp",		ch->pcdata->exp,			fread_number(fp));
				break;

			case 'F':
				break;

			case 'G':

				NKEY("Gold",		ch->gold,					fread_number(fp));
				break;

			case 'H':
				AKEY("History",		ch->pcdata->history,		fread_number(fp));
				SKEY("HtmlAddy",	ch->pcdata->html_address,	fread_string(fp));
				NKEY("Hitroll",	ch->hitroll,				fread_number(fp));

				if (!strcasecmp( word, "HpMaMv" ) )
				{
					ch->hit		= fread_number(fp);
					ch->max_hit	= fread_number(fp);
					ch->mana		= fread_number(fp);
					ch->max_mana	= fread_number(fp);
					ch->move		= fread_number(fp);
					ch->max_move	= fread_number(fp);
					fMatch = TRUE;
					break;
				}
				break;

			case 'J':
				NKEY("Jaildate",   ch->pcdata->jaildate,   fread_number(fp ) );
				NKEY("Jailtime",   ch->pcdata->jailtime,   fread_number(fp ) );
				break;

			case 'K':
				NKEY("K_Played",	ch->pcdata->killer_played,	fread_number(fp));
				break;

			case 'L':
				NKEY("Level",       ch->level,                    fread_number(fp));
				SKEY("L_Descr",     ch->long_descr,               fread_string(fp));
				NKEY("Language",    ch->pcdata->language,         fread_number(fp));
				NKEY("LastImprove", ch->pcdata->last_improve,     fread_number(fp));
				NKEY("LastNote",    ch->pcdata->last_note,        fread_number(fp));
				NKEY("LastTime",    ch->pcdata->last_time,        fread_number(fp));
				SKEY("Login",       ch->pcdata->host,             fread_string(fp));
				NKEY("LastRoom",    ch->pcdata->last_real_room,   fread_number(fp));
				break;

			case 'M':
				AKEY("Mclass",		ch->pcdata->mclass,				fread_number(fp));
				SKEY("MailAddy",	ch->pcdata->mail_address,		fread_string(fp));
				NKEY("Morph",		ch->pcdata->morph_points,		fread_number(fp));
				break;

			case 'N':
				break;

			case 'O':
				NKEY("ObjVerNr",	ch->pcdata->obj_version_number,	fread_number(fp));
				NKEY("O_Played",	ch->pcdata->outcast_played,		fread_number(fp));
				break;

			case 'P':
				NKEY("Psw",			ch->pcdata->password,		fread_number(fp));
				NKEY("Password",		ch->pcdata->password,		fread_number(fp));
				NKEY("Played",			ch->pcdata->played,			fread_number(fp));
				NKEY("Practice",		ch->pcdata->practice,		fread_number(fp));
				NKEY("Prefix",			ch->pcdata->honorific,		fread_number(fp));
				NKEY("Prev_hrs",		ch->pcdata->previous_hours,	fread_number(fp));
				NKEY("Portsize",		ch->pcdata->port_size,		fread_number(fp));
				SKEY("Prompt",			ch->pcdata->prompt_layout,	fread_string(fp));
				NKEY("Pvnum",			ch->pcdata->pvnum,			fread_number(fp));

				if (!strcmp(word, "PK_ATTACKS"))
				{
					int cnt2;

					cnt = fread_number(fp);

					for (cnt2 = 0 ; cnt2 < cnt ; cnt2++)
					{
						if (cnt2 >= MAX_PK_ATTACKS)
						{
							fread_number(fp);
							fread_number(fp);
							fread_string(fp);
						}
						else
						{
							ch->pcdata->last_pk_attack_time[cnt2]	= fread_number(fp);
							ch->pcdata->last_pk_attack_pvnum[cnt2]	= fread_number(fp);
							STRFREE(ch->pcdata->last_pk_attack_name[cnt2]);
							ch->pcdata->last_pk_attack_name[cnt2]	= fread_string(fp);
						}
					}
					fMatch = TRUE;
					break;
				}
				if (!strcmp( word, "POISON_DATA" ) )
				{
					POISON_DATA *pd;
					pd = fread_poison_data(fp );
					if (pd != NULL && ch->poison != NULL)
					{
						pd->next = ch->poison;
						ch->poison = pd;
					}
					else if (pd != NULL)
					{
						ch->poison = pd;
					}
					fMatch = TRUE;
					break;
				}
				if (!strcmp(word, "Playtime"))
				{
					ch->pcdata->played += fread_number(fp);
					fMatch = TRUE;
					break;
				}
				break;

			case 'Q':
				if (!strcasecmp(word, "Qstb"))
				{
					int qn, byt;
					byt = fread_number(fp );
					qn  = fread_number(fp );
					ALLOCMEM(ch->pcdata->quest[qn], bool, MAX_QUEST_BYTES);
					for (cnt = 0 ; cnt < byt ; cnt++)
					{
						ch->pcdata->quest[qn][cnt] = fread_number(fp );
					}
					fMatch = TRUE;
				}
				break;

			case 'R':
				NKEY("Race",			ch->race,                     fread_number(fp));
				NKEY("Reinc",			ch->pcdata->reincarnation,    fread_number(fp));
				NKEY("Recall",			ch->pcdata->recall,           fread_number(fp));
				NKEY("Room",			ch->pcdata->was_in_room,      fread_number(fp));
				break;

			case 'S':
				NKEY("Saves",            ch->saving_throw,             fread_number(fp));
				NKEY("Sex",              ch->sex,                      fread_number(fp));
				NKEY("Size_h",           ch->pcdata->screensize_h,     fread_number(fp));
				NKEY("Size_v",           ch->pcdata->screensize_v,     fread_number(fp));
				SKEY("S_Descr",          ch->short_descr,              fread_string(fp));
				NKEY("Speed",            ch->speed,                    fread_number(fp));
				NKEY("Speak",            ch->pcdata->speak,            fread_number(fp));
				NKEY("Spam",             ch->pcdata->spam,             fread_number(fp));

				if ( !strcasecmp( word, "Skill" ) )
				{
					int sn;
					int value;

					value = fread_number(fp );

					value = URANGE(0, value, 99);

					sn = skill_lookup( fread_word(fp ) );

					if (sn < 0)
					{
						bug("Fread_char: %s - unknown skill.", ch->name);
					}
					else
					{
						cnt = multi_pick(ch, sn);

						if (cnt >= 0 && value > class_table[cnt].skill_master)
						{
							value = class_table[cnt].skill_master;
						}
						ch->pcdata->learned[sn] = value;
					}
					fMatch = TRUE;
				}
				break;

			case 'T':
				AKEY("Topic",       ch->pcdata->topic_stamp,      fread_number(fp ) );
				NKEY("Trust",       ch->trust,                    fread_number(fp ) );
				NKEY("Tact_v",      ch->pcdata->tactical_size_v,  fread_number(fp ) );
				NKEY("Tact_c",      ch->pcdata->tactical_size_c,  fread_number(fp ) );
				NKEY("Tact_f",      ch->pcdata->tactical_flags,   fread_number(fp ) );
				SKEY("Tact_i",      ch->pcdata->tactical_index,   fread_string(fp ) );
				NKEY("Trivia",      ch->pcdata->trivia_points,    fread_number(fp ) );

				if (!strcasecmp(word, "Title"))
				{
					STRFREE(ch->pcdata->title);
					ch->pcdata->title = fread_string(fp);
					set_title(ch, ch->pcdata->title);

					fMatch = TRUE;
					break;
				}
				break;

			case 'V':
				NKEY("Version",	ch->pcdata->version,	fread_number(fp));
				NKEY("Vt100_F",	ch->pcdata->vt100_flags,	fread_number(fp));
				break;

			case 'W':
				NKEY("Whichgod",	ch->pcdata->god,		fread_number(fp));
				NKEY("Wimpy",		ch->pcdata->wimpy,		fread_number(fp));
				break;
		}

		if ( !fMatch )
		{
			bug("fread_char: no match: (word) %s (line) %s", word, fread_line(fp));
		}
	}
	pop_call();
	return;
}

void fread_obj( CHAR_DATA *ch, FILE *fp )
{
	OBJ_DATA *obj;
	char *word;
	int BasicVnum, WearLoc;
	bool fMatch, iNest;

	push_call("fread_obj(%p,%p)",ch,fp);

	BasicVnum		= 0;
	WearLoc		= WEAR_NONE;
	iNest		= 0;

	ALLOCMEM(obj, OBJ_DATA, 1);

	obj->pIndexData		= obj_index[OBJ_VNUM_MUSHROOM];

	for ( ; ; )
	{
		word				= feof(fp ) ? "End" : fread_word(fp );
		fMatch			= FALSE;

		switch ( UPPER(word[0]) )
		{
			case 'A':


				if (!strcmp(word, "AffectData"))
				{
					AFFECT_DATA *paf;
					int sn;

					ALLOCMEM(paf, AFFECT_DATA, 1);

					sn = skill_lookup(fread_word(fp));

					if (sn < 0)
					{
						log_string("fread_obj: unknown skill.");
					}
					else
					{
						paf->type = sn;
					}
					paf->duration	= fread_number(fp );
					paf->modifier	= fread_number(fp );
					paf->location	= fread_number(fp );
					paf->bittype   = fread_number(fp );
					paf->bitvector	= fread_number(fp );

					LINK (paf, obj->first_affect, obj->last_affect, next, prev);
					fMatch = TRUE;
					break;
				}

				if (!strcmp(word, "AffectData1"))
				{
					AFFECT_DATA *paf;
					int sn;

					ALLOCMEM(paf, AFFECT_DATA, 1);

					sn = skill_lookup(fread_word(fp));

					if (sn < 0)
					{
						log_string("fread_obj: unknown skill.");
					}
					else
					{
						paf->type = sn;
					}
					paf->duration	= fread_number(fp );
					paf->modifier	= fread_number(fp );
					paf->location	= fread_number(fp );
					paf->bittype   = fread_number(fp );
					paf->bitvector	= fread_number(fp );

					LINK (paf, obj->first_affect, obj->last_affect, next, prev);
					fMatch = TRUE;
					break;
				}
				break;

			case 'B':
				NKEY("BasicVnum", 	BasicVnum,		fread_number(fp ) );
				break;

			case 'C':
				if (!strcmp(word, "Compact"))
				{
					int byt, cnt, vnum;

					fread_to_eol(fp );

					vnum = fread_number(fp);

					if ((obj->pIndexData = get_obj_index(vnum)) == NULL)
					{
						if (IS_SET(mud->flags, MUD_EMUD_REALGAME))
						{
							log_printf("fread_obj: bad vnum %d.", vnum);
						}
						obj->pIndexData = obj_index[OBJ_VNUM_MUSHROOM];
					}
					obj->extra_flags    = fread_number(fp );
					obj->wear_flags     = fread_number(fp );
					obj->item_type      = fread_number(fp );
					obj->weight         = fread_number(fp );
					obj->level          = fread_number(fp );
					obj->timer          = fread_number(fp );
					obj->cost           = fread_number(fp );
					obj->owned_by       = fread_number(fp );
					obj->value[0]       = fread_number(fp );
					obj->value[1]       = fread_number(fp );
					obj->value[2]       = fread_number(fp );
					obj->value[3]       = fread_number(fp );
					obj->value[4]       = fread_number(fp );
					obj->value[5]       = fread_number(fp );
					obj->value[6]       = fread_number(fp );
					obj->value[7]       = fread_number(fp );
					byt                 = fread_number(fp );

					if (byt)
					{
						ALLOCMEM( obj->obj_quest, unsigned char, MAX_QUEST_BYTES );
						for (cnt = 0 ; cnt < byt ; cnt++)
						{
							obj->obj_quest[cnt] = fread_number(fp );
						}
					}
					fMatch = TRUE;
					break;
				}
				break;

			case 'D':
				SKEY("Desc",		obj->description,	fread_string(fp ) );
				break;

			case 'E':
				if (!strcmp(word, "End"))
				{
					if (BasicVnum == 0)
					{
						if (obj->owned_by 		== 0
						&&  obj->first_affect	== NULL
						&&  obj->value[0] 		== obj->pIndexData->value[0]
						&&  obj->value[1] 		== obj->pIndexData->value[1]
						&&  obj->value[2] 		== obj->pIndexData->value[2]
						&&  obj->value[3] 		== obj->pIndexData->value[3]
						&&  obj->timer			== 0
						&&  obj->obj_quest		== NULL
						&&  obj->poison		== NULL
						&&  obj->short_descr	== NULL)
						{
							REMOVE_BIT(obj->extra_flags, ITEM_MODIFIED);
						}
						else
						{
							SET_BIT(obj->extra_flags, ITEM_MODIFIED);
						}
					}
					else
					{
						OBJ_INDEX_DATA *pObjIndex;

						if ((pObjIndex = get_obj_index(BasicVnum)) == NULL)
						{
							if (IS_SET(mud->flags, MUD_EMUD_REALGAME))
							{
								log_printf("fread_obj: bad vnum %d.", BasicVnum);
							}
							BasicVnum	= OBJ_VNUM_MUSHROOM;
							pObjIndex	= get_obj_index(BasicVnum);
						}
						obj->pIndexData	= pObjIndex;

						obj->level		= pObjIndex->level;

						obj->name			= STRDUPE(pObjIndex->name);
						obj->short_descr	= STRDUPE(pObjIndex->short_descr);
						obj->long_descr	= STRDUPE(pObjIndex->long_descr);
						obj->description	= STRDUPE(pObjIndex->description);
						obj->item_type		= pObjIndex->item_type;
						obj->extra_flags	= pObjIndex->extra_flags;
						obj->wear_flags	= pObjIndex->wear_flags;
						obj->value[0]		= pObjIndex->value[0];
						obj->value[1]		= pObjIndex->value[1];
						obj->value[2]		= pObjIndex->value[2];
						obj->value[3]		= pObjIndex->value[3];
						obj->weight		= pObjIndex->weight;
						obj->cost			= obj_worth(pObjIndex);
						obj->level		= pObjIndex->level;
					}

					/*
						Special chains of the Gods
					*/

					if (BasicVnum == 0)
					{
						if (obj->level <= 0 || obj->level < obj->pIndexData->level)
						{
							obj->level = obj->pIndexData->level;
						}

						if (obj->name == NULL)
						{
//							STRFREE( obj->name );
							obj->name = STRDUPE(obj->pIndexData->name);
						}
						if (obj->short_descr == NULL)
						{
//							STRFREE( obj->short_descr );
							obj->short_descr = STRDUPE(obj->pIndexData->short_descr);
						}
						if (obj->long_descr == NULL)
						{
//							STRFREE( obj->long_descr );
							obj->long_descr = STRDUPE(obj->pIndexData->long_descr);
						}
						if (obj->description == NULL)
						{
//							STRFREE( obj->description );
							obj->description = STRDUPE(obj->pIndexData->description);
						}
					}
					obj->wear_loc    = WearLoc;
					rgObjNest[iNest] = obj;

					if (iNest == 0)
					{
						obj_to_char(obj, ch);
					}
					else
					{
						obj_to_obj(obj, rgObjNest[iNest-1]);
					}

					LINK(obj, mud->f_obj, mud->l_obj, next, prev);
					LINK(obj, obj->pIndexData->first_instance, obj->pIndexData->last_instance, next_instance, prev_instance);
					add_obj_ref_hash( obj );

					mud->total_obj++;
					obj->pIndexData->total_objects++;

					pop_call();
					return;
				}
				break;

			case 'L':
				SKEY("Long",		obj->long_descr,	fread_string(fp));
				break;

			case 'N':
				NKEY("Nest",		iNest,			fread_number(fp));
				SKEY("Name",		obj->name,		fread_string(fp));
				break;

			case 'P':
				if (!strcmp(word, "POISON_DATA"))
				{
					POISON_DATA *pd;
					pd = fread_poison_data(fp );

					if (pd != NULL)
					{
						if (obj->poison != NULL)
						{
							pd->next = obj->poison;
							obj->poison = pd;
						}
						else	if (pd != NULL)
						{
							obj->poison = pd;
						}
					}
					fMatch = TRUE;
					break;
				}
				break;
			case 'Q':
				/*
					do not remove this section for backward compatibility
				*/
				if (!strcasecmp( word, "Quick2"))
				{
					int byt, cnt;

					fread_to_eol(fp );
					iNest			= fread_number(fp );
					WearLoc			= fread_number(fp );
					obj->name			= fread_string(fp );
					obj->short_descr	= fread_string(fp );
					obj->long_descr	= fread_string(fp );
					obj->description	= fread_string(fp );
					cnt				= fread_number(fp);

					if ((obj->pIndexData = get_obj_index(cnt)) == NULL)
					{
						if (IS_SET(mud->flags, MUD_EMUD_REALGAME))
						{
							log_printf("fread_obj: bad vnum %d.", cnt);
						}
						obj->pIndexData = obj_index[OBJ_VNUM_MUSHROOM];
					}
					obj->extra_flags	= fread_number(fp );
					obj->wear_flags	= fread_number(fp );
					obj->item_type		= fread_number(fp );
					obj->weight		= fread_number(fp );
					obj->level		= fread_number(fp );
					obj->timer		= fread_number(fp );
					obj->cost			= fread_number(fp );
					obj->owned_by		= fread_number(fp );
					obj->value[0]		= fread_number(fp );
					obj->value[1]		= fread_number(fp );
					obj->value[2]		= fread_number(fp );
					obj->value[3]		= fread_number(fp );
					byt 				= fread_number(fp );
					if (byt != 0)
					{
						ALLOCMEM( obj->obj_quest, unsigned char, MAX_QUEST_BYTES );
						for (cnt = 0 ; cnt < byt ; cnt++)
						{
							obj->obj_quest[cnt] = fread_number(fp );
						}
					}
					fMatch = TRUE;
					break;
				}
				break;

			case 'R':
				NKEY("RefKey",	obj->obj_ref_key,	fread_number(fp));
				break;
			case 'S':
				SKEY("Short",		obj->short_descr,	fread_string(fp));
				if ( !strcasecmp( word, "Spell" ) )
				{
					int iValue, sn;

					iValue	= fread_number(fp );
					sn		= skill_lookup( fread_word(fp ) );
					if (iValue < 1 || iValue > 3)
					{
						log_printf("Fread_obj: bad iValue %d.", iValue );
					}
					else if (sn < 0)
					{
						log_string("Fread_obj: unknown skill.");
					}
					else
					{
						obj->value[iValue] = sn;
					}
					fMatch = TRUE;
					break;
				}
				break;
			case 'W':
				NKEY("WearLoc",	WearLoc,		fread_number(fp));
				break;
		}
		if (!fMatch)
		{
			fread_to_eol(fp );
			log_printf("Fread_obj: no match: %s", word);
		}
	}
}



void roll_race( CHAR_DATA *ch )
{
	push_call("roll_race(%p)",ch);

	/*
		set average to 15.0 - Scandum 06/02/02
	*/
	ch->pcdata->perm_str = 15 + race_table[ch->race].race_mod[0];
	ch->pcdata->perm_dex = 15 + race_table[ch->race].race_mod[1];
	ch->pcdata->perm_int = 15 + race_table[ch->race].race_mod[2];
	ch->pcdata->perm_wis = 15 + race_table[ch->race].race_mod[3];
	ch->pcdata->perm_con = 15 + race_table[ch->race].race_mod[4];

	/*
		each stat raises by one per reinc! - Manwe 4/12/1999
	*/
	ch->pcdata->perm_str += ch->pcdata->reincarnation;
	ch->pcdata->perm_dex += ch->pcdata->reincarnation;
	ch->pcdata->perm_int += ch->pcdata->reincarnation;
	ch->pcdata->perm_wis += ch->pcdata->reincarnation;
	ch->pcdata->perm_con += ch->pcdata->reincarnation;

	ch->pcdata->language = race_table[ch->race].language;
	ch->pcdata->speak    = race_table[ch->race].language;

	add_language(ch);
	add_language(ch);

	pop_call();
	return;
}


/*
	This routine makes sure that files are not cross linked of chopped
	Chaos  - 5/30/96
*/

bool is_valid_file( CHAR_DATA *ch, FILE *fp )
{
	char buf[MAX_INPUT_LENGTH];
	sh_int cnt, cf;

	push_call("is_valid_File(%p,%p)",ch,fp);

	cf  = ' ';
	cnt = 0;

	while (cf != '#' && cnt > -50)
	{
		cnt--;
		fseek(fp, cnt, SEEK_END);
		cf = fgetc(fp);
	}

	if (cnt == -50)
	{
		log_printf("Didn't find an #END on %s", ch->name);
		rewind(fp );
		pop_call();
		return( FALSE );
	}

	for (cf = '#', cnt = 0 ; cf != '\n' && cf != EOF ; cnt++)
	{
		buf[cnt] = cf;

		cf = fgetc(fp);
	}
	buf[cnt] = '\0';

	if (str_prefix("#END ", buf))
	{
		log_printf("Didn't find an #END on %s", ch->name);
		rewind(fp );
		pop_call();
		return( FALSE );
	}

	if (!strcasecmp(ch->name, &buf[5]))
	{
		rewind(fp );
		pop_call();
		return( TRUE );
	}

	log_printf("Cross linked file %s on %s", buf, ch->name );
	rewind(fp );
	pop_call();
	return( FALSE );
}

/*
	Let's save that poison data   -  Chaos 4/20/99
*/

void fwrite_poison_data( POISON_DATA *pd, FILE *fp )
{
	push_call("fwrite_poison_data(%p,%p)",pd,fp);

	fprintf(fp, "POISON_DATA 1 %d %d %d %d %d %d %d %d %d\n",
		pd->for_npc?1:0,
		(int) pd->poison_type,
		pd->instant_damage_low,
		pd->instant_damage_high,
		pd->constant_duration,
		pd->constant_damage_low,
		pd->constant_damage_high,
		pd->owner,
		pd->poisoner );

	if (pd->next != NULL)
	{
		fwrite_poison_data( pd->next, fp );
	}
	pop_call();
	return;
}

POISON_DATA *fread_poison_data( FILE *fp )
{
	POISON_DATA *pd;
	int ptype;

	push_call("fread_poison_data(%p)",fp);

	ALLOCMEM( pd, POISON_DATA, 1 );

	ptype = fread_number(fp );

	if (ptype == 1)
	{
		ptype = fread_number(fp );
		if (ptype == 0)
		{
			pd->for_npc = FALSE;
		}
		else
		{
			pd->for_npc = FALSE;
		}

		pd->poison_type		= (sh_int)fread_number(fp);
		pd->instant_damage_low	= fread_number(fp);
		pd->instant_damage_high	= fread_number(fp);
		pd->constant_duration	= fread_number(fp);
		pd->constant_damage_low	= fread_number(fp);
		pd->constant_damage_high	= fread_number(fp);
		pd->owner				= fread_number(fp);
		pd->poisoner			= fread_number(fp);
		pd->next				= NULL;

		pop_call();
		return( pd );
	}
	else
	{
		fread_to_eol(fp );
	}

	FREEMEM( pd );

	pop_call();
	return( NULL );
}

void save_world(void)
{
	push_call("save_world(void)");

	log_string("Saving Areas...");
	do_savearea(NULL, "forreal");

	log_string("Saving Clans...");
	save_all_clans();

	log_string("Saving Hiscores...");
	save_hiscores();

	log_string("Saving Timeinfo...");
	save_timeinfo();

	log_string("Saving Notes...");
	save_notes();

	pop_call();
	return;
}

/*
	Hypnos 20020326: Check player directories
*/

int check_dirs( void )
{
	int  i, j, dir;
	char	tempbuf[320];

	push_call("check_dirs()");

	dir = 0;

	if (mkdir(PLAYER_DIR, XX_DIRMASK) == -1)
	{
		if (errno != EEXIST)
		{
			pop_call();
			return -1;
		}
	}
	else
	{
		dir++;
	}

	for (i = 0 ; i < 26 ; i++)
	{
		sprintf(tempbuf, "%s/%c", PLAYER_DIR, 'a' + i);

		if (mkdir(tempbuf, XX_DIRMASK) == -1)
		{
			if (errno != EEXIST)
			{
				pop_call();
				return -1;
			}
		}
		else
		{
			dir++;
		}
	}

	for (i = 0 ; i < 26 ; i++)
	{
		for (j = 0 ; j < 26 ; j++)
		{
			sprintf(tempbuf, "%s/%c/%c", PLAYER_DIR, 'a' + i, 'a' + j);

			if (mkdir(tempbuf, XX_DIRMASK) == -1)
			{
				if (errno != EEXIST)
				{
					pop_call();
					return -1;
				}
			}
			else
			{
				dir++;
			}

			sprintf(tempbuf, "%s/%c/%c/dmp", PLAYER_DIR, 'a' + i, 'a' + j);

			if (mkdir(tempbuf, XX_DIRMASK) == -1)
			{
				if (errno != EEXIST)
				{
					pop_call();
					return -1;
				}
			}
			else
			{
				dir++;
			}

			sprintf(tempbuf, "%s/%c/%c/del", PLAYER_DIR, 'a' + i, 'a' + j);

			if (mkdir(tempbuf, XX_DIRMASK) == -1)
			{
				if (errno != EEXIST)
				{
					pop_call();
					return -1;
				}
			}
			else
			{
				dir++;
			}

			sprintf(tempbuf, "%s/%c/%c/bak", PLAYER_DIR, 'a' + i, 'a' + j);

			if (mkdir(tempbuf, XX_DIRMASK) == -1)
			{
				if (errno != EEXIST)
				{
					pop_call();
					return -1;
				}
			}
			else
			{
				dir++;
			}
		}
	}

	pop_call();
	return dir;
}
