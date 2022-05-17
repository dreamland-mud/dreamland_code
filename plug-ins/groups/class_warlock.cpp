/* $Id: class_warlock.cpp,v 1.1.2.11.6.16 2009/09/01 22:29:51 rufina Exp $
 * 
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/
#include "playerattributes.h"

#include "skill.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"
#include "spelltemplate.h"
#include "affecthandlertemplate.h"

#include "affect.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "desire.h"
#include "object.h"


#include "act_move.h"
#include "mercdb.h"
#include "magic.h"
#include "fight.h"
#include "vnum.h"
#include "handler.h"
#include "effects.h"
#include "damage_impl.h"
#include "act.h"
#include "merc.h"
#include "interp.h"
#include "def.h"

GSN(blink);

/*
 * 'blink' skill command
 */

SKILL_RUNP( blink )
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument , arg);

    if (!ch->is_npc() && !gsn_blink->usable( ch ))
    {
        ch->pecho("Это умение тебе недоступно.");
        return;
    }

    if (arg[0] == '\0' )
    {
        ch->printf("Во время боя ты {W%sмерцаешь{x.\n\r",
                    IS_SET(ch->act, PLR_BLINK_ON) ? "" : "не ");
        return;
    }

    if (arg_is_switch_on( arg ))
        {
            ch->pecho("Ты будешь мерцать, уклоняясь от атак.");
            SET_BIT(ch->act,PLR_BLINK_ON);
             return;
        }

    if (arg_is_switch_off( arg ))
        {
         REMOVE_BIT(ch->act,PLR_BLINK_ON);
         ch->pecho("Ты больше не будешь мерцать, уклоняясь от атак.");
         return;
        }
    
    ch->pecho("Укажи {lRвкл или выкл{lEon или off{lx в качестве аргумента."); 
}

