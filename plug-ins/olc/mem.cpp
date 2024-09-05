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

#include "olc.h"
#include "clanreference.h"
#include "merc.h"

#include "def.h"

CLAN(none);

// Globals
extern int top_area;

EXIT_DATA *exit_free;
void free_extra_descr(EXTRA_DESCR_DATA * pExtra);

EXIT_DATA *new_exit(void)
{
    EXIT_DATA *pExit;

    if (!exit_free) {
        pExit = new EXIT_DATA;
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
    pExit->orig_door = -1;
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

MOB_INDEX_DATA *new_mob_index(void)
{
    MOB_INDEX_DATA *pMob;

    pMob = new MOB_INDEX_DATA;

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
    pMob->size = NO_FLAG;
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
    pMob->behavior = 0;
    return pMob;
}

