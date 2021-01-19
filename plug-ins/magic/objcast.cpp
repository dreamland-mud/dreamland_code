/* $Id: objcast.cpp,v 1.1.2.17.4.13 2010-09-01 21:20:45 rufina Exp $
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
#include <vector>

#include "skillreference.h"
#include "skill.h"
#include "spell.h"
#include "spelltarget.h"
#include "skillmanager.h"
#include "commandtemplate.h"
#include "defaultspell.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "core/object.h"

#include "dreamland.h"
#include "act_move.h"
#include "mercdb.h"

#include "magic.h"
#include "fight.h"
#include "fight_exception.h"
#include "clanreference.h"
#include "vnum.h"
#include "merc.h"
#include "../anatolia/handler.h"
#include "act.h"
#include "interp.h"
#include "def.h"

CLAN(battlerager);
GSN(none);
short get_wear_level( Character *ch, Object *obj );

static bool oprog_quaff( Object *obj, Character *ch )
{
    FENIA_CALL( obj, "Quaff", "C", ch );
    FENIA_NDX_CALL( obj, "Quaff", "OC", obj, ch );
    return false;
}

/*
 * 'quaff' skill command
 */
CMDRUNP( quaff )
{
    char arg[MAX_INPUT_LENGTH];
    Object *obj;

    one_argument( argument, arg );

    if(!ch->is_npc( ) && ch->getClan( ) == clan_battlerager && !ch->is_immortal( )) {
        ch->send_to("Ты же воин клана Ярости, а не презренный МАГ!\n\r");
        return;
    }

    if (arg[0] == '\0') {
        ch->send_to("Осушить что?\n\r");
        return;
    }

    if (( obj = get_obj_carry( ch, arg ) ) == 0) {
        ch->send_to("У тебя нет такого снадобья.\n\r");
        return;
    }

    if (obj->item_type != ITEM_POTION) {
        ch->send_to("Осушать можно только снадобья.\n\r");
        return;
    }

      if (get_wear_level( ch, obj ) > ch->getModifyLevel( )) {
        ch->pecho("Эта смесь чересчур сильна, чтобы ты мог%1$Gло||ла выпить её.", ch);
        return;
    }

    act_p( "$c1 осушает $o4.", ch, obj, 0, TO_ROOM,POS_RESTING );
    act_p( "Ты осушаешь $o4.", ch, obj, 0 ,TO_CHAR,POS_RESTING );
    
    if (oprog_quaff( obj, ch ))
        return;

    spell_by_item( ch, obj );

    if (ch->is_adrenalined( ) || ch->fighting)
         ch->setWaitViolence( 2 );
    
    extract_obj( obj );
    obj_to_char( create_object(get_obj_index(OBJ_VNUM_POTION_VIAL),0),ch);
}



