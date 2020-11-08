/* $Id$
 *
 * ruffina, 2004
 */
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config.h"

#include "grammar_entities_impl.h"
#include "fenia/register-impl.h"                                                

#include "room.h"
#include "affect.h"

#include "clanreference.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

CLAN(none);

// Globals
extern int top_reset;
extern int top_area;
extern int top_exit;
extern int top_ed;
extern int top_room;

EXIT_DATA *exit_free;
RESET_DATA *reset_free;
void free_extra_descr(EXTRA_DESCR_DATA * pExtra);


RESET_DATA *new_reset_data(void)
{
    RESET_DATA *pReset;

    if (!reset_free) {
        pReset = (RESET_DATA*)alloc_perm(sizeof(*pReset));
        top_reset++;
    }
    else {
        pReset = reset_free;
        reset_free = reset_free->next;
    }
    pReset->next = NULL;
    pReset->command = 'X';
    pReset->arg1 = 0;
    pReset->arg2 = 0;
    pReset->arg3 = 0;
    pReset->arg4 = 0;
    return pReset;
}

void free_reset_data(RESET_DATA * pReset)
{
    pReset->next = reset_free;
    reset_free = pReset;
    return;
}

void init_area(AreaIndexData *pArea) 
{
    top_area++;
    pArea->next = NULL;
    pArea->name = str_dup("New area");
    pArea->area_flag = AREA_ADDED;
    pArea->security = 9;
    pArea->authors = str_dup("");
    pArea->altname = str_dup("None");
    pArea->translator = str_dup("None");
    pArea->speedwalk = str_dup("");
    pArea->min_vnum = 0;
    pArea->max_vnum = 0;
    pArea->low_range = 0;
    pArea->high_range = 0;
    pArea->age = 0;
    pArea->nplayer = 0;
    pArea->empty = true;
    pArea->credits = str_dup("None");
    pArea->resetmsg = str_dup("");
    pArea->vnum = top_area - 1;
}

AreaIndexData *new_area(void)
{
    AreaIndexData *pArea;
    pArea = new AreaIndexData; 
    init_area(pArea);
    return pArea;
}

void free_area(AreaIndexData * pArea)
{
    free_string(pArea->name);
    free_string(pArea->authors);
    free_string(pArea->altname);
    free_string(pArea->translator);
    free_string(pArea->speedwalk);
    delete pArea;
    return;
}

EXIT_DATA *new_exit(void)
{
    EXIT_DATA *pExit;

    if (!exit_free) {
        pExit = (EXIT_DATA*)alloc_perm(sizeof(*pExit));
        top_exit++;
    }
    else {
        pExit = exit_free;
        exit_free = exit_free->next;
    }

    pExit->u1.to_room = NULL;
    pExit->next = NULL;
    pExit->exit_info = 0;
    pExit->key = 0;
    pExit->keyword = &str_empty[0];
    pExit->short_descr = &str_empty[0];
    pExit->description = &str_empty[0];
    pExit->exit_info_default = 0;
    pExit->level = 0;
    return pExit;
}

void free_exit(EXIT_DATA * pExit)
{
    free_string(pExit->keyword);
    free_string(pExit->short_descr);
    free_string(pExit->description);

    pExit->next = exit_free;
    exit_free = pExit;
    return;
}

Room *new_room_index(void)
{
    Room *pRoom;
    int door;

    top_room++;

    pRoom = new Room;

    pRoom->next = NULL;
    pRoom->people = NULL;
    pRoom->contents = NULL;
    pRoom->extra_descr = NULL;
    pRoom->area = NULL;
    pRoom->extra_exit = NULL;

    for (door = 0; door < DIR_SOMEWHERE; door++)
        pRoom->exit[door] = NULL;

    pRoom->name = &str_empty[0];
    pRoom->description = &str_empty[0];
    pRoom->vnum = 0;
    pRoom->room_flags = 0;
    pRoom->light = 0;
    pRoom->sector_type = 0;
    pRoom->owner = &str_empty[0];
    pRoom->clan = clan_none;
    pRoom->heal_rate = 100;
    pRoom->mana_rate = 100;
    pRoom->heal_rate_default = 100;
    pRoom->mana_rate_default = 100;
    return pRoom;
}

OBJ_INDEX_DATA *new_obj_index(void)
{
    static OBJ_INDEX_DATA zeroObjIndex;
    OBJ_INDEX_DATA *pObj;
    int value;

    pObj = new OBJ_INDEX_DATA;
    *pObj = zeroObjIndex;

    pObj->next = NULL;
    pObj->extra_descr = NULL;
    pObj->affected = NULL;
    pObj->area = NULL;
    pObj->name = str_dup("no name");
    pObj->short_descr = str_dup("(no short description)");
    pObj->description = str_dup("(no description)");
    pObj->vnum = 0;
    pObj->item_type = ITEM_TRASH;
    pObj->extra_flags = 0;
    pObj->wear_flags = 0;
    pObj->count = 0;
    pObj->weight = 0;
    pObj->cost = 0;
    pObj->material = str_dup("none");
    pObj->condition = 100;
    for (value = 0; value < 5; value++)
        pObj->value[value] = 0;
    pObj->new_format = true;
    pObj->behavior = 0; 
    pObj->limit = -1;
    pObj->level = 0;
    return pObj;
}

void free_obj_index(OBJ_INDEX_DATA * pObj)
{
    EXTRA_DESCR_DATA *pExtra;
    Affect *pAf, *pAfNext;

    free_string(pObj->name);
    free_string(pObj->short_descr);
    free_string(pObj->description);

    for (pAf = pObj->affected; pAf; pAf = pAfNext) {
        pAfNext = pAf->next;
        ddeallocate(pAf);
    }

    for (pExtra = pObj->extra_descr; pExtra; pExtra = pExtra->next) {
        free_extra_descr(pExtra);
    }
    
    delete pObj;
}

MOB_INDEX_DATA *new_mob_index(void)
{
    MOB_INDEX_DATA *pMob;

    pMob = new MOB_INDEX_DATA;
//        top_mob_index++;

    pMob->next = NULL;
    pMob->spec_fun = NULL;
    pMob->area = NULL;
    pMob->player_name = str_dup("no name");
    pMob->short_descr = str_dup("(no short description)");
    pMob->long_descr = str_dup("(no long description)\n\r");
    pMob->description = &str_empty[0];
    pMob->vnum = 0;
    pMob->count = 0;
    pMob->killed = 0;
    pMob->sex = 0;
    pMob->level = 0;
    pMob->act = ACT_IS_NPC;
    pMob->affected_by = 0;
    pMob->detection = 0;
    pMob->dam_type = 0;
    pMob->alignment = 0;
    pMob->hitroll = 0;
    pMob->race = str_dup("human");
    pMob->form = 0;
    pMob->parts = 0;
    pMob->imm_flags = 0;
    pMob->res_flags = 0;
    pMob->vuln_flags = 0;
    pMob->material = str_dup("none");
    pMob->off_flags = 0;
    pMob->size = SIZE_MEDIUM;
    pMob->ac[AC_PIERCE] = 0;
    pMob->ac[AC_BASH] = 0;
    pMob->ac[AC_SLASH] = 0;
    pMob->ac[AC_EXOTIC] = 0;
    pMob->hit[DICE_NUMBER] = 0;
    pMob->hit[DICE_TYPE] = 0;
    pMob->hit[DICE_BONUS] = 0;
    pMob->mana[DICE_NUMBER] = 0;
    pMob->mana[DICE_TYPE] = 0;
    pMob->mana[DICE_BONUS] = 0;
    pMob->damage[DICE_NUMBER] = 0;
    pMob->damage[DICE_TYPE] = 0;
    pMob->damage[DICE_NUMBER] = 0;
    pMob->start_pos = POS_STANDING;
    pMob->default_pos = POS_STANDING;
    pMob->wealth = 0;
    pMob->new_format = true;
    pMob->behavior = 0;
    return pMob;
}

void free_mob_index(MOB_INDEX_DATA * pMob)
{
    free_string(pMob->player_name);
    free_string(pMob->short_descr);
    free_string(pMob->long_descr);
    free_string(pMob->description);

    delete pMob;
}

Affect *new_affect()
{
    Affect *paf_new;
    paf_new = dallocate( Affect );
    return paf_new;
}

void free_affect(Affect *paf)
{
    ddeallocate(paf);
}
