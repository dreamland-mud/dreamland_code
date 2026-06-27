/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************h
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/
/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT                           *        
 *     ANATOLIA has been brought to you by ANATOLIA consortium                   *
 *         Serdar BULUT {Chronos}                bulut@rorqual.cc.metu.edu.tr       *
 *         Ibrahim Canpunar  {Mandrake}        canpunar@rorqual.cc.metu.edu.tr    *        
 *         Murat BICER  {KIO}                mbicer@rorqual.cc.metu.edu.tr           *        
 *         D.Baris ACAR {Powerman}        dbacar@rorqual.cc.metu.edu.tr           *        
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *        
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*        ROM 2.4 is copyright 1993-1995 Russ Taylor                           *
*        ROM has been brought to you by the ROM consortium                   *
*            Russ Taylor (rtaylor@pacinfo.com)                                   *
*            Gabrielle Taylor (gtaylor@pacinfo.com)                           *
*            Brian Moore (rom@rom.efn.org)                                   *
*        By using this code, you have agreed to follow the terms of the           *
*        ROM license, in the file Rom24/doc/rom.license                           *
***************************************************************************/

#include <algorithm>

#include <sys/types.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "logstream.h"

#include "skill.h"

#include "room.h"
#include "affect.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"

#include "dreamland.h"
#include "loadsave.h"
#include "merc.h"

#include "act.h"
#include "interp.h"
#include "magic.h"
#include "clanreference.h"

#include "fight.h"
#include "save.h"
#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"
#include "def.h"
#include "vnum.h"
  
CLAN(battlerager);
CLAN(ruler);
PROF(thief);
PROF(ninja);
GSN(armor);
GSN(bless);
GSN(blindness);
GSN(cure_blindness);
GSN(cure_disease);
GSN(cure_poison);
GSN(curse);
GSN(heal);
GSN(jail);
GSN(plague);
GSN(poison);
GSN(refresh);
GSN(remove_curse);
GSN(warcry);

#define OBJ_VNUM_WHISTLE           2116
#define MOB_VNUM_PATROLMAN           2106
#define GROUP_VNUM_TROLLS           2100
#define GROUP_VNUM_OGRES           2101
#define ADEPT_MAX_LEVEL        (PK_MIN_LEVEL + 14)

/*
 * The following special functions are available for mobiles.
 */
/*
 * Core procedure for dragons.
 */
bool dragon( Character *ch, const char *spell_name )
{
    Character *victim;
    Character *v_next;

    if ( ch->position != POS_FIGHTING )
        return false;

   for ( victim = ch->in_room->people; victim != 0; victim = v_next)
    {
        v_next = victim->next_in_room;
        if (victim->fighting == ch && number_bits( 3 ) == 0)
            break;
    }

    if (victim == 0 )
        return false;
    
    return spell( SkillManager::getThis( )->lookup( spell_name ), 
                  ch->getModifyLevel( ), ch, victim, FSPELL_NOTRIGGER );
}



/*
 * Special procedures for mobiles.
 */


bool spec_breath_acid( NPCharacter *ch )
{
    return dragon( ch, "acid breath" );
}



bool spec_breath_fire( NPCharacter *ch )
{
    return dragon( ch, "fire breath" );
}



bool spec_breath_frost( NPCharacter *ch )
{
    return dragon( ch, "frost breath" );
}



bool spec_breath_gas( NPCharacter *ch )
{
    if (ch->position != POS_FIGHTING)
        return false;

    return spell( SkillManager::getThis( )->lookup( "gas breath" ), 
                 ch->getModifyLevel( ), ch, ch->in_room );
}



bool spec_breath_lightning( NPCharacter *ch )
{
    return dragon( ch, "lightning breath" );
}



bool spec_breath_any( NPCharacter *ch )
{
    if ( ch->position != POS_FIGHTING )
        return false;

    if ( number_percent() < 50 )
        return false;

    switch (number_range(0,6))
    {
        case 0: return spec_breath_fire( ch );
        case 1: return spec_breath_frost( ch );
        case 2: return spec_breath_lightning( ch );
        case 3: return spec_breath_gas( ch );
        case 4: return spec_breath_acid( ch );
    }

    return false;
}

/* 
 * The function table 
 */
struct  spec_type    local_spec_table[] =
{
    {        "spec_breath_any",                spec_breath_any                },
    {        "spec_breath_acid",                spec_breath_acid        },
    {        "spec_breath_fire",                spec_breath_fire        },
    {        "spec_breath_frost",                spec_breath_frost        },
    {        "spec_breath_gas",                spec_breath_gas                },
    {        "spec_breath_lightning",        spec_breath_lightning        },        
    {        0,                                0                        }
};

