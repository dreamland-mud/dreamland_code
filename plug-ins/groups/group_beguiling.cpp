/* $Id: group_beguiling.cpp,v 1.1.2.15.6.11 2009/08/16 02:50:31 rufina Exp $
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
 
#include "group_beguiling.h"
#include "spelltemplate.h"
#include "affecthandlertemplate.h"

#include "so.h"
#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "object.h"
#include "affect.h"
#include "magic.h"
#include "fight.h"
#include "act_move.h"


#include "merc.h"
#include "mercdb.h"
#include "occupations.h"
#include "handler.h"
#include "act.h"
#include "vnum.h"
#include "def.h"


PROF(none);

#define OBJ_VNUM_MAGIC_JAR                93



/*
 * magic jar behavior
 */
void MagicJar::get( Character *ch )
{
    if (!ch->is_npc( ) && strstr(obj->getName( ), ch->getNameC()) != 0) {
        oldact("Вот это удача!",ch,obj,0,TO_CHAR);
        extract_obj(obj);
    }
    else
        oldact("Ты заполучи%gло|л|ла блудную душу.",ch,obj,0,TO_CHAR);
} 

bool MagicJar::extract( bool fCount )
{
    Character *wch;

    for (wch = char_list; wch != 0 ; wch = wch->next)
    {
        if (wch->is_npc()) 
            continue;

        if (strstr(obj->getName( ),wch->getNameC()) != 0)
        {
            if (IS_SET( wch->act, PLR_NO_EXP )) {
                REMOVE_BIT(wch->act,PLR_NO_EXP);
                wch->pecho("Твоя душа возвращается к тебе.");
            }

            break;
        }
    }

    return ObjectBehavior::extract( fCount );
}

bool MagicJar::quit( Character *ch, bool count )
{
    extract_obj( obj );
    return true;
}

bool MagicJar::area( )
{
    Character *carrier = obj->carried_by;
    
    if (!carrier
        || carrier->is_npc( )
        || IS_SET(carrier->in_room->room_flags, 
                  ROOM_SAFE|ROOM_SOLITARY|ROOM_PRIVATE)
        || !carrier->in_room->pIndexData->guilds.empty( ))
    {
        extract_obj( obj );
        return true;
    }
    else
        return false;
}
    
SPELL_DECL(MagicJar);
VOID_SPELL(MagicJar)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Object *vial;
    Object *jar;
    char buf[MAX_STRING_LENGTH];

    if (victim == ch)
        {
        ch->pecho("Твоя душа всегда с тобой!");
        return;
        }

    if (victim->is_npc())
        {
        ch->pecho("Душа этого противника неподвластна тебе!.");
        return;
        }

    if ( IS_SET( victim->act, PLR_NO_EXP ) )
    {
        ch->pecho("Душа твоего противника где-то далеко...");
        return;
    }


    if (saves_spell(level ,victim,DAM_MENTAL, ch, DAMF_MAGIC))
       {
        ch->pecho("Твоя попытка закончилась неудачей.");
        return;
       }

    for( vial=ch->carrying; vial != 0; vial=vial->next_content )
        if ( vial->pIndexData->vnum == OBJ_VNUM_POTION_VIAL )
            break;

    if (  vial == 0 )  {
        ch->pecho("У тебя нет пустого сосуда, чтоб заточить в него дух противника.");
        return;
    }
    
    extract_obj(vial);

    jar        = create_object(get_obj_index(OBJ_VNUM_MAGIC_JAR), 0);
//    jar->setOwner(ch->getNameC());
    jar->from = str_dup(ch->getNameC());
    jar->level = ch->getRealLevel( );

    jar->fmtName( jar->getName( ), victim->getNameC());
    jar->fmtShortDescr( jar->getShortDescr( ), victim->getNameC());
    jar->fmtDescription( jar->getDescription( ), victim->getNameC());

    sprintf( buf,jar->pIndexData->extra_descr->description, victim->getNameC() );
    jar->extra_descr = new_extra_descr();
    jar->extra_descr->keyword = str_dup( jar->pIndexData->extra_descr->keyword );
    jar->extra_descr->description = str_dup( buf );
    jar->extra_descr->next = 0;

    jar->level = ch->getRealLevel( );
    jar->cost = 0;
    obj_to_char( jar , ch );

    SET_BIT(victim->act,PLR_NO_EXP);
    oldact("Дух $C2 теперь заточен в сосуде и находится в твоей власти.", ch, 0, victim, TO_CHAR);
    oldact("$c1 {Rзаточил твой дух в сосуде.{x", ch, 0, victim, TO_VICT);
}

