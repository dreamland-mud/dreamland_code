/* $Id: class_warrior.cpp,v 1.1.2.14.6.6 2008/05/27 21:30:03 rufina Exp $
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

#include "skill.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"

#include "act_move.h"
#include "mercdb.h"
#include "handler.h"

#include "magic.h"
#include "fight.h"
#include "vnum.h"
#include "merc.h"
#include "act.h"
#include "interp.h"
#include "def.h"
#include "skill_utils.h"

GSN(creativity);
GSN(smithing);

/*
 * 'smithing' skill command
 */

SKILL_RUNP( smithing )
{
    char arg[MAX_INPUT_LENGTH];
    Object *obj;
    Object *hammer;

    if ( ch->is_npc() || !gsn_smithing->usable( ch ))
    {
        ch->pecho("Чего?");
        return;
    }


    if ( ch->fighting )
    {
        ch->pecho("Подожди пока сражение закончится.");
        return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
        ch->pecho("Какую вещь ты хочешь восстановить?");
        return;
    }

    if (( obj = get_obj_carry(ch, arg)) == 0)
    {
        ch->pecho("У тебя нет этого.");
        return;
    }

   if (obj->condition >= 100)
    {
        ch->pecho("Но это не повреждено.");
        return;
    }

    if (( hammer = get_eq_char(ch, wear_hold)) == 0)
    {
        ch->pecho("Сначала возьми в руки кузнечный молот.");
        return;
    }

    if ( hammer->pIndexData->vnum != OBJ_VNUM_HAMMER )
    {
        ch->pecho("Тебе понадобится специальный молот -- ищи его в Королевстве Дварфов.");
        return;
    }

    ch->setWaitViolence( 2 );

    if (ch->isAffected( gsn_creativity )) {
        ch->pecho( "%1$^O1 по%1$nет|ют под твоими руками, обретая первозданный вид.", obj );
        ch->recho( "%1$^O1 по%1$nет|ют под руками %2$C2, обретая первозданный вид.", obj, ch );
        obj->condition = 100;
    }
    else if ( number_percent() > gsn_smithing->getEffective( ch ) + skill_level_bonus(*gsn_smithing, ch) ) {
        gsn_smithing->improve( ch, false );
        oldact("$c1 пробует восстановить $o4, но безуспешно.",ch,obj,0,TO_ROOM);
        oldact("У тебя не получилось восстановить $o4.",ch,obj,0,TO_CHAR);
        hammer->condition -= 25;
    }
    else {
        gsn_smithing->improve( ch, true );
        oldact("$c1 восстанавливает $o4.",ch,obj,0,TO_ROOM);
        oldact("Ты восстанавливаешь $o4.\n\r",ch,obj,0,TO_CHAR);

        obj->condition += ( gsn_smithing->getEffective( ch ) + skill_level_bonus(*gsn_smithing, ch) ) / 2;
        obj->condition = max( 100, obj->condition );
        hammer->condition -= 25;
    }
    
    if (hammer->condition < 1)  
        extract_obj( hammer );
}

