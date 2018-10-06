// $Id: olc_savecompat.cpp,v 1.1.2.11 2010-09-01 21:20:46 rufina Exp $

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <config.h>
#include "logstream.h"
#include "grammar_entities_impl.h"
#include "dlfileop.h"

#include <affect.h>
#include <object.h>
#include <character.h>
#include <pcharacter.h>
#include "room.h"
#include "race.h"
#include "liquid.h"
#include "helpmanager.h"

#include <commandmanager.h>

#include "fenia/register-impl.h"

#include "mobilebehaviormanager.h"
#include "objectbehaviormanager.h"
#include "areabehaviormanager.h"
#include "roombehaviormanager.h"

#include "dreamland.h"
#include "merc.h"
#include "interp.h"
#include "mercdb.h"
#include "loadsave.h"
#include "act.h"
#include "clanreference.h"

#include "olc.h"
#include "olcstate.h"
#include "security.h"


#include "def.h"

#include <list>

CLAN(none);
PROF(none);
LIQ(none);
LIQ(water);

using namespace std;

char *pflag(int flag)
{
    int count, pos = 0;
    int tbit = 1;
    static char buf[56];

    buf[0] = '\0';
    for (count = 0; count < 32; count++) {
        if (IS_SET(flag, tbit)) {
            if (count < 26)
                buf[pos] = 'A' + count;
            else
                buf[pos] = 'a' + (count - 26);
            pos++;
        }
        tbit *= 2;
    }
    buf[pos] = '\0';
    if (buf[0] == '\0') {
        buf[0] = '0';
        buf[1] = '\0';
    }
    return buf;
}

// Name:          fix_string
// Purpose:       Returns a string without \r and ~.
const char *fix_string(const char *str)
{
    static char *strfix = 0;
    int i, o;

    if (str == NULL)
	return ""; //NULL; XXX?

    if(strfix)
	free_string(strfix);

    strfix = str_dup(str);
    
    for (o = i = 0; str[i]; i++) {
	if (str[i] != '\r' && str[i] != '~')
	    strfix[o++] = str[i];
    }
    
    strfix[o] = '\0';
    
    return strfix;
}


// Name:          save_mobile
// Purpose:       Save one mobile to file, new format -- Hugin
// Called by:     save_mobiles (below).
void save_mobile(FILE * fp, const MOB_INDEX_DATA * pMobIndex)
{
    int bv;
    Race *race = raceManager->find( pMobIndex->race );
    
    fprintf(fp, "%s~\n", pMobIndex->player_name);
    fprintf(fp, "%s~\n", fix_string(pMobIndex->short_descr));
    fprintf(fp, "%s~\n", fix_string(pMobIndex->long_descr));
    fprintf(fp, "%s~\n", fix_string(pMobIndex->description));
    fprintf(fp, "%s~\n", pMobIndex->race);
    fprintf(fp, "%s ", pflag(pMobIndex->act & ~race->getAct( )));
    fprintf(fp, "%s ", pflag(pMobIndex->affected_by & ~race->getAff( )));
    fprintf(fp, "%d %d\n", pMobIndex->alignment, pMobIndex->group);
    fprintf(fp, "%d ", pMobIndex->level);
    fprintf(fp, "%d ", pMobIndex->hitroll);
    fprintf(fp, "%dd%d+%d ", pMobIndex->hit[DICE_NUMBER],
	       pMobIndex->hit[DICE_TYPE],
	       pMobIndex->hit[DICE_BONUS]);
    fprintf(fp, "%dd%d+%d ", pMobIndex->mana[DICE_NUMBER],
	       pMobIndex->mana[DICE_TYPE],
	       pMobIndex->mana[DICE_BONUS]);
    fprintf(fp, "%dd%d+%d ", pMobIndex->damage[DICE_NUMBER],
	       pMobIndex->damage[DICE_TYPE],
	       pMobIndex->damage[DICE_BONUS]);
    fprintf(fp, "%s\n", weapon_flags.name(pMobIndex->dam_type).c_str( ));
    fprintf(fp, "%d %d %d %d\n", pMobIndex->ac[AC_PIERCE] / 10,
	       pMobIndex->ac[AC_BASH] / 10,
	       pMobIndex->ac[AC_SLASH] / 10,
	       pMobIndex->ac[AC_EXOTIC] / 10);
    fprintf(fp, "%s ", pflag(pMobIndex->off_flags & ~race->getOff( )));
    fprintf(fp, "%s ", pflag(pMobIndex->imm_flags & ~race->getImm( )));
    fprintf(fp, "%s ", pflag(pMobIndex->res_flags & ~race->getRes( )));
    fprintf(fp, "%s\n", pflag(pMobIndex->vuln_flags & ~race->getVuln( )));
    fprintf(fp, "%s ", position_table.name(pMobIndex->start_pos).c_str());
    fprintf(fp, "%s ", position_table.name(pMobIndex->default_pos).c_str());
    fprintf(fp, "%s %ld\n", sex_table.name(pMobIndex->sex ).c_str( ), pMobIndex->wealth);
    fprintf(fp, "%s ", pflag(pMobIndex->form & ~race->getForm( )));
    fprintf(fp, "%s ", pflag(pMobIndex->parts & ~race->getParts( )));

    fprintf(fp, "%s ", size_table.name(pMobIndex->size).c_str());
    fprintf(fp, "%s\n", /*material_lookup*/(pMobIndex->material));
    
    if( (bv = (pMobIndex->detection & ~race->getDet( ))) )
	fprintf(fp, "D %s\n", pflag(bv));
    if( (bv = (~pMobIndex->act & race->getAct( ))) )
	fprintf(fp, "F act %s\n", pflag(bv));
    if( (bv = (~pMobIndex->affected_by & race->getAff( ))) )
	fprintf(fp, "F aff %s\n", pflag(bv));
    if( (bv = (~pMobIndex->off_flags & race->getOff( ))) )
	fprintf(fp, "F off %s\n", pflag(bv));
    if( (bv = (~pMobIndex->imm_flags & race->getImm( ))) )
	fprintf(fp, "F imm %s\n", pflag(bv));
    if( (bv = (~pMobIndex->res_flags & race->getRes( ))) )
	fprintf(fp, "F res %s\n", pflag(bv));
    if( (bv = (~pMobIndex->vuln_flags & race->getVuln( ))) )
	fprintf(fp, "F vul %s\n", pflag(bv));
    if( (bv = (~pMobIndex->form & race->getForm( ))) )
	fprintf(fp, "F for %s\n", pflag(bv));
    if( (bv = (~pMobIndex->parts & race->getParts( ))) )
	fprintf(fp, "F par %s\n", pflag(bv));

    const char *c = 0;
    
    if(pMobIndex->spec_fun.func)
	c = spec_name(pMobIndex->spec_fun.func);

    if(!c)
	c = pMobIndex->spec_fun.name.c_str();

    if(c && *c) {
	fprintf(fp, "S %s\n", c);
    }
	
    if (!pMobIndex->practicer.empty( )) 
	fprintf( fp, "G %s~\n", pMobIndex->practicer.toString( ).c_str( ) );
    
    if (pMobIndex->gram_number != Grammar::Number::SINGULAR)
	fprintf(fp, "N %s\n", pMobIndex->gram_number.toString());

    if (!pMobIndex->smell.empty( )) 
	fprintf(fp, "X smell %s~\n", pMobIndex->smell.c_str( ));

    if (pMobIndex->behavior)
	MobileBehaviorManager::save( pMobIndex, fp );
}


// Name:      save_object
// Purpose:   Save one object to file.
//            new ROM format saving -- Hugin
// Called by: save_objects (below).
void save_object(FILE * fp, const OBJ_INDEX_DATA * pObjIndex)
{
    char letter;
    Affect *pAf;
    EXTRA_DESCR_DATA *pEd;

    fprintf(fp, "%s~\n", pObjIndex->name);
    fprintf(fp, "%s~\n", fix_string(pObjIndex->short_descr));
    fprintf(fp, "%s~\n", fix_string(pObjIndex->description));
    fprintf(fp, "%s~\n", /*material_lookup*/(pObjIndex->material));
    fprintf(fp, "%s ", (pObjIndex->item_type == NO_FLAG) ?
		    "none" : 
		    item_table.name(pObjIndex->item_type).c_str( ));

    fprintf(fp, "%s ", pflag(pObjIndex->extra_flags));
    fprintf(fp, "%s\n", pflag(pObjIndex->wear_flags));

    switch (pObjIndex->item_type) {
    default:
	fprintf(fp, "%d ", pObjIndex->value[0]);
	fprintf(fp, "%d ", pObjIndex->value[1]);
	fprintf(fp, "%d ", pObjIndex->value[2]);
	fprintf(fp, "%d ", pObjIndex->value[3]);
	fprintf(fp, "%d\n", pObjIndex->value[4]);
	break;

    case ITEM_MONEY:
	fprintf(fp, "%d %d 0 0 0\n", pObjIndex->value[0],
		   pObjIndex->value[1]);
	break;

    case ITEM_FOUNTAIN:
	fprintf(fp, "%d %d '%s' %d %d\n", pObjIndex->value[0],
		   pObjIndex->value[1],
		   liquidManager->find( pObjIndex->value[2] )->getName( ).c_str( ),
		   pObjIndex->value[3],
		   pObjIndex->value[4]);
	break;

    case ITEM_DRINK_CON:
	fprintf(fp, "%d %d '%s' %s %d\n", pObjIndex->value[0],
		   pObjIndex->value[1],
		   liquidManager->find( pObjIndex->value[2] )->getName( ).c_str( ),
		   pflag(pObjIndex->value[3]),
		   pObjIndex->value[4]);
	break;

    case ITEM_CONTAINER:
	fprintf(fp, "%d %s %d %d %d\n", pObjIndex->value[0],
		   pflag(pObjIndex->value[1]),
		   pObjIndex->value[2],
		   pObjIndex->value[3],
		   pObjIndex->value[4]);
	break;
    case ITEM_BOAT:
	fprintf(fp, "%d %d %s 0 0\n",
	            pObjIndex->value[0],
	            pObjIndex->value[1],
		    pflag(pObjIndex->value[2]));
	break;
    case ITEM_FOOD:
	fprintf(fp, "%d %d 0 %s %d\n", pObjIndex->value[0],
		   pObjIndex->value[1],
		   pflag(pObjIndex->value[3]),
	/*unused*/ pObjIndex->value[4]);
	break;

    case ITEM_PORTAL:
	fprintf(fp, "%d %s ", pObjIndex->value[0], pflag(pObjIndex->value[1]));
	fprintf(fp, "%s %d 0\n", pflag(pObjIndex->value[2]), pObjIndex->value[3]);
	break;

    case ITEM_FURNITURE:
	fprintf(fp, "%d ", pObjIndex->value[0]);
	fprintf(fp, "%d ", pObjIndex->value[1]);
	fprintf(fp, "%s %d ", pflag(pObjIndex->value[2]),
		   pObjIndex->value[3]);
	fprintf(fp, "%d\n", pObjIndex->value[4]);
	break;

    case ITEM_WEAPON:
	fprintf(fp, "%s %d %d %s %s\n",
		   weapon_class.name(pObjIndex->value[0]).c_str(),
		   pObjIndex->value[1],
		   pObjIndex->value[2],
		   weapon_flags.name(pObjIndex->value[3]).c_str( ),
		   pflag(pObjIndex->value[4]));
	break;

    case ITEM_ARMOR:
	fprintf(fp, "%d ", pObjIndex->value[0]);
	fprintf(fp, "%d ", pObjIndex->value[1]);
	fprintf(fp, "%d ", pObjIndex->value[2]);
	fprintf(fp, "%d ", pObjIndex->value[3]);
	fprintf(fp, "%d\n", pObjIndex->value[4]);
	break;

    case ITEM_PILL:
    case ITEM_POTION:
    case ITEM_SCROLL:
	fprintf(fp, "%d ", pObjIndex->value[0] > 0 ?
		   pObjIndex->value[0] : 0);
	fprintf(fp, "'%s' '%s' '%s' '%s'\n",
		   get_skill_name( pObjIndex->value[1], true ),
		   get_skill_name( pObjIndex->value[2], true ),
		   get_skill_name( pObjIndex->value[3], true ),
		   get_skill_name( pObjIndex->value[4], true ));
	break;

    case ITEM_STAFF:
    case ITEM_WAND:
	fprintf(fp, "%d ", pObjIndex->value[0]);
	fprintf(fp, "%d ", pObjIndex->value[1]);
	fprintf(fp, "%d ", pObjIndex->value[2]);
	fprintf(fp, "'%s' ", get_skill_name( pObjIndex->value[3], true ));
	fprintf(fp, "%d\n", pObjIndex->value[4]);
	break;
    }

    fprintf(fp, "%d ", pObjIndex->level);
    fprintf(fp, "%d ", pObjIndex->weight);
    fprintf(fp, "%d ", pObjIndex->cost);

    if (pObjIndex->condition > 90)
	letter = 'P';
    else if (pObjIndex->condition > 75)
	letter = 'G';
    else if (pObjIndex->condition > 50)
	letter = 'A';
    else if (pObjIndex->condition > 25)
	letter = 'W';
    else if (pObjIndex->condition > 10)
	letter = 'D';
    else if (pObjIndex->condition > 0)
	letter = 'B';
    else
	letter = 'R';

    fprintf(fp, "%c\n", letter);

    list<Affect *> afflist;
    for (pAf = pObjIndex->affected; pAf; pAf = pAf->next)
	afflist.push_back(pAf);
    
    while (!afflist.empty()) {
	pAf = afflist.back();
	afflist.pop_back();

	if(pAf->where == TO_OBJECT)
	    fprintf(fp, "A\n%d %d\n", pAf->location, pAf->modifier);
	else {
	    char t;
	    switch(pAf->where) {
		case TO_AFFECTS:
		    t = 'A';
		    break;
		case TO_IMMUNE:
		    t = 'I';
		    break;
		case TO_RESIST:
		    t = 'R';
		    break;
		case TO_VULN:
		    t = 'V';
		    break;
		case TO_DETECTS:
		    t = 'D';
		    break;
		default:
		    t = 0;
	    }
	    if(t)
		fprintf(fp, "F\n%c %d %d %s\n", 
			t, pAf->location, pAf->modifier, 
			pflag(pAf->bitvector));
	    else
		bug( "Save_object: Bad where on flag set.", 0 );
	}
    }

    list<EXTRA_DESCR_DATA *> edlist;
    
    for (pEd = pObjIndex->extra_descr; pEd; pEd = pEd->next)
	edlist.push_back(pEd);

    while(!edlist.empty()) {
	pEd = edlist.back();
	edlist.pop_back();

	fprintf(fp, "E\n%s~\n%s~\n", pEd->keyword,
		   fix_string(pEd->description));
    }

    fprintf(fp, "G %s\n", pObjIndex->gram_gender.toString());

    if(pObjIndex->limit >= 0) {
	fprintf(fp, "L %d\n", pObjIndex->limit);
    }

    if (!pObjIndex->sound.empty( )) 
	fprintf(fp, "X sound %s~\n", pObjIndex->sound.c_str( ));

    if (!pObjIndex->smell.empty( )) 
	fprintf(fp, "X smell %s~\n", pObjIndex->smell.c_str( ));

    if (pObjIndex->behavior)
	ObjectBehaviorManager::save( pObjIndex, fp );
}


void save_helps(FILE *fp, AREA_DATA *pArea)
{
    HelpArticles::const_iterator a;

    fprintf(fp, "#HELPS\n");

    /* XXX - not sure about ordering... */

    for (a = helpManager->getArticles( ).begin( ); a != helpManager->getArticles( ).end( ); a++) 
	if ((*a)->areafile == pArea->area_file)
	    fprintf(fp, "%d %s~\n%s~\n", 
		    (*a)->getLevel( ), 
		    (*a)->getKeyword( ).c_str( ), 
		    fix_string((*a)->c_str( )));
    
    fprintf(fp, "0 $~\n\n\n\n");
}
    
void save_room(FILE *fp, Room *pRoomIndex)
{
    EXTRA_DESCR_DATA *pEd;
    EXTRA_EXIT_DATA *peexit;
    EXIT_DATA *pExit;
    int door;
    
    fprintf(fp, "#%u\n", pRoomIndex->vnum);
    fprintf(fp, "%s~\n", pRoomIndex->name);
    fprintf(fp, "%s~\n", fix_string(pRoomIndex->description));
    fprintf(fp, "0 ");
    fprintf(fp, "%s ", pflag(pRoomIndex->room_flags));
    fprintf(fp, "%u\n", pRoomIndex->sector_type);
    
    if (pRoomIndex->clan != clan_none)
	fprintf(fp, "C\n%s~\n", pRoomIndex->clan->getName( ).c_str( ));
    
    if (!pRoomIndex->guilds.empty( )) { 
	DLString gbuf = pRoomIndex->guilds.toString( );

	if (!gbuf.empty( ))
	    fprintf(fp, "G\n%s~\n", gbuf.c_str( ));
    }
    
    for (int ddoor = 0; ddoor < DIR_SOMEWHERE; ddoor++) {
	for (door = 0; door < DIR_SOMEWHERE; door++) {
	    if ((pExit = pRoomIndex->exit[door]) && pExit->orig_door == ddoor) {
		int locks;
		switch(pExit->exit_info_default & ~(EX_CLOSED | EX_LOCKED)) {
		    case 0:
			locks = 0;
			break;
		    case EX_ISDOOR:
			locks = 1;
			break;
		    case EX_ISDOOR | EX_PICKPROOF:
			locks = 2;
			break;
		    case EX_ISDOOR | EX_NOPASS:
			locks = 3;
			break;
		    case EX_ISDOOR | EX_PICKPROOF | EX_NOPASS:
			locks = 4;
			break;
		    case EX_NOFLEE:
			locks = 5;
			break;
		    default:
			locks = 6;
		}

		fprintf(fp, "D%d\n", pExit->orig_door);
		fprintf(fp, "%s~\n", fix_string(pExit->description));
                // Door short description is not saved here.
		fprintf(fp, "%s~\n", pExit->keyword);
		fprintf(fp, "%d ", locks);
		if(locks == 6)
		    fprintf(fp, "%s ", pflag(pExit->exit_info_default));
		fprintf(fp, "%d %d\n", 
		        (pExit->key > 0 ? pExit->key : -1), 
			pExit->u1.to_room ? pExit->u1.to_room->vnum : -1);
	    }
	}
    }

    list<EXTRA_EXIT_DATA *> eelist;
    for(peexit = pRoomIndex->extra_exit; peexit; peexit = peexit->next)
	eelist.push_back(peexit);
    
    while(!eelist.empty()) {
	peexit = eelist.back();
	eelist.pop_back();

	fprintf(fp, "D6\n%s~\n", fix_string(peexit->description));
	fprintf(fp, "%s~\n", fix_string(peexit->keyword));
	fprintf(fp, "%s~\n", fix_string(peexit->short_desc_from));
	fprintf(fp, "%s~\n", fix_string(peexit->short_desc_to));
	fprintf(fp, "%s~\n", fix_string(peexit->room_description));
	fprintf(fp, "%s %d %d %d\n",
		pflag(peexit->exit_info_default),
		(peexit->key > 0 ? peexit->key : -1),
		peexit->u1.to_room ? peexit->u1.to_room->vnum : 0,
		peexit->max_size_pass);
	fprintf(fp, "%d %d %d %d\n",
		peexit->moving_from,
		peexit->moving_mode_from,
		peexit->moving_to,
		peexit->moving_mode_to
		);
    }
    
    list<EXTRA_DESCR_DATA *> edlist;
    for (pEd = pRoomIndex->extra_descr; pEd; pEd = pEd->next)
	edlist.push_back(pEd);
    
    while(!edlist.empty()) {
	pEd = edlist.back();
	edlist.pop_back();
	fprintf(fp, "E\n%s~\n%s~\n", pEd->keyword,
		   fix_string(pEd->description));
    }

    if (pRoomIndex->mana_rate_default != 100 || pRoomIndex->heal_rate_default != 100)
	fprintf(fp, "M %d H %d\n", pRoomIndex->mana_rate_default,
		   pRoomIndex->heal_rate_default);

    if (pRoomIndex->liquid != liq_none && pRoomIndex->liquid != liq_water) 
	fprintf(fp, "X liquid '%s'\n", pRoomIndex->liquid->getName( ).c_str( ));

    if (pRoomIndex->behavior) {
	fprintf( fp, "B\n" );
	RoomBehaviorManager::save( pRoomIndex, fp );
    }
    
    fprintf(fp, "S\n\n\n\n");
}

// Saves all rooms in area
void save_rooms(FILE * fp, AREA_DATA * pArea)
{
    fprintf(fp, "#ROOMS\n");

    for (map<int, Room *>::iterator i = pArea->rooms.begin( ); i != pArea->rooms.end( ); i++) {
	save_room(fp, i->second);
    }

    fprintf(fp, "#0\n\n\n\n");
}

// Saves all mobiles in area
void save_mobiles(FILE * fp, AREA_DATA * pArea)
{
    MOB_INDEX_DATA *pMobIndex;

    fprintf(fp, "#MOBILES\n");

    for (int v = pArea->min_vnum; v <= pArea->max_vnum; v++) {
	pMobIndex = get_mob_index(v);
	if(pMobIndex) {
	    fprintf(fp, "#%u\n", pMobIndex->vnum);
	    save_mobile(fp, pMobIndex);
	}
    }

    fprintf(fp, "#0\n\n\n\n");
}

// Saves all objects in area
void save_objects(FILE * fp, AREA_DATA * pArea)
{
    OBJ_INDEX_DATA *pObjIndex;

    fprintf(fp, "#OBJECTS\n");

    for (int v = pArea->min_vnum; v <= pArea->max_vnum; v++) {
	pObjIndex = get_obj_index(v);
	if(pObjIndex && !IS_SET(pObjIndex->extra_flags, ITEM_DELETED)) {
	    fprintf(fp, "#%u\n", pObjIndex->vnum);
	    save_object(fp, pObjIndex);
	}
    }

    fprintf(fp, "#0\n\n\n\n");
}

// This function is obsolete.  It it not needed but has been left here
// for historical reasons.  It is used currently for the same reason.
// I don't think it's obsolete in ROM -- Hugin.
void save_door_resets(FILE * fp, AREA_DATA * pArea)
{
    Room *pRoomIndex;
    EXIT_DATA *pExit;
    int door;

    for (map<int, Room *>::iterator i = pArea->rooms.begin( ); i != pArea->rooms.end( ); i++) {
	pRoomIndex = i->second;

	for (door = 0; door < DIR_SOMEWHERE; door++) {
	    if ((pExit = pRoomIndex->exit[door])
		&& pExit->u1.to_room
		&& (IS_SET(pExit->exit_info_default, EX_CLOSED)
		    || IS_SET(pExit->exit_info_default, EX_LOCKED))) 
	    {
		fprintf(fp, "D 0 %u ", pRoomIndex->vnum);
		fprintf(fp, "%d %d\n", pExit->orig_door,
			IS_SET(pExit->exit_info_default, EX_LOCKED) ? 2 : 1);
	    }
	}
    }
}

// Name:          save_resets
// Purpose:       Saves the #RESETS section of an area file.
// Called by:     save_area(olc_save.c)
void save_resets(FILE * fp, AREA_DATA * pArea)
{
    RESET_DATA *pReset;
    Room *pRoom;

    fprintf(fp, "#RESETS\n");

    save_door_resets(fp, pArea);

    for (map<int, Room *>::iterator i = pArea->rooms.begin( ); i != pArea->rooms.end( ); i++) {
	pRoom = i->second;

	for (pReset = pRoom->reset_first; pReset; pReset = pReset->next)
	    switch(pReset->command) {
	    case 'E':
		fprintf(fp, "%c     0 %5u %5d %s %5d\n",
			    pReset->command,
			    pReset->arg1, 
			    0,
			    wearlocationManager->find( pReset->arg3 )->getName( ).c_str( ), 
			    0);
		break;
	    case 'G':
		fprintf(fp, "%c     0 %5u %5d %5u %5d\n",
			    pReset->command,
			    pReset->arg1, 0, 0, 0);
		break;
	    case 'O':
		fprintf(fp, "%c     0 %5u %5d %5u %5d\n",
			    pReset->command,
			    pReset->arg1, 0, pReset->arg3, 0);
		break;
	    default:
		fprintf(fp, "%c     0 %5u %5d %5u %5d\n",
			    pReset->command,
			    pReset->arg1, pReset->arg2,
			    pReset->arg3, pReset->arg4);
	    }
    }

    fprintf(fp, "S\n\n\n\n");
}


    
void save_area_header(FILE *fp, const AREA_DATA *pArea)
{
    fprintf(fp, "#AREADATA\n");

    if( pArea->name)
	fprintf(fp, "Name %s~\n", pArea->name);
    
    if(pArea->credits)
	fprintf(fp, "Credits %s~\n", pArea->credits);
    
    if(pArea->authors)
	fprintf(fp, "Authors %s~\n", pArea->authors);
	
    if(pArea->altname)
	fprintf(fp, "Altname %s~\n", pArea->altname);

    if(pArea->translator)
	fprintf(fp, "Translator %s~\n", pArea->translator);

    fprintf(fp, "Security %d\n", pArea->security);
    fprintf(fp, "VNUMs %d %d\n", pArea->min_vnum, pArea->max_vnum);
    fprintf(fp, "LevelRange %d %d\n", pArea->low_range, pArea->high_range);
    fprintf(fp, "Flags %s\n", pflag(pArea->area_flag));
    
    if(pArea->resetmsg)
	fprintf(fp, "ResetMessage %s~\n", pArea->resetmsg);
    
    
    if (pArea->behavior) {
	fprintf(fp, "Behavior\n");
	AreaBehaviorManager::save( pArea, fp );
    }
    
    fprintf(fp, "End\n\n\n\n\n\n");
}

// Name:          save_area
// Purpose:       Save an area, note that this format is new.
// Called by:     do_asave(olc_save.c).
void save_area(AREA_DATA * pArea)
{
    FILE *fp;
    char buf[200];

    snprintf(buf, sizeof(buf), "%s", pArea->area_file->file_name);

    if ((fp = fopen(buf, "wb")) == NULL) {
	bug("Open_area: fopen", 0);
	perror(buf);
    }
    else {
	save_area_header(fp, pArea);
	save_helps(fp, pArea);
	save_mobiles(fp, pArea);
	save_objects(fp, pArea);
	save_rooms(fp, pArea);
	save_resets(fp, pArea);

	fprintf(fp, "#$\n");
	fclose(fp);
    }
}

// Name:          save_area_list
// Purpose:       Saves the listing of files to be loaded at startup.
// Called by:     do_asave(olc_save.c).
void save_area_list()
{
    struct area_file *afile;
    DLFileWrite areaListFile( dreamland->getAreaDir( ), dreamland->getAreaListFile( ) );

    if (!areaListFile.open( )) {
	LogStream::sendSystem( ) << "save_area_list " << dreamland->getAreaListFile( ) << endl;
    }
    else {
	list<area_file *> alist;
	for (afile = area_file_list; afile; afile = afile->next)
	    alist.push_back(afile);
	
	while(!alist.empty()) {
	    afile = alist.back();
	    alist.pop_back();
	    areaListFile.printf("%s\n", afile->file_name);
	}

	areaListFile.printf("$\n");
    }
}


// Name:          do_asave
// Purpose:       Entry point for saving area data.
// Called by:     interpreter(interp.c) 
CMD(asavecompat, 50, "", POS_DEAD, 103, LOG_ALWAYS, 
    "Save areas in original format.")
{
    char arg1[MAX_INPUT_LENGTH];
    AREA_DATA *pArea;
    FILE *fp;
    int value;

    fp = NULL;

    if (!ch) {
	save_area_list();
	for (pArea = area_first; pArea; pArea = pArea->next) {
	    if (IS_SET(pArea->area_flag, AREA_SAVELOCK)) {
		ptc(ch, "Area %s was not saved! (Locked from saving)\n\r", ch);
		continue;
	    }
	    REMOVE_BIT(pArea->area_flag, AREA_CHANGED);
	    save_area(pArea);
	}
	return;
    }

    strcpy(arg1, argument);
    if (arg1[0] == '\0') {
	stc("Syntax:\n\r", ch);
	stc("  asave <vnum>   - saves a particular area\n\r", ch);
	stc("  asave list     - saves the area.lst file\n\r", ch);
	stc("  asave area     - saves the area character currently in\n\r", ch);
	stc("  asave changed  - saves all changed zones\n\r", ch);
	stc("  asave world    - saves the world! (db dump)\n\r", ch);
	stc("\n\r", ch);
	return;
    }

    // Snarf the value (which need not be numeric)
    value = atoi(arg1);
    if (!(pArea = get_area_data(value)) && is_number(arg1)) {
	stc("That area does not exist.\n\r", ch);
	return;
    }

    // Save area of given vnum.
    if (is_number(arg1)) {
	if (!OLCState::can_edit(ch, pArea)) {
	    stc("You are not a builder for this area.\n\r", ch);
	    return;
	}
	if (IS_SET(pArea->area_flag, AREA_SAVELOCK)) {
	    ptc(ch, "Area %s was not saved! (Locked from saving)\n\r", ch);
	    return;
	}
	save_area_list();
	save_area(pArea);
	return;
    }

    // Save the world, only authorized areas
    if (!str_cmp("world", arg1)) {
	save_area_list();
	for (pArea = area_first; pArea; pArea = pArea->next) {
	    if (IS_SET(pArea->area_flag, AREA_SAVELOCK)) {
		ptc(ch, "Area %s was not saved! (Locked from saving)\n\r", ch);
		continue;
	    }
	    if (!OLCState::can_edit(ch, pArea))
		continue;

	    REMOVE_BIT(pArea->area_flag, AREA_CHANGED);

	    LogStream::sendNotice() << "Saving: " << pArea->area_file->file_name << endl;
	    save_area(pArea);
	}
	stc("Все арии сохранены.\n\r", ch);
	return;
    }

    // Save changed areas, only authorized areas
    if (!str_cmp("changed", arg1)) {
	char buf[MAX_INPUT_LENGTH];

	save_area_list();

	stc("Saved zones:\n\r", ch);
	sprintf(buf, "None.\n\r");

	for (pArea = area_first; pArea; pArea = pArea->next) {
	    /* Builder must be assigned this area. */
	    if (!OLCState::can_edit(ch, pArea))
		continue;

	    if (IS_SET(pArea->area_flag, AREA_SAVELOCK)) {
		ptc(ch, "Area %s was not saved! (Locked from saving)\n\r", ch);
		continue;
	    }
	    /* Save changed areas. */
	    if (IS_SET(pArea->area_flag, AREA_CHANGED)) {
		REMOVE_BIT(pArea->area_flag, AREA_CHANGED);
		save_area(pArea);
		ptc(ch, "%24s - '%s'\n\r", pArea->name, pArea->area_file->file_name);
	    }
	}
	if (!str_cmp(buf, "None.\n\r"))
	    stc(buf, ch);
	return;
    }

    // Save the area.lst file
    if (!str_cmp(arg1, "list")) {
	save_area_list();
	return;
    }

    // Save area being edited, if authorized
    if (!str_cmp(arg1, "area")) {
	pArea = ch->in_room->area;

	if (IS_SET(pArea->area_flag, AREA_SAVELOCK)) {
	    ptc(ch, "Area %s was not saved! (Locked from saving)\n\r", ch);
	    return;
	}
	if (!pArea || !OLCState::can_edit(ch, pArea)) {
	    stc("У вас нет прав изменять эту арию\n\r", ch);
	    return;
	}

	save_area_list();
	REMOVE_BIT(pArea->area_flag, AREA_CHANGED);
	save_area(pArea);
	stc("Area saved.\n\r", ch);
	return;
    }
    __do_asavecompat(ch, str_empty);
}
